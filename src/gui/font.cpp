/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vegastrike.h"

#include "font.h"

#include "guidefs.h"
#include "vs_globals.h"

// For some reason, the cumulative width of GLUT strings is smaller than the
//  actual width when they are painted.  If we add this factor to the reference
//  length of every GLUT character, things work a lot better.
// It looks like the reason is that the return type for getting GLUT reference lengths
//  is int, and the lengths are not int.  The character drawing code lets GLUT move the
//  origin for each character, and it looks like that movement is not integer in the
//  reference char space (it's clearly not going to be integer in the scaled space).
static const double GLUT_WIDTH_HACK = 0.6;

// The width of the space character in the outline font is too big, so we make it a special case.
static const char SPACE_CHAR = ' ';

// The GLUT characters are too close together.  Add this factor to the width of each
//  character to space them out more.  This is in reference units.
static const double EXTRA_WIDTH = 6.0;


// Calculate the metrics for this font.
// This does the real work, and doesn't check whether it needs to be done.
void Font::calcMetrics(void) {
    // This is a hack to adjust the font stroke width as the font size goes up.
    // Since the stroke width is in pixels, we scale the width up as the screen resolution gets
    //  higher.  (Currently, this is linear, which doesn't work well at small sizes.)
    // We also make sure the stroke width is at least 1.0.  Otherwise antialiasing causes
    //  some font features to fade out.
    const double referenceStrokeWidth = 7.0;              // Best width for a font size of 1.0 (big).
    const double referenceStrokeWidthResolution = 800;    // X-resolution stroke width measured in.
    const double minimumStrokeWidth = 1.0;
    const double nonClippedStrokeWidth = size() * referenceStrokeWidth * strokeWeight() *
        (g_game.x_resolution / referenceStrokeWidthResolution);

    // This is like a cache.  We modify the protected variables, but we
    //  aren't modifying the public state of the object.
    Font& thisObject = *const_cast<Font*>(this);

    thisObject.m_strokeWidth = guiMax( minimumStrokeWidth, nonClippedStrokeWidth );
    thisObject.m_needMetrics = false;

    // Recalculate the extra char width.
    thisObject.m_extraCharWidth = m_strokeWidth * EXTRA_WIDTH;

    // Space character width should be the same as a number.
    // Numbers are generally all the same width so you can assume they will go into columns.
    // We use '8' because if the numbers aren't all the same width, '8 is a good, wide number.
    // What we calculate here is the horiontal translation to get from the outline font width
    //  to the space char width we want.
    const double eightWidth = glutStrokeWidth(GLUT_STROKE_ROMAN, '8');
    thisObject.m_spaceCharFixup = eightWidth - glutStrokeWidth(GLUT_STROKE_ROMAN, SPACE_CHAR);
}

// Check whether we need to recalc the metrics, and do it in const object.
void Font::calcMetricsIfNeeded(void) const {
    if(m_needMetrics) {
        // calcmetrics is a cache.  Doesn't change the "real" state of the object.
        Font* s = const_cast<Font*>(this);
        s->calcMetrics();
    }
}

// Draw a character.
void Font::drawChar(char c) const {
    calcMetricsIfNeeded();

    glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    if(c == SPACE_CHAR) {
        // Need to translate back a bit -- the width of the space is too big.
        glTranslated(m_spaceCharFixup, 0.0, 0.0);
    } else {
        glTranslated(m_extraCharWidth, 0.0, 0.0);
    }
}

// The width of a character in reference units.
double Font::charWidth(char c) const {
    calcMetricsIfNeeded();

    if(c == SPACE_CHAR) {
        // Spaces have a special width.
        const double spaceCharWidth =
            glutStrokeWidth(GLUT_STROKE_ROMAN, SPACE_CHAR) + m_spaceCharFixup;
        return(spaceCharWidth + GLUT_WIDTH_HACK);
    }

    const double charWidth = glutStrokeWidth(GLUT_STROKE_ROMAN, c);
    return(charWidth + m_extraCharWidth + GLUT_WIDTH_HACK);
}

// The width of a string in reference units.
double Font::stringWidth(const std::string& str) const {
    calcMetricsIfNeeded();

    double result = 0.0;
    for(string::const_iterator i=str.begin(); i!=str.end(); i++) {
        result += charWidth(*i);
    }
    return result;
}

// Calculate the OpenGL stroke width for a font size+weight.
//  This value is cached in the font object.
double Font::strokeWidth(void) const {
    calcMetricsIfNeeded();

    return m_strokeWidth;
}
