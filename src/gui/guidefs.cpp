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

#include "guidefs.h"


// Draw a rectangle using the specified color.
void drawRect(const Rect& rect, const GFXColor& color) {
    glDisable(GL_TEXTURE_2D);

    glColor4f(color.r, color.g, color.b, color.a);
	glRectf(rect.left(), rect.bottom(), rect.right(), rect.top());

    glEnable(GL_TEXTURE_2D);
}

// Draw the outline of a rectangle using the specified color.
void drawRectOutline(const Rect& rect, const GFXColor& color, float lineWidth) {
    glDisable(GL_TEXTURE_2D);
    glLineWidth(lineWidth);

    glBegin(GL_LINE_LOOP);
    glColor4f(color.r, color.g, color.b, color.a);
    glVertex2f(rect.left(), rect.top());
    glVertex2f(rect.right(), rect.top());
    glVertex2f(rect.right(), rect.bottom());
    glVertex2f(rect.left(), rect.bottom());
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

// Draw upper-left part of rectangle's "shadow".
void drawUpLeftShadow(const Rect& rect, const GFXColor& color, float lineWidth) {
    glDisable(GL_TEXTURE_2D);
    glLineWidth(lineWidth);

    glBegin(GL_LINE_STRIP);
    glColor4f(color.r, color.g, color.b, color.a);
    glVertex2f(rect.origin.x, rect.origin.y);
    glVertex2f(rect.origin.x, rect.origin.y+rect.size.height);
    glVertex2f(rect.origin.x+rect.size.width, rect.origin.y+rect.size.height);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

// Draw lower-right part of rectangle's "shadow".
void drawLowRightShadow(const Rect& rect, const GFXColor& color, float lineWidth) {
    glDisable(GL_TEXTURE_2D);
    glLineWidth(lineWidth);

    glBegin(GL_LINE_STRIP);
    glColor4f(color.r, color.g, color.b, color.a);
    glVertex2f(rect.origin.x, rect.origin.y);
    glVertex2f(rect.origin.x+rect.size.width, rect.origin.y);
    glVertex2f(rect.origin.x+rect.size.width, rect.origin.y+rect.size.height);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

// Fill a closed polygon.
void drawFilledPolygon(const std::vector<Point>& coords, const GFXColor& color) {
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_POLYGON);
    glColor4f(color.r, color.g, color.b, color.a);
    for(std::vector<Point>::const_iterator i=coords.begin(); i!=coords.end(); i++) {
        glVertex2f(i->x, i->y);
    }
    glEnd();

    glEnable(GL_TEXTURE_2D);
}


