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

#ifndef __GUIDEFS_H__
#define __GUIDEFS_H__

#include "gfxlib.h"

// Location in 2d.
struct Point {
    float x, y;

    // OPERATORS
    bool operator==(const Point& other) {
        return(x == other.x && y == other.y);
    }
    bool operator!=(const Point& other) {
        return(!(*this == other));
    }

    // CONSTRUCTION
    Point() : x(0.0), y(0.0) {};
    Point(float cx, float cy) :
        x(cx), y(cy)
        {};
};

// Extent of an area in 2d.  This is not the same as a point.
struct Size {
    float width, height;

    // OPERATORS
    bool operator==(const Size& other) {
        return(width == other.width && height == other.height);
    }
    bool operator!=(const Size& other) {
        return(!(*this == other));
    }

    // CONSTRUCTION
    Size() : width(0.0), height(0.0) {};
    Size(float cwidth, float cheight) :
        width(cwidth), height(cheight)
        {};
};

// Rectangle in 2d.
class Rect {
public:
    // Data
    Point origin;
    Size size;

    float left(void) const   { return origin.x; };
    float right(void) const  { return origin.x + size.width; };
    float bottom(void) const { return origin.y; };
    float top(void) const    { return origin.y + size.height; };

    // The center of this rectangle.
    Point center(void) const { return Point(origin.x+size.width/2, origin.y+size.height/2); };

    // Whether a Point is inside this Rect.
    bool inside(const Point& p) const {
        return( p.x>=left() && p.x<right() && p.y>=bottom() && p.y<top() );
    };

    // Make a new Rect that is inset by the specified margins.
    void inset(const Size& s) {
        origin.x += s.width;
        origin.y += s.height;
        size.width -= s.width*2;
        size.height -= s.height*2;
    }

    // Return a copy of the rect inset by specified margins.
    Rect copyAndInset(const Size s) {
        Rect result = *this;
        result.inset(s);
        return result;
    }

    // OPERATORS
    bool operator==(const Rect& other) {
        return(origin == other.origin && size == other.size);
    }
    bool operator!=(const Rect& other) {
        return(!(*this == other));
    }

    // CONSTRUCTION
    Rect() {};
    Rect(Point& p, Size& s) :
        origin(p), size(s)
        {};
    Rect(float x, float y, float width, float height) :
        origin(x,y), size(width,height)
        {};
};

// Rect that describes the coordinates of a full screen.
static const Rect FULL_SCREEN_RECT(-1,-1,2,2);


// Type of input event.
typedef enum {
    KEY_DOWN_EVENT,
    KEY_UP_EVENT,
    MOUSE_DOWN_EVENT,
    MOUSE_UP_EVENT,
    MOUSE_MOVE_EVENT,
    MOUSE_DRAG_EVENT
} InputEventType;

// Event modifiers as a bit mask: buttons or keys.
typedef unsigned int EventModMask;

// The code for a key in a keyboard event.
typedef unsigned int EventKeyCode;

// The code for a button in a mouse event.
// (Currently, this must be the same type as a key code.)
typedef EventKeyCode EventButton;

// Mouse button constants
static const int LEFT_MOUSE_BUTTON = 1;
static const int MIDDLE_MOUSE_BUTTON = 2;
static const int RIGHT_MOUSE_BUTTON = 3;


/* This describes an event from an input device: mouse, keyboard, etc.
 * It does *not* describe a command event.
 */
struct InputEvent
{
    InputEventType type;      // Kind of event.
    unsigned int code;        // Key or mouse button.
    EventModMask mask;        // Modifier keys or buttons.
    Point loc;                // Coordinate of mouse.

    // CONSTRUCTION
    InputEvent(InputEventType t, unsigned int c, EventModMask m, const Point& l)
        : type(t), code(c), mask(m), loc(l) {};
    InputEvent() : type(MOUSE_UP_EVENT), code(LEFT_MOUSE_BUTTON), mask(0), loc(Point(0,0)) {};
};




// Text justification possibilities.
enum Justification {
    RIGHT_JUSTIFY,
    CENTER_JUSTIFY,
    LEFT_JUSTIFY
};

    


// Predefined colors.
const GFXColor GUI_OPAQUE_BLACK(0.0,0.0,0.0,1.0);
const GFXColor GUI_OPAQUE_WHITE(1.0,1.0,1.0,1.0);
const GFXColor GUI_CLEAR       (0.0,0.0,0.0,0.0);

// Whether a color is clear -- totally transparent.
static bool isClear(const GFXColor& c) { return c.r <= 0.0; };

// Compare two colors.
// Used in upgrade/downgrade to decide whether an item is OK.
static bool equalColors(const GFXColor& c1, const GFXColor& c2) {
    return( c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a );
}


//////////////////   DRAWING UTILITIES   ////////////////////

// Draw a rectangle using the specified color.
void drawRect(const Rect& rect, const GFXColor& color);

// Draw the outline of a rectangle using the specified color.
void drawRectOutline(const Rect& rect, const GFXColor& color, float lineWidth);

// Draw upper-left part of rectangle's "shadow".
void drawUpLeftShadow(const Rect& rect, const GFXColor& color, float lineWidth);

// Draw lower-right part of rectangle's "shadow".
void drawLowRightShadow(const Rect& rect, const GFXColor& color, float lineWidth);

// Fill a closed polygon.
void drawFilledPolygon(const std::vector<Point>& coords, const GFXColor& color);


#endif   // __GUIDEFS_H__
