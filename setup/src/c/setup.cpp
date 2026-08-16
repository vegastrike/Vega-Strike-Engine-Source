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
//   - Launch options on the asset screen: optional command template around a
//     fixed %command% marker (e.g. "prime-run %command%") honored by Launch.

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#define GL_GLEXT_PROTOTYPES 1   // GL 3.0 FBO/blit functions (glGenFramebuffers, glBlitFramebuffer)
#include <GL/gl.h>

#include "modern_ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
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
static bool reset_pending = false;  // staged "Reset Config": save() will overwrite the user config with the asset's
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
static bool invalid_popup = false;              // warn about an asset lacking Version.txt
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
        char *kw = head + 1;
        // next_parm(kw) below null-terminates the first token of head+1 (clobbering it), and the
        // option-match scan also mutates the buffer it walks, so keep TWO copies: marker_orig is
        // used for the rewritten marker line output (kept whole, so all the marker's options are
        // preserved), and marker_scan is walked by the match loop (next_parm may modify it).
        char *marker_orig = strdup(kw);
        char *marker_scan = strdup(kw);
        char *args = next_parm(kw);

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
            char *grp = args; (void)next_parm(grp);   // advances + null-terminates the group token
            if (grp && strcmp(grp, group.c_str()) == 0) {
                if (setting == 1) fprintf(wp, "#set %s none\n", group.c_str());
                else fprintf(wp, "#set %s %s\n", group.c_str(), name.c_str());
            } else { fprintf(wp, "%s\n", line); }
            continue;
        }

        // an option-marker "#..." line: does it mention `name`? Scan marker_scan so ALL options
        // are checked (head+1 was clobbered to the first token by next_parm(kw) above).
        bool match = false;
        char *t = marker_scan;
        while (t && !match && (args = next_parm(t)) != NULL) {
            if (strcmp(t, name.c_str()) == 0) match = true;
            t = args;
        }
        free(marker_scan);
        if (match) commenting = setting;
        if (commenting == 0) { fprintf(wp, "%s\n", line); free(marker_orig); continue; }

        fprintf(wp, "%s", p.before);
        // Output from marker_orig (whole), prefixed with '#': p.inside is clobbered to the first
        // token by next_parm(kw), so using it would drop the other options of the marker.
        if (commenting == 1) fprintf(wp, "<!-- #%s", marker_orig);
        else fprintf(wp, "<!-- #%s -->", marker_orig);
        fprintf(wp, "%s\n", p.after);
        free(marker_orig);
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
    if (config_file.empty()) return;   // no active asset -> nothing to save
    if (reset_pending) {
        // Rebase the user config on the asset's shipped config (discard the
        // old user file), then fall through so any selections the user made
        // after Reset are still applied on top of that fresh base.
        std::string asset_cfg = data_dir + "/vegastrike.config";
        FILE *in = fopen(asset_cfg.c_str(), "r");
        if (in) {
            FILE *out = fopen(config_file.c_str(), "w");
            if (out) {
                char buf[4096]; size_t n;
                while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
                fclose(out);
            }
            fclose(in);
        }
        reset_pending = false;
        // NOTE: do NOT set g.original = g.current here, or the user's new
        // selections would be marked as already-saved and skipped below.
    }
    for (auto &g : groups)
        if (g.current != g.original) {
            rewrite(config_file, g.name, g.original, 1);
            rewrite(config_file, g.name, g.current, 2);
            g.original = g.current;
        }
}

// True if any group has a staged change (or a pending reset) not yet committed.
static bool has_unsaved(void) {
    if (reset_pending) return true;
    for (auto &g : groups)
        if (g.current != g.original) return true;
    return false;
}

// Reload the config model from the ASSET's shipped file, discarding any user
// overrides, and stage a full overwrite for the next save().
static void reset_config(void) {
    if (data_dir.empty() || active_asset.empty()) return;
    std::string asset_cfg = data_dir + "/vegastrike.config";
    if (!file_exists(asset_cfg)) return;
    groups.clear();
    FILE *fp = fopen(asset_cfg.c_str(), "r");
    if (!fp) return;
    parse_config(fp);
    fclose(fp);
    build_display();
    for (auto &g : groups) g.original = g.current;
    reset_pending = true;
}

// Launch command template, persisted in ~/.config/vs-05/launch_command.
// Format: "<prefix> %command% <suffix>", e.g. "prime-run %command%".
// %command% is the game itself (engine binary + -D data dir). The marker is
// mandatory; the UI enforces this by rendering it as fixed text between the
// two editable fields. An empty file or a bare "%command%" means direct launch.
static std::string xdg_config_dir(void);   // defined below (XDG section)
static std::string launch_command_file(void) { return xdg_config_dir() + "/vs-05/launch_command"; }
static std::string read_launch_command(void) {
    FILE *f = fopen(launch_command_file().c_str(), "r");
    if (!f) return "";
    char buf[256];
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return ""; }
    fclose(f);
    chomp(buf);
    return buf;
}
static void write_launch_command(const std::string &cmd) {
    FILE *f = fopen(launch_command_file().c_str(), "w");
    if (f) { fputs(cmd.c_str(), f); fputc('\n', f); fclose(f); }
}

// Edit buffers for the launch-options fields on the asset screen. The stored
// template is split around the fixed %command% marker for display.
static char launch_pre_buf[128] = "";
static char launch_suf_buf[128] = "";

static void launch_load_bufs(void) {
    std::string cmd = read_launch_command();
    size_t at = cmd.find("%command%");
    if (at == std::string::npos) {      // missing/legacy file -> direct launch form
        launch_pre_buf[0] = '\0';
        launch_suf_buf[0] = '\0';
        return;
    }
    std::string pre = cmd.substr(0, at);
    std::string suf = cmd.substr(at + 9);    // strlen("%command%")
    while (!pre.empty() && isspace((unsigned char) pre[pre.size()-1])) pre.erase(pre.size()-1);
    while (!pre.empty() && isspace((unsigned char) pre[0])) pre.erase(0, 1);
    while (!suf.empty() && isspace((unsigned char) suf[0])) suf.erase(0, 1);
    while (!suf.empty() && isspace((unsigned char) suf[suf.size()-1])) suf.erase(suf.size()-1);
    snprintf(launch_pre_buf, sizeof(launch_pre_buf), "%s", pre.c_str());
    snprintf(launch_suf_buf, sizeof(launch_suf_buf), "%s", suf.c_str());
}

// Normalize the edit buffers into a template and persist it. Called on every
// edit; the marker always survives (it is fixed UI text), so the result is
// always valid. Both sides empty -> "%command%" (direct launch).
static void launch_commit_bufs(void) {
    std::string pre = launch_pre_buf;
    std::string suf = launch_suf_buf;
    while (!pre.empty() && isspace((unsigned char) pre[pre.size()-1])) pre.erase(pre.size()-1);
    while (!pre.empty() && isspace((unsigned char) pre[0])) pre.erase(0, 1);
    while (!suf.empty() && isspace((unsigned char) suf[0])) suf.erase(0, 1);
    while (!suf.empty() && isspace((unsigned char) suf[suf.size()-1])) suf.erase(suf.size()-1);
    std::string cmd = "%command%";
    if (!pre.empty()) cmd = pre + " " + cmd;
    if (!suf.empty()) cmd = cmd + " " + suf;
    write_launch_command(cmd);
}

// The engine binary (vs-05) sits next to this app in the install dir.
static std::string find_engine(void) {
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::string exe(buf);
        size_t slash = exe.rfind('/');
        if (slash != std::string::npos) {
            std::string cand = exe.substr(0, slash) + "/vs-05";
            if (file_exists(cand)) return cand;
        }
    }
    return data_dir + "/../vs-05";
}

// Split a launch-command template on whitespace into argv tokens. %command%
// expands to the engine binary + its -D<data> argument; launch_commit_bufs()
// normalizes the template so the marker stands alone as a token. No shell is
// involved - quoting/globbing in the template is not supported.
static std::vector<std::string> tokenize_cmd(const std::string &s) {
    std::vector<std::string> t;
    size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && isspace((unsigned char) s[i])) i++;
        if (i >= s.size()) break;
        size_t j = i;
        while (j < s.size() && !isspace((unsigned char) s[j])) j++;
        t.push_back(s.substr(i, j - i));
        i = j;
    }
    return t;
}

// Launch the engine, pointing it at the data dir (it finds the config from the
// home subdir derived from Version.txt). Commits staged changes first so the
// game sees the current settings. Honors the launch-command template from the
// asset screen: %command% expands to the engine + -D arg (empty/bare template
// launches directly).
static void launch(void) {
    if (active_asset.empty()) { fprintf(stderr, "No active asset to launch.\n"); return; }
    save();
    std::string engine = find_engine();
    std::string arg = "-D" + data_dir;
    std::vector<std::string> argv_s;
    bool expanded = false;
    for (auto &tok : tokenize_cmd(read_launch_command())) {
        if (tok == "%command%") {
            argv_s.push_back(engine);
            argv_s.push_back(arg);
            expanded = true;
        } else {
            argv_s.push_back(tok);
        }
    }
    if (argv_s.empty() || !expanded) {   // empty/bare template -> direct launch
        argv_s.push_back(engine);
        argv_s.push_back(arg);
    }
    std::vector<char *> argv;
    for (auto &s : argv_s) argv.push_back(const_cast<char *>(s.c_str()));
    argv.push_back(NULL);
    if (fork() == 0) {
        execvp(argv[0], argv.data());
        fprintf(stderr, "vssetup: failed to launch '%s': %s\n", argv[0], strerror(errno));
        _exit(1);
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

// True if the asset dir contains a Version.txt (empty = the 'none' option is fine).
static bool asset_has_version(const std::string &name) {
    if (name.empty()) return true;
    return file_exists(assets_dir_path() + "/" + name + "/Version.txt");
}

// Ensure the asset's Version.txt points the engine at the per-mod config dir
// (~/.config/vs-05/<mod>/), rewriting it if it still names the old .vs-05 subdir.
// Only touches a file that already exists; a missing Version.txt (i.e. not a real
// VegaStrike mod) is left alone and handled by the app's validity check.
static void ensure_asset_version(const std::string &name) {
    std::string vfile = assets_dir_path() + "/" + name + "/Version.txt";
    if (!file_exists(vfile)) return;
    std::string want = ".config/vs-05/" + name;
    std::string cur;
    FILE *f = fopen(vfile.c_str(), "r");
    if (f) {
        char buf[512];
        if (fgets(buf, sizeof(buf), f)) {
            cur = buf;
            if (!cur.empty() && cur[cur.size()-1] == '\n') cur.pop_back();
        }
        fclose(f);
    }
    if (cur != want) {
        FILE *w = fopen(vfile.c_str(), "w");
        if (w) { fprintf(w, "%s\n", want.c_str()); fclose(w); }
        fprintf(stderr, "Set %s Version.txt -> %s\n", name.c_str(), want.c_str());
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

static void copy_file(const std::string &src, const std::string &dst) {
    FILE *in = fopen(src.c_str(), "r");
    if (!in) return;
    FILE *out = fopen(dst.c_str(), "w");
    if (out) {
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
        fclose(out);
    }
    fclose(in);
}

// Load the config model for the active asset. With no active asset there is no
// table. The per-mod config (~/.config/vs-05/<mod>/vegastrike.config) is the
// edit target, initialized from the asset's shipped config on first use.
static bool load_config(void) {
    groups.clear();
    config_file.clear();
    if (active_asset.empty()) return false;
    std::string asset_dir = assets_dir_path() + "/" + active_asset;
    if (!file_exists(asset_dir + "/vegastrike.config")) return false;
    if (!asset_has_version(active_asset)) return false;   // not a valid VegaStrike mod

    columns = 4;
    program_name.clear();
    FILE *f = fopen((asset_dir + "/setup.config").c_str(), "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            chomp(line);
            if (line[0] == '#') continue;
            char *kw = line; char *val = next_parm(kw);
            if (val == NULL) continue;
            if (strcmp(kw, "program_name") == 0) program_name = val;
            else if (strcmp(kw, "columns") == 0) columns = atoi(val);
        }
        fclose(f);
    }

    // per-mod config dir (edit target); init from the shipped config if missing
    std::string pm_dir = xdg_config_dir() + "/vs-05/" + active_asset;
    std::string pm_cfg = pm_dir + "/vegastrike.config";
    if (!file_exists(pm_cfg)) {
        mkdir(xdg_config_dir().c_str(), 0755);
        mkdir((xdg_config_dir() + "/vs-05").c_str(), 0755);
        mkdir(pm_dir.c_str(), 0755);
        copy_file(asset_dir + "/vegastrike.config", pm_cfg);
    }
    config_file = pm_cfg;
    read_source = pm_cfg;
    data_dir = asset_dir;
    ensure_asset_version(active_asset);

    FILE *fp = fopen(read_source.c_str(), "r");
    if (!fp) return false;
    parse_config(fp);
    fclose(fp);
    build_display();
    for (auto &g : groups) g.original = g.current;
    vs05ui::init(active_asset, asset_dir);   // seed the modern model for the active asset
    return true;
}

// ---------------------------------------------------------------------------
// GUI
// ---------------------------------------------------------------------------

static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;

// Fixed logical resolution scaled up to fill the fullscreen window.
#define VSSETUP_LOGICAL_W 1280
#define VSSETUP_LOGICAL_H 720
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
    return "Program Configuration - " + t + " - Version " VSSETUP_VERSION;
}

// The asset-selection screen (opened by the main Assets button).
static void draw_assets_screen(void) {
    ImGui::Text("Asset Packs");
    ImGui::TextWrapped("Active asset: %s", active_asset.empty() ? "(none)" : active_asset.c_str());
    ImGui::Separator();
    float avail_w = ImGui::GetContentRegionAvail().x;
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;

    // Reserve room below the list for the launch-options box + the button row.
    float opt_pad = ImGui::GetStyle().WindowPadding.y;
    float opts_h  = opt_pad * 2
                  + ImGui::GetTextLineHeightWithSpacing()    // "Launch options" header
                  + ImGui::GetFrameHeightWithSpacing()       // prefix + marker + suffix row
                  + ImGui::GetTextLineHeightWithSpacing();   // description line
    float sep_h   = ImGui::GetStyle().ItemSpacing.y;

    ImGui::BeginChild("assets", ImVec2(0, -(btn_h + opts_h + sep_h * 2)), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    if (show_help || discovered.empty()) {
        ImGui::TextWrapped("%s", assets_help_text().c_str());
        if (!discovered.empty()) ImGui::Separator();
    }
    for (auto &a : discovered)
        if (ImGui::Selectable(a.c_str(), a == selected_asset)) {
            selected_asset = a;
            if (!asset_has_version(a)) invalid_popup = true;
        }
    ImGui::Separator();
    if (ImGui::Selectable("(none)", selected_asset.empty()))
        selected_asset = "";
    ImGui::EndChild();

    // Launch options: a bordered box so it reads as its own section. A fixed
    // %command% marker between two editable fields - the marker is UI text and
    // can never be removed (Steam-style, e.g. "prime-run %command%").
    // Persisted by launch_commit_bufs(); honored by launch().
    ImGui::BeginChild("launch_box", ImVec2(0, opts_h), ImGuiChildFlags_Borders);
    ImGui::Text("Launch options");
    float box_avail = ImGui::GetContentRegionAvail().x;
    float marker_w  = ImGui::CalcTextSize("%command%").x;
    float field_w   = fminf(fmaxf(80.0f, (box_avail - marker_w - ImGui::GetStyle().ItemSpacing.x * 2) * 0.5f), 240.0f);
    ImGui::SetNextItemWidth(field_w);
    if (ImGui::InputText("##launch_pre", launch_pre_buf, sizeof(launch_pre_buf)))
        launch_commit_bufs();
    ImGui::SameLine(0, 0);
    ImGui::TextUnformatted("%command%");
    ImGui::SameLine(0, 0);
    ImGui::SetNextItemWidth(field_w);
    if (ImGui::InputText("##launch_suf", launch_suf_buf, sizeof(launch_suf_buf)))
        launch_commit_bufs();
    ImGui::TextDisabled("%%command%% is the game itself - e.g. \"prime-run %%command%%\". Leave both sides empty for a normal launch.");
    ImGui::EndChild();

    // Help, Save, Close. Close turns red when something is unsaved (pending
    // config changes from the main screen, or an asset selection not yet saved)
    // so leaving doesn't silently discard it - mirrors the main screen's Exit.
    float btnw = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2 + 20;
    float gap = ImGui::GetStyle().ItemSpacing.x;
    bool sel_ok = selected_asset.empty() || asset_has_version(selected_asset);
    bool unsaved = has_unsaved() || selected_asset != active_asset;
    ImGui::SetCursorPosX((avail_w - (btnw * 3 + gap * 2)) * 0.5f);
    if (ImGui::Button("Help", ImVec2(btnw, 0))) show_help = !show_help;
    ImGui::SameLine();
    ImGui::BeginDisabled(!sel_ok);
    if (unsaved) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.45f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
    }
    if (ImGui::Button("Save", ImVec2(btnw, 0))) {
        active_asset = selected_asset;
        save_active_asset();
        load_config();   // rebuild the table for the new active asset
    }
    if (unsaved) ImGui::PopStyleColor(2);
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (unsaved) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    }
    if (ImGui::Button("Close", ImVec2(btnw, 0))) mode = 0;
    if (unsaved) ImGui::PopStyleColor(2);

    // Warning modal when the selected directory has no Version.txt.
    if (invalid_popup) { ImGui::OpenPopup("No Version.txt"); invalid_popup = false; }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("No Version.txt", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("This asset has no version.txt.\n\n"
                           "Are you sure this is a VegaStrike mod?\n\n"
                           "Save is disabled for this directory.\n");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    ensure_assets_dir();
    discover_assets();
    load_active_asset();
    selected_asset = active_asset;
    load_config();   // asset-driven; no active asset -> no table

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
    vs05ui::handle_event(event);
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    static bool want_quit = false;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    // Fix the layout resolution BEFORE the frame is built, so the UI and the
    // 1280x720 FBO render agree (otherwise the draw data is laid out in native
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
    ImGui::Text("Version %s", VSSETUP_VERSION);
    ImGui::Text("Active asset: %s", active_asset.empty() ? "(none)" : active_asset.c_str());
    // Mode toggle: Classic (edits the asset config) vs Modern (owns its own config).
    bool is_modern = vs05ui::mode() == vs05ui::MODE_MODERN;
    if (ImGui::Button(is_modern ? "Mode: Modern" : "Mode: Classic")) {
        vs05ui::set_mode(is_modern ? vs05ui::MODE_CLASSIC : vs05ui::MODE_MODERN);
    }
    ImGui::SameLine();
    ImGui::TextWrapped(is_modern ? "own presets -> generates vs-modern.config"
                                : "edits the asset's vegastrike.config");
    ImGui::Separator();

    float avail_w = ImGui::GetContentRegionAvail().x;
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;

    if (is_modern && !active_asset.empty()) {
        // Modern mode: the module draws its own display/input UI.
        vs05ui::draw();
    } else {

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
    if (groups.empty()) {
        ImGui::TextWrapped("No asset selected.\n\nUse the Assets button to choose an installed asset pack, which provides the configuration table.");
    } else {
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
    }
    ImGui::EndChild();
    }   // end Classic-mode table (Modern draws its own UI above)

    // Center the six buttons side by side. Save/Exit and Reset Config turn red when there
    // are unsaved changes. (Same centering method as classic.)
    float btnw = ImGui::CalcTextSize("Reset Config").x + ImGui::GetStyle().FramePadding.x * 2 + 20;
    float gap = ImGui::GetStyle().ItemSpacing.x;
    bool unsaved = is_modern ? vs05ui::has_unsaved() : has_unsaved();
    ImGui::SetCursorPosX((avail_w - (btnw * 6 + gap * 5)) * 0.5f);
    if (unsaved) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.45f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
    }
    if (ImGui::Button("Save", ImVec2(btnw, 0))) {
        if (is_modern) vs05ui::save();
        else save();
    }
    if (unsaved) ImGui::PopStyleColor(2);
    ImGui::SameLine();
    if (ImGui::Button("View Readme", ImVec2(btnw, 0))) view_readme();
    ImGui::SameLine();
    bool rpending = reset_pending;   // capture before the click (the click sets reset_pending)
    if (rpending) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    }
    if (ImGui::Button("Reset Config", ImVec2(btnw, 0))) {
        if (is_modern) vs05ui::reset();
        else reset_config();
    }
    if (rpending) ImGui::PopStyleColor(2);
    ImGui::SameLine();
    if (ImGui::Button("Assets", ImVec2(btnw, 0))) { discover_assets(); selected_asset = active_asset; launch_load_bufs(); mode = 1; }
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
    // Render ImGui at 1280x720 into the offscreen FBO.
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
