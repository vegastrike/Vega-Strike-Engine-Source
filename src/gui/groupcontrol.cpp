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

#include "groupcontrol.h"

#include "window.h"


// Add a new control to this collection.
void GroupControl::addChild(Control* child) {
    m_controls.push_back(child);
}

// Delete a control that is in this collection.
    //  Returns true if successful.
bool GroupControl::deleteControl(Control* c) {
    vector<Control*>::iterator iter;
    for(iter = m_controls.begin() ; iter != m_controls.end() ; iter++ ) {
        Control* currentControl = *iter;
        if(c == currentControl) {
            // Found it in this group.
            m_controls.erase(iter);
            delete currentControl;
            return true;
        }
        if(currentControl->hasGroupChildren()) {
            // Check the children.
            GroupControl* group = dynamic_cast<GroupControl*>( currentControl );
            if(group->deleteControl(c)) {
                return true;
            }
        }
    }

    return false;
}

// Take a control away from this collection and save it elsewhere.
Control* GroupControl::removeControlFromGroup(Control* c) {
    vector<Control*>::iterator iter;
    for(iter = m_controls.begin() ; iter != m_controls.end() ; iter++ ) {
        Control* currentControl = *iter;
        if(c == currentControl) {
            // Found it in this group.
            m_controls.erase(iter);
            return currentControl;
        }
        if(currentControl->hasGroupChildren()) {
            // Check the children.
            GroupControl* group = dynamic_cast<GroupControl*>( currentControl );
            Control* ret = group->removeControlFromGroup(c);
            if(ret) {
                return ret;
            }
        }
    }

    return NULL;
}

// Find a control using its id.  NULL returned if none found.
// Note that the control may be hidden.
Control* GroupControl::findControlById(const std::string& id) {
    vector<Control*>::iterator iter;
    for(iter = m_controls.begin() ; iter != m_controls.end() ; iter++ ) {
        Control* currentControl = *iter;
        if(currentControl->id() == id) {
            // Found it in this group.
            return currentControl;
        }
        if(currentControl->hasGroupChildren()) {
            // Check the children.
            GroupControl* group = dynamic_cast<GroupControl*>( currentControl );
            Control* ret = group->findControlById(id);
            if(ret) {
                return ret;
            }
        }
    }

    return NULL;
}

// Draw the control.  Return true if anything is drawn.
// This should not draw outside its rectangle!
bool GroupControl::draw(void) {
    bool result = false;
    vector<Control*>::iterator iter;
    for(iter = m_controls.begin() ; iter != m_controls.end() ; iter++ ) {
        Control* currentControl = *iter;
        if(!currentControl->hidden()) {
            // If it's not hidden, draw it and its children.
            result |= currentControl->draw();
            if(currentControl->hasGroupChildren()) {
                GroupControl* group = dynamic_cast<GroupControl*>( currentControl );
                result |= group->draw();
            }
        }
    }

    return result;
}

// OVERRIDES
bool GroupControl::processMouseDown(const InputEvent& event) {
    vector<Control*>::reverse_iterator iter;

    // Give this to the appropriate control.
    for(iter = m_controls.rbegin() ; iter != m_controls.rend() ; iter++ ) {
        Control& control = **iter;
        if(!control.hidden()) {
            if(control.hasGroupChildren()) {
                // Do children first.
                GroupControl& group = dynamic_cast<GroupControl&>( control );
                if(group.processMouseDown(event)) {
                    return true;
                }
            }
            if(control.hitTest(event.loc)) {
                return control.processMouseDown(event);
            }
        }
    }

    return false;
}

bool GroupControl::processMouseUp(const InputEvent& event) {
    vector<Control*>::reverse_iterator iter;

    // Give this to the appropriate control.
    for(iter = m_controls.rbegin() ; iter != m_controls.rend() ; iter++ ) {
        Control& control = **iter;
        if(!control.hidden()) {
            if(control.hasGroupChildren()) {
                // Do children first.
                GroupControl& group = dynamic_cast<GroupControl&>( control );
                if(group.processMouseUp(event)) {
                    return true;
                }
            }
            if(control.hitTest(event.loc)) {
                return control.processMouseUp(event);
            }
        }
    }

    return false;
}

bool GroupControl::processMouseMove(const InputEvent& event) {
    vector<Control*>::reverse_iterator iter;

    // Give this to the appropriate control.
    for(iter = m_controls.rbegin() ; iter != m_controls.rend() ; iter++ ) {
        Control& control = **iter;
        if(!control.hidden()) {
            if(control.hasGroupChildren()) {
                // Do children first.
                GroupControl& group = dynamic_cast<GroupControl&>( control );
                if(group.processMouseMove(event)) {
                    return true;
                }
            }
            if(control.hitTest(event.loc)) {
                return control.processMouseMove(event);
            }
        }
    }

    return false;
}

bool GroupControl::processMouseDrag(const InputEvent& event) {
    vector <Control*>::reverse_iterator iter;

    // Give this to the appropriate control.
    for(iter = m_controls.rbegin() ; iter != m_controls.rend() ; iter++ ) {
        Control& control = **iter;
        if(!control.hidden()) {
            if(control.hasGroupChildren()) {
                // Do children first.
                GroupControl& group = dynamic_cast<GroupControl&>( control );
                if(group.processMouseDrag(event)) {
                    return true;
                }
            }
            if(control.hitTest(event.loc)) {
                return control.processMouseDrag(event);
            }
        }
    }

    return false;
}


GroupControl::~GroupControl() {
    for(int i=0; i < m_controls.size(); i++) {
        delete m_controls[i];
    }
}
