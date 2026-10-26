/*
 * hud.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alan Shieh, ace123, dan_w, jacks, klaussfreire, pyramid3d
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
#ifndef VEGA_STRIKE_ENGINE_GFX_HUD_H
#define VEGA_STRIKE_ENGINE_GFX_HUD_H

#include <string>
#include "src/gfxlib_struct.h"

#include <cstdint>
using ImU32 = std::uint32_t;

class Texture;

// Text in a specific color
struct TextSegment {
    std::string text;
    ImU32 color;
};

struct TextLine {
    float width;
    std::vector<TextSegment> segments;
};

class TextPlane {
    std::string myText;

//Texture *myFont;
    Vector myFontMetrics;     //i = width, j = height
    Vector myDims;
    int numlet{};
    // Per-text-box layout resolution + screen-absolute letterbox offset. When a
    // resolution is set, normalized coords map against it (instead of native pixels)
    // and the offset shifts the text into the letterboxed base window.
    float m_resW = 0.0f;   // 0 = use native resolution (legacy behaviour)
    float m_resH = 0.0f;
    float m_offX = 0.0f;   // screen-absolute pixel offset for the drawn position
    float m_offY = 0.0f;
/*
 *  struct GlyphPosition {
 *       float left, right, top, bottom;
 *  } myGlyphPos[256];
 */
    std::vector<TextLine> ParseText(const std::string& text, ImU32 default_color);

public:
    ImU32 color, background_color;
    TextPlane(const struct GFXColor &col = GFXColor(1, 1, 1, 1), const struct GFXColor &bgcol = GFXColor(0, 0, 0, 0));
    ~TextPlane();

    void SetPos(float x, float y) {
        myFontMetrics.k = y;
        myDims.k = x;
    }

    void SetCharSize(float x, float y) {
        myFontMetrics.i = x;
        myFontMetrics.j = y;
    }

    void GetCharSize(float &x, float &y) {
        x = myFontMetrics.i;
        y = myFontMetrics.j;
    }

    void GetPos(float &y, float &x) {
        y = myFontMetrics.k;
        x = myDims.k;
    }

    void SetSize(float x, float y) {
        myDims.i = x;
        myDims.j = y;
    }

    void GetSize(float &x, float &y) {
        x = myDims.i;
        y = myDims.j;
    }

    int Draw(int offset = 0); //returns number of lines
    int Draw(const std::string &text,
            int offset = 0,
            bool start_one_line_lower = false,
            bool force_highquality = false,
            bool automatte = false);

    void SetText(const std::string &newText) {
        myText = newText;
    }

    std::string GetText() const {
        return myText;
    }

    // The resolution to lay out against (0 = use native resolution, the legacy
    // behaviour). Used by the base so text scales with the base art.
    void setResolution(float w, float h) {
        m_resW = w; m_resH = h;
    }
    // A screen-absolute pixel offset added to the drawn position, so text lands
    // inside the letterboxed base window rather than at the screen edge.
    void setOffset(float x, float y) {
        m_offX = x; m_offY = y;
    }
};

#endif //VEGA_STRIKE_ENGINE_GFX_HUD_H
