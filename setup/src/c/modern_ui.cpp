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

// Font picker. The font choice drives high_quality_font[_computer] + font_antialias (bitmap vs
// vector), and the bitmap name/size drives graphics/font + graphics/basefont. The vector height
// uses the existing text_height_buf (font_point).
enum { FONT_AA_VEC = 0, FONT_VEC, FONT_HELVETICA, FONT_TIMES, FONT_FIXED };
static const char *font_type_names[] = { "Antialiased Vector", "Vector", "Helvetica", "Times", "Fixed" };
static int sel_font_type = FONT_AA_VEC;
// Selected bitmap size index per family (index into the family's sizes array; -1 = suggested).
static int sel_bitmap_size[3] = { -1, -1, -1 };   // 0=Helvetica 1=Times 2=Fixed

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

// Bitmap font families and their fixed sizes. Order matches the freeglut faces.
// Each size is the bitmap's pixel height (glutBitmapHeight). The hud.cpp getFont()
// reads graphics/font + graphics/basefont as these names; the Font class uses the
// closest bitmap <= the requested height once the auto-select fix is in.
struct BitmapFamily {
    const char *name;
    int  n;                 // number of sizes
    const char **size_names;
    const int *px;
};
static const char *helv_sizes[] = { "10", "12", "18" };
static const int   helv_px[]    = { 14, 16, 23 };
static const char *times_sizes[] = { "10", "24" };
static const int   times_px[]   = { 14, 29 };
static const char *fixed_sizes[] = { "8x13", "9x15" };
static const int   fixed_px[]   = { 14, 16 };
static const BitmapFamily bitmap_families[] = {
    { "Helvetica", 3, helv_sizes, helv_px },
    { "Times", 2, times_sizes, times_px },
    { "Fixed", 2, fixed_sizes, fixed_px },
};

// The ideal font height (font_point) for the current resolution.
static int ideal_font_height() {
    return sel_res_h > 0 ? (int)(0.0125 * sel_res_h + 2.5 + 0.5) : 16;
}

static int suggested_bitmap_size(int family);   // defined below

// Encode the current font picker state as the '#font' header line (e.g. 'aa_vec', 'vec',
// 'helvetica 12', 'times 24', 'fixed 9x15'). Persisted in the app-owned header so the picker
// restores across launches, independent of the (now-removed) Text preset group.
static std::string font_header_value() {
    if (sel_font_type == FONT_AA_VEC) return "aa_vec";
    if (sel_font_type == FONT_VEC) return "vec";
    int fam = sel_font_type - FONT_HELVETICA;
    const BitmapFamily &f = bitmap_families[fam];
    int sz = sel_bitmap_size[fam];
    if (sz < 0) sz = suggested_bitmap_size(fam);
    if (sz < 0 || sz >= f.n) sz = 0;
    return std::string(f.name) + " " + f.size_names[sz];
}

// Suggest the bitmap size (index) nearest the ideal vector height.
static int suggested_bitmap_size(int family) {
    const BitmapFamily &f = bitmap_families[family];
    int ideal = ideal_font_height();
    int best = 0, bestdiff = 1000000;
    for (int i = 0; i < f.n; i++) {
        int d = f.px[i] < ideal ? ideal - f.px[i] : f.px[i] - ideal;
        if (d < bestdiff) { bestdiff = d; best = i; }
    }
    return best;
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

// Bindings: staged copies of the <bind> elements (key/mouse/joystick), loaded from the
// model on dialog open, committed to the model on Accept. Display-only grouping into
// categories; conflict detection marks an input bound to 2+ different actions.
enum { BC_COCKPIT = 0, BC_MOVEMENT, BC_WEAPONS, BC_TARGETING, BC_COMMS, BC_SHIP, BC_MISC, BC_COUNT };
static const char *bind_cat_names[BC_COUNT] = { "Cockpit / Camera", "Movement / Flight", "Weapons / Combat",
    "Targeting", "Comms", "Ship / Systems", "Interface / Misc" };
struct BindRow {
    std::string command, device;   // device: "key" | "mouse" | "joystick"
    std::string key, button, device_idx, modifier;
    std::string hat_idx;           // joystick digital-hatswitch index ("0" for hat-0)
    std::string hat_dir;           // joystick digital-hatswitch direction ("up" etc.), else ""
    int  category = BC_MISC;
    bool remove = false, dirty = false, conflict = false;
};
static std::vector<BindRow> bindrows;
static int bind_cat = BC_COCKPIT;
static bool bind_dialog_open = false;
static bool bind_pending = false;
static bool bind_capturing = false;
static bool bind_capture_requested = false;
static std::string bind_capture_cmd;
static bool cap_valid = false;
static bool cap_open = false;
static std::string cap_device, cap_idx, cap_btn, cap_key, cap_modifier;
static std::string cap_hat_idx, cap_hat_dir;   // hat capture (empty unless a hat was captured)
static int bind_rebind_row = -1;               // index into bindrows being re-mapped, or -1 = add-new

// SDL3 joystick events report the device's instance ID (which), but the config uses the
// joystick INDEX (the `joystick="N"` attribute). Convert an instance ID to its index in
// SDL_GetJoysticks(); -1 if the device is no longer present.
static int joystick_index_of(SDL_JoystickID which) {
    int n = 0; SDL_JoystickID *ids = SDL_GetJoysticks(&n);
    int idx = -1;
    for (int i = 0; i < n; i++) if (ids[i] == which) { idx = i; break; }
    SDL_free(ids);
    return idx;
}

// Map an SDL_HAT_* bitmask to its direction name; returns NULL for CENTERED (=0).
static const char *hat_value_name(Uint8 v) {
    switch (v) {
        case SDL_HAT_UP:       return "up";
        case SDL_HAT_RIGHT:    return "right";
        case SDL_HAT_DOWN:     return "down";
        case SDL_HAT_LEFT:     return "left";
        case SDL_HAT_RIGHTUP:  return "rightup";
        case SDL_HAT_RIGHTDOWN: return "rightdown";
        case SDL_HAT_LEFTUP:   return "leftup";
        case SDL_HAT_LEFTDOWN: return "leftdown";
        default:               return NULL;   // CENTERED / unknown
    }
}
static ImVec2 cap_frame_pos, cap_frame_size;

static bool starts_with(const std::string &s, const char *prefix) {
    size_t n = strlen(prefix);
    return s.size() >= n && s.compare(0, n, prefix) == 0;
}
static int bind_category(const std::string &cmd) {
    if (starts_with(cmd, "Cockpit::")) return BC_COCKPIT;
    if (starts_with(cmd, "Thrust") || starts_with(cmd, "Flight:") || starts_with(cmd, "Joystick:")
        || cmd == "ToggleWarpDrive") return BC_MOVEMENT;
    if (cmd == "FireKey" || cmd == "MissileKey" || cmd == "WeapSelKey" || cmd == "MisSelKey"
        || cmd == "ReverseWeapSelKey" || cmd == "ReverseMisSelKey" || cmd == "SheltonKey" || cmd == "ECMKey"
        || starts_with(cmd, "Turret") || cmd == "SwitchCombatMode" || cmd == "ABKey"
        || cmd == "AccelKey" || cmd == "DecelKey") return BC_WEAPONS;
    if (cmd.find("Target") != std::string::npos) return BC_TARGETING;
    if (starts_with(cmd, "Comm") || cmd == "TextMessage" || cmd == "ChangeCommStatus") return BC_COMMS;
    if (cmd == "CloakKey" || cmd == "EjectKey" || cmd == "EjectCargoKey" || cmd == "JumpKey"
        || cmd == "DockKey" || cmd == "UnDockKey" || cmd == "PauseKey" || cmd == "StartKey" || cmd == "StopKey"
        || cmd == "Respawn" || cmd == "SuicideKey" || cmd == "SwitchSecured" || cmd == "SwitchWebcam"
        || cmd == "SetVelocityRefKey" || cmd == "SetVelocityNullKey" || cmd == "RequestClearenceKey") return BC_SHIP;
    return BC_MISC;
}
static std::string bind_input(const BindRow &r) {
    if (r.device == "key") return "k:" + r.key + ":" + r.modifier;
    if (r.device == "joystick" && !r.hat_dir.empty())
        return r.device + ":hat:" + r.device_idx + ":" + r.hat_idx + ":" + r.hat_dir;
    return r.device + ":" + r.device_idx + ":" + r.button;
}
static void compute_bind_conflicts(void) {
    std::map<std::string, std::vector<std::string>> input_cmds;
    for (auto &r : bindrows) {
        if (r.remove) continue;
        std::string in = bind_input(r);
        if (std::find(input_cmds[in].begin(), input_cmds[in].end(), r.command) == input_cmds[in].end())
            input_cmds[in].push_back(r.command);
    }
    for (auto &r : bindrows) {
        r.conflict = false;
        if (r.remove) continue;
        r.conflict = input_cmds[bind_input(r)].size() > 1;
    }
}

// Read the <bind> elements from the live model into the staged bindrows (the source of
// truth is the model, NOT the shipped bindings.xml - the file only seeded the config).
static void load_bindings_staging() {
    bindrows.clear();
    const std::vector<vs05cfg::Element> *b = vs05cfg::bindings(g_model);
    if (b) {
        for (auto &e : *b) {
            if (e.name != "bind") continue;
            BindRow r;
            r.command  = vs05cfg::attr(e, "command");
            r.category = bind_category(r.command);
            if (vs05cfg::has_attr(e, "key")) {
                r.device = "key"; r.key = vs05cfg::attr(e, "key");
                std::string m = vs05cfg::attr(e, "modifier"); r.modifier = m.empty() ? "none" : m;
            } else if (vs05cfg::has_attr(e, "mouse")) {
                r.device = "mouse"; r.button = vs05cfg::attr(e, "button"); r.device_idx = vs05cfg::attr(e, "mouse");
            } else if (vs05cfg::has_attr(e, "joystick")) {
                r.device = "joystick"; r.device_idx = vs05cfg::attr(e, "joystick");
                if (vs05cfg::has_attr(e, "digital-hatswitch")) {
                    r.hat_dir = vs05cfg::attr(e, "direction");   // hatswitch direction, not a button
                    r.hat_idx = vs05cfg::attr(e, "digital-hatswitch");
                } else {
                    r.button = vs05cfg::attr(e, "button");
                }
            }
            bindrows.push_back(r);
        }
    }
    compute_bind_conflicts();
}

// Reset the model's binds to the shipped bindings.xml defaults, then reload the staging so
// the dialog reflects the reset. Called by the Reset Binds button (immediately, before Save).
static void reset_binds_to_default() {
    std::string bf = shipped_bindings_file();
    if (!bf.empty()) vs05cfg::apply_bindings_file(g_model, bf);
    load_bindings_staging();
    g_dirty = true;
}

// Commit the staged bindrows to the model: replace every <bind> under <bindings> with the
// non-removed staged rows. Called on Accept. Matches apply_bindings_file's replace semantics.
static void apply_bindrows_to_model() {
    vs05cfg::Element *b = vs05cfg::bindings_elem(g_model);
    // drop all existing <bind> children (keep non-bind like <axis>)
    for (size_t i = 0; i < b->children.size();) {
        if (b->children[i].name == "bind") b->children.erase(b->children.begin() + i);
        else i++;
    }
    for (auto &r : bindrows) {
        if (r.remove) continue;
        std::vector<std::pair<std::string,std::string>> attrs;
        if (r.device == "key") {
            attrs.push_back(std::make_pair("key", r.key));
            attrs.push_back(std::make_pair("modifier", r.modifier));
        } else if (r.device == "mouse") {
            attrs.push_back(std::make_pair("mouse", r.device_idx));
            attrs.push_back(std::make_pair("button", r.button));
            attrs.push_back(std::make_pair("modifier", "none"));
        } else {
            attrs.push_back(std::make_pair("joystick", r.device_idx));
            if (!r.hat_dir.empty()) {
                attrs.push_back(std::make_pair("digital-hatswitch", r.hat_idx.empty() ? "0" : r.hat_idx));
                attrs.push_back(std::make_pair("direction", r.hat_dir));
            } else {
                attrs.push_back(std::make_pair("button", r.button));
            }
            attrs.push_back(std::make_pair("modifier", "none"));
        }
        attrs.push_back(std::make_pair("command", r.command));
        vs05cfg::add_bind(g_model, attrs);
    }
}

// Mark every bind row sharing the currently-captured input (other actions) as removed, so the
// new capture becomes the exclusive owner. Only rows that are not the target action are cleared.
static void clear_other_binds(void) {
    std::string in;
    if (cap_device == "key") in = "k:" + cap_key + ":" + cap_modifier;
    else if (cap_device == "joystick" && !cap_hat_dir.empty())
        in = "joystick:hat:" + cap_idx + ":" + cap_hat_idx + ":" + cap_hat_dir;
    else in = cap_device + ":" + cap_idx + ":" + cap_btn;
    if (in.empty()) return;
    for (auto &r : bindrows) {
        if (r.remove || r.command == bind_capture_cmd) continue;
        if (bind_input(r) == in) { r.remove = true; r.dirty = true; }
    }
    compute_bind_conflicts();
}

// Actions already bound to the currently-captured input (excluding the target action), for the
// capture-window warning. Empty when the captured input is free.
static std::vector<std::string> capture_conflicts(void) {
    std::string in;
    if (cap_device == "key") in = "k:" + cap_key + ":" + cap_modifier;
    else if (cap_device == "joystick" && !cap_hat_dir.empty())
        in = "joystick:hat:" + cap_idx + ":" + cap_hat_idx + ":" + cap_hat_dir;
    else in = cap_device + ":" + cap_idx + ":" + cap_btn;
    if (in.empty()) return {};
    std::vector<std::string> out;
    for (auto &r : bindrows) {
        if (r.remove || r.command == bind_capture_cmd) continue;
        if (bind_input(r) == in)
            if (std::find(out.begin(), out.end(), r.command) == out.end())
                out.push_back(r.command);
    }
    return out;
}

// Map an SDL3 keycode to the config's key-name convention (lowercase letters, special names).
static const char *config_key_name(SDL_Keycode key) {
    switch (key) {
        case SDLK_RETURN: return "return";
        case SDLK_ESCAPE: return "esc";
        case SDLK_TAB: return "tab";
        case SDLK_SPACE: return "space";
        case SDLK_BACKSPACE: return "backspace";
        case SDLK_LEFT: return "cursor-left";
        case SDLK_RIGHT: return "cursor-right";
        case SDLK_UP: return "cursor-up";
        case SDLK_DOWN: return "cursor-down";
        case SDLK_HOME: return "cursor-home";
        case SDLK_END: return "cursor-end";
        case SDLK_PAGEUP: return "cursor-pageup";
        case SDLK_PAGEDOWN: return "cursor-pagedown";
        case SDLK_INSERT: return "cursor-insert";
        case SDLK_DELETE: return "cursor-delete";
        case SDLK_CAPSLOCK: return "capslock";
        case SDLK_SCROLLLOCK: return "scrollock";
        case SDLK_NUMLOCKCLEAR: return "keypad-numlock";
        default: {
            if (key >= SDLK_F1 && key <= SDLK_F15) {
                static char b[16]; snprintf(b, sizeof(b), "function-%d", (int)(key - SDLK_F1) + 1);
                return b;
            }
            if (key >= ' ' && key <= '~') {
                static char b[2]; b[0] = (char)key; b[1] = 0; return b;
            }
            return NULL;
        }
    }
}

// Commit a captured input to a new bind row for the current action.
static void accept_capture(void) {
    BindRow nr;
    nr.command = bind_capture_cmd; nr.category = bind_category(bind_capture_cmd);
    nr.remove = false; nr.dirty = true; nr.conflict = false;
    nr.device = cap_device;
    if (cap_device == "key") { nr.key = cap_key; nr.modifier = cap_modifier; }
    else if (cap_device == "joystick" && !cap_hat_dir.empty()) {
        nr.hat_idx = cap_hat_idx; nr.hat_dir = cap_hat_dir; nr.device_idx = cap_idx;
    }
    else { nr.button = cap_btn; nr.device_idx = cap_idx; }
    if (bind_rebind_row >= 0 && bind_rebind_row < (int)bindrows.size()) {
        // Re-mapping an existing bind in place: keep the command, replace the input.
        BindRow &r = bindrows[bind_rebind_row];
        r.key.clear(); r.button.clear(); r.device_idx.clear(); r.modifier = "none";
        r.hat_idx.clear(); r.hat_dir.clear();
        r.device = nr.device; r.key = nr.key; r.button = nr.button;
        r.device_idx = nr.device_idx; r.modifier = nr.modifier;
        r.hat_idx = nr.hat_idx; r.hat_dir = nr.hat_dir;
        r.dirty = true;
    } else {
        bindrows.push_back(nr);
    }
    bind_rebind_row = -1;
    cap_valid = false;
    compute_bind_conflicts();
}

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
        if (g.name == "Resolution" || g.name == "Mouse" || g.name == "Text") continue;
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
    // Font choice -> high_quality_font[_computer] + font_antialias + the bitmap name for
    // hud.cpp getFont()/getFontHeight(). Vector keeps font_point; bitmap uses the chosen size.
    if (sel_font_type == FONT_AA_VEC) {
        model_set_var("graphics", "high_quality_font", "false");
        model_set_var("graphics", "high_quality_font_computer", "false");
        model_set_var("graphics", "font_antialias", "true");
    } else if (sel_font_type == FONT_VEC) {
        model_set_var("graphics", "high_quality_font", "false");
        model_set_var("graphics", "high_quality_font_computer", "false");
        model_set_var("graphics", "font_antialias", "false");
    } else {
        int fam = sel_font_type - FONT_HELVETICA;
        const BitmapFamily &f = bitmap_families[fam];
        int sz = sel_bitmap_size[fam];
        if (sz < 0) sz = suggested_bitmap_size(fam);
        if (sz < 0 || sz >= f.n) sz = 0;
        model_set_var("graphics", "high_quality_font", "true");
        model_set_var("graphics", "high_quality_font_computer", "true");
        model_set_var("graphics", "font_antialias", "false");
        // Name = lowercase family + size the hud.cpp getFont() expects (helvetica12, times24,
        // fixed13/fixed15 - the Fixed sizes are labeled 8x13/9x15 but the config uses the px).
        std::string nm;
        if (fam == 0) nm = "helvetica" + std::string(f.size_names[sz]);
        else if (fam == 1) nm = "times" + std::string(f.size_names[sz]);
        else nm = "fixed" + std::string(f.size_names[sz]).substr(2);   // "8x13"->"13", "9x15"->"15"
        model_set_var("graphics", "font", nm);
        model_set_var("graphics", "basefont", nm);
    }
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
    // Seed the bindings from the shipped bindings.xml ONLY when the model has no <bind>
    // elements yet (fresh model / reset). This carries the <bind mouse=0 player=0> entry
    // that makes mouse flight work. Once present, the model (the live config) is the source
    // of truth - the Bindings dialog edits it directly and re-applying here would clobber
    // those edits on every save.
    const std::vector<vs05cfg::Element> *blk = vs05cfg::bindings(g_model);
    bool has_binds = false;
    if (blk) for (auto &e : *blk) if (e.name == "bind") { has_binds = true; break; }
    if (!has_binds) {
        std::string bf = shipped_bindings_file();
        if (!bf.empty()) vs05cfg::apply_bindings_file(g_model, bf);
    }
}

static void restore_preset_selections(const std::string &modern_cfg);   // defined below (after init)
static void restore_font_state();                                       // defined below (after init)

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
    restore_font_state();
    rendered_crosshair = model_get_var("graphics", "draw_rendered_crosshairs") != "false";
    load_joystick_staging();   // restore joystick settings (deadband, ffb, axes) on load
    refresh_screen_aspect_text();
    return g_loaded;
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

// The #font header value parsed from vs-modern.config (empty if none). Restores the picker's
// explicit type/size choice across launches.
static std::string font_header_parsed;

// Apply a '#font <value>' header line (e.g. 'aa_vec', 'helvetica 12') to the picker state.
static void apply_font_header_value(const std::string &v) {
    sel_font_type = FONT_AA_VEC;
    for (int i = 0; i < 3; i++) sel_bitmap_size[i] = -1;
    if (v == "vec") { sel_font_type = FONT_VEC; return; }
    // Family keyword before the first space.
    size_t sp = v.find(' ');
    std::string famname = (sp == std::string::npos) ? v : v.substr(0, sp);
    std::string sizestr = (sp == std::string::npos) ? "" : v.substr(sp + 1);
    for (int fam = 0; fam < 3; fam++) {
        if (bitmap_families[fam].name == famname) {
            sel_font_type = FONT_HELVETICA + fam;
            const BitmapFamily &f = bitmap_families[fam];
            if (!sizestr.empty())
                for (int i = 0; i < f.n; i++)
                    if (std::string(f.size_names[i]) == sizestr) { sel_bitmap_size[fam] = i; break; }
            return;
        }
    }
}

// Restore the font picker state: prefer the app-header #font line, else derive from the model's
// high_quality_font / font_antialias / font vars. Defaults to AA Vector if nothing is set.
static void restore_font_state() {
    if (!font_header_parsed.empty()) { apply_font_header_value(font_header_parsed); return; }
    std::string hqf = model_get_var("graphics", "high_quality_font");
    std::string aa  = model_get_var("graphics", "font_antialias");
    std::string fn  = model_get_var("graphics", "font");
    sel_font_type = FONT_AA_VEC;
    for (int i = 0; i < 3; i++) sel_bitmap_size[i] = -1;
    if (hqf == "true") {
        int fam = -1;
        std::string base;
        if (fn.compare(0, 8, "helvetica") == 0) { fam = 0; base = fn.substr(8); }
        else if (fn.compare(0, 5, "times") == 0) { fam = 1; base = fn.substr(5); }
        else if (fn.compare(0, 5, "fixed") == 0) { fam = 2; base = fn.substr(5); }
        if (fam >= 0) {
            sel_font_type = FONT_HELVETICA + fam;
            const BitmapFamily &f = bitmap_families[fam];
            for (int i = 0; i < f.n; i++) {
                std::string want = (fam == 2) ? ("8x" + base) : base;
                if (std::string(f.size_names[i]) == want) { sel_bitmap_size[fam] = i; break; }
            }
        }
    } else {
        sel_font_type = (aa == "true") ? FONT_AA_VEC : FONT_VEC;
    }
}

// Build the app-owned #set header (active preset per group) to persist the
// selections inside vs-modern.config (the engine ignores the comment).
// Restore the persisted #set selections from vs-modern.config (overriding the
// shipped defaults). The engine ignores the comment header.
static void restore_preset_selections(const std::string &modern_cfg) {
    FILE *f = fopen(modern_cfg.c_str(), "r");
    if (!f) return;
    font_header_parsed.clear();   // no #font line => derive from the model
    char line[1024];
    bool in_presets = false;
    while (fgets(line, sizeof(line), f)) {
        char *t = line;
        while (*t == ' ' || *t == '\t') t++;
        if (strncmp(t, "<!-- vssetup presets", 20) == 0) { in_presets = true; continue; }
        if (strncmp(t, "-->", 3) == 0) { in_presets = false; continue; }
        if (!in_presets) continue;
        if (strncmp(t, "#font ", 6) == 0) {
            std::string v = t + 6;
            size_t e = v.find_first_of(" \t\r\n");
            v = v.substr(0, e == std::string::npos ? std::string::npos : e);
            // Keep the rest (e.g. the bitmap size) after the family keyword.
            std::string full = t + 6;
            size_t e2 = full.find_first_of("\r\n");
            full = full.substr(0, e2 == std::string::npos ? std::string::npos : e2);
            // trim trailing spaces
            while (!full.empty() && (full.back() == ' ' || full.back() == '\t')) full.pop_back();
            font_header_parsed = full;
            continue;
        }
        if (strncmp(t, "#baseaspect ", 12) == 0) {
            std::string v = t + 12;
            int idx = atoi(v.c_str());
            if (idx >= 0 && idx < (int)(sizeof(aspect_opts) / sizeof(aspect_opts[0]))) {
                sel_base_aspect = idx;
                base_aspect_text = aspect_opts[idx];
            }
            continue;
        }
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
    // Persist the font picker state (type + bitmap size) so it restores on next load.
    h += "#font " + font_header_value() + "\n";
    // Persist the base aspect ratio so it restores on next load.
    h += "#baseaspect " + std::to_string(sel_base_aspect) + "\n";
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
    std::string fp = model_get_var("graphics", "font_point");
    if (!fp.empty()) snprintf(text_height_buf, sizeof(text_height_buf), "%s", fp.c_str());
    restore_font_state();
    sel_base_aspect = 0;
    base_aspect_text = aspect_opts[0];
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
    g_dirty = true;   // a mode switch is a pending change until Saved
}

// Patch ONLY the first-line mode switch in vs-modern.config to reflect the current
// mode, without touching the rest of the modern config (so a classic save doesn't
// rewrite the modern model). The engine reads this switch to decide which config to
// load. Called on Save; the mode switch becomes real only then.
void write_mode_switch() {
    if (g_asset.empty()) return;
    std::string mf = modern_config_file();
    FILE *f = fopen(mf.c_str(), "r");
    if (!f) return;   // no modern config to patch (pure classic)
    // read the whole file
    std::string content;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) content.append(buf, n);
    fclose(f);
    // replace the first line (the mode switch) with the current mode's switch
    size_t nl = content.find('\n');
    std::string new_switch = "<!-- vssetup:mode=" + std::string(g_mode == MODE_MODERN ? "modern" : "classic") + " -->\n";
    std::string out = (nl == std::string::npos) ? new_switch : new_switch + content.substr(nl + 1);
    FILE *w = fopen(mf.c_str(), "w");
    if (w) { fputs(out.c_str(), w); fclose(w); }
}

std::string engine_config_file() {
    return modern_config_file();
}

// ---------------------------------------------------------------------------
// Event handling (capture + joystick) — extended as dialogs are added
// ---------------------------------------------------------------------------

void handle_event(const SDL_Event *event) {
    // Binding capture: while the Capture Binding modal is open and capturing, swallow the
    // next key / mouse-click-in-frame / joystick button and record it (before ImGui sees it).
    if (bind_capturing && cap_open && !bind_capture_cmd.empty()) {
        if (event->type == SDL_EVENT_KEY_DOWN) {
            SDL_Keycode kc = event->key.key;
            if (kc == SDLK_LSHIFT || kc == SDLK_RSHIFT || kc == SDLK_LCTRL || kc == SDLK_RCTRL
                || kc == SDLK_LALT || kc == SDLK_RALT || kc == SDLK_LGUI || kc == SDLK_RGUI)
                return;   // a modifier alone isn't a binding; keep waiting
            const char *nm = config_key_name(kc);
            if (nm) {
                std::string k = nm;
                if ((event->key.mod & SDL_KMOD_SHIFT) && k.size() == 1 && k[0] >= 'a' && k[0] <= 'z')
                    k[0] = (char)(k[0] - 'a' + 'A');   // shift is encoded in the key, not a modifier
                cap_device = "key"; cap_key = k;
                cap_modifier = (event->key.mod & SDL_KMOD_CTRL) ? "ctrl"
                             : ((event->key.mod & SDL_KMOD_ALT) ? "alt" : "none");
                cap_valid = true;
            }
            return;   // swallow the key
        }
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            bool inframe = event->button.x >= cap_frame_pos.x && event->button.x <= cap_frame_pos.x + cap_frame_size.x
                        && event->button.y >= cap_frame_pos.y && event->button.y <= cap_frame_pos.y + cap_frame_size.y;
            if (inframe) {
                cap_device = "mouse"; cap_btn = std::to_string(event->button.button);
                cap_idx = "0"; cap_modifier = "none"; cap_valid = true;
                return;   // captured; swallow
            }
            // outside the frame: fall through so the mouse works normally
        }
        if (event->type == SDL_EVENT_JOYSTICK_BUTTON_DOWN) {
            int idx = joystick_index_of(event->jbutton.which);
            cap_device = "joystick"; cap_btn = std::to_string(event->jbutton.button);
            cap_idx = (idx < 0) ? std::to_string(event->jbutton.which) : std::to_string(idx);
            cap_modifier = "none"; cap_valid = true;
            return;   // captured; swallow
        }
        if (event->type == SDL_EVENT_JOYSTICK_HAT_MOTION) {
            const char *dir = hat_value_name(event->jhat.value);
            if (dir) {   // ignore CENTERED (a hat release, not a capturable state)
                int idx = joystick_index_of(event->jhat.which);
                cap_device = "joystick";
                cap_idx = (idx < 0) ? std::to_string(event->jhat.which) : std::to_string(idx);
                cap_hat_idx = std::to_string(event->jhat.hat);
                cap_hat_dir = dir;
                cap_btn.clear();
                cap_valid = true;
            }
            return;   // swallow (centered or not - hat motion is consumed while capturing)
        }
        // everything else passes through
    }

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
static void draw_bindings_dialog();  // defined below
static void draw_capture_dialog();   // defined below

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
    ImGui::Text("Font"); ImGui::SameLine();
    if (ImGui::Button(font_type_names[sel_font_type])) ImGui::OpenPopup("##pick_font");
    if (ImGui::BeginPopup("##pick_font")) {
        for (size_t i = 0; i < sizeof(font_type_names) / sizeof(font_type_names[0]); ++i)
            if (ImGui::MenuItem(font_type_names[i])) { sel_font_type = (int)i; g_dirty = true; }
        ImGui::EndPopup();
    }
    if (sel_font_type == FONT_AA_VEC || sel_font_type == FONT_VEC) {
        ImGui::SameLine(); ImGui::Text("Text Height"); ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        if (ImGui::InputText("##textheight", text_height_buf, sizeof(text_height_buf), ImGuiInputTextFlags_CharsDecimal))
            g_dirty = true;
    } else {
        int fam = sel_font_type - FONT_HELVETICA;
        const BitmapFamily &f = bitmap_families[fam];
        int sz = sel_bitmap_size[fam];
        if (sz < 0) sz = suggested_bitmap_size(fam);
        if (sz < 0 || sz >= f.n) sz = 0;
        ImGui::SameLine();
        if (ImGui::Button(f.size_names[sz])) ImGui::OpenPopup("##pick_bsize");
        if (ImGui::BeginPopup("##pick_bsize")) {
            for (int i = 0; i < f.n; i++) {
                bool sel = (i == sz);
                if (ImGui::Selectable(f.size_names[i], sel)) { sel_bitmap_size[fam] = i; g_dirty = true; }
            }
            ImGui::EndPopup();
        }
    }
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
    if (ImGui::Button("Bindings", ImVec2(-1, 0))) { load_bindings_staging(); bind_capture_cmd.clear(); bind_rebind_row = -1; bind_capturing = false; bind_dialog_open = true; }
    if (ImGui::Checkbox("Rendered Crosshair", &rendered_crosshair))
        g_dirty = true;
    ImGui::EndChild();
    if (mouse_dialog_open) ImGui::OpenPopup("Mouse Settings");
    if (joy_dialog_open) ImGui::OpenPopup("Joystick Settings");
    if (bind_dialog_open) ImGui::OpenPopup("Bindings");
    draw_mouse_dialog();
    draw_joystick_dialog();
    draw_bindings_dialog();
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

// Capture Binding modal: waits for a key, a click in the mouse frame, or a joystick
// button, then shows Accept/Retry/Cancel. Esc is capturable (matches modern app).
static void draw_capture_dialog(void) {
    if (bind_capture_requested) { ImGui::OpenPopup("Capture Binding"); bind_capture_requested = false; }
    cap_open = ImGui::BeginPopupModal("Capture Binding", &bind_capturing, ImGuiWindowFlags_AlwaysAutoResize);
    if (cap_open) {
        ImGui::Text("Capture binding for %s", bind_capture_cmd.c_str());
        ImGui::Text("Press a key, joystick button, or move a hat (anywhere),");
        ImGui::Text("or click the frame below for a mouse button:");
        ImGui::BeginChild("capframe", ImVec2(280, 56), ImGuiChildFlags_Borders);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "    click here for a mouse bind");
        cap_frame_pos = ImGui::GetWindowPos(); cap_frame_size = ImGui::GetWindowSize();
        ImGui::EndChild();
        if (cap_valid) {
            std::string disp;
            if (cap_device == "key")
                disp = cap_key + (cap_modifier == "none" ? "" : " " + cap_modifier);
            else if (cap_device == "joystick" && !cap_hat_dir.empty())
                disp = "joystick hat-" + cap_hat_idx + " " + cap_hat_dir;
            else
                disp = cap_device + " " + cap_btn;
            ImGui::Text("Captured: %s", disp.c_str());
            auto conflicts = capture_conflicts();
            if (!conflicts.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
                                   "! This input is already bound to:");
                for (auto &c : conflicts)
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "    - %s", c.c_str());
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
                                   "  Accepting will share the input (not recommended).");
                if (ImGui::Button("Clear other Bind")) clear_other_binds();
            }
        }
        ImGui::Separator();
        if (cap_valid && ImGui::Button("Accept")) { accept_capture(); bind_pending = true; g_dirty = true; bind_capturing = false; ImGui::CloseCurrentPopup(); }
        if (cap_valid) ImGui::SameLine();
        if (ImGui::Button("Retry")) { cap_valid = false; cap_hat_dir.clear(); cap_hat_idx.clear(); }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { bind_capturing = false; cap_valid = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// Bindings dialog: a vertical column of category buttons on the left; the binds in the
// selected category in a 5-column table on the right (Action | Input | Binding | Add | Delete).
static void draw_bindings_dialog(void) {
    ImGui::SetNextWindowSize(ImVec2(820, 540), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Bindings", &bind_dialog_open)) {
        ImGui::BeginChild("catcol", ImVec2(190, 440), ImGuiChildFlags_Borders);
        for (int c = 0; c < BC_COUNT; ++c)
            if (ImGui::Selectable(bind_cat_names[c], bind_cat == c, 0, ImVec2(180, 0))) bind_cat = c;
        ImGui::EndChild();
        ImGui::SameLine();
        float tbl_w = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("bindlist", ImVec2(tbl_w, 440), ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        if (bindrows.empty()) {
            ImGui::TextWrapped("No bindings loaded (model has no <bindings> block).");
        } else {
            std::vector<std::string> cmds;
            for (auto &r : bindrows)
                if (std::find(cmds.begin(), cmds.end(), r.command) == cmds.end()) cmds.push_back(r.command);
            std::sort(cmds.begin(), cmds.end());
            if (ImGui::BeginTable("bindtable", 5,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Action");
                ImGui::TableSetupColumn("Input");
                ImGui::TableSetupColumn("Binding");
                ImGui::TableSetupColumn("Add");
                ImGui::TableSetupColumn("Delete");
                ImGui::TableHeadersRow();
                int row = 0;
                for (auto &cmd : cmds) {
                    if (bind_category(cmd) != bind_cat) continue;
                    bool any = false;
                    for (size_t i = 0; i < bindrows.size(); ++i) {
                        BindRow &r = bindrows[i];
                        if (r.command != cmd || r.remove) continue;
                        any = true;
                        std::string dev = (r.device == "key") ? "keyboard" : r.device;
                        std::string disp;
                        if (r.device == "key")
                            disp = r.key + (r.modifier == "none" ? "" : " " + r.modifier);
                        else if (r.device == "joystick" && !r.hat_dir.empty())
                            disp = "hat-" + r.hat_idx + " " + r.hat_dir;
                        else
                            disp = "btn " + r.button;
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(cmd.c_str());
                        ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(dev.c_str());
                        ImGui::TableSetColumnIndex(2);
                        char bid[24]; snprintf(bid, sizeof(bid), "##bind%d", row);
                        if (r.conflict) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                        if (ImGui::Button((disp + bid).c_str())) {
                            bind_capture_cmd = cmd; bind_rebind_row = (int)i;
                            bind_capturing = true; cap_valid = false; cap_hat_dir.clear(); cap_hat_idx.clear();
                            bind_capture_requested = true;
                        }
                        if (r.conflict) ImGui::PopStyleColor();
                        ImGui::TableSetColumnIndex(3);
                        char aid[24]; snprintf(aid, sizeof(aid), "+##%d", row);
                        if (ImGui::Button(aid)) { bind_capture_cmd = cmd; bind_rebind_row = -1; bind_capturing = true; cap_valid = false; cap_hat_dir.clear(); cap_hat_idx.clear(); bind_capture_requested = true; }
                        ImGui::TableSetColumnIndex(4);
                        char xid[24]; snprintf(xid, sizeof(xid), "X##%d", row);
                        if (ImGui::Button(xid)) { r.remove = true; r.dirty = true; compute_bind_conflicts(); }
                        row++;
                    }
                    if (!any) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(cmd.c_str());
                        ImGui::TableSetColumnIndex(1); ImGui::Text("(none)");
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TableSetColumnIndex(3);
                        char aid[24]; snprintf(aid, sizeof(aid), "+##%d", row);
                        if (ImGui::Button(aid)) { bind_capture_cmd = cmd; bind_rebind_row = -1; bind_capturing = true; cap_valid = false; cap_hat_dir.clear(); cap_hat_idx.clear(); bind_capture_requested = true; }
                        row++;
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
        ImGui::Separator();
        if (ImGui::Button("Reset Binds")) reset_binds_to_default();
        ImGui::SameLine();
        if (ImGui::Button("Accept")) { bind_pending = true; g_dirty = true; apply_bindrows_to_model(); bind_capturing = false; bind_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { bind_capturing = false; bind_dialog_open = false; ImGui::CloseCurrentPopup(); }
        draw_capture_dialog();
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
        if (g.name == "Resolution" || g.name == "Mouse" || g.name == "Text") continue;
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
