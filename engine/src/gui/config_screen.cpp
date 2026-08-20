// config_screen.cpp — in-game configuration screen for vs-settings-ng.
//
// Port of the vs-05 modern UI Display frame into the engine, reading/writing
// the engine's Configuration object directly. Drawn inside the in-game ImGui
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
#include <vector>
#include <string>
#include <cstdio>

namespace vs_settings_ng {

namespace {
// Current display frame state (loaded from Configuration on open).
bool full_screen = false;
int  res_x = 1280, res_y = 960;
int  font_point = 16;
bool font_aa = false;
bool high_quality_font = false;
// Dirtiness: any unsaved change.
bool dirty = false;

// Static buffer for the font-point text (decimal input).
char font_point_buf[8] = "16";
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

    // Load current values from Configuration once (first frame).
    static bool loaded = false;
    if (!loaded) {
        const auto& g = configuration().graphics;
        full_screen = g.full_screen;
        res_x = g.resolution_x;
        res_y = g.resolution_y;
        font_point = (int)g.font_point_flt;
        font_aa = g.font_antialias;
        high_quality_font = g.high_quality_font;
        snprintf(font_point_buf, sizeof(font_point_buf), "%d", font_point);
        loaded = true;
        dirty = false;
    }

    // Fullscreen toggle.
    if (ImGui::Checkbox("Fullscreen", &full_screen)) dirty = true;

    // Resolution inputs.
    if (ImGui::InputInt("Resolution X", &res_x)) dirty = true;
    if (ImGui::InputInt("Resolution Y", &res_y)) dirty = true;

    // Font point size.
    if (ImGui::InputText("Font point", font_point_buf, sizeof(font_point_buf),
                         ImGuiInputTextFlags_CharsDecimal)) dirty = true;

    // Font quality toggles.
    if (ImGui::Checkbox("Font antialias", &font_aa)) dirty = true;
    if (ImGui::Checkbox("High quality font", &high_quality_font)) dirty = true;

    ImGui::Separator();
    ImGui::TextUnformatted(dirty ? "(unsaved changes)" : "(saved)");

    // Apply/save: parse the staged values back into Configuration (write-back to
    // files and live re-apply are deferred).
    if (ImGui::Button("Save")) {
        if (dirty) {
            auto& g = const_cast<vega_config::Configuration&>(configuration()).graphics;
            g.full_screen = full_screen;
            g.resolution_x = res_x;
            g.resolution_y = res_y;
            g.font_point_flt = (float)atoi(font_point_buf);
            g.font_antialias = font_aa;
            g.high_quality_font = high_quality_font;
            dirty = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Close")) {
        loaded = false;   // reload from Configuration next open
        if (_Universe) {
            _Universe->ToggleOptionsActive();   // close the overlay; hide cursor on inactive
        }
    }

    ImGui::End();
}

} // namespace vs_settings_ng
