/*
 * font.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Mike Byron
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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

#include "src/vegastrike.h"

#include "font.h"

#include "guidefs.h"
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "configuration/configuration.h"

//For some reason, the cumulative width of GLUT strings is smaller than the
//actual width when they are painted.  If we add this factor to the reference
//length of every GLUT character, things work a lot better.
//It looks like the reason is that the return type for getting GLUT reference lengths
//is int, and the lengths are not int.  The character drawing code lets GLUT move the
//origin for each character, and it looks like that movement is not integer in the
//reference char space (it's clearly not going to be integer in the scaled space).
static const double GLUT_WIDTH_HACK = 0.6;

//The width of the space character in the outline font is too big, so we make it a special case.
static const char SPACE_CHAR = ' ';

bool useStroke() {
    const bool tmp = configuration()->graphics.high_quality_font_computer;
    return !tmp;
}

//Calculate the metrics for this font.
//This does the real work, and doesn't check whether it needs to be done.
void Font::calcMetrics(void) {
    //This is a hack to adjust the font stroke width as the font size goes up.
    //Since the stroke width is in pixels, we scale the width up as the screen resolution gets
    //higher.  (Currently, this is linear, which doesn't work well at small sizes.)
    //We also make sure the stroke width is at least 1.0.  Otherwise antialiasing causes
    //some font features to fade out.
    //My OpenGL (Windows XP) has a max line width of 10.  So we get stroke width truncation for font
    //size over 0.5.  This is OK because the curves look really ugly that big anyway.
    const double referenceStrokeWidth = 20.0;                          //Best width for a font size of 1.0 (big).
    const double referenceStrokeWidthResolution = 800;          //X-resolution stroke width measured in.
    const double minimumStrokeWidth = 1.0;

    const double nonClippedStrokeWidth = size() * referenceStrokeWidth * strokeWeight()
            * (configuration()->graphics.resolution_x / referenceStrokeWidthResolution);

    m_strokeWidth = guiMax(minimumStrokeWidth, nonClippedStrokeWidth);
    m_needMetrics = false;

    //Vertical scaling factor:
    m_verticalScaling = size() / REFERENCE_LINE_SPACING;
    //Horizontal scaling factor.  Same as vertical, except we need to make the coord system
    //the same distance in all directions, so we need to apply the ratio of vert / horiz
    //resolution.  Otherwise the fonts are slightly stretched horizontally -- there
    //are more pixels horizontally than vertically per unit in the identity coord space.
    if (useStroke()) {
        m_horizontalScaling = (m_verticalScaling * configuration()->graphics.resolution_y) / configuration()->graphics.resolution_x;
    } else {
        //Calculation above seems broken... this seems to work for most sizes with bitmap.
        m_horizontalScaling = m_verticalScaling / (1.6 * configuration()->graphics.resolution_x / 1000);
    }
    //The size of a horizontal pixel in reference space.
    const double horizPixelInRefSpace = REFERENCE_LINE_SPACING / (configuration()->graphics.resolution_x / 2) / size();

    //Recalculate the extra char width.
    m_extraCharWidth = horizPixelInRefSpace * m_strokeWidth;

    //Space character width should be the same as a number.
    //Numbers are generally all the same width so you can assume they will go into columns.
    //We use '8' because if the numbers aren't all the same width, '8' is a good, wide number.
    //What we calculate here is the horizontal translation to get from the actual outline font
    //space char width to the space char width we want.
    const double eightWidth = glutStrokeWidth(GLUT_STROKE_ROMAN, '8') + m_extraCharWidth;
    m_spaceCharFixup = eightWidth - glutStrokeWidth(GLUT_STROKE_ROMAN, SPACE_CHAR);
}

//Check whether we need to recalc the metrics, and do it in const object.
void Font::calcMetricsIfNeeded(void) const {
    if (m_needMetrics) {
        //calcmetrics is a cache.  Doesn't change the "real" state of the object.
        Font *s = const_cast< Font * > (this);
        s->calcMetrics();
    }
}

//Draw a character.
float Font::drawChar(char c) const {
    calcMetricsIfNeeded();
    if (useStroke()) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
        if (c == SPACE_CHAR) {
            //Need to translate back a bit -- the width of the space is too big.
            glTranslated(m_spaceCharFixup, 0.0, 0.0);
        } else {
            glTranslated(m_extraCharWidth, 0.0, 0.0);
        }
        return 0;
    } else {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        return glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, c);
    }
}

//The width of a character in reference units.
double Font::charWidth(char c) const {
    calcMetricsIfNeeded();
    if (useStroke()) {
        if (c == SPACE_CHAR) {
            //Spaces have a special width.
            const double spaceCharWidth =
                    glutStrokeWidth(GLUT_STROKE_ROMAN, SPACE_CHAR) + m_spaceCharFixup;
            return spaceCharWidth + GLUT_WIDTH_HACK;
        }
        const double charWidth = glutStrokeWidth(GLUT_STROKE_ROMAN, c);
        return charWidth + m_extraCharWidth + GLUT_WIDTH_HACK;
    } else {
        return glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, c) / (size() * 2);
    }
}

//The width of a string in reference units.
double Font::stringWidth(const std::string &str) const {
    calcMetricsIfNeeded();

    double result = 0.0;
    for (string::const_iterator i = str.begin(); i != str.end(); i++) {
        result += charWidth(*i);
    }
    return result;
}

//Calculate the OpenGL stroke width for a font size+weight.
//This value is cached in the font object.
double Font::strokeWidth(void) const {
    calcMetricsIfNeeded();

    return m_strokeWidth;
}

//Vertical scaling factor to be used to image this font.
double Font::verticalScaling(void) const {
    calcMetricsIfNeeded();

    return m_verticalScaling;
}

//Horizontal scaling factor to be used to image this font.
double Font::horizontalScaling(void) const {
    calcMetricsIfNeeded();

    return m_horizontalScaling;
}

