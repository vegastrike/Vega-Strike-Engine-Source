// vssetup: Vega Strike settings configurator — Dear ImGui + SDL3 + OpenGL3.
// Self-contained drop-in replacement for the GTK vssetup. Same binary name,
// same launch (from the data dir, or --target DATADIR), same behaviour:
//   - finds the data dir (setup.config + Version.txt), switches to the user
//     home subdir named in Version.txt
//   - reads setup.config (columns/program name) + the #groups/#cat/#set/#desc
//     header of vegastrike.config
//   - shows one dropdown per group; on change, toggles the <!-- --> comment
//     blocks so the engine sees exactly one value per option, and updates the
//     "#set <group>" line.

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#define GL_GLEXT_PROTOTYPES 1   // GL 3.0 FBO/blit functions (glGenFramebuffers, glBlitFramebuffer)
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>

#include <string>
#include <vector>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

// ---------------------------------------------------------------------------
// String / file helpers
// ---------------------------------------------------------------------------

static void chomp(char *line) {
    size_t n = strlen(line);
    while (n && (line[n-1] == '\n' || line[n-1] == '\r')) { line[--n] = '\0'; }
}

// NUL-terminate the first token of `s` and return a pointer to the next one.
static char *next_parm(char *s) {
    if (s == NULL || *s == '\0') return NULL;
    while (*s && *s != ' ' && *s != '\t') s++;
    if (*s == '\0') return s;
    *s++ = '\0';
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static bool file_exists(const std::string &p) { struct stat st; return stat(p.c_str(), &st) == 0; }
static long file_mtime(const std::string &p)  { struct stat st; return stat(p.c_str(), &st) == 0 ? (long)st.st_mtime : -1; }

// Split a line that may hold an XML comment "<!-- ... -->" into the part
// before it, inside it, and after it (all pointers into `line`).
struct comment_parts { const char *before; const char *inside; const char *after; };
static struct comment_parts split_comment(char *line) {
    char *op = strstr(line, "<!--");
    char *inside, *after;
    if (op) { *op = '\0'; inside = op + 4; while (*inside == ' ') inside++; }
    else { inside = line; }
    char *cl = strstr(inside, "-->");
    if (cl) { char *end = cl; if (end > inside && end[-1] == ' ') end--; *end = '\0'; after = cl + 3; }
    else { after = inside + strlen(inside); }
    struct comment_parts p = { line, inside, after };
    return p;
}

// ---------------------------------------------------------------------------
// Config model
// ---------------------------------------------------------------------------

struct Option { std::string name; std::string desc; };
struct Group {
    std::string name;
    std::string current;              // current #set value (option name)
    std::string original;             // the #set value as loaded (for save())
    std::vector<Option> options;      // in #cat order
    std::vector<std::string> display; // per-option combo text (desc, else name)
    std::vector<const char*> items;   // pointers into display, for ImGui::Combo
};

static std::vector<Group> groups;
static std::string config_file;   // path we edit (always the home copy)
static std::string read_source;   // path we read the model from (newer of home/data)
static std::string data_dir;      // the data directory (for the readme)
static std::string program_name;  // from setup.config (for the window title)
static int columns = 4;

// Asset selection state
static int mode = 0;                            // 0 = main settings, 1 = asset screen
static std::string active_asset;                // the active asset pack (persisted)
static std::string selected_asset;              // asset highlighted in the asset screen
static std::vector<std::string> discovered;     // installed asset dirs
static bool show_help = false;                  // show the asset-help text
static const char *ASSET_HELP =
    "To add an asset pack, put its folder in:\n"
    "  ~/.local/share/vs-05/assets/<packname>/\n\n"
    "Each pack is a complete data tree (units/, sectors/, meshes/, \n"
    "textures/, ...). Select a pack below to make it the active asset.";

static Group *find_group(const std::string &name) {
    for (auto &g : groups) if (g.name == name) return &g;
    return NULL;
}

static void parse_config(FILE *fp) {
    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        chomp(line);
        struct comment_parts p = split_comment(line);
        char *head = (char*)p.inside;
        if (head[0] != '#') continue;
        char *rest = head + 1;
        if (rest[0] == '#') continue;
        char *kw = rest; char *args = next_parm(kw);
        if (args == NULL) continue;
        if (strcmp(kw, "endheader") == 0) return;
        if (strcmp(kw, "groups") == 0) {
            char *t = args, *p2;
            while (t && (p2 = next_parm(t)) != NULL) { groups.push_back({ t }); t = p2; }
        } else if (strcmp(kw, "cat") == 0) {
            char *grp = args; char *opts = next_parm(grp);
            Group *g = find_group(grp);
            if (g && opts) { char *t = opts, *p2; while (t && (p2 = next_parm(t)) != NULL) { g->options.push_back({ t }); t = p2; } }
        } else if (strcmp(kw, "set") == 0) {
            char *grp = args; char *val = next_parm(grp);
            Group *g = find_group(grp);
            if (g && val) g->current = val;
        } else if (strcmp(kw, "desc") == 0) {
            char *name = args; char *desc = next_parm(name);
            for (auto &g : groups) for (auto &o : g.options) if (o.name == name) { o.desc = desc; break; }
        }
    }
}

// Fill each group's combo text once (desc, falling back to the option name).
static void build_display(void) {
    for (auto &g : groups) {
        g.display.reserve(g.options.size());
        for (auto &o : g.options) g.display.push_back(o.desc.empty() ? o.name : o.desc);
        g.items.reserve(g.display.size());
        for (auto &s : g.display) g.items.push_back(s.c_str());
    }
}

// ---------------------------------------------------------------------------
// Rewrite: toggle the <!-- --> comment blocks so only `name` stays active,
// and update the "#set <group>" line. setting 1 = comment `name` out,
// 2 = uncomment `name` in (mirrors the original Modconfig).
// ---------------------------------------------------------------------------

static int rewrite(const std::string &path, const std::string &group,
                   const std::string &name, int setting) {
    FILE *rp = fopen(path.c_str(), "r");
    if (!rp) { fprintf(stderr, "Unable to read %s\n", path.c_str()); return -1; }
    std::string tmp = path + ".tmp";
    FILE *wp = fopen(tmp.c_str(), "w");
    if (!wp) { fclose(rp); fprintf(stderr, "Unable to write %s\n", tmp.c_str()); return -1; }

    int commenting = 0;   // 0 scanning, 1 opening a comment, 2 closing one
    char line[4096];
    while (fgets(line, sizeof(line), rp)) {
        chomp(line);
        char copy[4096]; strcpy(copy, line);
        struct comment_parts p = split_comment(copy);
        char *head = (char*)p.inside;

        if (head[0] != '#') { fprintf(wp, "%s\n", line); continue; }
        char *kw = head + 1; char *args = next_parm(kw);

        if (strcmp(kw, "endheader") == 0) { fprintf(wp, "%s\n", line); continue; }
        if (strcmp(kw, "end") == 0) {
            if (commenting == 1) fprintf(wp, "#end -->\n");
            else if (commenting == 2) fprintf(wp, "<!-- #end -->\n");
            else fprintf(wp, "%s\n", line);
            commenting = 0;
            continue;
        }
        if (strcmp(kw, "groups") == 0 || strcmp(kw, "cat") == 0 || strcmp(kw, "desc") == 0) {
            fprintf(wp, "%s\n", line); continue;
        }
        if (strcmp(kw, "set") == 0) {
            char *grp = args; char *val = next_parm(grp);
            if (grp && strcmp(grp, group.c_str()) == 0) {
                if (setting == 1) fprintf(wp, "#set %s none\n", group.c_str());
                else fprintf(wp, "#set %s %s\n", group.c_str(), name.c_str());
            } else { fprintf(wp, "%s\n", line); }
            continue;
        }

        // an option-marker "#..." line: does it mention `name`?
        bool match = false;
        char *t = head + 1;
        while (t && !match && (args = next_parm(t)) != NULL) {
            if (strcmp(t, name.c_str()) == 0) match = true;
            t = args;
        }
        if (match) commenting = setting;
        if (commenting == 0) { fprintf(wp, "%s\n", line); continue; }

        fprintf(wp, "%s", p.before);
        if (commenting == 1) fprintf(wp, "<!-- %s", p.inside);
        else fprintf(wp, "<!-- %s -->", p.inside);
        fprintf(wp, "%s\n", p.after);
    }
    fclose(rp);
    fclose(wp);
    if (rename(tmp.c_str(), path.c_str()) != 0) { fprintf(stderr, "Unable to commit %s\n", path.c_str()); return -1; }
    return 0;
}

// Stage a dropdown change: only updates the in-memory model. save() commits it.
static void apply_change(Group *g, const std::string &new_option) {
    if (g->current != new_option)
        g->current = new_option;
}

// Commit any staged changes to the config file (comment old, uncomment new, update #set).
static void save(void) {
    for (auto &g : groups)
        if (g.current != g.original) {
            rewrite(config_file, g.name, g.original, 1);
            rewrite(config_file, g.name, g.current, 2);
            g.original = g.current;
        }
}

// True if any group has a staged change not yet committed to disk.
static bool has_unsaved(void) {
    for (auto &g : groups)
        if (g.current != g.original) return true;
    return false;
}

// The engine binary sits next to this app in the install dir.
static std::string find_engine(void) {
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::string exe(buf);
        size_t slash = exe.rfind('/');
        if (slash != std::string::npos) {
            std::string cand = exe.substr(0, slash) + "/vegastrike";
            if (file_exists(cand)) return cand;
        }
    }
    return data_dir + "/../vegastrike";
}

// Launch the engine, pointing it at the data dir (it finds the config from the
// home subdir derived from Version.txt). Commits staged changes first so the
// game sees the current settings.
static void launch(void) {
    save();
    std::string engine = find_engine();
    std::string arg = "-D" + data_dir;
    if (fork() == 0) {
        execlp(engine.c_str(), engine.c_str(), arg.c_str(), NULL);
        _exit(0);
    }
}

// ---------------------------------------------------------------------------
// XDG asset discovery (assets in ~/.local/share/vs-05/assets, config+saves in
// ~/.config/vs-05)
// ---------------------------------------------------------------------------

static std::string xdg_data_dir(void) {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    return home + "/.local/share";
}
static std::string xdg_config_dir(void) {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    return home + "/.config";
}
static std::string assets_dir_path(void) { return xdg_data_dir() + "/vs-05/assets"; }
static std::string active_asset_file(void) { return xdg_config_dir() + "/vs-05/active_asset"; }

static void ensure_assets_dir(void) {
    mkdir(xdg_data_dir().c_str(), 0755);
    mkdir((xdg_data_dir() + "/vs-05").c_str(), 0755);
    mkdir(assets_dir_path().c_str(), 0755);
}

static void discover_assets(void) {
    discovered.clear();
    DIR *dir = opendir(assets_dir_path().c_str());
    if (!dir) return;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        struct stat st;
        std::string p = assets_dir_path() + "/" + ent->d_name;
        if (stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            discovered.push_back(ent->d_name);
    }
    closedir(dir);
    std::sort(discovered.begin(), discovered.end());
}

static void load_active_asset(void) {
    FILE *f = fopen(active_asset_file().c_str(), "r");
    if (f) {
        char buf[256];
        if (fgets(buf, sizeof(buf), f)) { buf[strcspn(buf, "\n")] = '\0'; active_asset = buf; }
        fclose(f);
    }
}

static void save_active_asset(void) {
    mkdir(xdg_config_dir().c_str(), 0755);
    mkdir((xdg_config_dir() + "/vs-05").c_str(), 0755);
    FILE *f = fopen(active_asset_file().c_str(), "w");
    if (f) { fputs(active_asset.c_str(), f); fclose(f); }
}

// The installed assets_help.txt (next to this app), cached; falls back to the
// embedded ASSET_HELP text if it can't be read.
static const std::string &assets_help_text(void) {
    static std::string cached;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        std::string path;
        char buf[4096];
        ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            std::string exe(buf);
            size_t slash = exe.rfind('/');
            if (slash != std::string::npos) path = exe.substr(0, slash) + "/assets_help.txt";
        }
        FILE *f = !path.empty() ? fopen(path.c_str(), "r") : NULL;
        if (f) {
            char line[512];
            while (fgets(line, sizeof(line), f)) cached += line;
            fclose(f);
        }
        if (cached.empty()) cached = ASSET_HELP;
    }
    return cached;
}

// ---------------------------------------------------------------------------
// Startup: find the data dir, read setup.config, locate the config we edit
// ---------------------------------------------------------------------------

static bool discover(int argc, char **argv) {
    // candidate data dirs, mirroring the original setup.cpp search
    char cwd[4096];
    std::string launch = getcwd(cwd, sizeof(cwd)) ? cwd : "";
    std::vector<std::string> candidates;
    if (argc > 2 && strcmp(argv[1], "--target") == 0) candidates.push_back(argv[2]);
#ifdef DATA_DIR
    candidates.push_back(DATA_DIR);
#endif
    if (!launch.empty()) {
        candidates.push_back(launch);
        candidates.push_back(launch + "/data");
        candidates.push_back(launch + "/../data");
        candidates.push_back(launch + "/data4.x");
        candidates.push_back(launch + "/../data4.x");
        candidates.push_back(launch + "/../Resources");
    }
    std::string found;
    for (auto &c : candidates)
        if (file_exists(c + "/setup.config") && file_exists(c + "/Version.txt")) { found = c; break; }
    if (found.empty()) { fprintf(stderr, "Error: Failed to find data directory (setup.config + Version.txt).\n"); return false; }
    data_dir = found;

    FILE *f = fopen((data_dir + "/setup.config").c_str(), "r");
    if (!f) { fprintf(stderr, "Unable to read setup.config\n"); return false; }
    {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            chomp(line);
            if (line[0] == '#') continue;
            char *kw = line; char *val = next_parm(kw);
            if (val == NULL) continue;
            if (strcmp(kw, "program_name") == 0) program_name = val;
            else if (strcmp(kw, "config_file") == 0) read_source = val;
            else if (strcmp(kw, "columns") == 0) columns = atoi(val);
        }
    }
    fclose(f);
    if (read_source.empty()) read_source = "vegastrike.config";

    // home subdir named in Version.txt
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    std::string sub;
    FILE *v = fopen((data_dir + "/Version.txt").c_str(), "r");
    if (v) { int c; while ((c = fgetc(v)) != EOF && !isspace(c)) sub += (char)c; fclose(v); }
    if (sub.empty()) { fprintf(stderr, "Error: Failed to find Version.txt anywhere.\n"); return false; }
    std::string home_dir = home + "/" + sub;
    mkdir(home_dir.c_str(), 0755);

    // we always edit the home copy; read from whichever copy is newer
    std::string home_cfg = home_dir + "/" + read_source;
    std::string data_cfg = data_dir + "/" + read_source;
    config_file = home_cfg;
    read_source = (file_exists(home_cfg) && (!file_exists(data_cfg) || file_mtime(home_cfg) >= file_mtime(data_cfg)))
                    ? home_cfg : data_cfg;
    return true;
}

// ---------------------------------------------------------------------------
// GUI
// ---------------------------------------------------------------------------

static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;

// Fixed logical resolution scaled up to fill the fullscreen window.
#define VSSETUP_LOGICAL_W 800
#define VSSETUP_LOGICAL_H 600
static GLuint fbo = 0, fbo_tex = 0;

static void view_readme(void) {
    std::string r = data_dir + "/documentation/readme.txt";
    // fork so xdg-open opens the readme while the settings app keeps running.
    if (fork() == 0) {
        execlp("xdg-open", "xdg-open", r.c_str(), NULL);
        _exit(0);
    }
}

static std::string window_title(void) {
    std::string t = program_name.empty() ? "Vega Strike" : program_name;
    return "Program Configuration - " + t + " - Version 0.5.1 Build 13218";
}

// The asset-selection screen (opened by the main Assets button).
static void draw_assets_screen(void) {
    ImGui::Text("Asset Packs");
    ImGui::TextWrapped("Active asset: %s", active_asset.empty() ? "(none)" : active_asset.c_str());
    ImGui::Separator();
    float avail_w = ImGui::GetContentRegionAvail().x;
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;

    ImGui::BeginChild("assets", ImVec2(0, -btn_h), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    if (show_help || discovered.empty()) {
        ImGui::TextWrapped("%s", assets_help_text().c_str());
        if (!discovered.empty()) ImGui::Separator();
    }
    for (auto &a : discovered)
        if (ImGui::Selectable(a.c_str(), a == selected_asset))
            selected_asset = a;
    ImGui::EndChild();

    // Help, Save, Close
    float btnw = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2 + 20;
    float gap = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX((avail_w - (btnw * 3 + gap * 2)) * 0.5f);
    if (ImGui::Button("Help", ImVec2(btnw, 0))) show_help = !show_help;
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(btnw, 0))) {
        if (!selected_asset.empty()) active_asset = selected_asset;
        save_active_asset();
    }
    ImGui::SameLine();
    if (ImGui::Button("Close", ImVec2(btnw, 0))) mode = 0;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!discover(argc, argv)) return SDL_APP_FAILURE;

    FILE *fp = fopen(read_source.c_str(), "r");
    if (!fp) { fprintf(stderr, "Unable to read %s\n", read_source.c_str()); return SDL_APP_FAILURE; }
    parse_config(fp);
    fclose(fp);
    build_display();
    for (auto &g : groups) g.original = g.current;   // baseline for save()

    ensure_assets_dir();
    discover_assets();
    load_active_asset();
    selected_asset = active_asset;

    if (!SDL_Init(SDL_INIT_VIDEO)) { fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError()); return SDL_APP_FAILURE; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    std::string title = window_title();
    window = SDL_CreateWindow(title.c_str(), 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN);
    if (!window) { fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError()); return SDL_APP_FAILURE; }
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) { fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError()); return SDL_APP_FAILURE; }
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontDefault();
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Offscreen 800x600 buffer that we scale up to fill the fullscreen window.
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, VSSETUP_LOGICAL_W, VSSETUP_LOGICAL_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "warning: vssetup FBO incomplete\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    fprintf(stderr, "vssetup-imgui ready: %zu groups\n", groups.size());
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    // Scale mouse coords from the fullscreen window into the 800x600 logical space.
    if (event->type == SDL_EVENT_MOUSE_MOTION || event->type == SDL_EVENT_MOUSE_BUTTON_DOWN
        || event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        int win_w, win_h;
        SDL_GetWindowSizeInPixels(window, &win_w, &win_h);
        float sx = (float)VSSETUP_LOGICAL_W / win_w;
        float sy = (float)VSSETUP_LOGICAL_H / win_h;
        if (event->type == SDL_EVENT_MOUSE_MOTION) { event->motion.x *= sx; event->motion.y *= sy; }
        else { event->button.x *= sx; event->button.y *= sy; }
    }
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    static bool want_quit = false;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    // Fix the layout resolution BEFORE the frame is built, so the UI and the
    // 800x600 FBO render agree (otherwise the draw data is laid out in native
    // fullscreen space and gets clipped to nothing at 800x600 -> black).
    ImGui::GetIO().DisplaySize = ImVec2(VSSETUP_LOGICAL_W, VSSETUP_LOGICAL_H);
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(VSSETUP_LOGICAL_W, VSSETUP_LOGICAL_H), ImGuiCond_Always);
    ImGui::Begin("##vssetup", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    if (mode == 1) {
        draw_assets_screen();
    } else {
    ImGui::Text("VS-05 Configuration Utility");
    ImGui::Text("Active asset: %s", active_asset.empty() ? "(none)" : active_asset.c_str());
    ImGui::Separator();

    float avail_w = ImGui::GetContentRegionAvail().x;
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;

    // Frame holding the settings table (fills the space between the header and
    // the buttons; the table just scrolls inside it).
    ImGui::BeginChild("frame", ImVec2(0, -btn_h), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    // Center the table horizontally within the frame.
    int idx = 0;
    std::vector<float> colw(columns, 0.0f);
    for (auto &g : groups) {
        float cw = 0;
        for (auto &s : g.display) cw = fmaxf(cw, ImGui::CalcTextSize(s.c_str()).x);
        cw += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
        cw = fmaxf(cw, ImGui::CalcTextSize(g.name.c_str()).x);
        colw[idx % columns] = fmaxf(colw[idx % columns], cw);
        idx++;
    }
    float total_w = 0;
    for (auto c : colw) total_w += c;
    ImGui::SetCursorPosX(fmaxf(0.0f, (ImGui::GetContentRegionAvail().x - total_w) * 0.5f));
    if (ImGui::BeginTable("grid", columns, ImGuiTableFlags_SizingFixedFit)) {
        for (auto &g : groups) {
            ImGui::TableNextColumn();
            ImGui::Text("%s", g.name.c_str());
            int current = -1;
            for (size_t j = 0; j < g.options.size(); j++)
                if (g.options[j].name == g.current) { current = (int)j; break; }
            int sel = current;
            // Width = widest option text + the dropdown arrow + padding, so the
            // arrow never overlaps the text.
            float cw = 0;
            for (auto &s : g.display) cw = fmaxf(cw, ImGui::CalcTextSize(s.c_str()).x);
            ImGui::SetNextItemWidth(cw + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
            char lbl[64]; snprintf(lbl, sizeof(lbl), "##%s", g.name.c_str());
            if (ImGui::Combo(lbl, &sel, g.items.data(), (int)g.items.size()))
                if (sel >= 0 && sel != current) apply_change(&g, g.options[sel].name);
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    // Center the five buttons side by side. Launch and Exit turn red when there
    // are unsaved changes.
    float btnw = ImGui::CalcTextSize("View Readme").x + ImGui::GetStyle().FramePadding.x * 2 + 20;
    float gap = ImGui::GetStyle().ItemSpacing.x;
    bool unsaved = has_unsaved();
    ImGui::SetCursorPosX((avail_w - (btnw * 5 + gap * 4)) * 0.5f);
    if (ImGui::Button("Save", ImVec2(btnw, 0))) save();
    ImGui::SameLine();
    if (ImGui::Button("View Readme", ImVec2(btnw, 0))) view_readme();
    ImGui::SameLine();
    if (ImGui::Button("Assets", ImVec2(btnw, 0))) { discover_assets(); selected_asset = active_asset; mode = 1; }
    ImGui::SameLine();
    if (unsaved) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    }
    if (ImGui::Button("Launch", ImVec2(btnw, 0))) launch();
    if (unsaved) ImGui::PopStyleColor(2);
    ImGui::SameLine();
    if (unsaved) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    }
    if (ImGui::Button("Exit", ImVec2(btnw, 0))) want_quit = true;
    if (unsaved) ImGui::PopStyleColor(2);
    }
    ImGui::End();

    ImGui::Render();
    // Render ImGui at 800x600 into the offscreen FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, VSSETUP_LOGICAL_W, VSSETUP_LOGICAL_H);
    glClearColor(0.10f, 0.11f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Blit the 800x600 buffer scaled up to fill the fullscreen window.
    int win_w, win_h;
    SDL_GetWindowSizeInPixels(window, &win_w, &win_h);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, VSSETUP_LOGICAL_W, VSSETUP_LOGICAL_H, 0, 0, win_w, win_h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    SDL_GL_SwapWindow(window);
    return want_quit ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    fprintf(stderr, "vssetup-imgui quit\n");
}
