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

#ifndef __SIMPLEPICKER_H__
#define __SIMPLEPICKER_H__

#include "picker.h"

// See cpp file for detailed descriptions of classes, functions, etc.

class SimplePickerCells;      // Forward reference.


// One entry in a SimplePicker.
// This is a simple value class
class SimplePickerCell : public PickerCell
{
public:
    // The text to be displayed.
    virtual std::string text(void) const { return m_text; };
    // A unique identifier for the cell.
    virtual std::string id(void) const { return m_id; };
    // The color of the text.
    virtual GFXColor textColor(void) const { return m_textColor; };
    // List of children.
    virtual PickerCells* children(void) const { return (PickerCells*)m_children; };
    // Unique identifier for this cell.
    virtual int tag(void) const { return m_tag; };

    virtual void setTextColor(const GFXColor& c) { m_textColor = c; };

    // Add a child to the list of children of this cell.
    void addChild(const SimplePickerCell& c);
    // Make sure there is an empty list for children.
    SimplePickerCells* createEmptyChildList(void);

    // CONSTRUCTION
    SimplePickerCell(const std::string& t, const std::string& id = "", const GFXColor& c = GUI_CLEAR, int newTag = 0);
    virtual ~SimplePickerCell(void);

    // OPERATIONS
    SimplePickerCell& operator=(const SimplePickerCell& cell);
    SimplePickerCell(const SimplePickerCell& cell);

protected:
    std::string m_text;
    std::string m_id;
    GFXColor m_textColor;
    int m_tag;
    SimplePickerCells* m_children;
};

// A list of Picker cells to show.
class SimplePickerCells : public PickerCells
{
public:
    // Number of cells in this list.
    virtual int count(void) const { return m_cells.size(); };
    // Get a particular cell.
    virtual PickerCell* cellAt(int index) { return &m_cells[index]; };

    // Add a new cell to this list.
    void addCell(const PickerCell& c) { m_cells.push_back(*(dynamic_cast<const SimplePickerCell*>(&c))); };

    // Clear out all the cells.
    void clear(void) { m_cells.clear(); };

    // CONSTRUCTION
    SimplePickerCells(void);
    virtual ~SimplePickerCells(void) {};

protected:
    std::vector<SimplePickerCell> m_cells;
};

// The Picker class supports a list of items that can be
//  scrolled and selected.
// The list can be a tree.  When a branch is selected, its children
//  are displayed.  When it is selected again, they are hidden.
class SimplePicker : public Picker
{
public:
    // Add a new cell to this control.
    void addCell(const SimplePickerCell& c) { m_realCells.addCell(c); };

    // Clear out all the cells.
    void clear(void);

    // Get cell collection.
    PickerCells* cells(void) { return &m_realCells; };

    // CONSTRUCTION
public:
    SimplePicker(void);
    virtual ~SimplePicker(void);

    // VARIABLES
protected:
    SimplePickerCells m_realCells;
};

#endif   // __SIMPLEPICKER_H__
