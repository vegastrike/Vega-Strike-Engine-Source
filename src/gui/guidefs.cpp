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
#include "xml_support.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "guidefs.h"
GFXColor getConfigColor(const char * name, GFXColor defaul) {
  float color[4];
  color[0]=defaul.r;
  color[1]=defaul.g;
  color[2]=defaul.b;
  color[3]=defaul.a;
  vs_config->getColor(std::string("default"), std::string(name), color, true);
  return GFXColor(color[0], color[1], color[2], color[3]);
}

GFXColor SaturatedColor(float r, float g, float b, float a=1.0f) {
  static float Saturation=XMLSupport::parse_float(vs_config->getVariable("graphics","base_saturation","1.0"));
  
  return GFXColor((r*Saturation*3+(r+b+g)*(1-Saturation))/3,
                  (g*Saturation*3+(r+b+g)*(1-Saturation))/3,
                  (b*Saturation*3+(r+b+g)*(1-Saturation))/3,a);
}
GFXColor GUI_OPAQUE_BLACK(){
  static GFXColor gui_black = getConfigColor("base_black",GFXColor(0,0,0,1));
  return gui_black;
}
GFXColor GUI_OPAQUE_WHITE(){
  static GFXColor gui_white = getConfigColor("base_white",GFXColor(1,1,1,1));
  return gui_white;
}

GFXColor GUI_OPAQUE_LIGHT_GRAY(){
  static GFXColor gui_light_gray = getConfigColor("base_light_gray",GFXColor(.25,.25,.25,1));
  return gui_light_gray;
}
GFXColor GUI_OPAQUE_MEDIUM_GRAY(){
  static GFXColor gui_gray = getConfigColor("base_gray",GFXColor(.5,.5,.5,1));
  return gui_gray;
}
GFXColor GUI_OPAQUE_DARK_GRAY(){
  static GFXColor gui_dark_gray = getConfigColor("base_dark_gray",GFXColor(.75,.75,.75,1));
  return gui_dark_gray;
}

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


