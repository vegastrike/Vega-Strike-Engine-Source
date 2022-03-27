/*
 * Copyright (C) 2001-2022 Daniel Horn, Mike Byron, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __FONT_H__
#define __FONT_H__

#include <string>

//Some pre-defined weights for the outline stroke font.
static const float LIGHT_STROKE = 0.6f;
static const float NORMAL_STROKE = 1.0f;
static const float BOLD_STROKE = 1.5f;

//The GLUT outline font (GLUT_STROKE_ROMAN) has a maximum ascender over the baseline of
//119.05, and a max descender under the baseline of 33.33.
//We add a bit of extra space to make sure the characters don't run into each other vertically.
//Not too much, because the max ascender almost never runs into the max descender, and
//we don't want too much white space between lines.  (This spacing is a guess -- tweak
//it if necessary.)
//These constants describe the reference vertical spacing of the font:
static const double REFERENCE_BASELINE_POS = 33.33;
static const double REFERENCE_FONT_ASCENDER = 119.05;
static const double REFERENCE_LINE_SPACING = REFERENCE_FONT_ASCENDER + REFERENCE_BASELINE_POS;

//Font object.
//Right now, this only supports the GLUT outline font.
//We try to choose the best options for a given Font size.  May or may not do
//antialiasing.  The line width goes up as the font size goes up.  You can
//add extra line width to make a bolder look, or use negative line width to give it
//a lighter look.
class Font {
public:
//Font size.  Vertical distance in identity space.
    float size(void) const {
        return m_size * 0.5f;
    }

    void setSize(float s) {
        m_size = s;
        m_needMetrics = true;
    }

//The "boldness" of the font.  See predefined weights above for examples.
    float strokeWeight(void) const {
        return m_strokeWeight;
    }

    void setStrokeWeight(float w) {
        m_strokeWeight = w;
        m_needMetrics = true;
    }

//Draw a character.  Assumes scaling is done, current color set, etc.
    float drawChar(char c) const;

//The width of a character in reference units.
    double charWidth(char c) const;

//The width of a string in reference units.
    double stringWidth(const std::string &str) const;

//Vertical scaling factor to be used to image this font.
    double verticalScaling(void) const;

//Horizontal scaling factor to be used to image this font.
    double horizontalScaling(void) const;

//CONSTRUCTION
    Font(float newsize = .1, float weight = NORMAL_STROKE) :
            m_size(newsize),
            m_strokeWeight(weight),
            m_needMetrics(true),
            m_strokeWidth(1.0),
            m_extraCharWidth(0.5),
            m_spaceCharFixup(10.0),
            m_verticalScaling(1.0),
            m_horizontalScaling(1.0) {
    }

//METRICS
    double strokeWidth(void) const; //Get the stroke width.

//OPERATORS
    bool operator==(const Font &other) {
        return m_size == other.m_size && m_strokeWeight == other.m_strokeWeight;
    }

    bool operator!=(const Font &other) {
        return !(*this == other);
    }

protected:
//INTERNAL IMPLEMENTATION

//Calculate the metrics for this font.
//This does the real work, and doesn't check whether it needs to be done.
    void calcMetrics(void);

//Check whether we need to recalc the metrics, and do it in const object.
    void calcMetricsIfNeeded(void) const;

protected:
//VARIABLES
    float m_size;          //Size of font. Vertical distance in identity space.
    float m_strokeWeight;  //"Look" of stroke font.  Weight of stroke in characters.

//METRICS
    bool m_needMetrics;       //True = we need to recalc the metrics for this font.
    double m_strokeWidth;       //The OpenGL stroke width for this size chars.
    double m_extraCharWidth;    //We add extra char width to compensate for stroke overlap.
    double m_spaceCharFixup;    //Negative translation to apply to space character.
    double m_verticalScaling;   //Vertical factor from char reference space to identity space.
    double m_horizontalScaling; //Horizontal factor from char reference space to identity space.
};

#endif   //__FONT_H__

