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
#include <map>
#include <set>

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

// HUD: use the rendered crosshair (the better-aiming reticle) vs the sprite image.
static bool rendered_crosshair = true;

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

// Path of the shipped bindings file, resolved next to the running binary.
static std::string shipped_bindings_file() {
    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        exe[n] = '\0';
        std::string p = std::string(exe);
        size_t slash = p.find_last_of('/');
        if (slash != std::string::npos) {
            std::string dir = p.substr(0, slash);
            std::string cand = dir + "/bindings.xml";
            struct stat st;
            if (stat(cand.c_str(), &st) == 0) return cand;
        }
    }
    const char *candidates[] = { "/opt/vs-05-lts/bindings.xml", "setup/bindings.xml", NULL };
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
// Input-device state (mouse + joystick)
// ---------------------------------------------------------------------------

// Mouse: the always-active standalone vars + a flight-mode index.
struct MouseStaging {
    int  mode_idx = -1;                    // index into the Mouse group's options (-1 = none)
    bool cam_pancam = false, cam_pantgt = false, cam_chasecam = true;
    char warp_zone[8] = "200", exponent[8] = "1.5", deadband[8] = "0.05";
};
static MouseStaging mouse_stg;
static bool mouse_dialog_open = false;
static bool mouse_pending = false;       // unsaved mouse-dialog changes

// Joystick: the four flight roles (x/y/z/throttle) + the hand-rolled vars.
enum { JR_X = 0, JR_Y, JR_Z, JR_THROTTLE, JR_COUNT };
static const char *joy_role_names[JR_COUNT] = { "x", "y", "z", "throttle" };
static int  joy_bind_stick[JR_COUNT], joy_bind_axis[JR_COUNT];
static bool joy_bind_inv[JR_COUNT];
static char joy_deadband[8] = "0.05";
static bool joy_ffb = false;
static char joy_ff_device[8] = "0";
static bool joy_mouse_cursor = false;
static bool joy_dialog_open = false;
static bool joy_pending = false;
static bool joy_auto_sampling = false;
static float joy_auto_timer = 0.0f, joy_auto_max = 0.0f;

static int joy_role_index(const std::string &name) {
    for (int r = 0; r < JR_COUNT; ++r) if (joy_role_names[r] == name) return r;
    return -1;
}

// Which flight role (if any) maps to this physical (stick, axis).
static int role_for_axis(int stick, int axis) {
    for (int r = 0; r < JR_COUNT; ++r)
        if (joy_bind_stick[r] == stick && joy_bind_axis[r] == axis) return r;
    return -1;
}

// Load the mouse dialog staging from the model.
static void load_mouse_staging() {
    mouse_stg.cam_pancam   = model_get_var("joystick", "mouse_cursor_pancam") == "true";
    mouse_stg.cam_pantgt   = model_get_var("joystick", "mouse_cursor_pantgt") == "true";
    mouse_stg.cam_chasecam = model_get_var("joystick", "mouse_cursor_chasecam") == "true";
    std::string wz = model_get_var("joystick", "warp_mouse_zone");
    std::string ex = model_get_var("joystick", "mouse_exponent");
    std::string db = model_get_var("joystick", "deadband");
    snprintf(mouse_stg.warp_zone, sizeof(mouse_stg.warp_zone), "%s", wz.empty() ? "200" : wz.c_str());
    snprintf(mouse_stg.exponent,  sizeof(mouse_stg.exponent),  "%s", ex.empty() ? "1.5" : ex.c_str());
    snprintf(mouse_stg.deadband,  sizeof(mouse_stg.deadband),  "%s", db.empty() ? "0.05" : db.c_str());
    mouse_stg.mode_idx = -1;
    for (auto &g : g_presets)
        if (g.name == "Mouse") {
            for (size_t i = 0; i < g.options.size(); ++i)
                if (g.options[i].name == g.current) { mouse_stg.mode_idx = (int)i; break; }
        }
}

// Read the x/y/z/throttle <axis> binds from the model (clean XML).
static void load_joystick_axes() {
    for (int r = 0; r < JR_COUNT; ++r) { joy_bind_stick[r] = 0; joy_bind_axis[r] = -1; joy_bind_inv[r] = false; }
    for (int r = 0; r < JR_COUNT; ++r) {
        const vs05cfg::Element *ax = vs05cfg::find_axis(g_model, joy_role_names[r]);
        if (!ax) continue;
        joy_bind_stick[r] = atoi(vs05cfg::attr(*ax, "joystick").c_str());
        joy_bind_axis[r]  = atoi(vs05cfg::attr(*ax, "axis").c_str());
        joy_bind_inv[r]   = vs05cfg::attr(*ax, "inverse") == "true";
    }
}

// Load the joystick dialog staging from the model.
static void load_joystick_staging() {
    load_joystick_axes();
    std::string db = model_get_var("joystick", "deadband");
    std::string ff = model_get_var("joystick", "force_feedback");
    std::string fd = model_get_var("joystick", "ff_device");
    snprintf(joy_deadband, sizeof(joy_deadband), "%s", db.empty() ? "0.05" : db.c_str());
    joy_ffb = ff == "true";
    snprintf(joy_ff_device, sizeof(joy_ff_device), "%s", fd.empty() ? "0" : fd.c_str());
    // NOTE: 'Mouse cursor in flight' is NOT inherited from the shared mouse_cursor var
    // (which mouse mode sets true). joy_mouse_cursor stays at its own value (false unless
    // the user enables it in the Joystick dialog), so joystick mode never shows a cursor
    // by default.
}

// Sample the bound axes for Auto Deadband (max deflection over ~1s).
static float joy_sample_bound_axes() {
    float m = 0.0f;
    SDL_UpdateJoysticks();   // refresh axis state before sampling
    int n = 0; SDL_JoystickID *ids = SDL_GetJoysticks(&n);
    for (int r = 0; r < JR_COUNT; ++r) {
        int s = joy_bind_stick[r];
        if (s < 0 || s >= n || joy_bind_axis[r] < 0) continue;
        SDL_Joystick *joy = SDL_OpenJoystick(ids[s]);
        if (!joy) continue;
        float v = SDL_GetJoystickAxis(joy, joy_bind_axis[r]) / 32768.0f;
        if (v < 0) v = -v;
        if (v > m) m = v;
        SDL_CloseJoystick(joy);
    }
    return m;
}


// ---------------------------------------------------------------------------
// Apply the current UI state (presets + display + flight) to the model
// ---------------------------------------------------------------------------

static void apply_presets_to_model() {
    // Apply each preset group's active selection from the shipped presets.xml.
    // The hand-rolled groups (Resolution/Monitor via display frame, Mouse via its
    // dialog) are skipped here; their vars are written by those controls. (The
    // Joystick group was removed from presets.xml - axis mapping is hand-rolled.)
    for (auto &g : g_presets) {
        if (g.name == "Resolution" || g.name == "Mouse") continue;
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
    model_set_var("graphics", "draw_rendered_crosshairs", rendered_crosshair ? "true" : "false");
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

// Apply the mouse-dialog staging (the always-active mouse vars). Uses sane defaults
// so an empty buffer never zeroes the mouse flight curve (which broke steering).
static const char *mouse_or(const char *buf, const char *def) { return (buf && buf[0]) ? buf : def; }
static void apply_mouse_to_model() {
    // Mouse flight mode: apply the mode preset (glide/warp -> the mouse <axis> binds +
    // mouse_cursor/warp_mouse) whenever Mouse is the active flight device, so the mouse
    // always binds to x/y even without a dialog change.
    if (flight_control == FC_MOUSE) {
        bool sets_cursor = false;
        for (auto &g : g_presets) {
            if (g.name != "Mouse" || g.current.empty()) continue;
            vs05cfg::apply_preset(g, g.current, g_model);
            // if the active mode's preset doesn't declare mouse_cursor, it should be off
            for (auto &o : g.options)
                if (o.name == g.current)
                    for (auto &kv : o.vars)
                        if (kv.first == "mouse_cursor") sets_cursor = true;
        }
        if (!sets_cursor)
            model_set_var("joystick", "mouse_cursor", "false");
    }
    // The always-active standalone mouse vars (gate on the dialog having staged changes).
    if (!mouse_pending) return;
    model_set_var("joystick", "mouse_cursor_pancam",   mouse_stg.cam_pancam   ? "true" : "false");
    model_set_var("joystick", "mouse_cursor_pantgt",   mouse_stg.cam_pantgt   ? "true" : "false");
    model_set_var("joystick", "mouse_cursor_chasecam", mouse_stg.cam_chasecam ? "true" : "false");
    model_set_var("joystick", "warp_mouse_zone", mouse_or(mouse_stg.warp_zone, "200"));
    model_set_var("joystick", "mouse_exponent",   mouse_or(mouse_stg.exponent,  "1.5"));
    model_set_var("joystick", "deadband",          mouse_or(mouse_stg.deadband,  "0.05"));
}

// Apply the joystick-dialog staging (deadband, ffb, axes) when joystick is active.
static void apply_joystick_to_model() {
    if (flight_control != FC_JOYSTICK) return;
    model_set_var("joystick", "deadband",       joy_deadband[0] ? joy_deadband : "0.05");
    model_set_var("joystick", "force_feedback", joy_ffb ? "true" : "false");
    model_set_var("joystick", "ff_device",      joy_ff_device[0] ? joy_ff_device : "0");
    model_set_var("joystick", "mouse_cursor",   joy_mouse_cursor ? "true" : "false");
    // Write the x/y/z/throttle axes: remove unbound roles, add/update bound ones.
    for (int r = 0; r < JR_COUNT; ++r) {
        if (joy_bind_axis[r] < 0) vs05cfg::remove_axis(g_model, joy_role_names[r]);
        else vs05cfg::set_axis(g_model, joy_role_names[r], joy_bind_stick[r], joy_bind_axis[r], joy_bind_inv[r]);
    }
}

static void apply_all_to_model() {
    apply_presets_to_model();
    apply_display_to_model();
    apply_flight_to_model();
    apply_mouse_to_model();
    apply_joystick_to_model();
    // Apply the shipped bindings (keyboard/mouse/joystick <bind> elements), which set
    // the mouse joystick's player=0 via the <bind mouse=0 player=0> entry. This is what
    // makes mouse flight work (ProcessInput polls only player-0 joysticks).
    std::string bf = shipped_bindings_file();
    if (!bf.empty()) vs05cfg::apply_bindings_file(g_model, bf);
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
    rendered_crosshair = model_get_var("graphics", "draw_rendered_crosshairs") != "false";
    load_joystick_staging();   // restore joystick settings (deadband, ffb, axes) on load
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
        if (strncmp(t, "#fc ", 4) == 0) {
            std::string rest(t + 4);
            size_t sp = rest.find_first_of(" \t\n");
            std::string mode = (sp == std::string::npos) ? rest : rest.substr(0, sp);
            for (int i = 0; i < 3; i++)
                if (mode == fc_names[i]) flight_control = i;
            continue;
        }
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
    // Persist the flight-control mode so it restores on next load.
    h += "#fc " + std::string(fc_names[flight_control]) + "\n";
    for (auto &g : g_presets) {
        if (g.name == "Resolution") continue;
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
    // Joystick hotplug: when a device is added/removed mid-run, refresh the
    // joystick state so a newly-connected joystick's axes go live immediately.
    if (event->type == SDL_EVENT_JOYSTICK_ADDED || event->type == SDL_EVENT_JOYSTICK_REMOVED)
        SDL_UpdateJoysticks();
}

// ---------------------------------------------------------------------------
// Drawing — the Modern screen (display frame + flight control)
// ---------------------------------------------------------------------------

static void draw_mouse_dialog();     // defined below
static void draw_joystick_dialog();  // defined below

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
        for (int i = 0; i < 3; ++i) {
            if (ImGui::MenuItem(fc_names[i])) {
                flight_control = i;
                if (i == FC_MOUSE) {
                    mouse_pending = true;
                    for (auto &g : g_presets)
                        if (g.name == "Mouse" && g.current == "no_mouse") { g.current = "glide_mouse"; break; }
                } else if (i == FC_JOYSTICK) {
                    joy_pending = true;
                    load_joystick_staging();
                } else {
                    mouse_pending = true;
                }
                g_dirty = true;
            }
        }
        ImGui::EndPopup();
    }
    if (flight_control != FC_MOUSE) ImGui::BeginDisabled();
    if (ImGui::Button("Mouse Settings", ImVec2(-1, 0))) { load_mouse_staging(); mouse_dialog_open = true; }
    if (flight_control != FC_MOUSE) ImGui::EndDisabled();
    if (flight_control != FC_JOYSTICK) ImGui::BeginDisabled();
    if (ImGui::Button("Joystick Settings", ImVec2(-1, 0))) { load_joystick_staging(); joy_dialog_open = true; }
    if (flight_control != FC_JOYSTICK) ImGui::EndDisabled();
    if (ImGui::Checkbox("Rendered Crosshair", &rendered_crosshair))
        g_dirty = true;
    ImGui::EndChild();
    if (mouse_dialog_open) ImGui::OpenPopup("Mouse Settings");
    if (joy_dialog_open) ImGui::OpenPopup("Joystick Settings");
    draw_mouse_dialog();
    draw_joystick_dialog();
}

// Mouse Settings dialog: the always-active mouse vars + flight-mode picker.
// Accept stages (red buttons); Close discards; Save on the main screen commits.
static void draw_mouse_dialog() {
    if (ImGui::BeginPopupModal("Mouse Settings", &mouse_dialog_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        // Flight-mode picker (from the Mouse group's options).
        for (auto &g : g_presets) {
            if (g.name != "Mouse" || g.options.empty()) continue;
            int sel = (mouse_stg.mode_idx < 0) ? 0 : mouse_stg.mode_idx;
            if (sel >= (int)g.options.size()) sel = 0;
            ImGui::Text("Flight mode"); ImGui::SameLine();
            ImGui::SetNextItemWidth(180);
            std::vector<const char*> items;
            for (auto &o : g.options) items.push_back(o.desc.empty() ? o.name.c_str() : o.desc.c_str());
            if (ImGui::Combo("##mmode", &sel, items.data(), (int)items.size())) {
                mouse_stg.mode_idx = sel;
                g.current = g.options[sel].name;
                mouse_pending = true;
                g_dirty = true;
            }
            break;
        }
        ImGui::Separator();
        ImGui::Checkbox("Mouse cursor on pan camera", &mouse_stg.cam_pancam);
        ImGui::Checkbox("Mouse cursor on target pan camera", &mouse_stg.cam_pantgt);
        ImGui::Checkbox("Mouse cursor on chase camera", &mouse_stg.cam_chasecam);
        ImGui::Separator();
        ImGui::Text("Warp zone"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##wz", mouse_stg.warp_zone, sizeof(mouse_stg.warp_zone), ImGuiInputTextFlags_CharsDecimal))
            { mouse_pending = true; g_dirty = true; }
        ImGui::Text("Exponent"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##exp", mouse_stg.exponent, sizeof(mouse_stg.exponent), ImGuiInputTextFlags_CharsDecimal))
            { mouse_pending = true; g_dirty = true; }
        ImGui::Text("Deadband"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##db", mouse_stg.deadband, sizeof(mouse_stg.deadband), ImGuiInputTextFlags_CharsDecimal))
            { mouse_pending = true; g_dirty = true; }
        ImGui::Separator();
        if (ImGui::Button("Accept")) { mouse_pending = true; g_dirty = true; mouse_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { mouse_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// Joystick Settings dialog: per-axis live sliders + bind + invert, Auto Deadband,
// Force Feedback + device, No-Joystick placeholder. Accept stages; Close discards.
static void draw_joystick_dialog() {
    if (ImGui::BeginPopupModal("Joystick Settings", &joy_dialog_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Deflect an axis to identify it, then bind it to a flight role.");
        ImGui::Separator();
        SDL_UpdateJoysticks();   // refresh axis state (also for a just-hotplugged joystick)
        int n = 0; SDL_JoystickID *ids = SDL_GetJoysticks(&n);
        // Persistent open handles: open each device once and reuse across frames so a
        // hotplugged joystick's axis state stays live (open/close per frame is unstable).
        static std::map<SDL_JoystickID, SDL_Joystick*> g_joy_open;
        std::set<SDL_JoystickID> present;
        for (int i = 0; i < n; ++i) present.insert(ids[i]);
        // close any handle whose device was removed
        for (auto it = g_joy_open.begin(); it != g_joy_open.end();) {
            if (present.find(it->first) == present.end()) { SDL_CloseJoystick(it->second); it = g_joy_open.erase(it); }
            else ++it;
        }
        if (n <= 0 || !ids) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No Joystick Detected");
        } else {
            static const char *dd[] = { "none", "x", "y", "z", "throttle" };
            for (int i = 0; i < n; ++i) {
                SDL_Joystick *joy = g_joy_open[ids[i]];
                if (!joy) { joy = SDL_OpenJoystick(ids[i]); g_joy_open[ids[i]] = joy; }
                if (!joy) continue;
                const char *nm = SDL_GetJoystickName(joy);
                int na = SDL_GetNumJoystickAxes(joy);
                for (int a = 0; a < na; ++a) {
                    float val = SDL_GetJoystickAxis(joy, a) / 32768.0f;
                    int role = role_for_axis(i, a);
                    char id[32]; snprintf(id, sizeof(id), "##joy%d_a%d", i, a);
                    ImGui::Text("%s A%d", nm ? nm : "Joy", a); ImGui::SameLine();
                    ImGui::SetNextItemWidth(110);
                    ImGui::BeginDisabled();
                    ImGui::SliderFloat((std::string(id) + "live").c_str(), &val, -1.0f, 1.0f, "");
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    int cur = role < 0 ? 0 : role + 1;
                    ImGui::SetNextItemWidth(90);
                    if (ImGui::Combo((std::string(id) + "bind").c_str(), &cur, dd, 5)) {
                        int nr = cur - 1;
                        for (int r = 0; r < JR_COUNT; ++r)
                            if (joy_bind_stick[r] == i && joy_bind_axis[r] == a) { joy_bind_stick[r] = -1; joy_bind_axis[r] = -1; }
                        if (nr >= 0) { joy_bind_stick[nr] = i; joy_bind_axis[nr] = a; }
                        joy_pending = true; g_dirty = true;
                    }
                    ImGui::SameLine();
                    bool has = role >= 0;
                    if (!has) ImGui::BeginDisabled();
                    bool inv = has ? joy_bind_inv[role] : false;
                    if (ImGui::Checkbox((std::string(id) + "inv").c_str(), &inv) && has) {
                        joy_bind_inv[role] = inv; joy_pending = true; g_dirty = true;
                    }
                    if (!has) ImGui::EndDisabled();
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("Deadband"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##jdb", joy_deadband, sizeof(joy_deadband), ImGuiInputTextFlags_CharsDecimal))
            { joy_pending = true; g_dirty = true; }
        ImGui::SameLine();
        if (ImGui::Button("Auto Deadband")) { joy_auto_sampling = true; joy_auto_timer = 0.0f; joy_auto_max = 0.0f; }
        if (joy_auto_sampling) {
            float m = joy_sample_bound_axes();
            if (m > joy_auto_max) joy_auto_max = m;
            joy_auto_timer += ImGui::GetIO().DeltaTime;
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Sampling... keep the stick still");
            if (joy_auto_timer >= 1.0f) {
                float db = joy_auto_max + 0.05f;
                if (db < 0.0f) db = 0.0f; else if (db > 0.5f) db = 0.5f;
                snprintf(joy_deadband, sizeof(joy_deadband), "%.3f", db);
                joy_auto_sampling = false; joy_pending = true; g_dirty = true;
            }
        }
        if (ImGui::Checkbox("Force feedback", &joy_ffb)) { joy_pending = true; g_dirty = true; }
        if (ImGui::Checkbox("Mouse cursor in flight", &joy_mouse_cursor)) { joy_pending = true; g_dirty = true; }
        ImGui::Text("FFB device"); ImGui::SameLine(); ImGui::SetNextItemWidth(50);
        if (ImGui::InputText("##jff", joy_ff_device, sizeof(joy_ff_device), ImGuiInputTextFlags_CharsDecimal))
            { joy_pending = true; g_dirty = true; }
        ImGui::Separator();
        if (ImGui::Button("Reset Axis")) {
            for (int r = 0; r < JR_COUNT; ++r) { joy_bind_stick[r] = -1; joy_bind_axis[r] = -1; joy_bind_inv[r] = false; }
            joy_pending = true; g_dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Accept")) { joy_pending = true; g_dirty = true; joy_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { joy_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
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
        if (g.name == "Resolution" || g.name == "Mouse") continue;
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
