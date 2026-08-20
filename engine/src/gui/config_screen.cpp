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
#include "universe.h"
#include <boost/json.hpp>
#include <imgui.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cstdio>
#include <cstdlib>

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
static const char *aspect_opts[] = { "4:3 (1.33)", "16:10 (1.6)", "16:9 (1.78)", "5:4 (1.25)", "1:1 (1.0)" };
static const float aspect_vals[] = { 4.0f / 3.0f, 1.6f, 16.0f / 9.0f, 1.25f, 1.0f };
int  sel_base_aspect = 0;
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

// Bindings dialog state (declared here so draw_display_frame can open it).
static bool bind_dialog_open = false;
static bool bind_capturing = false;
static int  bind_rebind_row = -1;
static std::string bind_capture_cmd;

enum { FONT_AA_VEC = 0, FONT_VEC, FONT_HELVETICA, FONT_TIMES, FONT_FIXED };
static const char *font_type_names[] = { "Antialiased Vector", "Vector", "Helvetica", "Times", "Fixed" };
int  sel_font_type = FONT_AA_VEC;
int  sel_bitmap_size[3] = { -1, -1, -1 };   // 0=Helvetica 1=Times 2=Fixed

enum { FC_KEYBOARD = 0, FC_MOUSE = 1, FC_JOYSTICK = 2 };
int  flight_control = FC_KEYBOARD;
static const char *fc_names[] = { "Keyboard", "Mouse", "Joystick" };

struct BitmapFamily {
    const char *name;
    int  n;
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

bool dirty = false;

// The set of config paths changed (dirty). Populated by the apply_*_to_config
// functions on Save; consumed by write_out_dirty() (the single write-out entry
// point, deferred to Layer 3) so it knows exactly what to persist.
static std::set<std::string> g_dirty_paths;

// Record a changed config path and mark the config dirty.
static void mark_dirty(const std::string &path) {
    dirty = true;
    g_dirty_paths.insert(path);
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

// The ideal font height (font_point) for the current resolution.
static int ideal_font_height() {
    return sel_res_h > 0 ? (int)(0.0125 * sel_res_h + 2.5 + 0.5) : 16;
}

static void prefill_text_height() {
    if (sel_res_h > 0) {
        int fp = ideal_font_height();
        snprintf(text_height_buf, sizeof(text_height_buf), "%d", fp);
    }
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

// ---------------------------------------------------------------------------
// Configuration read/write (the vs-05 model_set_var/get_var, mapped to the
// engine's Configuration struct).
// ---------------------------------------------------------------------------

static vega_config::Configuration &cfg() {
    return const_cast<vega_config::Configuration &>(configuration());
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
    // Font type from high_quality_font + font_antialias (vector AA / vector / bitmap).
    if (!g.high_quality_font && g.font_antialias) sel_font_type = FONT_AA_VEC;
    else if (!g.high_quality_font && !g.font_antialias) sel_font_type = FONT_VEC;
    else sel_font_type = FONT_HELVETICA;   // bitmap (font name will refine; v1 defaults)
    base_aspect_text = aspect_opts[sel_base_aspect];
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
    if (sel_font_type == FONT_AA_VEC) {
        g.high_quality_font = false;
        g.high_quality_font_computer = false;
        g.font_antialias = true;
    } else if (sel_font_type == FONT_VEC) {
        g.high_quality_font = false;
        g.high_quality_font_computer = false;
        g.font_antialias = false;
    } else {
        int fam = sel_font_type - FONT_HELVETICA;
        const BitmapFamily &f = bitmap_families[fam];
        int sz = sel_bitmap_size[fam];
        if (sz < 0) sz = suggested_bitmap_size(fam);
        if (sz < 0 || sz >= f.n) sz = 0;
        g.high_quality_font = true;
        g.high_quality_font_computer = true;
        g.font_antialias = false;
        std::string nm;
        if (fam == 0) nm = "helvetica" + std::string(f.size_names[sz]);
        else if (fam == 1) nm = "times" + std::string(f.size_names[sz]);
        else nm = "fixed" + std::string(f.size_names[sz]).substr(2);
        g.font = nm;
        mark_dirty("graphics.font");
    }
    mark_dirty("graphics.high_quality_font");
    mark_dirty("graphics.high_quality_font_computer");
    mark_dirty("graphics.font_antialias");
    g.aspect_flt = sel_screen_aspect >= 0 ? aspect_vals[sel_screen_aspect] : current_screen_aspect();
    mark_dirty("graphics.aspect");
    g.draw_rendered_crosshairs = rendered_crosshair;
    mark_dirty("graphics.draw_rendered_crosshairs");
    // base_max_width/height and monitor (screen) selection: Configuration lacks
    // base_max fields; screen index is stored in g.screen.
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
    // Resolution selector (detected fullscreen modes, deduplicated).
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
    // Font type picker.
    ImGui::Text("Font"); ImGui::SameLine();
    if (ImGui::Button(font_type_names[sel_font_type])) ImGui::OpenPopup("##pick_font");
    if (ImGui::BeginPopup("##pick_font")) {
        for (size_t i = 0; i < sizeof(font_type_names) / sizeof(font_type_names[0]); ++i)
            if (ImGui::MenuItem(font_type_names[i])) { sel_font_type = (int)i; dirty = true; }
        ImGui::EndPopup();
    }
    if (sel_font_type == FONT_AA_VEC || sel_font_type == FONT_VEC) {
        // Text Height input for vector fonts.
        ImGui::SameLine(); ImGui::Text("Text Height"); ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        if (ImGui::InputText("##textheight", text_height_buf, sizeof(text_height_buf), ImGuiInputTextFlags_CharsDecimal))
            dirty = true;
    } else {
        // Bitmap size picker for Helvetica/Times/Fixed.
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
                if (ImGui::Selectable(f.size_names[i], sel)) { sel_bitmap_size[fam] = i; dirty = true; }
            }
            ImGui::EndPopup();
        }
    }
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
static bool joy_mouse_cursor = false;
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
}

// Load joystick staging from Configuration.joystick + input.axes.
static void load_joystick_staging() {
    const auto &j = configuration().joystick;
    snprintf(joy_deadband, sizeof(joy_deadband), "%.2f", j.deadband_flt);
    joy_ffb = j.force_feedback;
    snprintf(joy_ff_device, sizeof(joy_ff_device), "%d", j.ff_device);
    joy_mouse_cursor = j.mouse_cursor;
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
    if (flight_control == FC_MOUSE) {
        j.force_use_of_joystick = false;
        j.warp_mouse = true;
    } else if (flight_control == FC_JOYSTICK) {
        j.force_use_of_joystick = true;
    } else {
        j.force_use_of_joystick = false;
        j.warp_mouse = false;
    }
    mark_dirty("joystick.force_use_of_joystick");
    mark_dirty("joystick.warp_mouse");
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
    mark_dirty("joystick.mouse_cursor_pancam");
    mark_dirty("joystick.mouse_cursor_pantgt");
    mark_dirty("joystick.mouse_cursor_chasecam");
    mark_dirty("joystick.warp_mouse_zone");
    mark_dirty("joystick.mouse_exponent");
    mark_dirty("joystick.mouse_deadband");
}

// Apply joystick staging to Configuration.joystick + input.axes.
static void apply_joystick_to_config() {
    if (flight_control != FC_JOYSTICK) return;
    auto &j = cfg().joystick;
    j.deadband_flt = (float)atof(joy_deadband);
    j.force_feedback = joy_ffb;
    j.ff_device = atoi(joy_ff_device);
    j.mouse_cursor = joy_mouse_cursor;
    mark_dirty("joystick.deadband");
    mark_dirty("joystick.force_feedback");
    mark_dirty("joystick.ff_device");
    mark_dirty("joystick.mouse_cursor");
    // Write the x/y/z/throttle axes.
    auto &axes = cfg().axes;
    for (int r = 0; r < 4; ++r) {
        const char *role = joy_role_names[r];
        if (joy_bind_axis[r] < 0) {
            axes.erase(role);
        } else {
            axes[role].source = "joystick";
            axes[role].joystick = joy_bind_stick[r];
            axes[role].axis = joy_bind_axis[r];
            axes[role].inverse = joy_bind_inv[r];
        }
    }
}

// Write the accumulated dirty config paths out to the user config files.
// Single write-out entry point — deferred to Layer 3. g_dirty_paths holds exactly
// what changed, so once the write-back is implemented this persists just those.
// For now it logs what would be written.
static void write_out_dirty() {
    if (g_dirty_paths.empty()) return;
    fprintf(stderr, "[vs-settings-ng] config write-out deferred; %zu dirty path(s):\n", g_dirty_paths.size());
    for (const auto &p : g_dirty_paths)
        fprintf(stderr, "  %s\n", p.c_str());
    g_dirty_paths.clear();
}

// Mouse Settings dialog.
static void draw_mouse_dialog() {
    if (!mouse_dialog_open) return;
    ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Mouse Settings", &mouse_dialog_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        // Flight mode (Mouse preset).
        int sel = 0;
        ImGui::Text("Flight mode"); ImGui::SameLine(); ImGui::SetNextItemWidth(180);
        if (ImGui::Combo("##mmode", &sel, mouse_mode_names, 5)) { dirty = true; }
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
        if (ImGui::Checkbox("Mouse cursor in flight", &joy_mouse_cursor)) dirty = true;
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
    // Bindings changed -> mark the whole actions tree dirty (written out as bindings.json).
    mark_dirty("bindings.actions");
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

struct PresetGroupInfo {
    std::string key;              // config.json preset key (lowercase, e.g. "computer")
    std::string label;            // display name (e.g. "Computer")
    std::vector<std::string> options;  // option names (e.g. "4000MHz")
};

// Static preset category/option definitions, matching engine.json -> presets.
// (Categories/options verified against Assets engine.json 2026-08-20. The app
// records selectors in Configuration.preset; it does not expand the vars yet.)
static const PresetGroupInfo kPresetDefs[] = {
    { "computer", "Computer", { "800MHz", "1600MHz", "2000MHz", "3000MHz", "4000MHz", "4000MHzPlus" } },
    { "physics", "Physics", { "Default (1.0)", "Realistic" } },
    { "geometry", "Geometry", { "GeomRetro", "GeomLow", "GeomMedium", "GeomHigh", "GeomVeryHigh", "GeomExtreme" } },
    { "textures", "Textures", { "TexRetro", "Tex256", "Tex512", "Tex1024", "Tex2048", "TexMax" } },
    { "shaders", "Shaders", { "noshader", "lowshader", "mediumshader", "highshader", "extremeshader", "onboardshader" } },
    { "sound", "Sound", { "audio_off", "audio_3d_windows", "audio_3d_win_male", "audio_3d_linux", "audio_3d_linux_male" } },
    { "music", "MusicAndVolume", { "music_off", "windows_ext_music_low", "windows_ext_music_on", "windows_ext_music_high" } },
    { "faction_textures", "FactionTextures", { "faction_tex_off", "faction_tex_on" } },
    { "autodocker", "Autodocker", { "autodocker_off", "autodocker_on" } },
    { "censorship", "Censorship", { "uncensored", "censored" } },
};
static const size_t kPresetDefCount = sizeof(kPresetDefs) / sizeof(kPresetDefs[0]);

// Draw the preset grid (vs-05 layout: group + combo, centered, 3 columns).
static void draw_presets_frame() {
    int cols = 3;
    std::vector<float> colw(cols, 0.0f);
    for (size_t i = 0; i < kPresetDefCount; i++) {
        auto &g = kPresetDefs[i];
        float cw = 0;
        for (auto &o : g.options) cw = fmaxf(cw, ImGui::CalcTextSize(o.c_str()).x);
        cw += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
        cw = fmaxf(cw, ImGui::CalcTextSize(g.label.c_str()).x);
        colw[i % cols] = fmaxf(colw[i % cols], cw);
    }
    float total_w = 0;
    for (auto c : colw) total_w += c;
    ImGui::SetCursorPosX(fmaxf(0.0f, (ImGui::GetContentRegionAvail().x - total_w) * 0.5f));
    if (ImGui::BeginTable("modern_presets", cols, ImGuiTableFlags_SizingFixedFit)) {
        for (size_t i = 0; i < kPresetDefCount; i++) {
            auto &g = kPresetDefs[i];
            ImGui::TableNextColumn();
            ImGui::Text("%s", g.label.c_str());
            std::string cur = configuration().preset.count(g.key) ? configuration().preset.at(g.key) : "";
            int sel = 0;
            for (size_t j = 0; j < g.options.size(); ++j)
                if (g.options[j] == cur) { sel = (int)j; break; }
            float cw = 0;
            for (auto &o : g.options) cw = fmaxf(cw, ImGui::CalcTextSize(o.c_str()).x);
            ImGui::SetNextItemWidth(cw + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x);
            std::vector<const char *> items;
            for (auto &o : g.options) items.push_back(o.c_str());
            std::string lbl = "##mpre_" + g.key;
            if (ImGui::Combo(lbl.c_str(), &sel, items.data(), (int)items.size())) {
                if (sel >= 0) {
                    cfg().preset[g.key] = g.options[sel];
                    mark_dirty("preset." + g.key);
                }
            }
        }
        ImGui::EndTable();
    }
}

} // namespace

void DrawConfigScreen() {
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

    // Bottom button bar.
    //  Preview: live-apply all changes to Configuration, stay open (green when dirty).
    //  Save:     live-apply + write out + close (green when dirty).
    //  Close:    don't save anything, just close (red when dirty).
    auto apply_all = [&]() {
        apply_display_to_config();
        apply_flight_to_config();
        apply_mouse_to_config();
        apply_joystick_to_config();
    };
    auto close_overlay = [&]() {
        if (_Universe) _Universe->ToggleOptionsActive();   // close; hide cursor on inactive
    };
    float btnw = ImGui::CalcTextSize("Save").x + ImGui::GetStyle().FramePadding.x * 2 + 20;

    // Preview (green when dirty): live-apply, stay open.
    if (dirty) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.45f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
    }
    if (ImGui::Button("Preview", ImVec2(btnw, 0))) {
        if (dirty) {
            apply_all();
            // Stay open; changes are live in Configuration but not written out yet.
        }
    }
    if (dirty) ImGui::PopStyleColor(2);
    ImGui::SameLine();

    // Save (green when dirty): apply + write out + close.
    if (dirty) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.45f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.55f, 0.30f, 1.0f));
    }
    if (ImGui::Button("Save", ImVec2(btnw, 0))) {
        if (dirty) {
            apply_all();
            write_out_dirty();   // deferred; records the dirty paths
            dirty = false;
            close_overlay();
        }
    }
    if (dirty) ImGui::PopStyleColor(2);
    ImGui::SameLine();

    // Close (red when dirty): don't save anything, just close.
    if (dirty) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.35f, 0.35f, 1.0f));
    }
    if (ImGui::Button("Close", ImVec2(btnw, 0))) {
        // Don't save anything; just close. Dirty state is intentionally discarded.
        close_overlay();
    }
    if (dirty) ImGui::PopStyleColor(2);

    // Input dialogs (open as modals on top).
    if (mouse_dialog_open) ImGui::OpenPopup("Mouse Settings");
    if (joy_dialog_open) ImGui::OpenPopup("Joystick Settings");
    draw_mouse_dialog();
    draw_joystick_dialog();

    // Bindings dialog (modal on top).
    if (bind_dialog_open) ImGui::OpenPopup("Bindings");
    draw_bindings_dialog();

    // Presets frame.
    ImGui::Separator();
    draw_presets_frame();

    ImGui::End();
}

void HandleConfigEvent(const SDL_Event *event) {
    // Binding capture + joystick hotplug (only meaningful while the config screen is open).
    handle_bindings_event(event);
}

} // namespace vs_settings_ng
