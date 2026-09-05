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
//Right now, this only supports the GLUT outline font.
//We try to choose the best options for a given Font size.  May or may not do
//antialiasing.  The line width goes up as the font size goes up.  You can
//add extra line width to make a bolder look, or use negative line width to give it
//a lighter look.
class Font {
public:
//Font size.  Vertical distance in identity space.
    //
    // Removed the legacy `* 0.5` (added 2011, commit e3b743d16a, as a scoped
    // "stroke font size" fix) so size() returns the honest m_size. The font
    // authoring values are re-scaled to compensate in a separate change.
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

//VARIABLES
    float m_size;          //Size of font. Vertical distance in identity space.
    float m_strokeWeight;  //"Look" of stroke font.  Weight of stroke in characters.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_FONT_H
