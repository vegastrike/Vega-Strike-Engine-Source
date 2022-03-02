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

#include "groupcontrol.h"

#include "eventmanager.h"

#include "window.h"

//Add a new control to this collection.
void GroupControl::addChild(Control *child) {
    m_controls.push_back(child);
}

//Delete a control that is in this collection.
//Returns true if successful.
bool GroupControl::deleteControl(Control *c) {
    std::vector<Control *>::iterator iter;
    for (iter = m_controls.begin(); iter != m_controls.end(); iter++) {
        Control *currentControl = *iter;
        if (c == currentControl) {
            //Found it in this group.
            m_controls.erase(iter);
            EventManager::addToDeleteQueue(currentControl);
            return true;
        }
        if (currentControl->hasGroupChildren()) {
            //Check the children.
            GroupControl *group = static_cast< GroupControl * > (currentControl);
            if (group->deleteControl(c)) {
                return true;
            }
        }
    }
    return false;
}

//Take a control away from this collection and save it elsewhere.
Control *GroupControl::removeControlFromGroup(Control *c) {
    std::vector<Control *>::iterator iter;
    for (iter = m_controls.begin(); iter != m_controls.end(); iter++) {
        Control *currentControl = *iter;
        if (c == currentControl) {
            //Found it in this group.
            m_controls.erase(iter);
            return currentControl;
        }
        if (currentControl->hasGroupChildren()) {
            //Check the children.
            GroupControl *group = static_cast< GroupControl * > (currentControl);
            Control *ret = group->removeControlFromGroup(c);
            if (ret) {
                return ret;
            }
        }
    }
    return NULL;
}

//Find a control using its id.  NULL returned if none found.
//Note that the control may be hidden.
Control *GroupControl::findControlById(const std::string &id) {
    std::vector<Control *>::iterator iter;
    for (iter = m_controls.begin(); iter != m_controls.end(); iter++) {
        Control *currentControl = *iter;
        if (currentControl->id() == id) {
            //Found it in this group.
            return currentControl;
        }
        if (currentControl->hasGroupChildren()) {
            //Check the children.
            GroupControl *group = static_cast< GroupControl * > (currentControl);
            Control *ret = group->findControlById(id);
            if (ret) {
                return ret;
            }
        }
    }
    return NULL;
}

//Draw the control.
//This should not draw outside its rectangle!
void GroupControl::draw(void) {
    std::vector<Control *>::iterator iter;
    for (iter = m_controls.begin(); iter != m_controls.end(); iter++) {
        Control *currentControl = *iter;
        if (!currentControl->hidden()) {
            //If it's not hidden, draw it.
            currentControl->draw();
        }
    }
}

//OVERRIDES
bool GroupControl::processMouseDown(const InputEvent &event) {
    std::vector<Control *>::reverse_iterator iter;
    bool retval = false;
    //Give this to the appropriate control.
    for (iter = m_controls.rbegin(); iter != m_controls.rend(); iter++) {
        Control &control = **iter;
        if (!control.hidden()) {
            if (control.hasGroupChildren() && !retval) {
                //Do children first.
                GroupControl &group = static_cast< GroupControl & > (control);
                retval = group.processMouseDown(event);
            }
            if (control.hitTest(event.loc) && !retval) {
                retval = control.processMouseDown(event);
            } else {
                control.processUnfocus(event);
            }
        }
    }
    return retval;
}

bool GroupControl::processMouseUp(const InputEvent &event) {
    std::vector<Control *>::reverse_iterator iter;
    //Give this to the appropriate control.
    for (iter = m_controls.rbegin(); iter != m_controls.rend(); iter++) {
        Control &control = **iter;
        if (!control.hidden()) {
            if (control.hasGroupChildren()) {
                //Do children first.
                GroupControl &group = static_cast< GroupControl & > (control);
                if (group.processMouseUp(event)) {
                    return true;
                }
            }
            if (control.hitTest(event.loc)) {
                return control.processMouseUp(event);
            }
        }
    }
    return false;
}

bool GroupControl::processMouseMove(const InputEvent &event) {
    std::vector<Control *>::reverse_iterator iter;
    //Give this to the appropriate control.
    for (iter = m_controls.rbegin(); iter != m_controls.rend(); iter++) {
        Control &control = **iter;
        if (!control.hidden()) {
            if (control.hasGroupChildren()) {
                //Do children first.
                GroupControl &group = static_cast< GroupControl & > (control);
                if (group.processMouseMove(event)) {
                    return true;
                }
            }
            if (control.hitTest(event.loc)) {
                return control.processMouseMove(event);
            }
        }
    }
    return false;
}

bool GroupControl::processMouseDrag(const InputEvent &event) {
    std::vector<Control *>::reverse_iterator iter;
    //Give this to the appropriate control.
    for (iter = m_controls.rbegin(); iter != m_controls.rend(); iter++) {
        Control &control = **iter;
        if (!control.hidden()) {
            if (control.hasGroupChildren()) {
                //Do children first.
                GroupControl &group = static_cast< GroupControl & > (control);
                if (group.processMouseDrag(event)) {
                    return true;
                }
            }
            if (control.hitTest(event.loc)) {
                return control.processMouseDrag(event);
            }
        }
    }
    return false;
}

GroupControl::~GroupControl() {
    for (vector<Control *>::size_type i = 0; i < m_controls.size(); ++i) {
        EventManager::addToDeleteQueue(m_controls[i]);
    }
}

