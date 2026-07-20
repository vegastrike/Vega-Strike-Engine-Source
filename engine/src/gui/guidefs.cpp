/*
 * guidefs.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Mike Byron
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

#include "guidefs.h"
#include "src/vegastrike.h"
#include "root_generic/xml_support.h"
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "guidefs.h"

#include "configuration/configuration.h"
#include "gldrv/mouse_cursor.h"
#include "imgui_internal.h"

// Point
bool Point::operator==(const Point &other) {
    return x == other.x && y == other.y;
}

bool Point::operator!=(const Point &other) {
    return !(*this == other);
}

Point::Point() : x(0.0), y(0.0) {
}

Point::Point(float cx, float cy) :
        x(cx), y(cy) {
}

// Size
bool Size::operator==(const Size &other) {
    return width == other.width && height == other.height;
}

bool Size::operator!=(const Size &other) {
    return !(*this == other);
}

// Constructors
Size::Size() : width(0.0), height(0.0) {
}

Size::Size(float cwidth, float cheight) :
        width(cwidth), height(cheight) {
}

// Rect
// Constructors
Rect::Rect() {
}

Rect::Rect(Point &p, Size &s) :
        origin(p), size(s) {
}

Rect::Rect(float x, float y, float width, float height) :
        origin(x, y), size(width, height) {
}

float Rect::left(void) const {
    return origin.x;
}

float Rect::right(void) const {
    return origin.x + size.width;
}

float Rect::bottom(void) const {
    return origin.y;
}

float Rect::top(void) const {
    return origin.y + size.height;
}

//The center of this rectangle.
Point Rect::center(void) const {
    return Point(origin.x + size.width / 2, origin.y + size.height / 2);
}

//Whether a Point is inside this Rect.
bool Rect::inside(const Point &p) const {
    return p.x >= left() && p.x < right() && p.y >= bottom() && p.y < top();
}

//Make a new Rect that is inset by the specified margins.
void Rect::inset(const Size &s) {
    origin.x += s.width;
    origin.y += s.height;
    size.width -= s.width * 2;
    size.height -= s.height * 2;
}

//Return a copy of the rect inset by specified margins.
Rect Rect::copyAndInset(const Size &s) {
    Rect result = *this;
    result.inset(s);
    return result;
}

// Operators
bool Rect::operator==(const Rect &other) {
    return origin == other.origin && size == other.size;
}

bool Rect::operator!=(const Rect &other) {
    return !(*this == other);
}




// Other


GFXColor SaturatedColor(float r, float g, float b, float a) {
    const float Saturation = configuration().graphics.base_saturation_flt;

    return GFXColor((r * Saturation * 3 + (r + b + g) * (1 - Saturation)) / 3,
            (g * Saturation * 3 + (r + b + g) * (1 - Saturation)) / 3,
            (b * Saturation * 3 + (r + b + g) * (1 - Saturation)) / 3, a);
}

GFXColor GUI_OPAQUE_BLACK() {
    static GFXColor gui_black = vs_config->getColor("base_black", GFXColor(0, 0, 0, 1));
    return gui_black;
}

GFXColor GUI_OPAQUE_WHITE() {
    static GFXColor gui_white = vs_config->getColor("base_white", GFXColor(1, 1, 1, 1));
    return gui_white;
}

GFXColor GUI_OPAQUE_LIGHT_GRAY() {
    static GFXColor gui_light_gray = vs_config->getColor("base_light_gray", GFXColor(.25, .25, .25, 1));
    return gui_light_gray;
}

GFXColor GUI_OPAQUE_MEDIUM_GRAY() {
    static GFXColor gui_gray = vs_config->getColor("base_gray", GFXColor(.5, .5, .5, 1));
    return gui_gray;
}

GFXColor GUI_OPAQUE_DARK_GRAY() {
    static GFXColor gui_dark_gray = vs_config->getColor("base_dark_gray", GFXColor(.75, .75, .75, 1));
    return gui_dark_gray;
}

//Draw a rectangle using the specified color.
void drawRect(const Rect &rect, const GFXColor &color) {
    // Prevent segfault if called outside an ImGui window
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return; 
    }
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (!draw_list) return;

    float pMinX = NORM_TO_PIXEL_X(rect.left());
    float pMinY = NORM_TO_PIXEL_Y(rect.top());
    float pMaxX = NORM_TO_PIXEL_X(rect.right());
    float pMaxY = NORM_TO_PIXEL_Y(rect.bottom());

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));
    draw_list->AddRectFilled(ImVec2(pMinX, pMinY), ImVec2(pMaxX, pMaxY), col);
}

//Draw the outline of a rectangle using the specified color.
void drawRectOutline(const Rect &rect, const GFXColor &color, float lineWidth) {
    // Prevent segfault if called outside an ImGui window
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return; 
    }
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (!draw_list) return;

    float pMinX = NORM_TO_PIXEL_X(rect.left());
    float pMinY = NORM_TO_PIXEL_Y(rect.top());
    float pMaxX = NORM_TO_PIXEL_X(rect.right());
    float pMaxY = NORM_TO_PIXEL_Y(rect.bottom());

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));
    // ImGui's line thickness scaling can use lineWidth directly
    draw_list->AddRect(ImVec2(pMinX, pMinY), ImVec2(pMaxX, pMaxY), col, 0.0f, 0, lineWidth);
}

//Draw upper-left part of rectangle's "shadow".
void drawUpLeftShadow(const Rect &rect, const GFXColor &color, float lineWidth) {
    // Prevent segfault if called outside an ImGui window
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return; 
    }
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (!draw_list) return;

    float x1 = NORM_TO_PIXEL_X(rect.origin.x);
    float y1 = NORM_TO_PIXEL_Y(rect.origin.y);
    float x2 = NORM_TO_PIXEL_X(rect.origin.x);
    float y2 = NORM_TO_PIXEL_Y(rect.origin.y + rect.size.height);
    float x3 = NORM_TO_PIXEL_X(rect.origin.x + rect.size.width);
    float y3 = NORM_TO_PIXEL_Y(rect.origin.y + rect.size.height);

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));
    draw_list->AddPolyline(
        std::vector<ImVec2>{ {x1, y1}, {x2, y2}, {x3, y3} }.data(), 
        3, col, 0, lineWidth
    );
}

//Draw lower-right part of rectangle's "shadow".
void drawLowRightShadow(const Rect &rect, const GFXColor &color, float lineWidth) {
    // Prevent segfault if called outside an ImGui window
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return; 
    }
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (!draw_list) return;

    float x1 = NORM_TO_PIXEL_X(rect.origin.x);
    float y1 = NORM_TO_PIXEL_Y(rect.origin.y);
    float x2 = NORM_TO_PIXEL_X(rect.origin.x + rect.size.width);
    float y2 = NORM_TO_PIXEL_Y(rect.origin.y);
    float x3 = NORM_TO_PIXEL_X(rect.origin.x + rect.size.width);
    float y3 = NORM_TO_PIXEL_Y(rect.origin.y + rect.size.height);

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));
    draw_list->AddPolyline(
        std::vector<ImVec2>{ {x1, y1}, {x2, y2}, {x3, y3} }.data(), 
        3, col, 0, lineWidth
    );
}

//Fill a closed polygon.
void drawFilledPolygon(const std::vector<Point> &coords, const GFXColor &color) {
    // Prevent segfault if called outside an ImGui window
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return; 
    }
    if (coords.empty()) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (!draw_list) return;

    // Convert normalized Vega Strike Points to pixel-space ImVec2 objects
    std::vector<ImVec2> pixels(coords.size());
    for (size_t i = 0; i < coords.size(); ++i) {
        pixels[i] = ImVec2(
            NORM_TO_PIXEL_X(coords[i].x),
            NORM_TO_PIXEL_Y(coords[i].y)
        );
    }

    ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));

    // ImGui handles filled convex polygons natively
    draw_list->AddConvexPolyFilled(pixels.data(), static_cast<int>(pixels.size()), col);
}

