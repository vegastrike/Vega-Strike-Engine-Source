// modern_ui.cpp — the Modern mode of the VS-05 settings app.
//
// Clean rewrite: drives the vs05cfg::Model (config_model.{h,cpp}), seeds the
// base data from the asset config, applies presets.xml preset structure and
// canonical engine vars (display/input), and generates vs-modern.config with a
// mode-switch header. Classic mode in setup.cpp is untouched.
//
// UI drawing is ported from the exploratory_config branch (the display frame +
// flight control), re-mapped onto this clean model. No config-surgery code is
// carried over.
#include "modern_ui.h"
#include "config_model.h"

#include <imgui.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <algorithm>

namespace vs05ui {

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static Mode g_mode = MODE_CLASSIC;
static std::string g_asset;     // active asset name
static std::string g_data_dir;  // active asset data dir
static vs05cfg::Model g_model;  // the working model (seeded from asset)
static std::vector<vs05cfg::PresetGroup> g_presets;   // from shipped presets.xml

static bool g_loaded = false;   // model loaded
static bool g_dirty = false;    // any unsaved change

// Display frame state
static int  sel_monitor = 0;
static SDL_DisplayID sel_display_id = 0;
static int  sel_res_w = 0, sel_res_h = 0;
static std::string monitor_text, resolution_text;
static char text_height_buf[16] = "16";
static const char *aspect_opts[] = { "4:3 (1.33)", "16:10 (1.6)", "16:9 (1.78)", "5:4 (1.25)", "1:1 (1.0)" };
static const float aspect_vals[] = { 4.0f / 3.0f, 1.6f, 16.0f / 9.0f, 1.25f, 1.0f };
static int  sel_base_aspect = 0;
static std::string base_aspect_text = aspect_opts[0];
static int  sel_screen_aspect = -1;      // -1 = auto (W/H)
static std::string screen_aspect_text;
static bool display_inited = false;

// Flight control: 0=keyboard 1=mouse 2=joystick
enum { FC_KEYBOARD = 0, FC_MOUSE = 1, FC_JOYSTICK = 2 };
static int flight_control = FC_KEYBOARD;
static const char *fc_names[] = { "Keyboard", "Mouse", "Joystick" };

// ---------------------------------------------------------------------------
// Path helpers (mirror setup.cpp's XDG layout)
// ---------------------------------------------------------------------------

static std::string xdg_config_dir() {
    std::string home = getenv("HOME") ? getenv("HOME") : ".";
    return home + "/.config";
}

// Path of the shipped presets file, resolved next to the running binary
// (config-vs-05), falling back to the repo/asset copy for dev.
static std::string shipped_presets_file() {
    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        exe[n] = '\0';
        std::string p = std::string(exe);
        size_t slash = p.find_last_of('/');
        if (slash != std::string::npos) {
            std::string dir = p.substr(0, slash);
            std::string cand = dir + "/presets.xml";
            struct stat st;
            if (stat(cand.c_str(), &st) == 0) return cand;
        }
    }
    // dev fallback: repo copy
    const char *candidates[] = {
        "/opt/vs-05-lts/presets.xml",
        "setup/presets.xml",
        NULL
    };
    for (int i = 0; candidates[i]; i++) {
        struct stat st;
        if (stat(candidates[i], &st) == 0) return candidates[i];
    }
    return "";
}

// The engine config path for Modern mode.
static std::string modern_config_file() {
    return xdg_config_dir() + "/vs-05/" + g_asset + "/vs-modern.config";
}

static std::string app_state_file() {
    return xdg_config_dir() + "/vs-05/app_state";
}

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------

static float current_screen_aspect() {
    return sel_res_h > 0 ? (float)sel_res_w / sel_res_h : 0.0f;
}

static void refresh_screen_aspect_text() {
    if (sel_screen_aspect >= 0) screen_aspect_text = aspect_opts[sel_screen_aspect];
    else {
        char b[24]; snprintf(b, sizeof(b), "Auto (%.2f)", current_screen_aspect());
        screen_aspect_text = b;
    }
}

static void prefill_text_height() {
    if (sel_res_h > 0) {
        int fp = (int)(0.0125 * sel_res_h + 2.5 + 0.5);
        snprintf(text_height_buf, sizeof(text_height_buf), "%d", fp);
    }
}

// The highest-resolution mode a monitor supports (by pixel area). Returns false
// if none available.
static bool highest_monitor_resolution(SDL_DisplayID id, int &w, int &h) {
    int cnt = 0;
    SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(id, &cnt);
    if (!modes || cnt == 0) {
        const SDL_DisplayMode *cm = SDL_GetCurrentDisplayMode(id);
        if (cm) { w = cm->w; h = cm->h; return true; }
        return false;
    }
    int best_i = 0, best_area = -1;
    for (int i = 0; i < cnt; i++) {
        int area = modes[i]->w * modes[i]->h;
        if (area > best_area) { best_area = area; best_i = i; }
    }
    w = modes[best_i]->w; h = modes[best_i]->h;
    return true;
}

// True if the display supports this exact resolution (any mode with matching
// width/height).
static bool resolution_supported(SDL_DisplayID id, int w, int h) {
    int cnt = 0;
    SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(id, &cnt);
    if (modes && cnt > 0) {
        for (int i = 0; i < cnt; i++)
            if (modes[i]->w == w && modes[i]->h == h) return true;
        return false;   // enumerated and not found -> unsupported
    }
    // Can't enumerate fullscreen modes: fall back to the monitor's current mode,
    // else accept as-is (can't tell).
    const SDL_DisplayMode *cm = SDL_GetCurrentDisplayMode(id);
    if (cm) return cm->w == w && cm->h == h;
    return true;
}

// ---------------------------------------------------------------------------
// Model helpers (canonical vars on the clean model)
// ---------------------------------------------------------------------------

// Set a canonical var in the model's variables section (adds the section/var if
// absent, so the engine always has it).
static void model_set_var(const std::string &section, const std::string &var, const std::string &value) {
    vs05cfg::Element *v = NULL;
    for (auto &c : g_model.root.children)
        if (c.name == "variables") { v = &c; break; }
    if (!v) return;
    // ensure section exists
    vs05cfg::Element *sec = NULL;
    for (auto &c : v->children)
        if (c.name == "section" && vs05cfg::attr(c, "name") == section) { sec = &c; break; }
    if (!sec) {
        vs05cfg::Element s; s.name = "section";
        s.attrs.push_back(std::make_pair("name", section));
        v->children.push_back(s);
        sec = &v->children.back();
    }
    // set or add the var
    for (auto &c : sec->children) {
        if (c.name == "var" && vs05cfg::attr(c, "name") == var) {
            vs05cfg::set_attr(c, "value", value);
            return;
        }
    }
    vs05cfg::Element e; e.name = "var";
    e.attrs.push_back(std::make_pair("name", var));
    e.attrs.push_back(std::make_pair("value", value));
    sec->children.push_back(e);
}

static std::string model_get_var(const std::string &section, const std::string &var) {
    return vs05cfg::get_var(g_model, section, var);
}

// ---------------------------------------------------------------------------
// Apply the current UI state (presets + display + flight) to the model
// ---------------------------------------------------------------------------

static void apply_presets_to_model() {
    // Apply each preset group's active selection from the shipped presets.xml.
    // The hand-rolled groups (Resolution/Monitor via display frame, Mouse/Joystick
    // via their dialogs) are skipped here; their vars are written by the dialogs.
    for (auto &g : g_presets) {
        if (g.name == "Resolution" || g.name == "Mouse" || g.name == "Joystick") continue;
        if (!g.current.empty())
            vs05cfg::apply_preset(g, g.current, g_model);
    }
}

// Largest base resolution of the chosen base aspect that fits within the main
// resolution (letterboxed): width-constrained if the base aspect is wider than
// the screen, else height-constrained.
static void compute_base_max(int &w, int &h) {
    w = h = 0;
    if (sel_res_w <= 0 || sel_res_h <= 0) return;
    float A = aspect_vals[sel_base_aspect];
    if (A >= (float)sel_res_w / sel_res_h) { w = sel_res_w; h = (int)(sel_res_w / A); }
    else { h = sel_res_h; w = (int)(sel_res_h * A); }
}

static void apply_display_to_model() {
    char b[32];
    snprintf(b, sizeof(b), "%d", sel_res_w);   model_set_var("graphics", "x_resolution", b);
    snprintf(b, sizeof(b), "%d", sel_res_h);   model_set_var("graphics", "y_resolution", b);
    model_set_var("graphics", "font_point", text_height_buf);
    char asp[24];
    snprintf(asp, sizeof(asp), "%.3f", sel_screen_aspect >= 0 ? aspect_vals[sel_screen_aspect] : current_screen_aspect());
    model_set_var("graphics", "aspect", asp);
    int bw = 0, bh = 0; compute_base_max(bw, bh);
    if (bw > 0 && bh > 0) {
        snprintf(b, sizeof(b), "%d", bw); model_set_var("graphics", "base_max_width", b);
        snprintf(b, sizeof(b), "%d", bh); model_set_var("graphics", "base_max_height", b);
    }
}

static void apply_flight_to_model() {
    // Flight-control exclusivity via the joystick/mouse canonical vars.
    if (flight_control == FC_MOUSE) {
        model_set_var("joystick", "force_use_of_joystick", "false");
        model_set_var("joystick", "warp_mouse", "true");
    } else if (flight_control == FC_JOYSTICK) {
        model_set_var("joystick", "force_use_of_joystick", "true");
    } else {
        model_set_var("joystick", "force_use_of_joystick", "false");
        model_set_var("joystick", "warp_mouse", "false");
    }
}

static void apply_all_to_model() {
    apply_presets_to_model();
    apply_display_to_model();
    apply_flight_to_model();
}

static void restore_preset_selections(const std::string &modern_cfg);   // defined below (after init)

// ---------------------------------------------------------------------------
// Seed the model from the asset config
// ---------------------------------------------------------------------------

// Determine the startup mode: the vs-modern.config switch if present, else the
// persisted app_state, else Classic.
static void load_startup_mode() {
    std::string mf = modern_config_file();
    FILE *f = fopen(mf.c_str(), "r");
    if (f) {
        char line[256];
        if (fgets(line, sizeof(line), f) && strstr(line, "mode=modern") != NULL)
            g_mode = MODE_MODERN;
        fclose(f);
        return;   // the switch (if the file exists) is authoritative
    }
    // no modern config: fall back to app_state
    f = fopen(app_state_file().c_str(), "r");
    if (f) {
        char line[64];
        if (fgets(line, sizeof(line), f))
            g_mode = (strncmp(line, "modern", 6) == 0) ? MODE_MODERN : MODE_CLASSIC;
        fclose(f);
    }
}

bool init(const std::string &active_asset, const std::string &data_dir) {
    g_asset = active_asset;
    g_data_dir = data_dir;
    g_loaded = false;
    g_dirty = false;
    load_startup_mode();

    // Pick the first display (default monitor) so the resolution can seed.
    {
        int nd = 0; SDL_DisplayID *ids = SDL_GetDisplays(&nd);
        if (nd > 0) {
            sel_display_id = ids[0]; sel_monitor = 0;
            const char *nm = SDL_GetDisplayName(sel_display_id);
            monitor_text = std::to_string(sel_monitor) + "  " + (nm ? nm : "(unnamed)");
        }
    }

    // Load the modern config if it exists, else seed from the asset config.
    std::string mf = modern_config_file();
    struct stat st;
    if (stat(mf.c_str(), &st) == 0 && vs05cfg::parse(mf, g_model)) {
        g_loaded = true;
    } else {
        std::string asset_cfg = data_dir + "/vegastrike.config";
        if (vs05cfg::parse(asset_cfg, g_model)) g_loaded = true;
    }

    // Load the preset structure from the shipped presets file (defaults), then
    // overlay any persisted #set selections from an existing vs-modern.config.
    std::string pf = shipped_presets_file();
    if (!pf.empty()) vs05cfg::parse_presets(pf, g_presets);
    restore_preset_selections(mf);

    // Restore display state from the model. If the model has no resolution, or the
    // set resolution isn't actually supported by the monitor, fall back to the
    // highest the monitor supports.
    std::string xr = model_get_var("graphics", "x_resolution");
    std::string yr = model_get_var("graphics", "y_resolution");
    bool need_default = !( !xr.empty() && !yr.empty() );
    if (!need_default) {
        int w = atoi(xr.c_str()), h = atoi(yr.c_str());
        if (!resolution_supported(sel_display_id, w, h)) need_default = true;
        else { sel_res_w = w; sel_res_h = h; }
    }
    if (need_default && sel_display_id) {
        int w, h; if (highest_monitor_resolution(sel_display_id, w, h)) { sel_res_w = w; sel_res_h = h; }
    }
    if (sel_res_h > 0)
        resolution_text = std::to_string(sel_res_w) + "x" + std::to_string(sel_res_h);
    std::string fp = model_get_var("graphics", "font_point");
    if (!fp.empty()) snprintf(text_height_buf, sizeof(text_height_buf), "%s", fp.c_str());
    refresh_screen_aspect_text();
    return g_loaded;
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

// Build the app-owned #set header (active preset per group) to persist the
// selections inside vs-modern.config (the engine ignores the comment).
// Restore the persisted #set selections from vs-modern.config (overriding the
// shipped defaults). The engine ignores the comment header.
static void restore_preset_selections(const std::string &modern_cfg) {
    FILE *f = fopen(modern_cfg.c_str(), "r");
    if (!f) return;
    char line[1024];
    bool in_presets = false;
    while (fgets(line, sizeof(line), f)) {
        char *t = line;
        while (*t == ' ' || *t == '\t') t++;
        if (strncmp(t, "<!-- vssetup presets", 20) == 0) { in_presets = true; continue; }
        if (strncmp(t, "-->", 3) == 0) { in_presets = false; continue; }
        if (!in_presets) continue;
        if (strncmp(t, "#set ", 5) == 0) {
            std::string rest(t + 5);
            size_t sp = rest.find_first_of(" \t\n");
            if (sp == std::string::npos) continue;
            std::string gname = rest.substr(0, sp);
            std::string opt = rest.substr(sp);
            size_t sp2 = opt.find_first_not_of(" \t\n");
            if (sp2 == std::string::npos) continue;
            size_t end = opt.find_first_of(" \t\n", sp2);
            opt = opt.substr(sp2, end == std::string::npos ? std::string::npos : end - sp2);
            for (auto &g : g_presets)
                if (g.name == gname) { g.current = opt; break; }
        }
    }
    fclose(f);
}

static std::string preset_header() {
    std::string h;
    for (auto &g : g_presets) {
        if (g.name == "Resolution" || g.name == "Mouse" || g.name == "Joystick") continue;
        if (!g.current.empty()) h += "#set " + g.name + " " + g.current + "\n";
    }
    return h;
}

bool save() {
    if (!g_loaded) return false;
    apply_all_to_model();
    bool ok = vs05cfg::generate_modern_file(g_model, modern_config_file(), "modern", preset_header());
    if (ok) g_dirty = false;
    return ok;
}

void reset() {
    if (g_asset.empty()) return;
    // Re-seed the model from the asset config (discard all modern edits).
    std::string asset_cfg = g_data_dir + "/vegastrike.config";
    if (vs05cfg::parse(asset_cfg, g_model)) g_loaded = true;
    // Reset preset selections to the shipped defaults.
    std::string pf = shipped_presets_file();
    if (!pf.empty()) vs05cfg::parse_presets(pf, g_presets);
    g_dirty = true;   // the next Save regenerates from the fresh asset base
}

bool has_unsaved() {
    return g_dirty;
}

Mode mode() { return g_mode; }

// Persist the mode to ~/.config/vs-05/app_state (read at startup).
static void persist_mode() {
    FILE *f = fopen(app_state_file().c_str(), "w");
    if (f) {
        fprintf(f, "%s\n", g_mode == MODE_MODERN ? "modern" : "classic");
        fclose(f);
    }
}

void set_mode(Mode m) {
    g_mode = m;
    if (m == MODE_MODERN && !g_loaded && !g_asset.empty())
        init(g_asset, g_data_dir);   // (re)seed when entering modern
    persist_mode();
}

std::string engine_config_file() {
    return modern_config_file();
}

// ---------------------------------------------------------------------------
// Event handling (capture + joystick) — extended as dialogs are added
// ---------------------------------------------------------------------------

void handle_event(const SDL_Event *event) {
    (void)event;
}

// ---------------------------------------------------------------------------
// Drawing — the Modern screen (display frame + flight control)
// ---------------------------------------------------------------------------

static void draw_display_frame() {
    if (!display_inited) {
        int nd = 0; SDL_DisplayID *ids = SDL_GetDisplays(&nd);
        if (nd < 1) nd = 1;
        if (sel_monitor >= nd) sel_monitor = 0;
        sel_display_id = ids ? ids[sel_monitor] : 0;
        const char *nm = sel_display_id ? SDL_GetDisplayName(sel_display_id) : NULL;
        monitor_text = std::to_string(sel_monitor) + "  " + (nm ? nm : "(unnamed)");
        if (sel_res_h == 0) {
            int w = 0, h = 0;
            if (highest_monitor_resolution(sel_display_id, w, h)) { sel_res_w = w; sel_res_h = h; }
            else { const SDL_DisplayMode *cm = sel_display_id ? SDL_GetCurrentDisplayMode(sel_display_id) : NULL;
                   if (cm) { sel_res_w = cm->w; sel_res_h = cm->h; } }
            if (sel_res_h > 0) resolution_text = std::to_string(sel_res_w) + "x" + std::to_string(sel_res_h);
        }
        base_aspect_text = aspect_opts[sel_base_aspect];
        refresh_screen_aspect_text();
        display_inited = true;
    }

    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    float avail_w = ImGui::GetContentRegionAvail().x;
    float dpy_w = avail_w * 0.72f;
    float side_w = avail_w - dpy_w;

    ImGui::BeginChild("dpyframe", ImVec2(dpy_w, 6 * btn_h), ImGuiChildFlags_Borders);
    if (ImGui::Button("Monitor")) ImGui::OpenPopup("##pick_mon");
    ImGui::SameLine(); ImGui::TextUnformatted(monitor_text.c_str());
    if (ImGui::BeginPopup("##pick_mon")) {
        int nd = 0; SDL_DisplayID *ids = SDL_GetDisplays(&nd);
        for (int i = 0; i < nd; ++i) {
            const char *nm = SDL_GetDisplayName(ids[i]);
            char lbl[160]; snprintf(lbl, sizeof(lbl), "%d  %s", i, nm ? nm : "(unnamed)");
            if (ImGui::MenuItem(lbl)) {
                sel_monitor = i; sel_display_id = ids[i];
                monitor_text = lbl; g_dirty = true;
                const SDL_DisplayMode *cm = SDL_GetCurrentDisplayMode(ids[i]);
                if (cm) { sel_res_w = cm->w; sel_res_h = cm->h;
                    resolution_text = std::to_string(cm->w) + "x" + std::to_string(cm->h);
                    if (sel_screen_aspect < 0) refresh_screen_aspect_text(); }
            }
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Resolution")) ImGui::OpenPopup("##pick_res");
    ImGui::SameLine(); ImGui::TextUnformatted(resolution_text.c_str());
    if (ImGui::BeginPopup("##pick_res")) {
        int cnt = 0; SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(sel_display_id, &cnt);
        if (modes) {
            std::vector<std::string> seen;
            for (int i = 0; i < cnt; ++i) {
                char lbl[32]; snprintf(lbl, sizeof(lbl), "%dx%d", modes[i]->w, modes[i]->h);
                if (std::find(seen.begin(), seen.end(), lbl) != seen.end()) continue;
                seen.push_back(lbl);
                if (ImGui::MenuItem(lbl)) {
                    sel_res_w = modes[i]->w; sel_res_h = modes[i]->h;
                    resolution_text = lbl; prefill_text_height(); g_dirty = true;
                    if (sel_screen_aspect < 0) refresh_screen_aspect_text();
                }
            }
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Screen Aspect")) ImGui::OpenPopup("##pick_sa");
    ImGui::SameLine(); ImGui::TextUnformatted(screen_aspect_text.c_str());
    if (ImGui::BeginPopup("##pick_sa")) {
        if (ImGui::MenuItem("Auto (W/H)")) { sel_screen_aspect = -1; refresh_screen_aspect_text(); g_dirty = true; }
        ImGui::Separator();
        for (size_t i = 0; i < sizeof(aspect_opts) / sizeof(aspect_opts[0]); ++i)
            if (ImGui::MenuItem(aspect_opts[i])) { sel_screen_aspect = (int)i; refresh_screen_aspect_text(); g_dirty = true; }
        ImGui::EndPopup();
    }
    ImGui::Text("Text Height"); ImGui::SameLine();
    ImGui::SetNextItemWidth(90);
    if (ImGui::InputText("##textheight", text_height_buf, sizeof(text_height_buf), ImGuiInputTextFlags_CharsDecimal))
        g_dirty = true;
    if (ImGui::Button("Base Aspect Ratio")) ImGui::OpenPopup("##pick_asp");
    ImGui::SameLine(); ImGui::TextUnformatted(base_aspect_text.c_str());
    if (ImGui::BeginPopup("##pick_asp")) {
        for (size_t i = 0; i < sizeof(aspect_opts) / sizeof(aspect_opts[0]); ++i)
            if (ImGui::MenuItem(aspect_opts[i])) { sel_base_aspect = (int)i; base_aspect_text = aspect_opts[i]; g_dirty = true; }
        ImGui::EndPopup();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("dpybtns", ImVec2(side_w, 6 * btn_h), ImGuiChildFlags_Borders);
    if (ImGui::Button(("Flight Control: " + std::string(fc_names[flight_control])).c_str(), ImVec2(-1, 0)))
        ImGui::OpenPopup("##flight");
    if (ImGui::BeginPopup("##flight")) {
        for (int i = 0; i < 3; ++i)
            if (ImGui::MenuItem(fc_names[i])) { flight_control = i; g_dirty = true; }
        ImGui::EndPopup();
    }
    // Placeholder sub-screen buttons (dialogs added in a later phase).
    ImGui::TextDisabled("Bindings / Joystick / Mouse settings\n(coming in a later phase)");
    ImGui::EndChild();
}

static void draw_preset_table();   // defined below

void draw() {
    ImGui::Separator();
    draw_display_frame();
    ImGui::Separator();
    // Frame holding the presets (fills the space between the header and the
    // buttons, pushing the button row to the bottom of the screen - mirrors
    // classic's settings-table frame).
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("mframe", ImVec2(0, -btn_h), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    draw_preset_table();
    ImGui::EndChild();
}

// The preset dropdowns (from the shipped presets.xml), excluding the hand-rolled
// display/input groups which are configured via the frame + dialogs.
static void draw_preset_table() {
    if (g_presets.empty()) {
        ImGui::TextWrapped("No presets loaded (shipped presets.xml not found).");
        return;
    }
    // Collect the preset groups to show (skip hand-rolled display/input groups).
    std::vector<vs05cfg::PresetGroup*> show;
    for (auto &g : g_presets) {
        if (g.name == "Resolution" || g.name == "Mouse" || g.name == "Joystick") continue;
        if (!g.options.empty()) show.push_back(&g);
    }
    if (show.empty()) { ImGui::TextWrapped("No presets to show."); return; }

    int cols = 3;
    // Mirror classic's centering: per-column width from the widest (label or
    // combo), sum them, and SetCursorPosX to center the SizingFixedFit table.
    std::vector<float> colw(cols, 0.0f);
    for (size_t i = 0; i < show.size(); i++) {
        auto &g = *show[i];
        float cw = 0;
        for (auto &o : g.options) cw = fmaxf(cw, ImGui::CalcTextSize(o.desc.empty() ? o.name.c_str() : o.desc.c_str()).x);
        cw += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
        cw = fmaxf(cw, ImGui::CalcTextSize(g.name.c_str()).x);
        colw[i % cols] = fmaxf(colw[i % cols], cw);
    }
    float total_w = 0;
    for (auto c : colw) total_w += c;
    ImGui::SetCursorPosX(fmaxf(0.0f, (ImGui::GetContentRegionAvail().x - total_w) * 0.5f));
    if (ImGui::BeginTable("modern_presets", cols, ImGuiTableFlags_SizingFixedFit)) {
        for (size_t i = 0; i < show.size(); i++) {
            auto &g = *show[i];
            ImGui::TableNextColumn();
            ImGui::Text("%s", g.name.c_str());
            int current = -1;
            for (size_t j = 0; j < g.options.size(); j++)
                if (g.options[j].name == g.current) { current = (int)j; break; }
            int sel = current;
            float cw = 0;
            for (auto &o : g.options) cw = fmaxf(cw, ImGui::CalcTextSize(o.desc.empty() ? o.name.c_str() : o.desc.c_str()).x);
            ImGui::SetNextItemWidth(cw + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
            std::string lbl = "##mpre_" + g.name;
            std::vector<const char*> items;
            for (auto &o : g.options) items.push_back(o.desc.empty() ? o.name.c_str() : o.desc.c_str());
            if (ImGui::Combo(lbl.c_str(), &sel, items.data(), (int)items.size())) {
                if (sel >= 0 && sel != current) {
                    g.current = g.options[sel].name;
                    g_dirty = true;
                }
            }
        }
        ImGui::EndTable();
    }
}

} // namespace vs05ui
