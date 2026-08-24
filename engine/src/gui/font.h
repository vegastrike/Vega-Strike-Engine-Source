/*
 * font.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_FONT_H
#define VEGA_STRIKE_ENGINE_GUI_FONT_H

#include <string>

//Some pre-defined weights for the outline stroke font.
static const float LIGHT_STROKE = 0.6;
static const float NORMAL_STROKE = 1.0;
static const float BOLD_STROKE = 1.5;

//Font object.
//This is a thin {size, strokeWeight} carrier: it drives the size/weight of text rendered
//through ImGuiText's ImGui draw path. The actual glyphs and metrics come from ImGui's font
//atlas; the engine does not implement its own glyph renderer anymore. Callers author a
//font_point-relative size (font_point * scale) and the standard scaling handles the rest.
class Font {
public:
// Font size in pixels (glyph height), font_point-relative.
    float size(void) const {
        return m_size;
    }

    void setSize(float s) {
        m_size = s;
    }

//The "boldness" of the font.  See predefined weights above for examples.
    float strokeWeight(void) const {
        return m_strokeWeight;
    }

    void setStrokeWeight(float w) {
        m_strokeWeight = w;
    }

//CONSTRUCTION
    Font(float newsize = .1, float weight = NORMAL_STROKE) :
            m_size(newsize),
            m_strokeWeight(weight) {
    }

//OPERATORS
    bool operator==(const Font &other) {
        return m_size == other.m_size && m_strokeWeight == other.m_strokeWeight;
    }

    bool operator!=(const Font &other) {
        return !(*this == other);
    }

protected:
//VARIABLES
    float m_size;          //Size of font (pixel glyph height, font_point-relative).
    float m_strokeWeight;  //"Look" of stroke font.  Weight of stroke in characters.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_FONT_H
