/*
 * guidefs.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_GUI_DEFS_H
#define VEGA_STRIKE_ENGINE_GUI_GUI_DEFS_H

#include "src/gfxlib.h"
#include "gldrv/winsys.h"
//Location in 2d.
struct Point {
    float x, y;

    // Operators
    bool operator==(const Point &other);
    bool operator!=(const Point &other);

    // Constructors
    Point();
    Point(float cx, float cy);
};

//Extent of an area in 2d.  This is not the same as a point.
struct Size {
    float width, height;

    // Operators
    bool operator==(const Size &other);
    bool operator!=(const Size &other);

    // Constructors
    Size();
    Size(float cwidth, float cheight);
};

//Rectangle in 2d.
class Rect {
public:
//Data
    Point origin;
    Size size;

// Constructors
    Rect();
    Rect(Point &p, Size &s);
    Rect(float x, float y, float width, float height);

// Methods
    float left(void) const;
    float right(void) const;
    float bottom(void) const;
    float top(void) const;

    //The center of this rectangle.
    Point center(void) const;

    //Whether a Point is inside this Rect.
    bool inside(const Point &p) const;

    //Make a new Rect that is inset by the specified margins.
    void inset(const Size &s);

    //Return a copy of the rect inset by specified margins.
    Rect copyAndInset(const Size &s);

// Operators
    bool operator==(const Rect &other);
    bool operator!=(const Rect &other);
};

//Rect that describes the coordinates of a full screen.
static const Rect FULL_SCREEN_RECT(-1, -1, 2, 2);

//Type of input event.
typedef enum {
    KEY_DOWN_EVENT,
    KEY_UP_EVENT,
    MOUSE_DOWN_EVENT,
    MOUSE_UP_EVENT,
    MOUSE_MOVE_EVENT,
    MOUSE_DRAG_EVENT
} InputEventType;

//Event modifiers as a bit mask: buttons or keys.
typedef unsigned int EventModMask;

//The code for a key in a keyboard event.
typedef unsigned int EventKeyCode;

//The code for a button in a mouse event.
//(Currently, this must be the same type as a key code.)
typedef EventKeyCode EventButton;

//Mouse button constants
static const unsigned int LEFT_MOUSE_BUTTON = WS_LEFT_BUTTON;
static const unsigned int MIDDLE_MOUSE_BUTTON = WS_MIDDLE_BUTTON;
static const unsigned int RIGHT_MOUSE_BUTTON = WS_RIGHT_BUTTON;
static const unsigned int WHEELUP_MOUSE_BUTTON = WS_WHEEL_UP;
static const unsigned int WHEELDOWN_MOUSE_BUTTON = WS_WHEEL_DOWN;

/* This describes an event from an input device: mouse, keyboard, etc.
 * It does *not* describe a command event.
 */
struct InputEvent {
    InputEventType type;      //Kind of event.
    unsigned int code;      //Key or mouse button.
    EventModMask mask;      //Modifier keys or buttons.
    Point loc;                //Coordinate of mouse.

    //CONSTRUCTION
    InputEvent(InputEventType t, unsigned int c, EventModMask m, const Point &l) :
            type(t), code(c), mask(m), loc(l) {
    }

    InputEvent() : type(MOUSE_UP_EVENT), code(LEFT_MOUSE_BUTTON), mask(0), loc(Point(0, 0)) {
    }
};

//Text justification possibilities. TODO: Convert to enum class
enum Justification {
    RIGHT_JUSTIFY,
    CENTER_JUSTIFY,
    LEFT_JUSTIFY
};

GFXColor SaturatedColor(float r, float g, float b, float a = 1.0f);
//Predefined colors.
GFXColor GUI_OPAQUE_BLACK(); //(0.0,0.0,0.0);
GFXColor GUI_OPAQUE_WHITE(); //(1.0,1.0,1.0);
const GFXColor GUI_CLEAR(0.0, 0.0, 0.0, 0.0);
GFXColor GUI_OPAQUE_LIGHT_GRAY(); //(0.25,0.25,0.25);
GFXColor GUI_OPAQUE_MEDIUM_GRAY(); //(0.5,0.5,0.5);
GFXColor GUI_OPAQUE_DARK_GRAY(); //(0.75,0.75,0.75);

//Whether a color is clear -- totally transparent.
inline bool isClear(const GFXColor &c) {
    return c.a <= 0.0;
}

//Compare two colors.
//Used in upgrade/downgrade to decide whether an item is OK.
inline bool equalColors(const GFXColor &c1, const GFXColor &c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

//////////////////   DRAWING UTILITIES   ////////////////////

//Draw a rectangle using the specified color.
void drawRect(const Rect &rect, const GFXColor &color);

//Draw the outline of a rectangle using the specified color.
void drawRectOutline(const Rect &rect, const GFXColor &color, float lineWidth);

//Draw upper-left part of rectangle's "shadow".
void drawUpLeftShadow(const Rect &rect, const GFXColor &color, float lineWidth);

//Draw lower-right part of rectangle's "shadow".
void drawLowRightShadow(const Rect &rect, const GFXColor &color, float lineWidth);

//Fill a closed polygon.
void drawFilledPolygon(const std::vector<Point> &coords, const GFXColor &color);

//////////////////   MIN AND MAX FUNCTIONS   ////////////////////

#define guiMin(a, b) ( (a) < (b) ? (a) : (b) )
#define guiMax(a, b) ( (a) > (b) ? (a) : (b) )

#endif   //VEGA_STRIKE_ENGINE_GUI_GUI_DEFS_H
