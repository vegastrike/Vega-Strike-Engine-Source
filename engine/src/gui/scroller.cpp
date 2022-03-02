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

#include "vegastrike.h"
#include "scroller.h"
#include "newbutton.h"
#include "slider.h"

//This object contains several controls as a group.  These child controls
//should always be in the same places in the list of child controls.
static const int DOWN_BUTTON_INDEX = 0;
static const int UP_BUTTON_INDEX = 1;
static const int SLIDER_INDEX = 2;
static const int CHILD_CONTROL_COUNT = 3;

//The Scroller class is a Control that offers scroll positioning.
//It has a drag "thumb", and scroll buttons.
//This implementation keeps track of the scroll position internally,
//and uses an integer scroll position.
//The only "event" it communicates to the outside world is a change
//of scroll position.

//Set the position of this scroller.
void Scroller::setScrollPosition(int pos) {
    int newScrollPosition = pos;
    if (pos > m_maxValue) {
        newScrollPosition = m_maxValue;
    } else if (pos < m_minValue) {
        newScrollPosition = m_minValue;
    }
    if (m_scrollPosition != newScrollPosition) {
        m_scrollPosition = newScrollPosition;

        Slider *slider = static_cast< Slider * > ( childAt(SLIDER_INDEX));
        slider->setPosition(m_scrollPosition);

        sendCommand("Scroller::PositionChanged", this);
    }
}

//Parameters for the scrolling range:  Max value, visible values, optional min value.
void Scroller::setRangeValues(int max, int visible, int min) {
    const int newMax = guiMax(min, max - visible + 1);
    if (newMax != m_maxValue || min != m_minValue || visible != m_visible) {
        m_maxValue = newMax;
        m_minValue = min;
        m_visible = visible;
        m_needLayout = true;

        Slider *slider = static_cast< Slider * > ( childAt(SLIDER_INDEX));
        slider->setMaxMin(newMax, min);
        const float thumbLength = (max > min) ? (float) visible / (max - min + 1) : -1.;
        //Note that impossible thumb lengths turn off the thumb.
        slider->setThumbLength(thumbLength);
        slider->setPageSize(guiMax(1, visible - 1));
    }
}

//The outside boundaries of the control.
void Scroller::setRect(const Rect &r) {
    if (m_rect != r) {
        m_needLayout = true;
    }
    GroupControl::setRect(r);
}

//Background color of control.
void Scroller::setColor(const GFXColor &c) {
    assert(childCount() == CHILD_CONTROL_COUNT);
    childAt(DOWN_BUTTON_INDEX)->setColor(c);
    childAt(UP_BUTTON_INDEX)->setColor(c);
    if (isClear(m_thumbColor)) {
        //If we don't have an explicit thumb color, calculate it.
        Slider *slider = static_cast< Slider * > ( childAt(SLIDER_INDEX));
        slider->setThumbColorBasedOnColor(c);
    }
    GroupControl::setColor(c);
}

//The color of the thumb.
void Scroller::setThumbColor(const GFXColor &c, const GFXColor &outline) {
    assert(childCount() == CHILD_CONTROL_COUNT);
    Slider *slider = static_cast< Slider * > ( childAt(SLIDER_INDEX));
    slider->setThumbColor(c, outline);
}

//The color of the thumb.
void Scroller::setButtonColor(const GFXColor &c) {
    assert(childCount() == CHILD_CONTROL_COUNT);
    childAt(DOWN_BUTTON_INDEX)->setColor(c);
    childAt(UP_BUTTON_INDEX)->setColor(c);
}

//This is used as the color of the arrows on the scroller buttons.
void Scroller::setTextColor(const GFXColor &c) {
    assert(childCount() == CHILD_CONTROL_COUNT);
    childAt(DOWN_BUTTON_INDEX)->setTextColor(c);
    childAt(UP_BUTTON_INDEX)->setTextColor(c);

    GroupControl::setTextColor(c);
}

//Derived button class that can draw the arrow correctly.
class ScrollerButton : public NewButton {
public:
//Draw the button.
    virtual void draw(void);
//Which way the arrow points.
    enum ButtonArrow {
        LEFT_ARROW,
        RIGHT_ARROW,
        UP_ARROW,
        DOWN_ARROW
    };

//Set the direction the arrow points.
    void setArrowType(ButtonArrow arrow) {
        m_arrowType = arrow;
    }
//CONSTRUCTION
public:
    ScrollerButton() : m_arrowType(LEFT_ARROW) {
    }

    virtual ~ScrollerButton(void) {
    }
//VARIABLES
protected:
    ButtonArrow m_arrowType;    //Direction the arrow points.
};

//Draw the scroller button.
void ScrollerButton::draw(void) {
    //Draw the other stuff first, so our stuff goes on top.
    assert(label().empty());          //We are assuming it won't paint text on itself.
    NewButton::draw();
    static const float ARROW_POINT = .012;      //From center to point of arrow.
    static const float ARROW_WIDTH = .01;       //From center to side points.
    static const float ARROW_BACK = .01;      //From center back to "bottom" of arrow.
    Point center = m_rect.center();
    vector<Point> coords(3);     //3-element vector.
    switch (m_arrowType) {
        case LEFT_ARROW:
            coords[0] = Point(center.x + ARROW_POINT, center.y);
            coords[1] = Point(center.x - ARROW_BACK, center.y + ARROW_WIDTH);
            coords[2] = Point(center.x - ARROW_BACK, center.y - ARROW_WIDTH);
            break;
        case RIGHT_ARROW:
            coords[0] = Point(center.x - ARROW_POINT, center.y);
            coords[1] = Point(center.x + ARROW_BACK, center.y + ARROW_WIDTH);
            coords[2] = Point(center.x + ARROW_BACK, center.y - ARROW_WIDTH);
            break;
        case UP_ARROW:
            coords[0] = Point(center.x, center.y + ARROW_POINT);
            coords[1] = Point(center.x + ARROW_WIDTH, center.y - ARROW_BACK);
            coords[2] = Point(center.x - ARROW_WIDTH, center.y - ARROW_BACK);
            break;
        case DOWN_ARROW:
            coords[0] = Point(center.x, center.y - ARROW_POINT);
            coords[1] = Point(center.x + ARROW_WIDTH, center.y + ARROW_BACK);
            coords[2] = Point(center.x - ARROW_WIDTH, center.y + ARROW_BACK);
            break;
    }
    drawFilledPolygon(coords, textColor());
}

//Calculate the rects for the child controls.
void Scroller::calcLayout(void) {
    assert(childCount() == CHILD_CONTROL_COUNT);

    //Make the buttons slightly smaller than the scroller.
    static const Size BUTTON_INSET = Size(.005, .005);

    //Get pointers to the buttons.
    ScrollerButton *downButton = static_cast< ScrollerButton * > ( childAt(DOWN_BUTTON_INDEX));
    ScrollerButton *upButton = static_cast< ScrollerButton * > ( childAt(UP_BUTTON_INDEX));
    //Make the buttons square, and at the bottom/right of the scroller.
    if (m_rect.size.height >= m_rect.size.width) {
        //This is a vertical scroller.
        Rect rect = m_rect;
        rect.size.height = rect.size.width;
        downButton->setRect(rect.copyAndInset(BUTTON_INSET));
        downButton->setCommand("LineDown");
        downButton->setArrowType(ScrollerButton::DOWN_ARROW);

        rect.origin.y += rect.size.height;
        upButton->setRect(rect.copyAndInset(BUTTON_INSET));
        upButton->setCommand("LineUp");
        upButton->setArrowType(ScrollerButton::UP_ARROW);

        Rect thumbRect = m_rect;
        thumbRect.origin.y = rect.top();
        thumbRect.size.height -= thumbRect.origin.y - m_rect.origin.y;
        childAt(SLIDER_INDEX)->setRect(thumbRect);
    } else {
        //This is a horizontal scroller.
        //Need to flip the button commands -- "line up" does lower values.
        Rect rect = m_rect;
        rect.size.width = rect.size.height;
        downButton->setRect(rect.copyAndInset(BUTTON_INSET));
        downButton->setCommand("LineUp");
        downButton->setArrowType(ScrollerButton::LEFT_ARROW);

        rect.origin.x += rect.size.width;
        upButton->setRect(rect.copyAndInset(BUTTON_INSET));
        upButton->setCommand("LineDown");
        upButton->setArrowType(ScrollerButton::RIGHT_ARROW);

        Rect thumbRect = m_rect;
        thumbRect.origin.x = rect.right();
        thumbRect.size.width -= thumbRect.origin.x - m_rect.origin.x;
        childAt(SLIDER_INDEX)->setRect(thumbRect);
    }
}

//Create the child controls.
void Scroller::createControls(void) {
    //"Scroll Down" button.
    NewButton *down = new ScrollerButton;
    down->setCommand("LineDown");
    down->setColor(color());
    down->setTextColor(textColor());     //Arrow color.
    down->setCommandTarget(this);
    addChild(down);

    //"Scroll Up" button.
    NewButton *up = new ScrollerButton;
    up->setCommand("LineUp");
    up->setColor(color());
    up->setTextColor(textColor());     //Arrow color.
    up->setCommandTarget(this);
    addChild(up);

    //Slider control.
    Slider *slider = new Slider;
    slider->setColor(GUI_CLEAR);
    slider->setThumbColorBasedOnColor(color());
    slider->setCommandTarget(this);
    addChild(slider);
}

//Draw the control.
void Scroller::draw(void) {
    drawBackground();
    if (m_needLayout) {
        calcLayout();
    }
    GroupControl::draw();
}

//Process a command event.
bool Scroller::processCommand(const EventCommandId &command, Control *control) {
    if (command == "LineUp") {
        setScrollPosition(scrollPosition() - 1);
        return true;
    } else if (command == "LineDown") {
        setScrollPosition(scrollPosition() + 1);
        return true;
    } else if (command == "Slider::PositionChanged") {
        Slider *slider = static_cast< Slider * > ( childAt(SLIDER_INDEX));
        setScrollPosition(slider->position());
        return true;
    }
    return GroupControl::processCommand(command, control);
}

//CONSTRUCTION
Scroller::Scroller(void) :
        m_minValue(0),
        m_maxValue(10),
        m_visible(1),
        m_scrollPosition(m_minValue),
        m_thumbColor(GUI_CLEAR),
        m_thumbOutlineColor(GUI_CLEAR),
        m_needLayout(true) {
    createControls();
}

