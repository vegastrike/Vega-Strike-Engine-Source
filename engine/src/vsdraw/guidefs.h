/*
 * guidefs.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_GUI_DEFS_H
#define VEGA_STRIKE_ENGINE_GUI_GUI_DEFS_H

#include "src/gfxlib.h"
#include "gldrv/winsys.h"
#include "imgui/imgui.h"

// Namespace for coordinate conversion
namespace Coordinates {

    /// Converts normalized X coordinate [-1.0, 1.0] to pixel space [0, res_x]
    [[nodiscard]] inline float normToPixelX(float val, float res_x) noexcept {
        return (val + 1.0f) * 0.5f * res_x;
    }

    /// Converts normalized X coordinate [-1.0, 1.0] to display pixel space [0, DisplaySize.x]
    [[nodiscard]] inline float normToPixelX(float val) noexcept {
        return normToPixelX(val, ImGui::GetIO().DisplaySize.x);
    }

    /// Converts normalized Y coordinate [-1.0, 1.0] (where +1 is top) to pixel space [0, res_y]
    [[nodiscard]] inline float normToPixelY(float val, float res_y) noexcept {
        return (1.0f - val) * 0.5f * res_y;
    }

    /// Converts normalized Y coordinate [-1.0, 1.0] (where +1 is top) to display pixel space [0, DisplaySize.y]
    [[nodiscard]] inline float normToPixelY(float val) noexcept {
        return normToPixelY(val, ImGui::GetIO().DisplaySize.y);
    }

    /// Scales a normalized width [0.0, 2.0] relative to a width of res_x
    [[nodiscard]] inline float normToPixelW(float val, float res_x) noexcept {
        return val * 0.5f * res_x;
    }

    /// Scales a normalized width [0.0, 2.0] relative to display width
    [[nodiscard]] inline float normToPixelW(float val) noexcept {
        return normToPixelW(val, ImGui::GetIO().DisplaySize.x);
    }

    /// Scales a normalized height/font size [0.0, 2.0] to pixel height relative to res_y
    [[nodiscard]] inline float normToPixelH(float val, float res_y) noexcept {
        return val * 0.5f * res_y;
    }

    /// Scales a normalized height/font size [0.0, 2.0] to pixel height
    [[nodiscard]] inline float normToPixelH(float val) noexcept {
        return normToPixelH(val, ImGui::GetIO().DisplaySize.y);
    }

    /// Scales a normalized font size font size [0.0, 2.0] pixel height
    [[nodiscard]] inline float normToPixelFontSize(float val) noexcept {
        return Coordinates::normToPixelH(val);
    }

    /// Inverse of normToPixelH: converts a pixel height/length to normalized [-1..1] units.
    /// Used where a pixel-size (e.g. a font_point-relative Font::size()) must be laid out
    /// against a normalized -1..1 rect, so the two spaces stay consistent. 2.0 normalized
    /// == DisplaySize.y pixels.
    [[nodiscard]] inline float pixelToNormH(float pix) noexcept {
        return pix * 2.0f / ImGui::GetIO().DisplaySize.y;
    }

} // namespace Coordinates

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
