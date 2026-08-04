/*
 * navitemstodraw.h
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
// NO HEADER GUARD
#include "imgui/imgui.h"
#include <cmath>

static constexpr float NAV_LINE_WEIGHT = 2.0f; // Adjust as needed

// Helper to convert GFXColor/GFXColorf to ImU32 packed color
static inline ImU32 ToImColor(const GFXColor &col) {
    return IM_COL32(
        static_cast<int>(col.r * 255.0f),
        static_cast<int>(col.g * 255.0f),
        static_cast<int>(col.b * 255.0f),
        static_cast<int>(col.a * 255.0f)
    );
}

// Converts normalized screen coordinates to ImGui pixel screen coordinates
static inline ImVec2 NormToPixel(float x, float y) {
    return ImVec2(
        static_cast<float>(Coordinates::normToPixelX(x)),
        static_cast<float>(Coordinates::normToPixelY(y))
    );
}

// Helper to retrieve current ImGui draw list (Background layer)
static inline ImDrawList* GetNavDrawList() {
    return ImGui::GetBackgroundDrawList();
}

//**********************************
// Draws a 10x10 coordinate grid over the nav screen area
//**********************************
void NavigationSystem::DrawGrid(float &x1, float &x2, float &y1, float &y2, const GFXColor &col) {
    if (!configuration().graphics.hud.draw_nav_grid) {
        return;
    }

    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float deltax = (x2 - x1) / 10.0f;
    float deltay = (y2 - y1) / 10.0f;

    // Vertical grid lines
    for (int i = 1; i < 10; i++) {
        float x = x1 + i * deltax;
        drawList->AddLine(NormToPixel(x, y1), NormToPixel(x, y2), color, 1.0f);
    }

    // Horizontal grid lines
    for (int i = 1; i < 10; i++) {
        float y = y1 + i * deltay;
        drawList->AddLine(NormToPixel(x1, y), NormToPixel(x2, y), color, 1.0f);
    }
}

//**********************************
// Draws a circle icon centered at (x, y)
//**********************************
void NavigationSystem::DrawCircle(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float pixelRadius = Coordinates::normToPixelW(0.5f * size);

    drawList->AddCircle(NormToPixel(x, y), pixelRadius, color, 0, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a half circle centered at top 1/4 center
//**********************************
void NavigationSystem::DrawHalfCircleTop(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float radiusPx = Coordinates::normToPixelW(0.5f * size);
    ImVec2 center = NormToPixel(x, y);
    center.y -= radiusPx * 0.5f; // Offset in pixel space

    drawList->PathClear();
    // PathArcTo(center, radius, a_min, a_max, num_segments)
    // 0 to PI draws the arc
    drawList->PathArcTo(center, radiusPx, 0.0f, static_cast<float>(M_PI), 16);
    drawList->PathStroke(color, 0, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a half circle centered at bottom 1/4 center
//**********************************
void NavigationSystem::DrawHalfCircleBottom(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float radiusPx = Coordinates::normToPixelW(0.5f * size);
    ImVec2 center = NormToPixel(x, y);
    center.y += radiusPx * 0.5f; // Offset in pixel space

    drawList->PathClear();
    // PI to 2*PI draws the opposite arc
    drawList->PathArcTo(center, radiusPx, static_cast<float>(M_PI), static_cast<float>(M_PI * 2.0), 16);
    drawList->PathStroke(color, 0, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a planet icon (Circle with central axis lines)
//**********************************
void NavigationSystem::DrawPlanet(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    ImVec2 c = NormToPixel(x, y);
    float r = Coordinates::normToPixelW(0.5f * size);

    // Outer circle
    drawList->AddCircle(c, r, color, 0, NAV_LINE_WEIGHT);

    // Planet interior lines (all calculated in pure pixel space relative to center)
    drawList->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x, c.y + r * 0.4f), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x, c.y + r * 0.4f), ImVec2(c.x, c.y - r * 0.4f), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x, c.y - r * 0.4f), ImVec2(c.x + r, c.y), color, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a station icon (Square 3x3 grid pattern)
//**********************************
void NavigationSystem::DrawStation(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    ImVec2 c = NormToPixel(x, y);
    float halfSizePx = Coordinates::normToPixelW(0.5f * size);
    float segmentPx = (halfSizePx * 2.0f) / 3.0f;

    float startX = c.x - halfSizePx;
    float startY = c.y - halfSizePx;
    float endX   = c.x + halfSizePx;
    float endY   = c.y + halfSizePx;

    // Horizontal grid lines
    for (int i = 0; i < 4; i++) {
        float curY = startY + i * segmentPx;
        drawList->AddLine(ImVec2(startX, curY), ImVec2(endX, curY), color, NAV_LINE_WEIGHT);
    }

    // Vertical grid lines
    for (int i = 0; i < 4; i++) {
        float curX = startX + i * segmentPx;
        drawList->AddLine(ImVec2(curX, startY), ImVec2(curX, endY), color, NAV_LINE_WEIGHT);
    }
}

//**********************************
// Draws a jump node icon (Circle with cardinal arrow indicators)
//**********************************
void NavigationSystem::DrawJump(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    ImVec2 c = NormToPixel(x, y);
    float r = Coordinates::normToPixelW(0.5f * size);
    float off = r * 0.25f; // Offset relative to radius in pixel space

    // Outer circle
    drawList->AddCircle(c, r, color, 0, NAV_LINE_WEIGHT);

    // Cardinal arrows in pixel space
    // Top
    drawList->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x + off, c.y - off), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x - off, c.y - off), color, NAV_LINE_WEIGHT);
    // Bottom
    drawList->AddLine(ImVec2(c.x, c.y + r), ImVec2(c.x + off, c.y + off), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x, c.y + r), ImVec2(c.x - off, c.y + off), color, NAV_LINE_WEIGHT);
    // Left
    drawList->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x - off, c.y + off), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x - off, c.y - off), color, NAV_LINE_WEIGHT);
    // Right
    drawList->AddLine(ImVec2(c.x + r, c.y), ImVec2(c.x + off, c.y + off), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x + r, c.y), ImVec2(c.x + off, c.y - off), color, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a missile warning/target icon
//**********************************
void NavigationSystem::DrawMissile(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    ImVec2 c = NormToPixel(x, y);
    float s = Coordinates::normToPixelW(size);

    drawList->AddLine(ImVec2(c.x - 0.5f * s, c.y + 0.125f * s), ImVec2(c.x, c.y - 0.375f * s), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x + 0.5f * s, c.y + 0.125f * s), ImVec2(c.x, c.y - 0.375f * s), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x - 0.25f * s, c.y + 0.125f * s), ImVec2(c.x - 0.25f * s, c.y - 0.125f * s), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x + 0.25f * s, c.y + 0.125f * s), ImVec2(c.x + 0.25f * s, c.y - 0.125f * s), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x - 0.25f * s, c.y - 0.125f * s), ImVec2(c.x, c.y + 0.125f * s), color, NAV_LINE_WEIGHT);
    drawList->AddLine(ImVec2(c.x + 0.25f * s, c.y - 0.125f * s), ImVec2(c.x, c.y + 0.125f * s), color, NAV_LINE_WEIGHT);
}

//**********************************
// Draws corner brackets for targeting frames
//**********************************
void NavigationSystem::DrawTargetCorners(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    ImVec2 c = NormToPixel(x, y);
    float half  = Coordinates::normToPixelW(0.5f * size);
    float inner = Coordinates::normToPixelW(0.3f * size);

    // Top-Left corner
    drawList->AddLine(ImVec2(c.x - half, c.y - half), ImVec2(c.x - inner, c.y - half), color, NAV_LINE_WEIGHT * 2);
    drawList->AddLine(ImVec2(c.x - half, c.y - half), ImVec2(c.x - half, c.y - inner), color, NAV_LINE_WEIGHT * 2);

    // Top-Right corner
    drawList->AddLine(ImVec2(c.x + half, c.y - half), ImVec2(c.x + inner, c.y - half), color, NAV_LINE_WEIGHT * 2);
    drawList->AddLine(ImVec2(c.x + half, c.y - half), ImVec2(c.x + half, c.y - inner), color, NAV_LINE_WEIGHT * 2);

    // Bottom-Left corner
    drawList->AddLine(ImVec2(c.x - half, c.y + half), ImVec2(c.x - inner, c.y + half), color, NAV_LINE_WEIGHT * 2);
    drawList->AddLine(ImVec2(c.x - half, c.y + half), ImVec2(c.x - half, c.y + inner), color, NAV_LINE_WEIGHT * 2);

    // Bottom-Right corner
    drawList->AddLine(ImVec2(c.x + half, c.y + half), ImVec2(c.x + inner, c.y + half), color, NAV_LINE_WEIGHT * 2);
    drawList->AddLine(ImVec2(c.x + half, c.y + half), ImVec2(c.x + half, c.y + inner), color, NAV_LINE_WEIGHT * 2);
}

//**********************************
// Draws a 3D projected/oriented navigation circle grid (Accurate 3D perspective)
//**********************************
void NavigationSystem::DrawNavCircle(float x, float y, float size, float rot_x, float rot_y, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();

    constexpr int circles = 4;
    constexpr int segments = 20;
    constexpr int segments2 = 12;
    constexpr float TWO_PI = 2.0f * static_cast<float>(M_PI);

    // 1. Concentric web circles in true 3D projection
    for (int i = 0; i < segments; ++i) {
        float angle1 = i * (TWO_PI / segments);
        float angle2 = (i + 1) * (TWO_PI / segments);

        GFXColor ci(col.r, col.g, col.b * std::fabs(std::sin(angle1 / 2.0f)), col.a);
        ImU32 segmentColor = ToImColor(ci);

        QVector pos1(0.6 * size * std::cos(angle1), 0.6 * size * std::sin(angle1), 0.0);
        QVector pos2(0.6 * size * std::cos(angle2), 0.6 * size * std::sin(angle2), 0.0);

        pos1 = dxyz(pos1, 0, 0, rot_y);
        pos1 = dxyz(pos1, rot_x, 0, 0);
        pos2 = dxyz(pos2, 0, 0, rot_y);
        pos2 = dxyz(pos2, rot_x, 0, 0);

        float standard_unit = 0.25f * 1.2f * size;
        float zdistance1 = (1.2f * size) - pos1.k;
        float zdistance2 = (1.2f * size) - pos2.k;
        float zscale1 = standard_unit / zdistance1;
        float zscale2 = standard_unit / zdistance2;

        pos1 *= (zscale1 * 5.0f);
        pos2 *= (zscale2 * 5.0f);

        for (int j = circles; j > 0; j--) {
            float scale = static_cast<float>(j) / static_cast<float>(circles);
            QVector p1 = pos1 * scale;
            QVector p2 = pos2 * scale;

            drawList->AddLine(
                NormToPixel(x + p1.i, y + p1.j),
                NormToPixel(x + p2.i, y + p2.j),
                segmentColor, NAV_LINE_WEIGHT
            );
        }
    }

    // 2. Radial spoke lines in true 3D projection
    for (int i = 0; i < segments2; ++i) {
        float angle = i * (TWO_PI / segments2);

        GFXColor ci(col.r, col.g, col.b * std::fabs(std::sin(angle / 2.0f)), col.a);
        ImU32 spokeColor = ToImColor(ci);

        QVector pos1(0.6 * size * std::cos(angle) / (circles * 2), 0.6 * size * std::sin(angle) / (circles * 2), 0.0);
        QVector pos2(0.6 * size * std::cos(angle), 0.6 * size * std::sin(angle), 0.0);

        if ((std::fabs(angle - 1.57f) < 0.01f) || (std::fabs(angle - 3.14f) < 0.01f) || 
            (std::fabs(angle - 4.71f) < 0.01f) || (angle < 0.01f)) {
            pos2 *= 1.1f;
        }

        pos1 = dxyz(pos1, 0, 0, rot_y);
        pos1 = dxyz(pos1, rot_x, 0, 0);
        pos2 = dxyz(pos2, 0, 0, rot_y);
        pos2 = dxyz(pos2, rot_x, 0, 0);

        float standard_unit = 0.25f * 1.2f * size;
        float zdistance1 = (1.2f * size) - pos1.k;
        float zdistance2 = (1.2f * size) - pos2.k;
        float zscale1 = standard_unit / zdistance1;
        float zscale2 = standard_unit / zdistance2;

        pos1 *= (zscale1 * 5.0f);
        pos2 *= (zscale2 * 5.0f);

        drawList->AddLine(
            NormToPixel(x + pos1.i, y + pos1.j),
            NormToPixel(x + pos2.i, y + pos2.j),
            spokeColor, NAV_LINE_WEIGHT
        );
    }
}