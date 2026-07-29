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
        drawList->AddLine(
            NormToPixel(x, y1),
            NormToPixel(x, y2),
            color, 1.0f
        );
    }

    // Horizontal grid lines
    for (int i = 1; i < 10; i++) {
        float y = y1 + i * deltay;
        drawList->AddLine(
            NormToPixel(x1, y),
            NormToPixel(x2, y),
            color, 1.0f
        );
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

    const int segments = 10;
    float radius = 0.5f * size;
    float yOffset = y - 0.25f * size;

    drawList->PathClear();
    for (int i = 0; i <= segments; ++i) {
        float angle = i * (static_cast<float>(M_PI) / segments);
        float px = x + radius * std::cos(angle);
        float py = yOffset + radius * std::sin(angle);
        drawList->PathLineTo(NormToPixel(px, py));
    }
    drawList->PathStroke(color, 0, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a half circle centered at bottom 1/4 center
//**********************************
void NavigationSystem::DrawHalfCircleBottom(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    const int segments = 10;
    float radius = 0.5f * size;
    float yOffset = y + 0.25f * size;

    drawList->PathClear();
    for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(M_PI) + i * (static_cast<float>(M_PI) / segments);
        float px = x + radius * std::cos(angle);
        float py = yOffset + radius * std::sin(angle);
        drawList->PathLineTo(NormToPixel(px, py));
    }
    drawList->PathStroke(color, 0, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a planet icon (Circle with central axis lines)
//**********************************
void NavigationSystem::DrawPlanet(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    // Outer circle
    float radius = 0.5f * size;
    DrawCircle(x ,y, size, col);

    // Planet interior lines
    drawList->AddLine(NormToPixel(x - radius, y), NormToPixel(x, y + 0.2f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x, y + 0.2f * size), NormToPixel(x, y - 0.2f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x, y - 0.2f * size), NormToPixel(x + radius, y), color, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a station icon (3x3 grid pattern)
//**********************************
void NavigationSystem::DrawStation(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float segment = size / 3.0f;
    float startX = x - 0.5f * size;
    float startY = y - 0.5f * size;
    float endX = x + 0.5f * size;
    float endY = y + 0.5f * size;

    // Horizontal lines
    for (int i = 0; i < 4; i++) {
        float currentY = startY + i * segment;
        drawList->AddLine(NormToPixel(startX, currentY), NormToPixel(endX, currentY), color, NAV_LINE_WEIGHT);
    }

    // Vertical lines
    for (int i = 0; i < 4; i++) {
        float currentX = startX + i * segment;
        drawList->AddLine(NormToPixel(currentX, startY), NormToPixel(currentX, endY), color, NAV_LINE_WEIGHT);
    }
}

//**********************************
// Draws a jump node icon (Circle with cardinal arrow indicators)
//**********************************
void NavigationSystem::DrawJump(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float radius = 0.5f * size;
    float offset = 0.125f * size;

    // Outer circle
    DrawCircle(x ,y, size, col);

    // Cardinal arrows
    drawList->AddLine(NormToPixel(x, y + radius), NormToPixel(x + offset, y + offset), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x, y + radius), NormToPixel(x - offset, y + offset), color, NAV_LINE_WEIGHT);

    drawList->AddLine(NormToPixel(x, y - radius), NormToPixel(x + offset, y - offset), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x, y - radius), NormToPixel(x - offset, y - offset), color, NAV_LINE_WEIGHT);

    drawList->AddLine(NormToPixel(x - radius, y), NormToPixel(x - offset, y + offset), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x - radius, y), NormToPixel(x - offset, y - offset), color, NAV_LINE_WEIGHT);

    drawList->AddLine(NormToPixel(x + radius, y), NormToPixel(x + offset, y + offset), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x + radius, y), NormToPixel(x + offset, y - offset), color, NAV_LINE_WEIGHT);
}

//**********************************
// Draws a missile warning/target icon
//**********************************
void NavigationSystem::DrawMissile(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    drawList->AddLine(NormToPixel(x - 0.5f * size, y - 0.125f * size), NormToPixel(x, y + 0.375f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x + 0.5f * size, y - 0.125f * size), NormToPixel(x, y + 0.375f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x - 0.25f * size, y - 0.125f * size), NormToPixel(x - 0.25f * size, y + 0.125f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x + 0.25f * size, y - 0.125f * size), NormToPixel(x + 0.25f * size, y + 0.125f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x - 0.25f * size, y + 0.125f * size), NormToPixel(x, y - 0.125f * size), color, NAV_LINE_WEIGHT);
    drawList->AddLine(NormToPixel(x + 0.25f * size, y + 0.125f * size), NormToPixel(x, y - 0.125f * size), color, NAV_LINE_WEIGHT);
}

//**********************************
// Draws corner brackets for targeting frames
//**********************************
void NavigationSystem::DrawTargetCorners(float x, float y, float size, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();
    ImU32 color = ToImColor(col);

    float half = 0.5f * size;
    float inner = 0.3f * size;

    // Top-Left corner
    drawList->AddLine(NormToPixel(x - half, y + half), NormToPixel(x - inner, y + half), color, NAV_LINE_WEIGHT*2);
    drawList->AddLine(NormToPixel(x - half, y + half), NormToPixel(x - half, y + inner), color, NAV_LINE_WEIGHT*2);

    // Top-Right corner
    drawList->AddLine(NormToPixel(x + half, y + half), NormToPixel(x + inner, y + half), color, NAV_LINE_WEIGHT*2);
    drawList->AddLine(NormToPixel(x + half, y + half), NormToPixel(x + half, y + inner), color, NAV_LINE_WEIGHT*2);

    // Bottom-Left corner
    drawList->AddLine(NormToPixel(x - half, y - half), NormToPixel(x - inner, y - half), color, NAV_LINE_WEIGHT*2);
    drawList->AddLine(NormToPixel(x - half, y - half), NormToPixel(x - half, y - inner), color, NAV_LINE_WEIGHT*2);

    // Bottom-Right corner
    drawList->AddLine(NormToPixel(x + half, y - half), NormToPixel(x + inner, y - half), color, NAV_LINE_WEIGHT*2);
    drawList->AddLine(NormToPixel(x + half, y - half), NormToPixel(x + half, y - inner), color, NAV_LINE_WEIGHT*2);
}

//**********************************
// Draws a 3D projected navigation disc using smooth ImGui Ellipses
//**********************************
void NavigationSystem::DrawNavCircle(float x, float y, float size, float rot_x, float rot_y, const GFXColor &col) {
    ImDrawList* drawList = GetNavDrawList();

    constexpr int circles = 4;
    constexpr int segments2 = 12;
    constexpr float TWO_PI = 2.0f * static_cast<float>(M_PI);

    // Compute pixel radii based on 3D tilt
    ImVec2 center = NormToPixel(x, y);
    float baseRadiusPx = Coordinates::normToPixelW(0.6f * size);
    
    // Minor axis compresses based on pitch tilt (rot_x)
    float radiusX = baseRadiusPx;
    float radiusY = baseRadiusPx * std::fabs(std::cos(rot_x));

    // -------------------------------------------------------------------------
    // 1. Smooth Concentric Web Ellipses
    // -------------------------------------------------------------------------
    for (int j = circles; j > 0; --j) {
        float scale = static_cast<float>(j) / static_cast<float>(circles);
        float rx = radiusX * scale;
        float ry = radiusY * scale;

        // Draw rotated ellipse path
        drawList->PathClear();
        constexpr int ellipseSegments = 32; // Smooth auto-tessellated curve
        
        for (int i = 0; i < ellipseSegments; ++i) {
            float a = i * (TWO_PI / ellipseSegments);
            
            // Unrotated ellipse point
            float ex = rx * std::cos(a);
            float ey = ry * std::sin(a);

            // Apply yaw rotation (rot_y) on screen plane
            float rotX = ex * std::cos(rot_y) - ey * std::sin(rot_y);
            float rotY = ex * std::sin(rot_y) + ey * std::cos(rot_y);

            drawList->PathLineTo(ImVec2(center.x + rotX, center.y + rotY));
        }

        // Color with standard alpha
        ImU32 ringColor = ToImColor(col);
        drawList->PathStroke(ringColor, ImDrawFlags_Closed, NAV_LINE_WEIGHT);
    }

    // -------------------------------------------------------------------------
    // 2. Radial Spoke Lines (12 Rays from center outward)
    // -------------------------------------------------------------------------
    constexpr float angleStepSpoke = TWO_PI / static_cast<float>(segments2);
    constexpr float innerScale = 1.0f / static_cast<float>(circles * 2);

    for (int i = 0; i < segments2; ++i) {
        float angle = i * angleStepSpoke;

        // Fading blue channel gradient along the radial direction
        GFXColor ci(col.r, col.g, col.b * std::fabs(std::sin(angle * 0.5f)), col.a);
        ImU32 spokeColor = ToImColor(ci);

        bool isCardinal = (std::fabs(angle - 1.57f) < 0.01f) || 
                         (std::fabs(angle - 3.14f) < 0.01f) || 
                         (std::fabs(angle - 4.71f) < 0.01f) || 
                         (angle < 0.01f);

        float outerScale = isCardinal ? 1.1f : 1.0f;

        // Radial start and end points along ellipse boundary
        auto GetEllipsePoint = [&](float a, float scaleFactor) -> ImVec2 {
            float ex = radiusX * scaleFactor * std::cos(a);
            float ey = radiusY * scaleFactor * std::sin(a);
            float rotX = ex * std::cos(rot_y) - ey * std::sin(rot_y);
            float rotY = ex * std::sin(rot_y) + ey * std::cos(rot_y);
            return ImVec2(center.x + rotX, center.y + rotY);
        };

        ImVec2 pInner = GetEllipsePoint(angle, innerScale);
        ImVec2 pOuter = GetEllipsePoint(angle, outerScale);

        drawList->AddLine(pInner, pOuter, spokeColor, NAV_LINE_WEIGHT);
    }
}