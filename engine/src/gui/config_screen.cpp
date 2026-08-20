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
#include <imgui.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>
#include <string>
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
    g.resolution_x = sel_res_w;
    g.resolution_y = sel_res_h;
    g.full_screen = true;   // in-game config screen implies fullscreen (vs-05 used the FBO); keep the existing setting
    g.font_point_flt = (float)atoi(text_height_buf);
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
    }
    g.aspect_flt = sel_screen_aspect >= 0 ? aspect_vals[sel_screen_aspect] : current_screen_aspect();
    g.draw_rendered_crosshairs = rendered_crosshair;
    // base_max_width/height and monitor (screen) selection: Configuration lacks
    // base_max fields; screen index is stored in g.screen.
    g.screen = sel_monitor;
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
    ImGui::EndChild();
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

    // Rendered crosshair toggle (vs-05 had it in the display/button area).
    if (ImGui::Checkbox("Rendered Crosshair", &rendered_crosshair)) dirty = true;

    ImGui::Separator();
    ImGui::TextUnformatted(dirty ? "(unsaved changes)" : "(saved)");

    if (ImGui::Button("Save")) {
        if (dirty) {
            apply_display_to_config();
            dirty = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Close")) {
        display_inited = false;   // reload from Configuration next open
        if (_Universe) {
            _Universe->ToggleOptionsActive();   // close the overlay; hide cursor on inactive
        }
    }

    ImGui::End();
}

} // namespace vs_settings_ng
