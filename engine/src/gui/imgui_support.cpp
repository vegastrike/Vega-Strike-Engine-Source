/*
 * imgui_support.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Danny Gehl
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

#include <png.h>
#include "gui/imgui_support.h"
#include "gfx/sprite.h"
#include "imgui/imgui.h"
//#include "configuration/configuration.h"


static bool softwarePosition;
// virtual mouse x after sensitivity adjustment
static int mmx = 0;
// virtual mouse y after sensitivity adjustment
static int mmy = 0;

// Set the software mouse position
void SetSoftwareMousePosition(int x, int y) {
    softwarePosition = true;
    mmx = x;
    mmy = y;
}

/** Initialises a Frame with proper settings for ImGUI and mouse */
void StartGUIFrame(void) {
    softwarePosition = false;
    GFXHudMode(true);
    // this will hide whatever is behind (the cockpit)
    GFXClear(GFXTRUE);
}

void DrawMouseCursor(MousePointerStyle pointerStyle) {
    static VSSprite MouseOverVSSprite("mouseover.spr", BILINEAR, GFXTRUE);
    static VSSprite MouseVSSprite("mouse.spr", BILINEAR, GFXTRUE);

    if (pointerStyle != MOUSE_POINTER_NONE) {
        VSSprite *whichSprite = &MouseVSSprite;
        switch (pointerStyle) {
            case MOUSE_POINTER_NORMAL:
                whichSprite = &MouseVSSprite;
                break;
            case MOUSE_POINTER_HOVER:
                whichSprite = &MouseOverVSSprite;
                break;
            case MOUSE_POINTER_NONE:
                return;
        }

        float normX;
        float normY;
        // if(softwarePosition) {
        //     normX = mmx;
        //     normY = mmy;
        //     int xrez = configuration().graphics.resolution_x;
        //     const int whentodouble = configuration().joystick.double_mouse_position;
        //     const float factor = configuration().joystick.double_mouse_factor_flt;
        //     if (xrez >= whentodouble) {
        //         normX /= factor;
        //         normY /= factor;
        //     }
        // } else {
            ImVec2 mousePos = ImGui::GetMousePos();
            normX = (2.0f * mousePos.x / ImGui::GetIO().DisplaySize.x) - 1.0f;
            normY = (1.0f - 2.0f * mousePos.y / ImGui::GetIO().DisplaySize.y);
        // }


        // VS_LOG(error, (boost::format("MouseLoc X: %1% | Computed Norm X: %2%") % globalEventManager().mouseLoc().x % normX).str());
        whichSprite->SetPosition(normX, normY);

        whichSprite->DrawWithImGui(ImGui::GetForegroundDrawList());
    }
}

extern void ConditionalCursorDraw(bool);

/** End a Frame with proper cleanup for ImGUI and draw mouse cursor in the foreground */
void EndGUIFrame(MousePointerStyle pointerStyle) {
    DrawMouseCursor(pointerStyle);
    GFXHudMode(false);
}

