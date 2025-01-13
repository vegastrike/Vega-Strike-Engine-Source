/*
 * simplepicker.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GUI_SIMPLEPICKER_H
#define VEGA_STRIKE_ENGINE_GUI_SIMPLEPICKER_H

#include "picker.h"

//See cpp file for detailed descriptions of classes, functions, etc.

class SimplePickerCells;      //Forward reference.

//One entry in a SimplePicker.
//This is a simple value class
class SimplePickerCell : public PickerCell {
public:
//The text to be displayed.
    virtual std::string text(void) const {
        return m_text;
    }

//A unique identifier for the cell.
    virtual std::string id(void) const {
        return m_id;
    }

//The color of the text.
    virtual GFXColor textColor(void) const {
        return m_textColor;
    }

//List of children.
    virtual PickerCells *children(void) const {
        return (PickerCells *) m_children;
    }

//Unique identifier for this cell.
    virtual int tag(void) const {
        return m_tag;
    }

    virtual void setTextColor(const GFXColor &c) {
        m_textColor = c;
    }

//Add a child to the list of children of this cell.
    void addChild(SimplePickerCell *c);
//Make sure there is an empty list for children.
    SimplePickerCells *createEmptyChildList(void);

//CONSTRUCTION
    SimplePickerCell(const std::string &t, const std::string &id = "", const GFXColor &c = GUI_CLEAR, int newTag = 0);
    virtual ~SimplePickerCell(void);

//OPERATIONS
    SimplePickerCell &operator=(const SimplePickerCell &cell);
    SimplePickerCell(const SimplePickerCell &cell);

protected:
    std::string m_text;
    std::string m_id;
    GFXColor m_textColor;
    int m_tag;
    SimplePickerCells *m_children;
};

//A list of Picker cells to show.
class SimplePickerCells : public PickerCells {
public:
//Number of cells in this list.
    virtual int count(void) const {
        return m_cells.size();
    }

//Get a particular cell.
    virtual PickerCell *cellAt(int index) {
        return m_cells[index];
    }

    virtual const PickerCell *cellAt(int index) const {
        return m_cells[index];
    }

//Add a new cell to this list.
    virtual void addCell(PickerCell *c) {
        m_cells.push_back(c);
    }

//Clear out all the cells.
    virtual void clear(void);

//CONSTRUCTION
    SimplePickerCells(void);

    virtual ~SimplePickerCells(void) {
        clear();
    }

protected:
    std::vector<PickerCell *> m_cells;
};

//The Picker class supports a list of items that can be
//scrolled and selected.
//The list can be a tree.  When a branch is selected, its children
//are displayed.  When it is selected again, they are hidden.
class SimplePicker : public Picker {
public:
//Add a new cell to this control.
    void addCell(SimplePickerCell *c) {
        static_cast< SimplePickerCells * > (m_cells)->addCell(c);
    }

//Clear out all the cells.
    void clear(void);

    SimplePicker(void);
    virtual ~SimplePicker(void);
};

//This is a picker cell that can hold any type of data
//by using templates.
template<class T>
class ValuedPickerCell : public SimplePickerCell {
public:
//CONSTRUCTION
    ValuedPickerCell(T value,
            const std::string &text,
            const std::string &id = "",
            const GFXColor &c = GUI_CLEAR,
            int newTag = 0) :
            SimplePickerCell(text, id, c, newTag), m_value(value) {
    }

    virtual ~ValuedPickerCell(void) {
    }

//The type associated with the cell.
    T value(void) const {
        return m_value;
    }

    ValuedPickerCell(const ValuedPickerCell &cell) :
            SimplePickerCell(cell), m_value(cell.m_value) {
    }

protected:
    T m_value;
};

#endif   //VEGA_STRIKE_ENGINE_GUI_SIMPLEPICKER_H
