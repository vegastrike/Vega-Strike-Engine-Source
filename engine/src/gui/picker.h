/*
 * picker.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_PICKER_H
#define VEGA_STRIKE_ENGINE_GUI_PICKER_H

#include "control.h"
#include "painttext.h"

#include <list>

//See cpp file for detailed descriptions of classes, functions, etc.

//FORWARD REFERENCES.
class PickerCells;
class Scroller;

//One entry in a Picker.
//This is a virtual base class that is the minimum needed to display
//cells in a Picker.
class PickerCell {
public:
    virtual ~PickerCell() {
    }

//The text to be displayed.
    virtual std::string text(void) const = 0;
//A unique identifier for the cell.
    virtual std::string id(void) const = 0;
//The color of the text.
    virtual GFXColor textColor(void) const = 0;
//List of children.
    virtual PickerCells *children(void) const = 0;
//Unique identifier for this cell.
    virtual int tag(void) const = 0;

//Whether to hide or show child cells.
    virtual bool hideChildren(void) const {
        return m_hideChildren;
    }

//Set whether to hide or show child cells.
    virtual void setHideChildren(bool f) {
        m_hideChildren = f;
    }

    PickerCell(bool hideChildren = true) : m_hideChildren(hideChildren) {
    }

protected:
    bool m_hideChildren;
};

//A list of Picker cells to show.
//This is a virtual base class that is the minimum needed to display
//cells in a Picker.
class PickerCells {
public:
    virtual ~PickerCells() {
    }

//Number of cells in this list.
    virtual int count(void) const = 0;
//Get a particular cell.
    virtual PickerCell *cellAt(int index) = 0;
//Get a particular cell (const).
    virtual const PickerCell *cellAt(int index) const = 0;
//Find a cell by id.  Returns NULL if not found.
    virtual PickerCell *cellWithId(const std::string &id);

//Utility functions:
//Saves all open children categories in a list.
//Returns true if this list directly contains the selectedCell.
    bool saveOpenCategories(std::list<std::list<std::string> > &masterList,
            const std::list<std::string> &parentHier,
            PickerCell *selectedCell) const;
};

//The Picker class supports a list of items that can be
//scrolled and selected.
//The list can be a tree.  When a branch is selected, its children
//are displayed.  When it is selected again, they are hidden.
class Picker : public Control {
public:

    void saveOpenCategories(std::list<std::list<std::string> > &idList) const;
    int restoreOpenCategories(const std::list<std::list<std::string> > &idList);

//Draw the list.
    virtual void draw(void);

//Get cell collection.
    virtual PickerCells *cells(void) {
        return m_cells;
    }

    virtual const PickerCells *cells(void) const {
        return m_cells;
    }

//Make a cell selected.
    virtual void selectCell(PickerCell *cell, bool scroll = false);

//Return the cell that is currently selected.  Can be NULL.
    virtual PickerCell *selectedCell(void) {
        return m_selectedCell;
    }

//Return the index of the current selected cell in the list of cells.
//This can only be used if the list simple, not a tree.
//Returns -1 if no selection, or if the selection is a child.
    virtual int selectedItem(void);

//Make sure the cell is visible in the scroll area.  If it is, nothing
//happens.  If it's not, we move it into the visible section.
//If NULL, this routine does nothing.
    virtual void scrollToCell(const PickerCell *cell, bool middle = false);

//This should be called when the lists of cells have been changed
//in some way -- added to, "hide children" changed, etc.
//It causes this object to figure out from the cell lists which
//cells are actually being displayed in the control.
    virtual void setMustRecalc(void) {
        m_needRecalcDisplay = true;
    }

//Extra space between cells.  This is based on font size.
//1.0 = Cells are twice as high as they would normally be.
//Cells are vertically centered in the height alloted.
//This can be negative, which will remove space, but text
//quality may be affected.
    virtual float extraCellHeight(void) {
        return m_extraCellHeight;
    }

    virtual void setExtraCellHeight(float f) {
        m_extraCellHeight = f;
    }

//Background color when cell is selected.
    virtual GFXColor selectionColor(void) {
        return m_selectionColor;
    }

    virtual void setSelectionColor(const GFXColor &c) {
        m_selectionColor = c;
    }

//Text color when mouse is over button.
    virtual GFXColor selectionTextColor(void) {
        return m_selectionTextColor;
    }

    virtual void setSelectionTextColor(const GFXColor &c) {
        m_selectionTextColor = c;
    }

//Background color when mouse is over button.
    virtual GFXColor highlightColor(void) {
        return m_highlightColor;
    }

    virtual void setHighlightColor(const GFXColor &c) {
        m_highlightColor = c;
    }

//Text color when mouse is over button.
    virtual GFXColor highlightTextColor(void) {
        return m_highlightTextColor;
    }

    virtual void setHighlightTextColor(const GFXColor &c) {
        m_highlightTextColor = c;
    }

//Text margins.
    virtual Size textMargins(void) {
        return m_textMargins;
    }

    virtual void setTextMargins(const Size &s) {
        m_textMargins = s;
    }

//Set the object that takes care of scrolling.
    virtual void setScroller(Scroller *s);

//OVERRIDES
    virtual bool processMouseDown(const InputEvent &event);
    virtual bool processMouseUp(const InputEvent &event);
    virtual bool processMouseMove(const InputEvent &event);

//Process a command event.
    virtual bool processCommand(const EventCommandId &command, Control *control);

//CONSTRUCTION
public:
    Picker(void);
    virtual ~Picker(void);

protected:
//INTERNAL IMPLEMENTATION
//The total vertical space between displayed cells.
    float totalCellHeight(void) {
        return m_font.size() + m_extraCellHeight;
    }

//Find the cell that corresponds to a point in the control.
    PickerCell *cellForMouse(const Point &p);

//Reload the list of cells that are being displayed.
//This should be called when a change is made in the lists of cells, or
//when we scroll, which again changes the cells we display.
//It does not need to be called for text or color changes, only when
//cells are added or removed, etc.
    virtual void recalcDisplay(void);

//Recursive routine that goes through a cell list and the children
//of the cells and puts them on the display list.
    void addListToDisplay(PickerCells *list, int level);

//VARIABLES
protected:
//Struct to store displayed cells.
    struct DisplayCell {
        PickerCell *cell;
        int level;              //How much indent for this cell.  0 = no indent.
        PaintText paintText;  //Object containing drawn text.

        DisplayCell(PickerCell *c = NULL, int l = 0) : cell(c), level(l), paintText() {
        }
    };

    PickerCells *m_cells;               //The collection containing the list entries.
    GFXColor m_selectionColor;      //Selected cell background color.
    GFXColor m_selectionTextColor;  //Selected cell text color.
    GFXColor m_highlightColor;      //Highlighted cell background color.
    GFXColor m_highlightTextColor;  //Highlighted cell text color.
    float m_extraCellHeight;            //Extra height to be added to each cell. 1.0 for whole cell height.
    Size m_textMargins;                 //Inset area where no text appears.
    PickerCell *m_cellPressed;         //Item that mouse-down came on.
    const PickerCell *m_highlightedCell; //Cell that mouse is currently over.
    PickerCell *m_selectedCell;        //Cell that is currently selected.
    Scroller *m_scroller;
    size_t m_scrollPosition;               //Index of first display cell shown.
    bool m_needRecalcDisplay;         //True = Need to recalculate the displayed cells.
    std::vector<DisplayCell> m_displayCells;   //Array of cells currently displayed.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_PICKER_H
