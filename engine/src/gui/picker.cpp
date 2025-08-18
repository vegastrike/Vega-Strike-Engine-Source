/*
 * picker.cpp
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

#include "src/vegastrike.h"

#include "picker.h"

#include "eventmanager.h"
#include "scroller.h"
#include "painttext.h"

#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "root_generic/xml_support.h"

#include <list>

#include "configuration/configuration.h"

//Calculation for indenting children.  Use a factor times total cell height.
static const float CHILD_INDENT_FACTOR = 0.6;

//Make sure we don't get too many re-alloc's in the display vector for cells.
static const int DISPLAY_VECTOR_RESERVE = 30;

//When scrolling a cell into view, how many lines between the cell and the edge.
static const int SCROLL_EDGE_EXTRA = 1;

//Find a cell by id.  Returns NULL if not found.
PickerCell *PickerCells::cellWithId(const std::string &id) {
    for (int i = 0; i < count(); i++) {
        PickerCell *cell = cellAt(i);
        if (cell->id() == id) {
            //Found it.
            return cell;
        }
    }
    //Didn't find a cell with the specified id.
    return NULL;
}

bool PickerCells::saveOpenCategories(std::list<std::list<std::string> > &masterList,
        const std::list<std::string> &parentHier,
        PickerCell *selectedCell) const {
    bool hasSelectedCell = false;
    for (int i = 0; i < count(); i++) {
        const PickerCell *cell = cellAt(i);
        if (cell == selectedCell) {
            hasSelectedCell = true;
        }
        if (!cell->hideChildren()) {
            masterList.push_back(parentHier);
            std::list<std::string> *newItem = &masterList.back();
            (*newItem).push_back(cell->id());
            PickerCells *newCells = cell->children();
            bool savedCell = false;
            if (newCells) {
                savedCell = newCells->saveOpenCategories(masterList, (*newItem), selectedCell);
            }
            if (savedCell) {
                (*newItem).push_back(selectedCell->id());
            }
        }
    }
    return hasSelectedCell;
}

void Picker::saveOpenCategories(std::list<std::list<std::string> > &idList) const {
    std::list<std::string> base;
    cells()->saveOpenCategories(idList, base, m_selectedCell);
}

int Picker::restoreOpenCategories(const std::list<std::list<std::string> > &idList) {
    int numRestored = 0;
    PickerCell *selectedCell = NULL;
    for (std::list<std::list<std::string> >::const_iterator catIt = idList.begin();
            catIt != idList.end();
            ++catIt) {
        PickerCells *celllist = cells();
        for (std::list<std::string>::const_iterator travIt = (*catIt).begin();
                travIt != (*catIt).end();
                ++travIt) {
            PickerCell *cell = celllist->cellWithId((*travIt));
            if (!cell) {
                break;
            }
            celllist = cell->children();
            if (!celllist || celllist->count() == 0) {
                selectedCell = cell;
                break;
            }
            cell->setHideChildren(false);
        }
    }
    setMustRecalc();
    if (selectedCell) {
        selectCell(selectedCell, true);
    }
    return numRestored;
}

//Draw the picker
void Picker::draw(void) {
    //If we need to change the displayed cells, do that first.
    if (m_needRecalcDisplay) {
        recalcDisplay();
    }
    //Draw the background.
    drawBackground();

    const float cellHeight = totalCellHeight();

    //This is the current cell rect.  Start with control rect and adjust y.
    Rect rect(m_rect);
    rect.origin.y += m_rect.size.height - cellHeight;
    rect.size.height = cellHeight;
    for (vector<DisplayCell>::size_type i = m_scrollPosition;
            i < m_displayCells.size() && rect.origin.y > m_rect.origin.y;
            i++) {
        DisplayCell &display = m_displayCells[i];
        const PickerCell *cell = display.cell;         //Get the next cell.

        //Figure background and text color.
        GFXColor backgroundColor = GUI_CLEAR;
        GFXColor textColor = cell->textColor();
        if (cell == m_selectedCell) {
            //Selected state more important than highlighted state.
            backgroundColor = m_selectionColor;
            if (isClear(textColor)) {
                textColor = m_selectionTextColor;
            }
        }
        //Selection color might be clear, or might be highlighted cell.
        if (isClear(backgroundColor) && cell == m_highlightedCell) {
            //Highlighted cell.
            backgroundColor = m_highlightColor;
            if (isClear(textColor)) {
                textColor = m_highlightTextColor;
            }
        }
        if (!isClear(backgroundColor)) {
            drawRect(rect, backgroundColor);
        }
        //If we haven't got a text color yet, use the control's color.
        if (isClear(textColor)) {
            textColor = m_textColor;
        }
        //Include indent in drawing rect.
        //Indent is based on cell height.
        const float indentPerLevel = m_displayCells[i].level * cellHeight * CHILD_INDENT_FACTOR;
        Rect drawRect = rect;
        drawRect.inset(m_textMargins);
        drawRect.origin.x += indentPerLevel;
        drawRect.size.width -= indentPerLevel;

        //Paint the text.
        //There is a PaintText object in each DisplayCell so that we don't have to re-layout the text
        //all the time.  This code should be smarter about only setting the attributes of the text
        //object when things change, but that means cell changes need to be communicated back to
        //this object, and they aren't now.
        display.paintText.setRect(drawRect);
        display.paintText.setText(cell->text());
        display.paintText.setFont(m_font);
        display.paintText.setColor(textColor);
        display.paintText.draw();

        rect.origin.y -= cellHeight;
    }
}

//Return the index of the current selected cell in the list of cells.
//This can only be used if the list simple, not a tree.
//Returns -1 if no selection, or if the selection is a child.
int Picker::selectedItem(void) {
    if (m_cells != NULL && m_selectedCell != NULL) {
        //If we have a selection, find it in the list.  Won't find it if it's a child.
        for (int i = 0; i < m_cells->count(); i++) {
            PickerCell *cell = m_cells->cellAt(i);
            if (cell == m_selectedCell) {
                //Found it.
                return i;
            }
        }
    }
    //Didn't find it.
    return -1;
}

//Find the cell for a mouse point.
PickerCell *Picker::cellForMouse(const Point &point) {
    if (m_rect.inside(point)) {
        const vector<DisplayCell>::size_type index = float_to_int(
                (m_rect.top() - point.y) / totalCellHeight() + m_scrollPosition);
        if (index < m_displayCells.size()) {
            //It's within the cells we are displaying.
            return m_displayCells[index].cell;
        }
    }
    //Didn't find anything.
    return NULL;
}

//Actually cause a cell to be selected.
void Picker::selectCell(PickerCell *cell, bool scroll) {
    PickerCell *oldCell = m_selectedCell;
    m_selectedCell = cell;
    //If the cell has children, flip whether the children are displayed.
    if (cell != NULL) {
        PickerCells *list = cell->children();
        if (list != NULL && list->count() > 0) {
            const bool hideChildren = !cell->hideChildren();
            cell->setHideChildren(hideChildren);
            if (!hideChildren) {
                recalcDisplay();
                //Make sure the children are visible.
                PickerCells *loopList = list;
                PickerCell *lastChild = NULL;
                while (true) {
                    lastChild = loopList->cellAt(loopList->count() - 1);
                    if (lastChild->hideChildren()) {
                        //Can't see children below this.  Done.
                        break;
                    }
                    loopList = lastChild->children();
                    if (loopList == NULL || loopList->count() == 0) {
                        //lastChild has no children.
                        break;
                    }
                }
                scrollToCell(lastChild);
                //Now make sure the original parent is still visible.
                scrollToCell(cell);
            } else {
                setMustRecalc();
            }
        }
        if (scroll) {
            //Make sure the cell is visible.
            scrollToCell(cell);
        }
    }
    if (oldCell != m_selectedCell) {
        sendCommand("Picker::NewSelection", this);
    }
}

//Recursive routine that goes through a cell list and the children
//of the cells and puts them on the display list.
void Picker::addListToDisplay(PickerCells *list, int level) {
    //Go through all the cells in this list.
    for (int i = 0; i < list->count(); i++) {
        PickerCell *cell = list->cellAt(i);
        DisplayCell displayCell(cell, level);
        m_displayCells.push_back(displayCell);         //Add this cell to the list.
        PickerCells *children = cell->children();
        if (!cell->hideChildren() && children != NULL) {
            //We have children to show, so add them, too.
            addListToDisplay(children, level + 1);
        }
    }
}

//Reload the list of cells that are being displayed.
//This should be called when a change is made in the lists of cells, or
//when we scroll, which again changes the cells we display.
//It does not need to be called for text or color changes, only when
//cells are added or removed, etc.
void Picker::recalcDisplay(void) {
    //Clear out the old display list.
    m_displayCells.clear();

    //Recursively refill the display list.
    addListToDisplay(m_cells, 0);
    if (m_scroller) {
        //Update the scroller's view of the number of lines, and try to preserve the scroll position.
        int oldScrollPosition = m_scrollPosition;
        const int visibleCells = float_to_int(m_rect.size.height / totalCellHeight());
        m_scroller->setRangeValues(m_displayCells.size() - 1, visibleCells);
        m_scroller->setScrollPosition(oldScrollPosition);
    }
    //Mark that we don't need to recalc anymore.
    m_needRecalcDisplay = false;
}

//Make sure the cell is visible in the scroll area.  If it is, nothing
//happens.  If it's not, we move it into the visible section.
//If NULL, this routine does nothing.
void Picker::scrollToCell(const PickerCell *cell, bool middle) {
    if (!cell || !m_scroller) {
        return;
    }
    //If we need to change the displayed cells, do that first.
    if (m_needRecalcDisplay) {
        recalcDisplay();
    }
    for (vector<DisplayCell>::size_type i = 0; i < m_displayCells.size(); i++) {
        if (cell == m_displayCells[i].cell) {
            const int visibleCells = float_to_int(m_rect.size.height / totalCellHeight());
            if (middle) {
                //Regardless of where cell is, try to put it in the middle of
                //the visible area.
                m_scroller->setScrollPosition(i - visibleCells / 2);
            } else {
                //Just make sure we can see it.
                if (i < m_scrollPosition) {
                    //Cell is too "high".  Move it to the top line.
                    m_scroller->setScrollPosition(i - SCROLL_EDGE_EXTRA);
                } else if (i >= m_scrollPosition + visibleCells) {
                    //Cell is too "low".  Move it to the bottom line.
                    m_scroller->setScrollPosition(i - visibleCells + 1 + SCROLL_EDGE_EXTRA);
                }
            }
            //Found the cell.  Done with loop.
            break;
        }
    }
}

//Set the object that takes care of scrolling.
void Picker::setScroller(Scroller *s) {
    m_scroller = s;
    s->setCommandTarget(this);
}

//Process a command event.
bool Picker::processCommand(const EventCommandId &command, Control *control) {
    if (command == "Scroller::PositionChanged") {
        assert(control == m_scroller);
        m_scrollPosition = m_scroller->scrollPosition();
        return true;
    }
    return Control::processCommand(command, control);
}

//Mouse clicked down.
bool Picker::processMouseDown(const InputEvent &event) {
    const int zoominc = configuration().general.wheel_increment_lines;
    if (event.code == LEFT_MOUSE_BUTTON) {
        PickerCell *cell = cellForMouse(event.loc);
        if (cell != NULL) {
            //We found the cell that was clicked.
            m_cellPressed = cell;
            setModal(true);                 //Make sure we don't miss anything.
            //Make sure we see mouse events *first* until we get a mouse-up.
            globalEventManager().pushResponder(this);
            return true;
        }
    } else if (event.code == WHEELUP_MOUSE_BUTTON) {
        if (hitTest(event.loc)) {
            m_scroller->setScrollPosition(m_scroller->scrollPosition() - zoominc);
        }
    } else if (event.code == WHEELDOWN_MOUSE_BUTTON) {
        if (hitTest(event.loc)) {
            m_scroller->setScrollPosition(m_scroller->scrollPosition() + zoominc);
        }
    }
    return Control::processMouseDown(event);
}

//Mouse button up.
bool Picker::processMouseUp(const InputEvent &event) {
    if (m_cellPressed && event.code == LEFT_MOUSE_BUTTON) {
        //Select if the mouse goes up inside the pressed cell.
        //If not, consider the action cancelled.
        const bool newSelection = (cellForMouse(event.loc) == m_cellPressed);

        //Make sure we get off the event chain.
        globalEventManager().removeResponder(this, true);
        setModal(false);
        //Select a new cell, after we've cleaned up the event handling.
        if (newSelection) {
            selectCell(m_cellPressed);
        }
        m_cellPressed = NULL;

        return true;
    }
    return Control::processMouseUp(event);
}

//Mouse moved over this control.
bool Picker::processMouseMove(const InputEvent &event) {
    const PickerCell *cell = cellForMouse(event.loc);
    if (cell != NULL) {
        //Change the highlighted cell.
        m_highlightedCell = cell;
    } else {
        //Make sure it's clear.
        m_highlightedCell = NULL;
    }
    return true;
}

//CONSTRUCTION
Picker::Picker(void) :
        m_cells(NULL),
        m_selectionColor(GUI_CLEAR),
        m_selectionTextColor(GUI_CLEAR),
        m_highlightColor(GUI_CLEAR),
        m_highlightTextColor(GUI_CLEAR),
        m_extraCellHeight(0.0),
        m_textMargins(Size(0.0, 0.0)),
        m_cellPressed(NULL),
        m_highlightedCell(NULL),
        m_selectedCell(NULL),
        m_scroller(NULL),
        m_scrollPosition(0),
        m_needRecalcDisplay(true) {
    m_displayCells.reserve(DISPLAY_VECTOR_RESERVE);
}

Picker::~Picker(void) {
}

