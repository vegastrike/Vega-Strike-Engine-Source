// config_screen.cpp — in-game configuration screen for vs-settings-ng.
//
// Port of the vs-05 modern UI frames into the engine, reading/writing the
// engine's Configuration object directly. Drawn inside the in-game ImGui
// overlay (Alt+C -> DrawConfigOverlay -> this).
//
// Scope: this iteration focuses on the editing frames. Applying changes live to
// the running game (e.g. changing resolution) and writing back to config files
// are deferred until the config write-out work.

#include "config_screen.h"

#include "configuration/configuration.h"
#include "gldrv/winsys.h"
#include "gldrv/gl_init.h"
#include "universe.h"
#include "vegadisk/vsfilesystem.h"
#include "root_generic/vs_globals.h"
#include "config_xml.h"
#include "src/vs_exit.h"
#include "cmd/music.h"
#include "src/audiolib.h"
#include <boost/json.hpp>
#include <boost/filesystem.hpp>
#include <imgui.h>
#include "libraries/gui/gui.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <cstdio>
#include <cstdlib>

namespace fs = boost::filesystem;

namespace vs_settings_ng {

namespace {

// ---------------------------------------------------------------------------
// Display frame state (mirrors vs-05 modern_ui.cpp)
// ---------------------------------------------------------------------------
int  sel_monitor = 0;
SDL_DisplayID sel_display_id = 0;
int  sel_res_w = 0, sel_res_h = 0;
std::string monitor_text, resolution_text;
char text_height_buf[16] = "16";
static const char *aspect_opts[] = { "16:10 (1.6)", "16:9 (1.78)", "4:3 (1.33)", "5:4 (1.25)", "1:1 (1.0)" };
static const float aspect_vals[] = { 1.6f, 16.0f / 9.0f, 4.0f / 3.0f, 1.25f, 1.0f };
int  sel_base_aspect = 0;      // aspect_opts[0] = 16:10 (default base aspect)
std::string base_aspect_text = aspect_opts[0];
int  sel_screen_aspect = -1;      // -1 = auto (W/H)
std::string screen_aspect_text;
bool display_inited = false;

bool rendered_crosshair = true;
bool cfg_full_screen = true;

// Mouse / Joystick dialog open flags.
static bool mouse_dialog_open = false;
static bool joy_dialog_open = false;

// Forward declarations (defined below; used by draw_display_frame).
static void load_mouse_staging();
static void load_joystick_staging();
static void load_bindings_staging();
static void draw_bindings_dialog();
static void handle_bindings_event(const SDL_Event *event);
static boost::json::value read_config_value(const std::string &path);
// Read a config path from a specific Configuration (used to diff a changed value
// against the datadir default to detect a revert).
static boost::json::value read_config_value_from(const vega_config::Configuration &c, const std::string &path);

// Bindings dialog state (declared here so draw_display_frame can open it).
static bool bind_dialog_open = false;
static bool bind_capturing = false;
static int  bind_rebind_row = -1;
static std::string bind_capture_cmd;

enum { FC_KEYBOARD = 0, FC_MOUSE = 1, FC_JOYSTICK = 2 };
int  flight_control = FC_KEYBOARD;
static const char *fc_names[] = { "Keyboard", "Mouse", "Joystick" };

// Selectable fonts: index 0 is always the engine's embedded Roboto; the rest are
// the .ttf files found in the data fonts/ directory (e.g. Saira variants).
// graphics.font stores the value to persist: "Roboto" (sentinel) or a .ttf filename.
static std::vector<std::string> s_font_list;
static std::vector<std::string> s_font_values;   // parallel: graphics.font value for each entry
static int sel_font = 0;

// Scan the data fonts/ directory for .ttf files and (re)build the selectable font
// list. Roboto (engine-provided) is always first.
static void refresh_font_list() {
    s_font_list.clear();
    s_font_values.clear();
    s_font_list.push_back("Roboto");
    s_font_values.push_back("Roboto");
    const fs::path fonts_dir = VSFileSystem::datadir + "/fonts";
    if (fs::exists(fonts_dir) && fs::is_directory(fonts_dir)) {
        std::vector<std::string> ttf;
        for (fs::directory_iterator it(fonts_dir), end; it != end; ++it) {
            if (it->path().extension() == ".ttf") {
                ttf.push_back(it->path().filename().string());
            }
        }
        std::sort(ttf.begin(), ttf.end());
        for (const auto &f : ttf) {
            s_font_list.push_back(f);
            s_font_values.push_back(f);
        }
    }
    if (sel_font < 0 || sel_font >= static_cast<int>(s_font_list.size())) {
        sel_font = 0;
    }
}

// Set sel_font from a persisted graphics.font value ("Roboto" or a .ttf filename).
static void select_font_from_value(const std::string &value) {
    if (s_font_values.empty()) {
        refresh_font_list();
    }
    sel_font = 0;
    for (size_t i = 0; i < s_font_values.size(); ++i) {
        if (s_font_values[i] == value) {
            sel_font = static_cast<int>(i);
            return;
        }
    }
}

bool dirty = false;

// Set on Save when a shader config path changed; shows a "restart required"
// notice (shaders are written out but not hot-applied).
static bool shader_restart_notice = false;

// The set of config paths changed (dirty). Populated by the apply_*_to_config
// functions on Save; consumed by write_out_dirty() (the single write-out entry
// point, deferred to Layer 3) so it knows exactly what to persist.
static std::set<std::string> g_dirty_paths;

// Record a changed config path and mark the config dirty.
static void mark_dirty(const std::string &path) {
    dirty = true;
    g_dirty_paths.insert(path);
}

// Set an input-axis role and mark it dirty ONLY if it actually changed, so a Save
// writes just the changed axes (a sparse bindings.json overlay) rather than the
// whole axes tree on every Save.
static void mark_axis_dirty_if_changed(const std::string &role, const std::string &source, int joystick, int axis, bool inverse) {
    auto &ar = configuration().axes[role];
    if (ar.source != source || ar.joystick != joystick || ar.axis != axis || ar.inverse != inverse) {
        ar.source = source;
        ar.joystick = joystick;
        ar.axis = axis;
        ar.inverse = inverse;
        mark_dirty("bindings.axes." + role);
    }
}

// Compare two ActionBindings (keyboard/mouse/joystick/hat). Used to detect which
// commands actually changed so a Save writes only those (sparse bindings overlay).
static bool binding_equal(const vega_config::Configuration::ActionBinding &a,
                          const vega_config::Configuration::ActionBinding &b) {
    return a.key == b.key && a.modifier == b.modifier && a.button == b.button
        && a.joystick == b.joystick && a.is_mouse == b.is_mouse
        && a.hatswitch == b.hatswitch && a.direction == b.direction;
}
static bool action_bindings_equal(const vega_config::Configuration::ActionBindings &a,
                                  const vega_config::Configuration::ActionBindings &b) {
    if (a.keyboard.size() != b.keyboard.size() || a.mouse.size() != b.mouse.size()
        || a.joystick.size() != b.joystick.size() || a.hat.size() != b.hat.size()) return false;
    for (size_t i = 0; i < a.keyboard.size(); ++i) if (!binding_equal(a.keyboard[i], b.keyboard[i])) return false;
    for (size_t i = 0; i < a.mouse.size(); ++i) if (!binding_equal(a.mouse[i], b.mouse[i])) return false;
    for (size_t i = 0; i < a.joystick.size(); ++i) if (!binding_equal(a.joystick[i], b.joystick[i])) return false;
    for (size_t i = 0; i < a.hat.size(); ++i) if (!binding_equal(a.hat[i], b.hat[i])) return false;
    return true;
}

// Compare two AxisRoles (source/joystick/axis/inverse).
static bool axis_role_equal(const vega_config::Configuration::AxisRole &a,
                            const vega_config::Configuration::AxisRole &b) {
    return a.source == b.source && a.joystick == b.joystick
        && a.axis == b.axis && a.inverse == b.inverse;
}

// Build a Configuration holding ONLY the datadir defaults (no homedir overlay),
// mirroring the default-load sequence in vsfilesystem.cpp. Used to detect when a
// changed value has been reverted to the shipped default so the overlay can drop
// it instead of accumulating it forever.
static vega_config::Configuration load_default_config() {
    vega_config::Configuration d;
    for (const std::string &name : {"bindings.json", "theme.json", "engine.json", "config.json"}) {
        d.load_config(boost::filesystem::path(VSFileSystem::datadir + "/" + name));
    }
    return d;
}

// Remove a dotted path (e.g. "graphics.fog") from a nested boost::json::object.
// No-op if any segment is missing; prunes emptied intermediate objects.
static void remove_json_path(boost::json::object &root, const std::string &path) {
    size_t dot = path.find('.');
    if (dot == std::string::npos) { root.erase(path); return; }
    std::string head = path.substr(0, dot);
    std::string rest = path.substr(dot + 1);
    auto it = root.find(head);
    if (it == root.end() || !it->value().is_object()) return;
    remove_json_path(it->value().as_object(), rest);
    if (it->value().as_object().empty()) root.erase(it);
}

// ---------------------------------------------------------------------------
// Display-frame helpers (from vs-05)
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

// The ideal font height (font_point) for the current resolution. Font size scales
// linearly with the horizontal resolution: 14 at 1368x768, 27 at 2560x1440.
// Linear fit through those two points: font = (13/1192) * width - 0.919, rounded.
static int ideal_font_height() {
    if (sel_res_w <= 0) {
        return 16;
    }
    return static_cast<int>(std::lround((13.0 / 1192.0) * sel_res_w - 0.919));
}

static void prefill_text_height() {
    if (sel_res_h > 0) {
        int fp = ideal_font_height();
        snprintf(text_height_buf, sizeof(text_height_buf), "%d", fp);
    }
}

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

// The base-computer viewport (base_max_width/height) from the base aspect ratio.
static void compute_base_max(int &w, int &h) {
    w = h = 0;
    if (sel_res_w <= 0 || sel_res_h <= 0) return;
    float A = aspect_vals[sel_base_aspect];
    if (A >= (float)sel_res_w / sel_res_h) { w = sel_res_w; h = (int)(sel_res_w / A); }
    else { h = sel_res_h; w = (int)(sel_res_h * A); }
}

// Derive sel_base_aspect from the stored base viewport (bases.max_width/height)
// so reopening the screen reflects the saved base aspect (default 16:10).
static void load_base_aspect_from_config() {
    const auto &g = configuration().graphics;
    int bw = g.bases.max_width;
    int bh = g.bases.max_height;
    sel_base_aspect = 0;      // default = aspect_opts[0] (16:10)
    if (bw > 0 && bh > 0) {
        float ratio = (float)bw / (float)bh;
        int best = 0;
        float best_d = fabsf(ratio - aspect_vals[0]);
        for (size_t i = 1; i < sizeof(aspect_vals) / sizeof(aspect_vals[0]); ++i) {
            float d = fabsf(ratio - aspect_vals[i]);
            if (d < best_d) { best_d = d; best = (int)i; }
        }
        sel_base_aspect = best;
    }
    base_aspect_text = aspect_opts[sel_base_aspect];
}

// ---------------------------------------------------------------------------
// Configuration read/write (the vs-05 model_set_var/get_var, mapped to the
// engine's Configuration struct).
// ---------------------------------------------------------------------------

static vega_config::Configuration &cfg() {
    return configuration();
}

// Load the display-frame state from Configuration (on first open).
static void load_display_from_config() {
    const auto &g = configuration().graphics;
    sel_res_w = g.resolution_x;
    sel_res_h = g.resolution_y;
    rendered_crosshair = g.draw_rendered_crosshairs;
    cfg_full_screen = g.full_screen;
    int fp = (int)g.font_point_flt;
    snprintf(text_height_buf, sizeof(text_height_buf), "%d", fp);
    // screen (monitor) index.
    sel_monitor = g.screen;
    int nd = 0; SDL_DisplayID *ids = SDL_GetDisplays(&nd);
    if (nd < 1) nd = 1;
    if (sel_monitor >= nd) sel_monitor = 0;
    sel_display_id = ids ? ids[sel_monitor] : 0;
    const char *nm = sel_display_id ? SDL_GetDisplayName(sel_display_id) : NULL;
    monitor_text = std::to_string(sel_monitor) + "  " + (nm ? nm : "(unnamed)");
    if (ids) SDL_free(ids);
    // Select the configured font (Roboto or a .ttf filename) in the dropdown list.
    refresh_font_list();
    select_font_from_value(g.font);
    load_base_aspect_from_config();
    refresh_screen_aspect_text();
    resolution_text = std::to_string(sel_res_w) + "x" + std::to_string(sel_res_h);
    display_inited = true;
}

// Apply the display-frame state back to Configuration (Save).
static void apply_display_to_config() {
    auto &g = cfg().graphics;
    // Apply the resolution/fullscreen live to the running game (reuses the
    // windowed-mode resize cascade: SetWindowSize -> WINDOW_RESIZED ->
    // get_screen_measurements + Reshape). Respects the current windowed/fullscreen
    // choice.
    winsys_apply_resolution(sel_res_w, sel_res_h, cfg_full_screen);
    mark_dirty("graphics.resolution_x");
    mark_dirty("graphics.resolution_y");
    mark_dirty("graphics.full_screen");
    g.font_point_flt = (float)atoi(text_height_buf);
    mark_dirty("graphics.font_point");
    RequestImGuiFontSize(g.font_point_flt);   // live-rebuild the ImGui font atlas at the new size
    // Persist the selected font ("Roboto" sentinel or a .ttf filename); only hot-apply
    // a font change when the selection actually differs from the current font, so a
    // size-only Save does not force an atlas rebuild.
    if (sel_font >= 0 && sel_font < static_cast<int>(s_font_values.size())) {
        const std::string &new_font = s_font_values[sel_font];
        const bool font_changed = (new_font != g.font);
        g.font = new_font;
        mark_dirty("graphics.font");
        if (font_changed) {
            std::string font_path;
            if (new_font != "Roboto") {
                font_path = VSFileSystem::datadir + "/fonts/" + new_font;
            }
            RequestImGuiFont(font_path.c_str());   // live-reload the ImGui font atlas
        }
    }
    mark_dirty("graphics.font_antialias");
    g.aspect_flt = sel_screen_aspect >= 0 ? aspect_vals[sel_screen_aspect] : current_screen_aspect();
    mark_dirty("graphics.aspect");
    g.draw_rendered_crosshairs = rendered_crosshair;
    mark_dirty("graphics.draw_rendered_crosshairs");
    // Base viewport (graphics.bases.max_width/height) from the base aspect ratio.
    // compute_base_max() yields the largest viewport of the chosen aspect that fits
    // the selected resolution; the imgui bases render into this letterboxed viewport.
    {
        int bw = 0, bh = 0;
        compute_base_max(bw, bh);
        if (bw > 0 && bh > 0) {
            g.bases.max_width = bw;
            g.bases.max_height = bh;
            mark_dirty("graphics.bases.max_width");
            mark_dirty("graphics.bases.max_height");
        }
    }
    // screen (monitor) index.
    g.screen = sel_monitor;
    mark_dirty("graphics.screen");
}

// ---------------------------------------------------------------------------
// Display frame (full vs-05 version: monitor, resolution list, text height,
// screen aspect, base aspect, font picker)
// ---------------------------------------------------------------------------

void draw_display_frame() {
    if (!display_inited) {
        load_display_from_config();
        if (sel_res_h == 0) {
            int w = 0, h = 0;
            if (highest_monitor_resolution(sel_display_id, w, h)) { sel_res_w = w; sel_res_h = h; }
            if (sel_res_h > 0) resolution_text = std::to_string(sel_res_w) + "x" + std::to_string(sel_res_h);
        }
        refresh_screen_aspect_text();
    }

    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    float avail_w = ImGui::GetContentRegionAvail().x;
    float dpy_w = avail_w * 0.72f;
    float side_w = avail_w - dpy_w;

    ImGui::BeginChild("dpyframe", ImVec2(dpy_w, 6 * btn_h), ImGuiChildFlags_Borders);
    // Monitor selector.
    if (ImGui::Button("Monitor")) ImGui::OpenPopup("##pick_mon");
    ImGui::SameLine(); ImGui::TextUnformatted(monitor_text.c_str());
    if (ImGui::BeginPopup("##pick_mon")) {
        int nd = 0; SDL_DisplayID *ids = SDL_GetDisplays(&nd);
        for (int i = 0; i < nd; ++i) {
            const char *nm = SDL_GetDisplayName(ids[i]);
            char lbl[160]; snprintf(lbl, sizeof(lbl), "%d  %s", i, nm ? nm : "(unnamed)");
            if (ImGui::MenuItem(lbl)) {
                sel_monitor = i; sel_display_id = ids[i];
                monitor_text = lbl; dirty = true;
                const SDL_DisplayMode *cm = SDL_GetCurrentDisplayMode(ids[i]);
                if (cm) { sel_res_w = cm->w; sel_res_h = cm->h;
                    resolution_text = std::to_string(cm->w) + "x" + std::to_string(cm->h);
                    if (sel_screen_aspect < 0) refresh_screen_aspect_text(); }
            }
        }
        if (ids) SDL_free(ids);
        ImGui::EndPopup();
    }
    // Resolution selector (detected fullscreen modes, deduplicated). Each entry
    // also shows its aspect ratio (width/height).
    if (ImGui::Button("Resolution")) ImGui::OpenPopup("##pick_res");
    ImGui::SameLine(); ImGui::TextUnformatted(resolution_text.c_str());
    if (ImGui::BeginPopup("##pick_res")) {
        int cnt = 0; SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(sel_display_id, &cnt);
        if (modes) {
            std::vector<std::string> seen;
            for (int i = 0; i < cnt; ++i) {
                char lbl[40];
                float aspect = modes[i]->h > 0 ? static_cast<float>(modes[i]->w) / static_cast<float>(modes[i]->h) : 0.0f;
                snprintf(lbl, sizeof(lbl), "%dx%d (%.2f)", modes[i]->w, modes[i]->h, aspect);
                if (std::find(seen.begin(), seen.end(), lbl) != seen.end()) continue;
                seen.push_back(lbl);
                if (ImGui::MenuItem(lbl)) {
                    sel_res_w = modes[i]->w; sel_res_h = modes[i]->h;
                    resolution_text = lbl; prefill_text_height(); dirty = true;
                    if (sel_screen_aspect < 0) refresh_screen_aspect_text();
                }
            }
        }
        if (modes) SDL_free(modes);
        ImGui::EndPopup();
    }
    // Screen aspect.
    if (ImGui::Button("Screen Aspect")) ImGui::OpenPopup("##pick_sa");
    ImGui::SameLine(); ImGui::TextUnformatted(screen_aspect_text.c_str());
    if (ImGui::BeginPopup("##pick_sa")) {
        if (ImGui::MenuItem("Auto (W/H)")) { sel_screen_aspect = -1; refresh_screen_aspect_text(); dirty = true; }
        ImGui::Separator();
        for (size_t i = 0; i < sizeof(aspect_opts) / sizeof(aspect_opts[0]); ++i)
            if (ImGui::MenuItem(aspect_opts[i])) { sel_screen_aspect = (int)i; refresh_screen_aspect_text(); dirty = true; }
        ImGui::EndPopup();
    }
    // Font picker: Roboto (engine default) + .ttf fonts from the data fonts/ dir.
    if (s_font_list.empty()) {
        refresh_font_list();
    }
    ImGui::Text("Font"); ImGui::SameLine();
    if (ImGui::Button(s_font_list[sel_font].c_str())) ImGui::OpenPopup("##pick_font");
    if (ImGui::BeginPopup("##pick_font")) {
        for (size_t i = 0; i < s_font_list.size(); ++i)
            if (ImGui::MenuItem(s_font_list[i].c_str())) { sel_font = (int)i; dirty = true; }
        ImGui::EndPopup();
    }
    // Text Height input (font_point) — the size, independent of the font choice.
    ImGui::SameLine(); ImGui::Text("Text Height"); ImGui::SameLine();
    ImGui::SetNextItemWidth(60);
    if (ImGui::InputText("##textheight", text_height_buf, sizeof(text_height_buf), ImGuiInputTextFlags_CharsDecimal))
        dirty = true;
    // Base aspect ratio.
    if (ImGui::Button("Base Aspect Ratio")) ImGui::OpenPopup("##pick_asp");
    ImGui::SameLine(); ImGui::TextUnformatted(base_aspect_text.c_str());
    if (ImGui::BeginPopup("##pick_asp")) {
        for (size_t i = 0; i < sizeof(aspect_opts) / sizeof(aspect_opts[0]); ++i)
            if (ImGui::MenuItem(aspect_opts[i])) { sel_base_aspect = (int)i; base_aspect_text = aspect_opts[i]; dirty = true; }
        ImGui::EndPopup();
    }
    ImGui::EndChild();   // end dpyframe (left column)

    // Right column: Flight Control + Input buttons + Rendered Crosshair, side by
    // side with the monitor/resolution/display controls (as vs-05).
    ImGui::SameLine();
    ImGui::BeginChild("dpybtns", ImVec2(side_w, 6 * btn_h), ImGuiChildFlags_Borders);
    if (ImGui::Button(("Flight Control: " + std::string(fc_names[flight_control])).c_str(), ImVec2(-1, 0)))
        ImGui::OpenPopup("##flight");
    if (ImGui::BeginPopup("##flight")) {
        for (int i = 0; i < 3; ++i) {
            if (ImGui::MenuItem(fc_names[i])) {
                flight_control = i;
                if (i == FC_MOUSE) { load_mouse_staging(); }
                else if (i == FC_JOYSTICK) { load_joystick_staging(); }
                dirty = true;
            }
        }
        ImGui::EndPopup();
    }
    // Flight-control gating: only the active device's settings are enabled.
    if (flight_control != FC_MOUSE) ImGui::BeginDisabled();
    if (ImGui::Button("Mouse Settings", ImVec2(-1, 0))) { load_mouse_staging(); mouse_dialog_open = true; }
    if (flight_control != FC_MOUSE) ImGui::EndDisabled();
    if (flight_control != FC_JOYSTICK) ImGui::BeginDisabled();
    if (ImGui::Button("Joystick Settings", ImVec2(-1, 0))) { load_joystick_staging(); joy_dialog_open = true; }
    if (flight_control != FC_JOYSTICK) ImGui::EndDisabled();
    if (ImGui::Button("Bindings", ImVec2(-1, 0))) { load_bindings_staging(); bind_capture_cmd.clear(); bind_rebind_row = -1; bind_capturing = false; bind_dialog_open = true; }
    if (ImGui::Checkbox("Rendered Crosshair", &rendered_crosshair)) dirty = true;
    ImGui::EndChild();   // end dpybtns (right column)
}

// ---------------------------------------------------------------------------
// Flight Control + Mouse + Joystick (ported from vs-05 modern_ui.cpp)
// ---------------------------------------------------------------------------

// Mouse flight-mode names (the Mouse preset options in engine.json).
static const char *mouse_mode_names[] = { "glide_mouse", "inv_glide_mouse", "warp_mouse", "inv_warp_mouse", "no_mouse" };

// Mouse staging.
struct MouseStaging {
    bool cam_pancam = false, cam_pantgt = false, cam_chasecam = true;
    char warp_zone[8] = "200", exponent[8] = "1.5", deadband[8] = "0.05";
    int  mode_idx = 0;   // index into mouse_mode_names
};
static MouseStaging mouse_stg;

// Joystick flight roles (x/y/z/throttle).
static const char *joy_role_names[] = { "x", "y", "z", "throttle" };
static int  joy_bind_stick[4] = { 0, 0, 0, 0 };
static int  joy_bind_axis[4] = { -1, -1, -1, -1 };
static bool joy_bind_inv[4] = { false, false, false, false };
static char joy_deadband[8] = "0.05";
static bool joy_ffb = false;
static char joy_ff_device[8] = "0";
static bool joy_auto_sampling = false;
static float joy_auto_timer = 0.0f, joy_auto_max = 0.0f;

// Which flight role (if any) maps to this physical (stick, axis).
static int role_for_axis(int stick, int axis) {
    for (int r = 0; r < 4; ++r)
        if (joy_bind_stick[r] == stick && joy_bind_axis[r] == axis) return r;
    return -1;
}

// Sample the bound axes for Auto Deadband (max deflection over ~1s).
static float joy_sample_bound_axes() {
    float m = 0.0f;
    SDL_UpdateJoysticks();
    int n = 0; SDL_JoystickID *ids = SDL_GetJoysticks(&n);
    for (int r = 0; r < 4; ++r) {
        int s = joy_bind_stick[r];
        if (s < 0 || s >= n || joy_bind_axis[r] < 0) continue;
        SDL_Joystick *joy = SDL_OpenJoystick(ids[s]);
        if (!joy) continue;
        float v = SDL_GetJoystickAxis(joy, joy_bind_axis[r]) / 32768.0f;
        if (v < 0) v = -v;
        if (v > m) m = v;
        SDL_CloseJoystick(joy);
    }
    if (ids) SDL_free(ids);
    return m;
}

// Load mouse staging from Configuration.joystick.
static void load_mouse_staging() {
    const auto &j = configuration().joystick;
    mouse_stg.cam_pancam = j.mouse_cursor_pancam;
    mouse_stg.cam_pantgt = j.mouse_cursor_pantgt;
    mouse_stg.cam_chasecam = j.mouse_cursor_chasecam;
    snprintf(mouse_stg.warp_zone, sizeof(mouse_stg.warp_zone), "%d", j.warp_mouse_zone);
    snprintf(mouse_stg.exponent, sizeof(mouse_stg.exponent), "%.1f", j.mouse_exponent_flt);
    snprintf(mouse_stg.deadband, sizeof(mouse_stg.deadband), "%.2f", j.mouse_deadband_flt);
    // Load the persisted mouse mode (input.mouse_preset) into the picker.
    mouse_stg.mode_idx = 0;
    const std::string &mp = configuration().input.mouse_preset;
    for (int i = 0; i < 5; ++i)
        if (mouse_mode_names[i] == mp) { mouse_stg.mode_idx = i; break; }
}

// Load joystick staging from Configuration.joystick + input.axes.
static void load_joystick_staging() {
    const auto &j = configuration().joystick;
    snprintf(joy_deadband, sizeof(joy_deadband), "%.2f", j.deadband_flt);
    joy_ffb = j.force_feedback;
    snprintf(joy_ff_device, sizeof(joy_ff_device), "%d", j.ff_device);
    const auto &axes = configuration().axes;
    for (int r = 0; r < 4; ++r) {
        joy_bind_stick[r] = 0; joy_bind_axis[r] = -1; joy_bind_inv[r] = false;
        auto it = axes.find(joy_role_names[r]);
        if (it != axes.end() && it->second.axis >= 0) {
            joy_bind_stick[r] = it->second.joystick;
            joy_bind_axis[r] = it->second.axis;
            joy_bind_inv[r] = it->second.inverse;
        }
    }
}

// Apply flight-control exclusivity to Configuration.joystick.
static void apply_flight_to_config() {
    auto &j = cfg().joystick;
    // The triple-state device switch (input.device) drives flight-control mode.
    cfg().input.device = (flight_control == FC_MOUSE) ? "mouse"
                       : (flight_control == FC_JOYSTICK) ? "joystick" : "keyboard";
    mark_dirty("input.device");
    if (flight_control == FC_MOUSE) {
        j.force_use_of_joystick = false;
        j.warp_mouse = true;
    } else if (flight_control == FC_JOYSTICK) {
        j.force_use_of_joystick = true;
    } else {
        j.force_use_of_joystick = false;
        j.warp_mouse = false;
    }
    mark_dirty("input.joystick.force_use_of_joystick");
    mark_dirty("input.joystick.warp_mouse");
    // Route the x/y flight axes to the mouse (MOUSE_JOYSTICK) only when Mouse
    // flight control is selected. The engine's config_xml axis router keys off
    // axes.x/y.source; without this, selecting Mouse silently does nothing.
    // For Keyboard/Joystick modes, restore source to joystick (preserving any
    // existing axis/joystick numbers so we don't clobber joystick config).
    auto &axes = cfg().axes;
    if (flight_control == FC_MOUSE) {
        // Mark x/y as mouse-driven.  We deliberately do NOT overwrite the stored
        // axis/joystick numbers here -- bindKeys() forces the mouse x/y physical
        // axes (0/1) at bind time, so writing them would clobber the joystick
        // numbers and lose them on the next switch back to Joystick.  The inverse
        // flag is only what Mouse still needs from the role entry.
        mark_axis_dirty_if_changed("x", "mouse", axes["x"].joystick, axes["x"].axis, cfg().mouse.inverse_x);
        mark_axis_dirty_if_changed("y", "mouse", axes["y"].joystick, axes["y"].axis, cfg().mouse.inverse_y);
    } else {
        for (const char *role : {"x", "y"}) {
            auto it = axes.find(role);
            if (it != axes.end() && it->second.source == "mouse")
                mark_axis_dirty_if_changed(role, "joystick", it->second.joystick, it->second.axis, it->second.inverse);
        }
    }
}

// Apply mouse staging to Configuration.joystick.
static void apply_mouse_to_config() {
    auto &j = cfg().joystick;
    j.mouse_cursor_pancam = mouse_stg.cam_pancam;
    j.mouse_cursor_pantgt = mouse_stg.cam_pantgt;
    j.mouse_cursor_chasecam = mouse_stg.cam_chasecam;
    j.warp_mouse_zone = atoi(mouse_stg.warp_zone);
    j.mouse_exponent_flt = (float)atof(mouse_stg.exponent);
    j.mouse_deadband_flt = (float)atof(mouse_stg.deadband);
    mark_dirty("input.joystick.mouse_cursor_pancam");
    mark_dirty("input.joystick.mouse_cursor_pantgt");
    mark_dirty("input.joystick.mouse_cursor_chasecam");
    mark_dirty("input.joystick.warp_mouse_zone");
    mark_dirty("input.joystick.mouse_exponent");
    mark_dirty("input.joystick.mouse_deadband");
    // Apply the full mouse-mode preset (glide/inv_glide/warp/inv_warp/no_mouse):
    // the vars (mouse_cursor/warp_mouse/reverse_mouse_spr/sensitivity) + the
    // x/y axis routing with the per-mode y-axis inversion. Only meaningful when
    // Mouse is the active flight device.
    if (flight_control != FC_MOUSE) return;
    const char *mode = mouse_mode_names[mouse_stg.mode_idx];
    bool warp = (std::string(mode) == "warp_mouse" || std::string(mode) == "inv_warp_mouse");
    bool glide = (std::string(mode) == "glide_mouse" || std::string(mode) == "inv_glide_mouse");
    // Preset vars (defaults: mouse off, not inverted).
    j.mouse_cursor = glide;                 // cursor shown in glide, hidden in warp/no
    j.reverse_mouse_spr = (std::string(mode) == "inv_glide_mouse");
    j.warp_mouse = warp;
    j.mouse_sensitivity_flt = warp ? 120.0f : 40.0f;
    mark_dirty("input.joystick.mouse_cursor");
    mark_dirty("input.joystick.reverse_mouse_spr");
    mark_dirty("input.joystick.warp_mouse");
    mark_dirty("input.joystick.mouse_sensitivity");
    // Axis routing + y-inversion come from the mouse preset (glide/warp vs
    // inv_glide/inv_warp), matching the vs-05 convention: the non-inverted
    // modes (glide_mouse, warp_mouse) invert y (top-down screen y needs the
    // flip); the "inv_" modes do not. bindKeys() reads the mouse y-inversion
    // from cfg.mouse.inverse_y (not axes.y.inverse), so set and persist that.
    const bool invert_y = (std::string(mode).rfind("inv_", 0) == std::string::npos);
    mark_axis_dirty_if_changed("x", "mouse", cfg().axes["x"].joystick, 0, false);
    mark_axis_dirty_if_changed("y", "mouse", cfg().axes["y"].joystick, 1, invert_y);
    cfg().mouse.inverse_y = invert_y;
    mark_dirty("input.mouse.inverse_y");
}

// Apply joystick staging to Configuration.joystick + input.axes.
static void apply_joystick_to_config() {
    if (flight_control != FC_JOYSTICK) return;
    auto &j = cfg().joystick;
    j.deadband_flt = (float)atof(joy_deadband);
    j.force_feedback = joy_ffb;
    j.ff_device = atoi(joy_ff_device);
    // Joystick mode never shows a mouse cursor in flight (mouse_cursor is only
    // meaningful for mouse flight, where the glide preset sets it). Force it off
    // so switching from mouse back to joystick clears the cursor.
    j.mouse_cursor = false;
    mark_dirty("input.joystick.deadband");
    mark_dirty("input.joystick.force_feedback");
    mark_dirty("input.joystick.ff_device");
    mark_dirty("input.joystick.mouse_cursor");
    // Write the x/y/z/throttle axes. An unbound role is kept with axis=-1 (the
    // engine's "unbound" sentinel) rather than erased, so parseAxes' overlay
    // merge preserves the unbind on restart (an absent key would fall back to
    // the datadir default and re-bind it). Only roles that actually changed are
    // marked dirty (sparse bindings.json overlay).
    for (int r = 0; r < 4; ++r) {
        const char *role = joy_role_names[r];
        cfg().axes[role];   // ensures the role exists (even if unbound)
        if (joy_bind_axis[r] < 0) {
            mark_axis_dirty_if_changed(role, "joystick", 0, -1, false);
        } else {
            mark_axis_dirty_if_changed(role, "joystick", joy_bind_stick[r], joy_bind_axis[r], joy_bind_inv[r]);
        }
    }
}

// Write the accumulated dirty config paths out to the user config files.
// Set a dotted path ("graphics.fog") into a nested boost::json::object.
static void json_set_path(boost::json::object &root, const std::string &path, const boost::json::value &val) {
    size_t dot = path.find('.');
    if (dot == std::string::npos) { root[path] = val; return; }
    std::string head = path.substr(0, dot);
    std::string rest = path.substr(dot + 1);
    if (!root.contains(head) || !root[head].is_object())
        root[head] = boost::json::object();
    json_set_path(root[head].as_object(), rest, val);
}

// Deep-merge src into dst (src's values win). Objects merge recursively; all
// other types are replaced. Used to overlay the dirty paths onto the existing
// user overlay so a Save does not drop previously-saved overrides.
static void json_merge(boost::json::value &dst, const boost::json::value &src) {
    if (!src.is_object() || !dst.is_object()) { dst = src; return; }
    auto &d = dst.as_object();
    for (const auto &kv : src.as_object()) {
        if (d.contains(kv.key()) && d[kv.key()].is_object() && kv.value().is_object()) {
            json_merge(d[kv.key()], kv.value());
        } else {
            d[kv.key()] = kv.value();
        }
    }
}

// Read an existing user overlay file (if any) into an object, else empty.
static boost::json::object read_existing_overlay(const std::string &path) {
    boost::json::object obj;
    std::ifstream in(path);
    if (in) {
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        try {
            boost::json::value v = boost::json::parse(text);
            if (v.is_object()) obj = v.as_object();
        } catch (...) { /* corrupt/partial — start fresh */ }
    }
    return obj;
}

// Write the accumulated dirty config paths out to the user config files
// (VSFileSystem::homedir/config.json + bindings.json). Single write-out entry point.
static void write_out_dirty() {
    if (g_dirty_paths.empty()) return;
    boost::json::object config_out;
    // A config path that was changed but now equals the shipped (datadir) default
    // is a REVERT: it must be cleared from the overlay (so the default applies),
    // not re-written. The default reference is built from the datadir files only.
    const vega_config::Configuration dflt = load_default_config();
    std::set<std::string> reverted;
    for (const auto &path : g_dirty_paths) {
        // Bindings paths (whole, or per-command/per-axis) are handled by the
        // bindings write below, not config.json.
        if (path == "bindings.actions" || path.rfind("bindings.actions.", 0) == 0
            || path == "bindings.axes" || path.rfind("bindings.axes.", 0) == 0) {
            continue;
        }
        if (path.rfind("preset.", 0) == 0) {
            // preset selector: config.json -> preset.<cat>
            std::string cat = path.substr(7);
            auto it = configuration().preset.find(cat);
            if (it == configuration().preset.end()) continue;
            auto dit = dflt.preset.find(cat);
            if (dit != dflt.preset.end() && dit->second == it->second) {
                reverted.insert(path);   // selector reverted to default -> clear
                continue;
            }
            auto &preset_obj = config_out["preset"].is_object() ? config_out["preset"].as_object()
                               : (config_out["preset"] = boost::json::object()).as_object();
            preset_obj[cat] = it->second;
            continue;
        }
        boost::json::value v = read_config_value(path);
        if (v.is_null()) continue;
        // If the current value equals the datadir default, it was reverted -> clear.
        if (v == read_config_value_from(dflt, path)) {
            reverted.insert(path);
            continue;
        }
        json_set_path(config_out, path, v);
    }

    // Write config.json overlay. Merge the changed values onto the existing
    // overlay, and first clear any paths that reverted to the datadir default so
    // the overlay stays a true sparse set of differences (matching config.json).
    if (!config_out.empty() || !reverted.empty()) {
        fs::create_directories(VSFileSystem::homedir);
        const std::string path = VSFileSystem::homedir + "/config.json";
        boost::json::object existing = read_existing_overlay(path);
        for (const auto &p : reverted) remove_json_path(existing, p);
        boost::json::value merged = std::move(existing);
        json_merge(merged, boost::json::value(std::move(config_out)));
        std::ofstream out(path);
        out << boost::json::serialize(merged) << "\n";
        fprintf(stderr, "[vs-settings-ng] wrote config overlay to %s\n", path.c_str());
    }
    // Write bindings.json overlay (only the CHANGED bindings, merged onto the
    // existing overlay so a Save writes just the changes — matching config.json).
    // Dirty paths: "bindings.actions.<cmd>"/"bindings.actions" (all actions) and
    // "bindings.axes.<role>"/"bindings.axes" (all axes). The apply_* fns mark
    // granular paths, so a device/axis or single-binding change no longer dumps
    // the whole 125-action map (and doesn't add "hat":[] to untouched actions).
    {
        bool write_bindings = false;
        for (const auto &path : g_dirty_paths) {
            if (path == "bindings.actions" || path.rfind("bindings.actions.", 0) == 0
                || path == "bindings.axes" || path.rfind("bindings.axes.", 0) == 0) {
                write_bindings = true;
                break;
            }
        }
        if (write_bindings) {
            const std::string bpath = VSFileSystem::homedir + "/bindings.json";
            boost::json::object merged = read_existing_overlay(bpath);
            boost::json::object merged_actions = merged["actions"].is_object()
                    ? merged["actions"].as_object() : boost::json::object();
            boost::json::object merged_axes = merged["axes"].is_object()
                    ? merged["axes"].as_object() : boost::json::object();
            // Serialize one command's ActionBindings (keyboard/mouse/joystick/hat).
            auto serialize_action = [](const vega_config::Configuration::ActionBindings &ab) {
                boost::json::object o;
                o["keyboard"] = boost::json::array();
                o["mouse"] = boost::json::array();
                o["joystick"] = boost::json::array();
                o["hat"] = boost::json::array();
                for (const auto &b : ab.keyboard) {
                    boost::json::object kb; kb["key"] = b.key; kb["modifier"] = b.modifier;
                    o["keyboard"].as_array().push_back(kb);
                }
                for (const auto &b : ab.mouse) {
                    boost::json::object mb; mb["button"] = b.button; mb["modifier"] = b.modifier;
                    o["mouse"].as_array().push_back(mb);
                }
                for (const auto &b : ab.joystick) {
                    boost::json::object jb; jb["joystick"] = b.joystick; jb["button"] = b.button; jb["modifier"] = b.modifier;
                    o["joystick"].as_array().push_back(jb);
                }
                for (const auto &b : ab.hat) {
                    boost::json::object hb; hb["joystick"] = b.joystick; hb["hat"] = b.hatswitch; hb["direction"] = b.direction;
                    o["hat"].as_array().push_back(hb);
                }
                return o;
            };
            // Actions to write: all if "bindings.actions" dirty, else the named ones.
            std::vector<std::string> action_names;
            const bool all_actions = g_dirty_paths.count("bindings.actions") != 0;
            if (all_actions) {
                for (const auto &kv : configuration().actions) action_names.push_back(kv.first);
            } else {
                for (const auto &path : g_dirty_paths)
                    if (path.rfind("bindings.actions.", 0) == 0)
                        action_names.push_back(path.substr(std::string("bindings.actions.").size()));
            }
            for (const auto &name : action_names) {
                auto it = configuration().actions.find(name);
                if (it == configuration().actions.end()) continue;
                auto dit = dflt.actions.find(name);
                const vega_config::Configuration::ActionBindings empty_ab;
                const auto &def = (dit != dflt.actions.end()) ? dit->second : empty_ab;
                if (action_bindings_equal(it->second, def)) {
                    merged_actions.erase(name);   // reverted to the shipped default -> clear
                } else {
                    merged_actions[name] = serialize_action(it->second);
                }
            }
            // Axes to write: all if "bindings.axes" dirty, else the named ones.
            std::vector<std::string> axis_names;
            const bool all_axes = g_dirty_paths.count("bindings.axes") != 0;
            if (all_axes) {
                for (const auto &kv : configuration().axes) axis_names.push_back(kv.first);
            } else {
                for (const auto &path : g_dirty_paths)
                    if (path.rfind("bindings.axes.", 0) == 0)
                        axis_names.push_back(path.substr(std::string("bindings.axes.").size()));
            }
            for (const auto &name : axis_names) {
                auto it = configuration().axes.find(name);
                if (it == configuration().axes.end()) continue;
                auto dit = dflt.axes.find(name);
                const vega_config::Configuration::AxisRole empty_role;
                const auto &def = (dit != dflt.axes.end()) ? dit->second : empty_role;
                if (axis_role_equal(it->second, def)) {
                    merged_axes.erase(name);   // reverted to the shipped default -> clear
                } else {
                    const auto &ar = it->second;
                    boost::json::object aobj;
                    aobj["source"] = ar.source; aobj["joystick"] = ar.joystick;
                    aobj["axis"] = ar.axis; aobj["inverse"] = ar.inverse;
                    merged_axes[name] = aobj;
                }
            }
            if (!merged_actions.empty()) merged["actions"] = merged_actions;
            if (!merged_axes.empty()) merged["axes"] = merged_axes;
            fs::create_directories(VSFileSystem::homedir);
            std::ofstream out(bpath);
            out << boost::json::serialize(merged) << "\n";
            fprintf(stderr, "[vs-settings-ng] wrote bindings overlay to %s\n", bpath.c_str());
        }
    }
    g_dirty_paths.clear();
}

// Re-initialize the runtime from the in-memory configuration() after an in-game
// settings Save. The game historically assumed the config never changes in-game:
// config->runtime copies (g_game feature flags, gl_options, GL viewport, legacy
// font metrics, audio/music volume) ran once at bootstrap and went stale on an
// in-game change. This re-binds them so the change takes effect without a restart.
// A brief re-orient pause is expected (and acceptable). See gldrv/gl_init.h.
static void reinit_from_saved_config() {
    // Graphics: g_game feature flags + gl_options + GL viewport (reuses initfov
    // and reapply_gl_options, the same copy code as bootstrap).
    GFXReinitConfig();
    // Audio/volume: re-apply the live volume functions from the new config.
    AUDReapplyConfig();
    Music::SetVolume(configuration().audio.music_volume_flt, -1, false);
}

// Mouse Settings dialog.
static void draw_mouse_dialog() {
    if (!mouse_dialog_open) return;
    ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Mouse Settings", &mouse_dialog_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        // Flight mode (Mouse preset).
        ImGui::Text("Flight mode"); ImGui::SameLine(); ImGui::SetNextItemWidth(180);
        if (ImGui::Combo("##mmode", &mouse_stg.mode_idx, mouse_mode_names, 5)) {
            cfg().input.mouse_preset = mouse_mode_names[mouse_stg.mode_idx];
            mark_dirty("input.mouse_preset");
            dirty = true;
        }
        ImGui::Separator();
        ImGui::Checkbox("Mouse cursor on pan camera", &mouse_stg.cam_pancam);
        ImGui::Checkbox("Mouse cursor on target pan camera", &mouse_stg.cam_pantgt);
        ImGui::Checkbox("Mouse cursor on chase camera", &mouse_stg.cam_chasecam);
        ImGui::Separator();
        ImGui::Text("Warp zone"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##wz", mouse_stg.warp_zone, sizeof(mouse_stg.warp_zone), ImGuiInputTextFlags_CharsDecimal)) dirty = true;
        ImGui::Text("Exponent"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##exp", mouse_stg.exponent, sizeof(mouse_stg.exponent), ImGuiInputTextFlags_CharsDecimal)) dirty = true;
        ImGui::Text("Deadband"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##db", mouse_stg.deadband, sizeof(mouse_stg.deadband), ImGuiInputTextFlags_CharsDecimal)) dirty = true;
        ImGui::Separator();
        if (ImGui::Button("Accept")) { mouse_dialog_open = false; dirty = true; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { mouse_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// Joystick Settings dialog.
static void draw_joystick_dialog() {
    if (!joy_dialog_open) return;
    ImGui::SetNextWindowSize(ImVec2(460, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Joystick Settings", &joy_dialog_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Deflect an axis to identify it, then bind it to a flight role.");
        ImGui::Separator();
        SDL_UpdateJoysticks();   // refresh axis state (also for a just-hotplugged joystick)
        int n = 0; SDL_JoystickID *ids = SDL_GetJoysticks(&n);
        // Persistent open handles: open each device once and reuse across frames so a
        // hotplugged joystick's axis state stays live.
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
                        for (int r = 0; r < 4; ++r)
                            if (joy_bind_stick[r] == i && joy_bind_axis[r] == a) { joy_bind_stick[r] = -1; joy_bind_axis[r] = -1; }
                        if (nr >= 0) { joy_bind_stick[nr] = i; joy_bind_axis[nr] = a; }
                        dirty = true;
                    }
                    ImGui::SameLine();
                    bool has = role >= 0;
                    if (!has) ImGui::BeginDisabled();
                    bool inv = has ? joy_bind_inv[role] : false;
                    if (ImGui::Checkbox((std::string(id) + "inv").c_str(), &inv) && has) {
                        joy_bind_inv[role] = inv; dirty = true;
                    }
                    if (!has) ImGui::EndDisabled();
                }
            }
        }
        if (ids) SDL_free(ids);
        ImGui::Separator();
        ImGui::Text("Deadband"); ImGui::SameLine(); ImGui::SetNextItemWidth(90);
        if (ImGui::InputText("##jdb", joy_deadband, sizeof(joy_deadband), ImGuiInputTextFlags_CharsDecimal)) dirty = true;
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
                joy_auto_sampling = false; dirty = true;
            }
        }
        if (ImGui::Checkbox("Force feedback", &joy_ffb)) dirty = true;
        ImGui::Text("FFB device"); ImGui::SameLine(); ImGui::SetNextItemWidth(50);
        if (ImGui::InputText("##jff", joy_ff_device, sizeof(joy_ff_device), ImGuiInputTextFlags_CharsDecimal)) dirty = true;
        ImGui::Separator();
        if (ImGui::Button("Reset Axis")) {
            for (int r = 0; r < 4; ++r) { joy_bind_stick[r] = -1; joy_bind_axis[r] = -1; joy_bind_inv[r] = false; }
            dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Accept")) { joy_dialog_open = false; dirty = true; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { joy_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// ---------------------------------------------------------------------------
// Bindings (full vs-05 port, against Configuration.actions)
// ---------------------------------------------------------------------------

// Bind categories (vs-05 BC_*).
enum { BC_COCKPIT = 0, BC_MOVEMENT, BC_WEAPONS, BC_TARGETING, BC_COMMS, BC_SHIP, BC_MISC, BC_COUNT };
static const char *bind_cat_names[BC_COUNT] = { "Cockpit / Camera", "Movement / Flight", "Weapons / Combat",
    "Targeting", "Comms", "Ship / Systems", "Interface / Misc" };

struct BindRow {
    std::string command, device;   // device: "key" | "mouse" | "joystick"
    std::string key, button, device_idx, modifier;
    std::string hat_idx;           // joystick digital-hatswitch index
    std::string hat_dir;           // joystick digital-hatswitch direction, else ""
    int  category = BC_MISC;
    bool remove = false, dirty = false, conflict = false;
};
static std::vector<BindRow> bindrows;
static int  bind_cat = BC_COCKPIT;
static bool bind_pending = false;
static bool bind_capture_requested = false;
static bool cap_valid = false;
static bool cap_open = false;
static std::string cap_device, cap_idx, cap_btn, cap_key, cap_modifier;
static std::string cap_hat_idx, cap_hat_dir;
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

// Load Configuration.actions into the staged bindrows.
static void load_bindings_staging() {
    bindrows.clear();
    for (const auto &kv : configuration().actions) {
        const std::string &cmd = kv.first;
        const auto &ab = kv.second;
        for (const auto &b : ab.keyboard) {
            BindRow r; r.command = cmd; r.category = bind_category(cmd); r.device = "key";
            r.key = b.key; r.modifier = b.modifier; bindrows.push_back(r);
        }
        for (const auto &b : ab.mouse) {
            BindRow r; r.command = cmd; r.category = bind_category(cmd); r.device = "mouse";
            r.button = std::to_string(b.button); r.device_idx = "0"; r.modifier = "none"; bindrows.push_back(r);
        }
        for (const auto &b : ab.joystick) {
            BindRow r; r.command = cmd; r.category = bind_category(cmd); r.device = "joystick";
            r.button = std::to_string(b.button); r.device_idx = std::to_string(b.joystick); r.modifier = "none";
            bindrows.push_back(r);
        }
        for (const auto &b : ab.hat) {
            BindRow r; r.command = cmd; r.category = bind_category(cmd); r.device = "joystick";
            r.hat_idx = std::to_string(b.hatswitch); r.hat_dir = b.direction;
            r.device_idx = std::to_string(b.joystick); r.modifier = "none"; bindrows.push_back(r);
        }
    }
    compute_bind_conflicts();
}

// Commit the staged bindrows to Configuration.actions (on Accept).
static void apply_bindrows_to_config() {
    // Snapshot the current actions so we can mark only the commands that actually
    // changed (sparse bindings.json overlay). The bindings dialog rebuilds the
    // whole map, so we diff against the previous state to find the dirty commands.
    const auto old_actions = cfg().actions;
    auto &actions = cfg().actions;
    actions.clear();   // rebuild from the staged rows
    for (const auto &r : bindrows) {
        if (r.remove) continue;
        auto &ab = actions[r.command];
        vega_config::Configuration::ActionBinding b;
        b.modifier = "none";
        if (r.device == "key") {
            b.key = r.key; b.modifier = r.modifier; ab.keyboard.push_back(b);
        } else if (r.device == "mouse") {
            b.button = atoi(r.button.c_str()); b.is_mouse = true; b.joystick = -1; ab.mouse.push_back(b);
        } else if (!r.hat_dir.empty()) {
            b.hatswitch = atoi(r.hat_idx.c_str()); b.direction = r.hat_dir;
            b.joystick = atoi(r.device_idx.c_str()); ab.hat.push_back(b);
        } else {
            b.button = atoi(r.button.c_str()); b.joystick = atoi(r.device_idx.c_str()); ab.joystick.push_back(b);
        }
    }
    // Mark dirty only the commands whose bindings actually changed. (Commands the
    // dialog leaves empty are written as empty arrays, so no removal tombstone is
    // needed.)
    for (const auto &kv : actions) {
        auto old_it = old_actions.find(kv.first);
        if (old_it == old_actions.end() || !action_bindings_equal(old_it->second, kv.second)) {
            mark_dirty("bindings.actions." + kv.first);
        }
    }
}

// Mark every bind row sharing the currently-captured input (other actions) as removed.
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

// Actions already bound to the currently-captured input (excluding the target action).
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

// Map an SDL3 keycode to the config's key-name convention.
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

// Convert an SDL joystick instance ID to its index in SDL_GetJoysticks().
static int joystick_index_of(SDL_JoystickID which) {
    int n = 0; SDL_JoystickID *ids = SDL_GetJoysticks(&n);
    int idx = -1;
    for (int i = 0; i < n; i++) if (ids[i] == which) { idx = i; break; }
    SDL_free(ids);
    return idx;
}

// Map an SDL_HAT_* bitmask to its direction name; NULL for CENTERED.
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
        default:               return NULL;
    }
}

// Commit a captured input to a new/replaced bind row for the current action.
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

// Handle SDL events for binding capture + joystick hotplug.
static void handle_bindings_event(const SDL_Event *event) {
    if (bind_capturing && cap_open && !bind_capture_cmd.empty()) {
        if (event->type == SDL_EVENT_KEY_DOWN) {
            SDL_Keycode kc = event->key.key;
            if (kc == SDLK_LSHIFT || kc == SDLK_RSHIFT || kc == SDLK_LCTRL || kc == SDLK_RCTRL
                || kc == SDLK_LALT || kc == SDLK_RALT || kc == SDLK_LGUI || kc == SDLK_RGUI)
                return;
            const char *nm = config_key_name(kc);
            if (nm) {
                std::string k = nm;
                if ((event->key.mod & SDL_KMOD_SHIFT) && k.size() == 1 && k[0] >= 'a' && k[0] <= 'z')
                    k[0] = (char)(k[0] - 'a' + 'A');
                cap_device = "key"; cap_key = k;
                cap_modifier = (event->key.mod & SDL_KMOD_CTRL) ? "ctrl"
                             : ((event->key.mod & SDL_KMOD_ALT) ? "alt" : "none");
                cap_valid = true;
            }
            return;
        }
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            bool inframe = event->button.x >= cap_frame_pos.x && event->button.x <= cap_frame_pos.x + cap_frame_size.x
                        && event->button.y >= cap_frame_pos.y && event->button.y <= cap_frame_pos.y + cap_frame_size.y;
            if (inframe) {
                cap_device = "mouse"; cap_btn = std::to_string(event->button.button);
                cap_idx = "0"; cap_modifier = "none"; cap_valid = true;
                return;
            }
        }
        if (event->type == SDL_EVENT_JOYSTICK_BUTTON_DOWN) {
            int idx = joystick_index_of(event->jbutton.which);
            cap_device = "joystick"; cap_btn = std::to_string(event->jbutton.button);
            cap_idx = (idx < 0) ? std::to_string(event->jbutton.which) : std::to_string(idx);
            cap_modifier = "none"; cap_valid = true;
            return;
        }
        if (event->type == SDL_EVENT_JOYSTICK_HAT_MOTION) {
            const char *dir = hat_value_name(event->jhat.value);
            if (dir) {
                int idx = joystick_index_of(event->jhat.which);
                cap_device = "joystick";
                cap_idx = (idx < 0) ? std::to_string(event->jhat.which) : std::to_string(idx);
                cap_hat_idx = std::to_string(event->jhat.hat);
                cap_hat_dir = dir;
                cap_btn.clear();
                cap_valid = true;
            }
            return;
        }
    }
    if (event->type == SDL_EVENT_JOYSTICK_ADDED || event->type == SDL_EVENT_JOYSTICK_REMOVED)
        SDL_UpdateJoysticks();
}

// Capture Binding modal.
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
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "! This input is already bound to:");
                for (auto &c : conflicts)
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "    - %s", c.c_str());
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "  Accepting will share the input (not recommended).");
                if (ImGui::Button("Clear other Bind")) clear_other_binds();
            }
        }
        ImGui::Separator();
        if (cap_valid && ImGui::Button("Accept")) { accept_capture(); bind_pending = true; dirty = true; bind_capturing = false; ImGui::CloseCurrentPopup(); }
        if (cap_valid) ImGui::SameLine();
        if (ImGui::Button("Retry")) { cap_valid = false; cap_hat_dir.clear(); cap_hat_idx.clear(); }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { bind_capturing = false; cap_valid = false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// Bindings dialog: category column on the left; binds table on the right.
static void draw_bindings_dialog(void) {
    ImGui::SetNextWindowSize(ImVec2(960, 540), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Bindings", &bind_dialog_open)) {
        ImGui::BeginChild("catcol", ImVec2(220, 440), ImGuiChildFlags_Borders);
        for (int c = 0; c < BC_COUNT; ++c)
            if (ImGui::Selectable(bind_cat_names[c], bind_cat == c, 0, ImVec2(210, 0))) bind_cat = c;
        ImGui::EndChild();
        ImGui::SameLine();
        float tbl_w = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("bindlist", ImVec2(tbl_w, 440), ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        if (bindrows.empty()) {
            ImGui::TextWrapped("No bindings loaded (Configuration has no actions).");
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
        if (ImGui::Button("Accept")) { bind_pending = true; dirty = true; apply_bindrows_to_config(); bind_capturing = false; bind_dialog_open = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { bind_capturing = false; bind_dialog_open = false; ImGui::CloseCurrentPopup(); }
        draw_capture_dialog();
        ImGui::EndPopup();
    }
}

// ---------------------------------------------------------------------------
// Presets (dropdowns from engine.json presets; record selectors in
// Configuration.preset).
// ---------------------------------------------------------------------------

struct PresetOptionInfo {
    std::string name;
    std::vector<std::pair<std::string,std::string>> vars;   // (var-name, value)
};
struct PresetGroupInfo {
    std::string key;              // config.json preset key (lowercase, e.g. "computer")
    std::string label;            // display name (e.g. "Computer")
    std::vector<PresetOptionInfo> options;  // options + the vars each sets
};

static std::vector<PresetGroupInfo> g_preset_groups;
static bool g_presets_loaded = false;

// Map an engine.json preset category name (e.g. "MusicAndVolume", "FactionTextures")
// to the config.json preset key (lowercase snake_case: "music", "faction_textures").
// Known exceptions first (MusicAndVolume -> music), then camelCase -> snake_case.
static std::string preset_key_for_category(const std::string &cat) {
    if (cat == "MusicAndVolume") return "music";
    std::string key;
    key.reserve(cat.size() + 4);
    for (size_t i = 0; i < cat.size(); ++i) {
        char c = cat[i];
        if (c >= 'A' && c <= 'Z') {
            if (i > 0) key += '_';
            key += (char)(c - 'A' + 'a');
        } else {
            key += c;
        }
    }
    return key;
}

// Load preset categories/options from engine.json -> presets (via VSFileSystem::datadir).
static void load_presets() {
    if (g_presets_loaded) return;
    g_preset_groups.clear();
    std::ifstream in(VSFileSystem::datadir + "/engine.json");
    if (!in) return;
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    try {
        boost::json::value root = boost::json::parse(text);
        const boost::json::value *presets = root.if_object() ? root.as_object().if_contains("presets") : nullptr;
        if (presets && presets->is_object()) {
            for (const auto &kv : presets->as_object()) {
                if (!kv.value().is_object() || kv.value().as_object().empty()) continue;
                std::string key = preset_key_for_category(kv.key());
                // Skip the hand-rolled display/input/degenerate groups (as vs-05 does).
                if (key == "resolution" || key == "mouse" || key == "text"
                    || key == "accelerated_visual" || key == "color" || key == "joystick") continue;
                PresetGroupInfo g;
                g.key = key; g.label = kv.key();
                for (const auto &okv : kv.value().as_object()) {
                    PresetOptionInfo o;
                    o.name = okv.key();
                    if (okv.value().is_object()) {
                        for (const auto &vv : okv.value().as_object()) {
                            if (vv.value().is_string())
                                o.vars.push_back({vv.key(), boost::json::value_to<std::string>(vv.value())});
                        }
                    }
                    g.options.push_back(o);
                }
                if (!g.options.empty()) g_preset_groups.push_back(g);
            }
        }
    } catch (...) {}
    g_presets_loaded = true;
}

// ---- Single source of truth: config path <-> Configuration field ----
//
// C++ has no reflection, so every direction of config traffic (preset apply,
// write-back serialize, dirty-path tracking) used to hand-encode the same
// path<->field relationship as three separate if/else chains that could drift
// apart (and did: graphics.bases.max_*, graphics.screen, graphics.font and
// graphics.font_antialias were marked dirty but not readable, so they never
// persisted). This table is the only place that maps a dotted config path to its
// Configuration field.

struct ConfigAccessor {
    const char* path;   // dotted config.json path, e.g. "graphics.fog"
    // Read the field's current value (used by write_out_dirty). Never null.
    boost::json::value (*get)(const vega_config::Configuration&);
    // Apply a preset string value (used by apply_preset_var). Null if the key is
    // not settable from a preset (it is set by a dedicated apply_* function).
    void (*set)(vega_config::Configuration&, const std::string&);
};

static const ConfigAccessor kConfigAccessors[] = {
    // ---- graphics ----
    {"graphics.fog",                     [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.fog;},                   [](vega_config::Configuration&c,const std::string&v){c.graphics.fog=(v=="true"||v=="1");}},
    {"graphics.background",              [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.background;},            [](vega_config::Configuration&c,const std::string&v){c.graphics.background=(v=="true"||v=="1");}},
    {"graphics.blend_panels",            [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.blend_panels;},          [](vega_config::Configuration&c,const std::string&v){c.graphics.blend_panels=(v=="true"||v=="1");}},
    {"graphics.cockpit",                 [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.cockpit;},               [](vega_config::Configuration&c,const std::string&v){c.graphics.cockpit=(v=="true"||v=="1");}},
    {"graphics.color_depth",             [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.color_depth;},           [](vega_config::Configuration&c,const std::string&v){c.graphics.color_depth=atoi(v.c_str());}},
    {"graphics.force_lighting",          [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.force_lighting;},        [](vega_config::Configuration&c,const std::string&v){c.graphics.force_lighting=(v=="true"||v=="1");}},
    {"graphics.full_screen",             [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.full_screen;},           [](vega_config::Configuration&c,const std::string&v){c.graphics.full_screen=(v=="true"||v=="1");}},
    {"graphics.reflection",              [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.reflection;},            [](vega_config::Configuration&c,const std::string&v){c.graphics.reflection=(v=="true"||v=="1");}},
    {"graphics.smooth_lines",            [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.smooth_lines;},          [](vega_config::Configuration&c,const std::string&v){c.graphics.smooth_lines=(v=="true"||v=="1");}},
    {"graphics.star_blend",              [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.star_blend;},            [](vega_config::Configuration&c,const std::string&v){c.graphics.star_blend=(v=="true"||v=="1");}},
    {"graphics.draw_star_body",          [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.draw_star_body;},        [](vega_config::Configuration&c,const std::string&v){c.graphics.draw_star_body=(v=="true"||v=="1");}},
    {"graphics.draw_star_glow",          [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.draw_star_glow;},        [](vega_config::Configuration&c,const std::string&v){c.graphics.draw_star_glow=(v=="true"||v=="1");}},
    {"graphics.high_quality_font",       [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.high_quality_font;},     [](vega_config::Configuration&c,const std::string&v){c.graphics.high_quality_font=(v=="true"||v=="1");}},
    {"graphics.high_quality_font_computer",[](const vega_config::Configuration&c)->boost::json::value{return c.graphics.high_quality_font_computer;},[](vega_config::Configuration&c,const std::string&v){c.graphics.high_quality_font_computer=(v=="true"||v=="1");}},
    {"graphics.high_quality_sprites",    [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.high_quality_sprites;},  [](vega_config::Configuration&c,const std::string&v){c.graphics.high_quality_sprites=(v=="true"||v=="1");}},
    {"graphics.per_pixel_lighting",      [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.per_pixel_lighting;},    [](vega_config::Configuration&c,const std::string&v){c.graphics.per_pixel_lighting=(v=="true"||v=="1");}},
    {"graphics.specmap_with_reflection", [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.specmap_with_reflection;},[](vega_config::Configuration&c,const std::string&v){c.graphics.specmap_with_reflection=(v=="true"||v=="1");}},
    {"graphics.gl_accelerated_visual",   [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.gl_accelerated_visual;}, [](vega_config::Configuration&c,const std::string&v){c.graphics.gl_accelerated_visual=(v=="true"||v=="1");}},
    {"graphics.aspect",                  [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.aspect_flt;},            [](vega_config::Configuration&c,const std::string&v){c.graphics.aspect_flt=(float)atof(v.c_str());c.graphics.aspect_dbl=atof(v.c_str());}},
    {"graphics.font_point",              [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.font_point_flt;},        [](vega_config::Configuration&c,const std::string&v){c.graphics.font_point_flt=(float)atof(v.c_str());c.graphics.font_point_dbl=atof(v.c_str());}},
    {"graphics.model_detail",            [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.model_detail_flt;},      [](vega_config::Configuration&c,const std::string&v){c.graphics.model_detail_flt=(float)atof(v.c_str());c.graphics.model_detail_dbl=atof(v.c_str());}},
    {"graphics.mipmap_detail",           [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.mipmap_detail;},         [](vega_config::Configuration&c,const std::string&v){c.graphics.mipmap_detail=atoi(v.c_str());}},
    {"graphics.planet_detail_level",     [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.planet_detail_level;},   [](vega_config::Configuration&c,const std::string&v){c.graphics.planet_detail_level=atoi(v.c_str());}},
    {"graphics.resolution_x",            [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.resolution_x;},          [](vega_config::Configuration&c,const std::string&v){c.graphics.resolution_x=atoi(v.c_str());}},
    {"graphics.resolution_y",            [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.resolution_y;},          [](vega_config::Configuration&c,const std::string&v){c.graphics.resolution_y=atoi(v.c_str());}},
    {"graphics.max_cubemap_size",        [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.max_cubemap_size;},      [](vega_config::Configuration&c,const std::string&v){c.graphics.max_cubemap_size=atoi(v.c_str());}},
    {"graphics.max_movie_dimension",     [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.max_movie_dimension;},   [](vega_config::Configuration&c,const std::string&v){c.graphics.max_movie_dimension=atoi(v.c_str());}},
    {"graphics.max_texture_dimension",   [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.max_texture_dimension;}, [](vega_config::Configuration&c,const std::string&v){c.graphics.max_texture_dimension=atoi(v.c_str());}},
    {"graphics.technique_set",           [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.graphics.technique_set);},         [](vega_config::Configuration&c,const std::string&v){c.graphics.technique_set=v;}},
    {"graphics.mac_shader_name",         [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.graphics.mac_shader_name);},       [](vega_config::Configuration&c,const std::string&v){c.graphics.mac_shader_name=v;}},
    {"graphics.default_full_technique",  [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.graphics.default_full_technique);},[](vega_config::Configuration&c,const std::string&v){c.graphics.default_full_technique=v;}},
    {"graphics.default_simple_technique",[](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.graphics.default_simple_technique);},[](vega_config::Configuration&c,const std::string&v){c.graphics.default_simple_technique=v;}},
    {"graphics.faction_dependent_textures",[](const vega_config::Configuration&c)->boost::json::value{return c.graphics.faction_dependent_textures;},[](vega_config::Configuration&c,const std::string&v){c.graphics.faction_dependent_textures=(v=="true"||v=="1");}},
    {"graphics.draw_rendered_crosshairs",[](const vega_config::Configuration&c)->boost::json::value{return c.graphics.draw_rendered_crosshairs;},nullptr},
    {"graphics.bases.max_width",         [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.bases.max_width;},        nullptr},
    {"graphics.bases.max_height",        [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.bases.max_height;},       nullptr},
    {"graphics.font",                    [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.graphics.font);},                  nullptr},
    {"graphics.font_antialias",          [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.font_antialias;},        nullptr},
    {"graphics.screen",                  [](const vega_config::Configuration&c)->boost::json::value{return c.graphics.screen;},                nullptr},
    // ---- audio ----
    {"audio.ai_sound",                   [](const vega_config::Configuration&c)->boost::json::value{return c.audio.ai_sound;},                 [](vega_config::Configuration&c,const std::string&v){c.audio.ai_sound=(v=="true"||v=="1");}},
    {"audio.every_other_mount",          [](const vega_config::Configuration&c)->boost::json::value{return c.audio.every_other_mount;},        [](vega_config::Configuration&c,const std::string&v){c.audio.every_other_mount=(v=="true"||v=="1");}},
    {"audio.music",                      [](const vega_config::Configuration&c)->boost::json::value{return c.audio.music;},                    [](vega_config::Configuration&c,const std::string&v){c.audio.music=(v=="true"||v=="1");}},
    {"audio.sound",                      [](const vega_config::Configuration&c)->boost::json::value{return c.audio.sound;},                    [](vega_config::Configuration&c,const std::string&v){c.audio.sound=(v=="true"||v=="1");}},
    {"audio.positional",                 [](const vega_config::Configuration&c)->boost::json::value{return c.audio.positional;},               [](vega_config::Configuration&c,const std::string&v){c.audio.positional=(v=="true"||v=="1");}},
    {"audio.music_volume",               [](const vega_config::Configuration&c)->boost::json::value{return c.audio.music_volume_flt;},         [](vega_config::Configuration&c,const std::string&v){c.audio.music_volume_flt=(float)atof(v.c_str());c.audio.music_volume_dbl=atof(v.c_str());}},
    {"audio.volume",                     [](const vega_config::Configuration&c)->boost::json::value{return c.audio.volume_flt;},               [](vega_config::Configuration&c,const std::string&v){c.audio.volume_flt=(float)atof(v.c_str());c.audio.volume_dbl=atof(v.c_str());}},
    {"audio.max_single_sounds",          [](const vega_config::Configuration&c)->boost::json::value{return c.audio.max_single_sounds;},        [](vega_config::Configuration&c,const std::string&v){c.audio.max_single_sounds=atoi(v.c_str());}},
    {"audio.max_total_sounds",           [](const vega_config::Configuration&c)->boost::json::value{return c.audio.max_total_sounds;},         [](vega_config::Configuration&c,const std::string&v){c.audio.max_total_sounds=atoi(v.c_str());}},
    {"audio.sounds_extension_1",         [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.cockpit_audio.sounds_extension_1);},[](vega_config::Configuration&c,const std::string&v){c.cockpit_audio.sounds_extension_1=v;}},
    {"audio.sounds_extension_2",         [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.cockpit_audio.sounds_extension_2);},[](vega_config::Configuration&c,const std::string&v){c.cockpit_audio.sounds_extension_2=v;}},
    // ---- physics ----
    {"physics.game_speed",               [](const vega_config::Configuration&c)->boost::json::value{return c.physics.game_speed_flt;},         [](vega_config::Configuration&c,const std::string&v){c.physics.game_speed_flt=(float)atof(v.c_str());c.physics.game_speed_dbl=atof(v.c_str());}},
    {"physics.game_accel",               [](const vega_config::Configuration&c)->boost::json::value{return c.physics.game_accel_flt;},         [](vega_config::Configuration&c,const std::string&v){c.physics.game_accel_flt=(float)atof(v.c_str());c.physics.game_accel_dbl=atof(v.c_str());}},
    {"physics.inactive_system_time",     [](const vega_config::Configuration&c)->boost::json::value{return c.physics.inactive_system_time_flt;},[](vega_config::Configuration&c,const std::string&v){c.physics.inactive_system_time_flt=(float)atof(v.c_str());c.physics.inactive_system_time_dbl=atof(v.c_str());}},
    {"physics.num_running_systems",      [](const vega_config::Configuration&c)->boost::json::value{return c.physics.num_running_systems;},    [](vega_config::Configuration&c,const std::string&v){c.physics.num_running_systems=atoi(v.c_str());}},
    // ---- general ----
    {"general.num_old_systems",          [](const vega_config::Configuration&c)->boost::json::value{return c.general.num_old_systems;},        [](vega_config::Configuration&c,const std::string&v){c.general.num_old_systems=atoi(v.c_str());}},
    {"general.simulation_atom",          [](const vega_config::Configuration&c)->boost::json::value{return c.general.simulation_atom_flt;},    [](vega_config::Configuration&c,const std::string&v){c.general.simulation_atom_flt=(float)atof(v.c_str());c.general.simulation_atom_dbl=atof(v.c_str());}},
    // ---- joystick (input.joystick) ----
    {"input.joystick.mouse_cursor",      [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_cursor;},          [](vega_config::Configuration&c,const std::string&v){c.joystick.mouse_cursor=(v=="true"||v=="1");}},
    {"input.joystick.mouse_sensitivity", [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_sensitivity_flt;}, [](vega_config::Configuration&c,const std::string&v){c.joystick.mouse_sensitivity_flt=(float)atof(v.c_str());c.joystick.mouse_sensitivity_dbl=atof(v.c_str());}},
    {"input.joystick.reverse_mouse_spr", [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.reverse_mouse_spr;},     [](vega_config::Configuration&c,const std::string&v){c.joystick.reverse_mouse_spr=(v=="true"||v=="1");}},
    {"input.joystick.warp_mouse",        [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.warp_mouse;},            [](vega_config::Configuration&c,const std::string&v){c.joystick.warp_mouse=(v=="true"||v=="1");}},
    {"input.joystick.force_use_of_joystick",[](const vega_config::Configuration&c)->boost::json::value{return c.joystick.force_use_of_joystick;},[](vega_config::Configuration&c,const std::string&v){c.joystick.force_use_of_joystick=(v=="true"||v=="1");}},
    {"input.joystick.mouse_cursor_pancam",[](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_cursor_pancam;},  nullptr},
    {"input.joystick.mouse_cursor_pantgt",[](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_cursor_pantgt;},  nullptr},
    {"input.joystick.mouse_cursor_chasecam",[](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_cursor_chasecam;},nullptr},
    {"input.joystick.warp_mouse_zone",   [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.warp_mouse_zone;},       nullptr},
    {"input.joystick.mouse_exponent",    [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_exponent_flt;},   nullptr},
    {"input.joystick.mouse_deadband",    [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.mouse_deadband_flt;},   nullptr},
    {"input.joystick.deadband",          [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.deadband_flt;},         nullptr},
    {"input.joystick.force_feedback",    [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.force_feedback;},       nullptr},
    {"input.joystick.ff_device",         [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.ff_device;},            nullptr},
    {"input.joystick.enabled",           [](const vega_config::Configuration&c)->boost::json::value{return c.joystick.enabled;},              nullptr},
    // ---- input ----
    {"input.device",                     [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.input.device);},                  nullptr},
    {"input.mouse_preset",               [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.input.mouse_preset);},            nullptr},
    {"input.mouse.enabled",              [](const vega_config::Configuration&c)->boost::json::value{return c.mouse.enabled;},                 nullptr},
    {"input.mouse.inverse_x",            [](const vega_config::Configuration&c)->boost::json::value{return c.mouse.inverse_x;},               nullptr},
    {"input.mouse.inverse_y",            [](const vega_config::Configuration&c)->boost::json::value{return c.mouse.inverse_y;},               nullptr},
    // ---- splash / test ----
    {"splash.loading_sprite",            [](const vega_config::Configuration&c)->boost::json::value{return boost::json::value(c.splash.loading_sprite);},          [](vega_config::Configuration&c,const std::string&v){c.splash.loading_sprite=v;}},
    {"test.autodocker",                  [](const vega_config::Configuration&c)->boost::json::value{return c.test.autodocker;},                [](vega_config::Configuration&c,const std::string&v){c.test.autodocker=(v=="true"||v=="1");}},
};

static const ConfigAccessor* by_path(const std::string &path) {
    for (const auto &a : kConfigAccessors) {
        if (path == a.path) return &a;
    }
    return nullptr;
}

// Resolve a bare preset variable name (e.g. "fog") to its accessor by matching
// the leaf of the dotted path. Preset names are always the path leaf; a few leaves
// are ambiguous ("enabled": input.mouse vs input.joystick) but those are never
// preset-applied (their set is null), so leaf lookup is unambiguous for the
// preset set.
static const ConfigAccessor* by_leaf(const std::string &leaf) {
    for (const auto &a : kConfigAccessors) {
        std::string p(a.path);
        if (p.substr(p.find_last_of('.') + 1) == leaf) return &a;
    }
    return nullptr;
}

// Read the current value of a Configuration field given its dotted path. Returns
// a JSON value, or null if unknown.
static boost::json::value read_config_value(const std::string &path) {
    return read_config_value_from(configuration(), path);
}

static boost::json::value read_config_value_from(const vega_config::Configuration &c, const std::string &path) {
    const ConfigAccessor* a = by_path(path);
    return a ? a->get(c) : boost::json::value(nullptr);
}

// ---------------------------------------------------------------------------
// Shader / technique handling
// ---------------------------------------------------------------------------
// The 4 shader/technique config paths set by the "shaders" preset. A shader
// change is PERSISTED on Save but is NOT applied to the running game in the hot
// path (it breaks the technique/shader rendering); it only takes effect on
// restart. The restart-required dialog tells the user this.
static const char *kShaderPaths[] = {
    "graphics.technique_set", "graphics.mac_shader_name",
    "graphics.default_full_technique", "graphics.default_simple_technique"
};

// Snapshot the current shader/technique values (as strings). Used on Save to
// detect an ACTUAL shader change: the presets re-apply the selected 'shaders'
// preset on every Save, so dirty-ness alone can't gate the notice — only a value
// change can.
static std::vector<std::string> shader_values_snapshot() {
    std::vector<std::string> vals;
    for (const char *p : kShaderPaths) {
        boost::json::value v = read_config_value(p);
        vals.push_back(v.is_string() ? std::string(v.as_string().c_str()) : "");
    }
    return vals;
}

// Restore the shader/technique values from a snapshot. Used after a Save so a
// shader change is written out but the running game keeps the previous shaders
// until a restart (the hot path must not apply a shader change).
static void restore_shader_values(const std::vector<std::string> &vals) {
    for (size_t i = 0; i < 4 && i < vals.size(); ++i) {
        const ConfigAccessor *a = by_path(kShaderPaths[i]);
        if (a != nullptr) a->set(configuration(), vals[i]);
    }
}

// Apply one preset variable (name,value) to the engine's Configuration, using
// the field names the engine's load_config reads. Returns true if it set something.
static bool apply_preset_var(const std::string &name, const std::string &value) {
    const ConfigAccessor* a = by_leaf(name);
    if (!a || !a->set) return false;
    a->set(cfg(), value);
    // Persist the resolved value to config.json via its real dotted path (so it
    // survives a restart), not the synthetic preset_var.<name> path.
    mark_dirty(a->path);
    return true;
}

// Apply the currently-selected option's vars for every displayed preset group
// (the active preset selection), so the displayed presets are always persisted
// to config.json on Save — not only when the user happens to change a combo.
// Called from the Save/apply_all path.
static void apply_presets_to_config() {
    for (const auto &g : g_preset_groups) {
        std::string cur = configuration().preset.count(g.key) ? configuration().preset.at(g.key) : "";
        if (cur.empty()) continue;
        for (const auto &o : g.options) {
            if (o.name == cur) {
                for (const auto &kv : o.vars) apply_preset_var(kv.first, kv.second);
                break;
            }
        }
    }
}

// Draw the preset grid (vs-05 layout: group + combo, centered, 3 columns).
static void draw_presets_frame() {
    load_presets();
    if (g_preset_groups.empty()) {
        ImGui::TextWrapped("No presets loaded (engine.json presets not found).");
        return;
    }
    int cols = 3;
    // Wrap the grid in its own bordered frame (own frame, above the button bar).
    // Auto-scale with the window: more columns when there's width, and the
    // frame fills the vertical space down to the bottom button bar.
    const float avail_w = ImGui::GetContentRegionAvail().x;
    if (avail_w < 700) cols = 1;
    else if (avail_w < 1200) cols = 2;
    else cols = 3;
    cols = std::min(cols, (int)g_preset_groups.size());
    // The frame always fills the space above the bottom button bar (stretches to
    // fit); the child window clips and scrolls any content that does not fit, so it
    // never spills over the frame's borders or the buttons.
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    float avail_h = ImGui::GetContentRegionAvail().y - btn_h
                    - ImGui::GetStyle().WindowPadding.y;
    float pres_h = avail_h;
    ImGui::BeginChild("presetsframe", ImVec2(-1.0f, pres_h), ImGuiChildFlags_Borders);
    std::vector<float> colw(cols, 0.0f);
    for (size_t i = 0; i < g_preset_groups.size(); i++) {
        auto &g = g_preset_groups[i];
        float cw = 0;
        for (auto &o : g.options) cw = fmaxf(cw, ImGui::CalcTextSize(o.name.c_str()).x);
        cw += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
        cw = fmaxf(cw, ImGui::CalcTextSize(g.label.c_str()).x);
        colw[i % cols] = fmaxf(colw[i % cols], cw);
    }
    float total_w = 0;
    for (auto c : colw) total_w += c;
    ImGui::SetCursorPosX(fmaxf(0.0f, (ImGui::GetContentRegionAvail().x - total_w) * 0.5f));
    if (ImGui::BeginTable("modern_presets", cols, ImGuiTableFlags_SizingFixedFit)) {
        for (size_t i = 0; i < g_preset_groups.size(); i++) {
            auto &g = g_preset_groups[i];
            ImGui::TableNextColumn();
            ImGui::Text("%s", g.label.c_str());
            std::string cur = configuration().preset.count(g.key) ? configuration().preset.at(g.key) : "";
            int sel = 0;
            for (size_t j = 0; j < g.options.size(); ++j)
                if (g.options[j].name == cur) { sel = (int)j; break; }
            float cw = 0;
            for (auto &o : g.options) cw = fmaxf(cw, ImGui::CalcTextSize(o.name.c_str()).x);
            ImGui::SetNextItemWidth(cw + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
            std::vector<const char *> items;
            for (auto &o : g.options) items.push_back(o.name.c_str());
            std::string lbl = "##mpre_" + g.key;
            if (ImGui::Combo(lbl.c_str(), &sel, items.data(), (int)items.size())) {
                if (sel >= 0) {
                    cfg().preset[g.key] = g.options[sel].name;
                    mark_dirty("preset." + g.key);
                    // The preset's vars are NOT applied here: they are applied (and
                    // persisted) by apply_presets_to_config() on Save, so a preset
                    // change (e.g. shaders/techniques) does NOT take effect instantly
                    // when the dropdown is clicked - it applies when Save is hit.
                    // Shader changes still require a restart (see the Save handler).
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();   // end presetsframe
}

} // namespace

// Persist the current window resolution to the user config overlay so a manual
// window resize survives a restart. The game re-creates the window at
// graphics.resolution_x/y on next launch, so a resized-to size is restored.
// Marked dirty + written out; does NOT hot-apply anything. Called from winsys on
// a settled (debounced) window resize.
void PersistWindowResolution(int w, int h) {
    auto &g = configuration().graphics;
    g.resolution_x = w;
    g.resolution_y = h;
    mark_dirty("graphics.resolution_x");
    mark_dirty("graphics.resolution_y");
    write_out_dirty();
}

void DrawConfigScreen() {
    // Load flight-control mode from the persisted input.device once.
    static bool s_loaded = false;
    if (!s_loaded) {
        s_loaded = true;
        const std::string &dev = configuration().input.device;
        flight_control = (dev == "mouse") ? FC_MOUSE : (dev == "joystick") ? FC_JOYSTICK : FC_KEYBOARD;
        if (flight_control == FC_MOUSE) load_mouse_staging();
        else if (flight_control == FC_JOYSTICK) load_joystick_staging();
    }
    // Cover the whole screen (game resolution), not a floating window.
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(configuration().graphics.resolution_x,
                                    configuration().graphics.resolution_y), ImGuiCond_Always);
    ImGui::Begin("Config", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("vs-settings-ng — Configuration");
    ImGui::Separator();

    draw_display_frame();

    // Fullscreen / windowed toggle (live-applied on Save).
    if (ImGui::Checkbox("Fullscreen", &cfg_full_screen)) dirty = true;

    // Rendered crosshair toggle (vs-05 had it in the display/button area).
    if (ImGui::Checkbox("Rendered Crosshair", &rendered_crosshair)) dirty = true;

    ImGui::Separator();
    ImGui::TextUnformatted(dirty ? "(unsaved changes)" : "(saved)");

    // Presets frame (own frame, above the button bar).
    ImGui::Separator();
    draw_presets_frame();

    // Bottom button bar, pinned to the bottom of the window and centered.
    auto apply_all = [&]() {
        apply_display_to_config();
        apply_presets_to_config();
        apply_flight_to_config();
        apply_mouse_to_config();
        apply_joystick_to_config();
        // Live-apply input bindings/axes/device: re-run the bind path so key,
        // mouse, joystick and flight-device changes take effect immediately
        // (not just after a restart). Only when an input-related setting changed.
        static const char *input_prefixes[] = {"bindings.", "input.", nullptr};
        bool input_changed = false;
        for (const auto &path : g_dirty_paths) {
            for (const char **p = input_prefixes; *p; ++p) {
                if (path.rfind(*p, 0) == 0) { input_changed = true; break; }
            }
            if (input_changed) break;
        }
        if (input_changed && vs_config != nullptr) {
            vs_config->bindKeys();   // virtual: GameVegaConfig::bindKeys()
        }
    };
    auto close_overlay = [&]() {
        if (_Universe) _Universe->ToggleOptionsActive();   // close; hide cursor on inactive
    };
    // Width from the longest button label so all three fit their text.
    float btnw = fmaxf(ImGui::CalcTextSize("Save and Apply").x,
                       fmaxf(ImGui::CalcTextSize("Close Settings").x,
                             ImGui::CalcTextSize("Exit VegaStrike").x))
                 + ImGui::GetStyle().FramePadding.x * 2 + 20;
    float btn_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    // Reserve space at the bottom for the button row (separator + buttons).
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - btn_h);
    ImGui::Separator();
    // Center the three buttons (Save and Apply + Close Settings + Exit VegaStrike).
    // SetCursorPosX must be applied AFTER the Separator, which resets the cursor
    // X to the left margin.
    float avail = ImGui::GetContentRegionAvail().x;
    float buttons_w = btnw * 3 + ImGui::GetStyle().ItemSpacing.x * 2;
    ImGui::SetCursorPosX(fmaxf(0.0f, (avail - buttons_w) * 0.5f));

    // Save (green when dirty): apply + persist, stay open. Only Close Settings closes.
    const bool save_was_dirty = dirty;
    if (save_was_dirty) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.45f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
    }
    if (ImGui::Button("Save and Apply", ImVec2(btnw, 0))) {
        if (dirty) {
            // Snapshot the shader settings before applying, so we can detect an
            // ACTUAL shader change (the presets' vars are applied here, by
            // apply_presets_to_config, on Save - not at selection). A shader
            // change is NOT applied in the hot path (it breaks the technique/
            // shader rendering); it's written out and takes effect on restart.
            const auto shader_before = shader_values_snapshot();
            apply_all();
            write_out_dirty();   // persist the dirty paths to the user overlay
            if (shader_values_snapshot() != shader_before) {
                // Tell the user the change takes effect on restart, and restore
                // the running shader state so the current visuals are kept until
                // then (the change is already persisted above).
                shader_restart_notice = true;
                restore_shader_values(shader_before);
            }
            // Re-initialize the runtime from the updated in-memory configuration().
            // The game assumed config never changes in-game, so config->runtime
            // copies (g_game feature flags, gl_options, GL viewport, legacy font
            // metrics, audio volume) ran only at bootstrap and went stale on an
            // in-game change. This re-binds them so the change takes effect
            // without a restart. A brief re-orient pause is expected.
            reinit_from_saved_config();
            dirty = false;
            // Stay open: the player can keep tweaking and Save again.
        }
    }
    if (save_was_dirty) ImGui::PopStyleColor(2);
    ImGui::SameLine();

    // Close Settings (red when dirty): don't save anything, just close.
    if (dirty) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    }
    if (ImGui::Button("Close Settings", ImVec2(btnw, 0))) {
        // Don't save anything; just close. Dirty state is intentionally discarded.
        close_overlay();
    }
    if (dirty) ImGui::PopStyleColor(2);
    ImGui::SameLine();

    // Quit/Exit (red): exit the game entirely. Unsaved changes are discarded. The full
    // cleanup path (flush logs, save game, close audio/window) is VSExit(0).
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    if (ImGui::Button("Exit VegaStrike", ImVec2(btnw, 0))) {
        VSExit(0);
    }
    ImGui::PopStyleColor(2);

    // Input dialogs (open as modals on top).
    if (mouse_dialog_open) ImGui::OpenPopup("Mouse Settings");
    if (joy_dialog_open) ImGui::OpenPopup("Joystick Settings");
    draw_mouse_dialog();
    draw_joystick_dialog();

    // Bindings dialog (modal on top).
    if (bind_dialog_open) ImGui::OpenPopup("Bindings");
    draw_bindings_dialog();

    // Shader-change notice (modal on top). Shaders are written out but not
    // hot-applied; tell the user a restart is required. Dismissed on OK.
    if (shader_restart_notice) {
        ImGui::OpenPopup("Shader Change");
    }
    if (ImGui::BeginPopupModal("Shader Change", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Shader changes take effect after a restart.");
        ImGui::TextUnformatted("The new settings were saved. Restart the game to apply them.");
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            shader_restart_notice = false;
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void HandleConfigEvent(const SDL_Event *event) {
    // Binding capture + joystick hotplug (only meaningful while the config screen is open).
    handle_bindings_event(event);
}

} // namespace vs_settings_ng
