/*
 * imgui_backend.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_IMGUI_BACKEND_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_IMGUI_BACKEND_H

#include <string>

#include "imgui/imgui.h"

#include "vega_draw/markup.h"
#include "vega_draw/text_box.h"
#include "vega_draw/text_layout.h"
#include "vega_draw/units.h"

// The ImGui adapter for the renderer-agnostic text core. This is the only part
// of vega_draw that depends on ImGui; the parser and layout stay ImGui-free.
// It requires an active ImGui context/font (i.e. call it inside a frame).
namespace vega_draw {

// A TextMeasurer backed by the current ImGui font. Measure("") returns the line
// height (taken from a representative string, since empty text has no height).
class ImGuiTextMeasurer : public TextMeasurer {
public:
    TextMetrics Measure(const std::string &text, float font_px) const override;
};

// Convert a vega_draw::Color to an ImGui packed colour. When the colour has no
// explicit tag (set == false), `fallback` is returned instead.
ImU32 ToImU32(const Color &color, ImU32 fallback);

// Draw a laid-out text block. `origin` is the layout's top-left in pixels; each
// run is drawn at origin + (run.x, line.y). `default_color` applies where a run
// carries no explicit colour. `clip_rect`, when non-null, is an ImGui
// pixel-space clip rectangle. Lines before `first_line` are skipped (scrolling).
void DrawTextLayout(ImDrawList *draw_list,
                    const ImVec2 &origin,
                    const TextLayout &layout,
                    float font_px,
                    ImU32 default_color,
                    const ImVec4 *clip_rect = nullptr,
                    int first_line = 0);

// Text primitive: parse `markup` and draw it at a 1000-grid position with a
// per-1000 font size (a fraction of the space height). Returns the drawn height
// in pixels. Embedded <br> breaks produce multiple lines; no word wrap (that is
// the text box's job).
float DrawText(ImDrawList *draw_list,
               const Viewport &viewport,
               float grid_x,
               float grid_y,
               float font_grid,
               const std::string &markup,
               ImU32 default_color,
               const ImVec4 *clip_rect = nullptr);

// Draw a text box: lay out `markup` inside the box (autofit/wrap/margins applied)
// and draw it, clipped to the inner region and starting at the box's scroll line.
// Draws no background (the caller owns any panel background).
void DrawTextBox(ImDrawList *draw_list,
                 const TextBox &box,
                 const std::string &markup,
                 const Viewport &viewport,
                 ImU32 default_color);

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_IMGUI_BACKEND_H
