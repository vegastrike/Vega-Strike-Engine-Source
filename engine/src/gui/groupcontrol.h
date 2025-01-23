/*
 * groupcontrol.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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
#ifndef VEGA_STRIKE_ENGINE_GUI_GROUP_CONTROL_H
#define VEGA_STRIKE_ENGINE_GUI_GROUP_CONTROL_H

#include "gui/control.h"

//See cpp file for detailed descriptions of classes, functions, etc.

//The group control is simply a collection of controls that can be
//manipulated as one.
//Note that this Control "owns" the controls under it.  They are deleted
//when this control is deleted.
class GroupControl : public Control {
public:
//Whether this control has children.
    virtual bool hasGroupChildren(void) {
        return true;
    }

//Add a new control to this collection.
    void addChild(Control *child);

//Delete a control that is in this collection.
//Returns true if successful.
    bool deleteControl(Control *c);

//Take a control away from this collection and save it elsewhere.
    Control *removeControlFromGroup(Control *c);

//Find a control using its id.  NULL returned if none found.
//Note that the control may be hidden.
    Control *findControlById(const std::string &id);

//Number of child controls.
    int childCount(void) {
        return m_controls.size();
    }

//A control in this group.
    Control *childAt(int index) {
        return m_controls[index];
    }

//Draw the control.
//This should not draw outside its rectangle!
    virtual void draw(void);

//OVERRIDES
    virtual bool processMouseDown(const InputEvent &event);
    virtual bool processMouseUp(const InputEvent &event);
    virtual bool processMouseMove(const InputEvent &event);
    virtual bool processMouseDrag(const InputEvent &event);

//CONSTRUCTION
public:
    GroupControl(void) {
    }

    virtual ~GroupControl(void);

//INTERNAL IMPLEMENTATION
protected:

//VARIABLES
protected:
    std::vector<Control *> m_controls;
};

#endif   //VEGA_STRIKE_ENGINE_GUI_GROUP_CONTROL_H
