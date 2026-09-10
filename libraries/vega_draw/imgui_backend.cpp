/*
 * imgui_backend.cpp
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
#include "vega_draw/imgui_backend.h"

#include <cfloat>

namespace vega_draw {

Viewport DisplayViewport() {
    const ImVec2 size = ImGui::GetIO().DisplaySize;
    return Viewport{size.x, size.y};
}

TextMetrics ImGuiTextMeasurer::Measure(const std::string &text, float font_px) const {
    TextMetrics metrics;
    ImFont *font = ImGui::GetFont();
    if (font != nullptr && font->IsLoaded()) {
        // Line height comes from a representative string; empty text measures to
        // zero width, which would also lose the height.
        const char *measure = text.empty() ? "Ag" : text.c_str();
        const ImVec2 size = font->CalcTextSizeA(font_px, FLT_MAX, 0.0f, measure);
        metrics.width = text.empty() ? 0.0f : size.x;
        metrics.height = size.y;
    } else {
        const ImVec2 size = ImGui::CalcTextSize(text.c_str());
        metrics.width = text.empty() ? 0.0f : size.x;
        metrics.height = size.y;
    }
    return metrics;
}

ImU32 ToImU32(const Color &color, ImU32 fallback) {
    if (!color.set) {
        return fallback;
    }
    return IM_COL32(color.r, color.g, color.b, color.a);
}

void DrawTextLayout(ImDrawList *draw_list,
                    const ImVec2 &origin,
                    const TextLayout &layout,
                    float font_px,
                    ImU32 default_color,
                    const ImVec4 *clip_rect,
                    int first_line) {
    if (draw_list == nullptr) {
        return;
    }
    const int start = (first_line < 0) ? 0 : first_line;
    for (std::size_t li = static_cast<std::size_t>(start); li < layout.lines.size(); ++li) {
        const LaidOutLine &line = layout.lines[li];
        const float y = origin.y + line.y;
        for (std::size_t ri = 0; ri < line.runs.size(); ++ri) {
            const LaidOutRun &run = line.runs[ri];
            if (run.text.empty()) {
                continue;
            }
            const ImU32 color = ToImU32(run.style.color, default_color);
            const ImVec2 pos(origin.x + run.x, y);
            // The run's stroke weight is carried but not yet rendered differently
            // (single-weight atlas today); faking bold with an offset shadow is
            // deliberately avoided. IsBoldWeight() is available when a backend
            // starts honouring it.
            draw_list->AddText(nullptr, font_px, pos, color, run.text.c_str(), nullptr, 0.0f, clip_rect);
        }
    }
}

float DrawText(ImDrawList *draw_list,
               const Viewport &viewport,
               float grid_x,
               float grid_y,
               float font_grid,
               const std::string &markup,
               ImU32 default_color,
               const ImVec4 *clip_rect) {
    const float font_px = FontGridToPixel(font_grid, viewport);
    const ImGuiTextMeasurer measurer;
    const TextLines parsed = ParseMarkup(markup);

    TextStyle style;
    style.font_px = font_px;
    // The primitive does not wrap; explicit <br> breaks still produce lines.

    const TextLayout layout = LayoutText(parsed, style, measurer);
    const ImVec2 origin(GridToPixelX(grid_x, viewport), GridToPixelY(grid_y, viewport));
    DrawTextLayout(draw_list, origin, layout, font_px, default_color, clip_rect);
    return layout.height;
}

void DrawTextBox(ImDrawList *draw_list,
                 const TextBox &box,
                 const std::string &markup,
                 const Viewport &viewport,
                 ImU32 default_color) {
    if (draw_list == nullptr) {
        return;
    }
    const ImGuiTextMeasurer measurer;
    const TextBoxLayout laid_out = LayoutTextBox(box, markup, viewport, measurer);
    const BoxRect inner = TextBoxInnerRectPx(box, viewport);
    const ImVec4 clip(inner.x, inner.y, inner.x + inner.width, inner.y + inner.height);
    const ImVec2 origin(inner.x, inner.y);
    DrawTextLayout(draw_list, origin, laid_out.layout, laid_out.font_px, default_color, &clip, box.scroll_start_line);
}

void DrawPicker(ImDrawList *draw_list,
                const PickerStyle &style,
                const std::vector<PickerRow> &rows,
                const Viewport &viewport,
                const PickerColors &colors,
                int selected_index,
                int highlighted_index,
                float scroll_px) {
    if (draw_list == nullptr) {
        return;
    }
    const ImGuiTextMeasurer measurer;
    const PickerLayout layout = LayoutPicker(style, rows, viewport, measurer, scroll_px);
    const ImVec2 clip_min(layout.viewport_x, layout.viewport_y);
    const ImVec2 clip_max(layout.viewport_x + layout.viewport_width, layout.viewport_y + layout.viewport_height);
    const ImVec4 clip(clip_min.x, clip_min.y, clip_max.x, clip_max.y);
    const float font_px = FontGridToPixel(style.font_grid, viewport);

    draw_list->PushClipRect(clip_min, clip_max, true);
    for (std::size_t i = 0; i < layout.visible_rows.size(); ++i) {
        const PickerRowLayout &row = layout.visible_rows[i];
        const float row_top = layout.viewport_y + row.y - layout.scroll_px;
        const float row_bottom = row_top + row.height;

        const bool selected = (row.index == selected_index);
        const bool highlighted = (row.index == highlighted_index);
        ImU32 background = colors.background;
        ImU32 text_color = colors.text;
        if (selected) {
            background = colors.selection_background;
            text_color = colors.selection_text;
        } else if (highlighted) {
            background = colors.highlight_background;
            text_color = colors.highlight_text;
        }

        const bool opaque = ((background >> IM_COL32_A_SHIFT) & 0xFF) != 0;
        if (opaque) {
            draw_list->AddRectFilled(ImVec2(clip_min.x, row_top), ImVec2(clip_max.x, row_bottom), background);
        }

        // A row's explicit colour overrides the per-state default. The row colour
        // is stored on the row; the laid-out text carries it too, so pass the state
        // colour as the fallback.
        const ImU32 row_default = ToImU32(row.text_color, text_color);
        const ImVec2 origin(layout.viewport_x + row.indent_px, row_top + row.padding_px);
        DrawTextLayout(draw_list, origin, row.text, font_px, row_default, &clip);
    }
    draw_list->PopClipRect();
}

void DrawRectFill(ImDrawList *draw_list, const Rect &grid_rect, const Viewport &viewport, ImU32 color) {
    if (draw_list == nullptr) {
        return;
    }
    const Rect rect = GridToPixelRect(grid_rect, viewport);
    draw_list->AddRectFilled(ImVec2(rect.left(), rect.top()), ImVec2(rect.right(), rect.bottom()), color);
}

void DrawRectOutline(ImDrawList *draw_list,
                     const Rect &grid_rect,
                     const Viewport &viewport,
                     ImU32 color,
                     float thickness_px) {
    if (draw_list == nullptr) {
        return;
    }
    const Rect rect = GridToPixelRect(grid_rect, viewport);
    draw_list->AddRect(ImVec2(rect.left(), rect.top()),
                       ImVec2(rect.right(), rect.bottom()), color, 0.0f, 0, thickness_px);
}

void DrawUpLeftShadow(ImDrawList *draw_list,
                      const Rect &grid_rect,
                      const Viewport &viewport,
                      ImU32 color,
                      float thickness_px) {
    if (draw_list == nullptr) {
        return;
    }
    const Rect r = GridToPixelRect(grid_rect, viewport);
    const ImVec2 points[3] = {
        {r.right(), r.top()}, {r.left(), r.top()}, {r.left(), r.bottom()},
    };
    draw_list->AddPolyline(points, 3, color, 0, thickness_px);
}

void DrawLowRightShadow(ImDrawList *draw_list,
                        const Rect &grid_rect,
                        const Viewport &viewport,
                        ImU32 color,
                        float thickness_px) {
    if (draw_list == nullptr) {
        return;
    }
    const Rect r = GridToPixelRect(grid_rect, viewport);
    const ImVec2 points[3] = {
        {r.left(), r.bottom()}, {r.right(), r.bottom()}, {r.right(), r.top()},
    };
    draw_list->AddPolyline(points, 3, color, 0, thickness_px);
}

void DrawScroller(ImDrawList *draw_list,
                  const ScrollerStyle &style,
                  const ScrollerModel &model,
                  const Viewport &viewport,
                  const ScrollerColors &colors) {
    if (draw_list == nullptr) {
        return;
    }
    const Rect track = GridToPixelRect(style.track, viewport);
    draw_list->AddRectFilled(ImVec2(track.left(), track.top()),
                             ImVec2(track.right(), track.bottom()), colors.track);
    const Rect thumb = GridToPixelRect(ScrollerThumbRect(style, model), viewport);
    draw_list->AddRectFilled(ImVec2(thumb.left(), thumb.top()),
                             ImVec2(thumb.right(), thumb.bottom()), colors.thumb);
}

} // namespace vega_draw
