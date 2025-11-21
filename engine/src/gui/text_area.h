/*
 * text_area.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: David Ranger
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
#ifndef VEGA_STRIKE_ENGINE_GUI_TEXT_AREA_H
#define VEGA_STRIKE_ENGINE_GUI_TEXT_AREA_H

//Version: 1.3  - February 27, 2002
//Changes:
//1.1 - Fixed misaligned down scroll button (FINALLY). Fixed text going past the text area (horizontal). Added multiline text areas.
//1.2 - Added sorting
//1.3 - Added DoMouse to classes. Also fixed bug where button class would always 1 for dragging

/* This class is designed to be self-sufficient.
 * The only external functions it requires that aren't provided by system libs are in glut_support.h
 */

#include "audio/Types.h"
#include <deque>

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if defined (__APPLE__) && defined (__MACH__)
    #include <gl.h>
    #include <glut.h>
#else
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif //tells VCC not to generate min/max macros
    #include <windows.h>
#endif
    #include <gl.h>
    #include <glut.h>
#endif

#include "glut_support.h"
#include "src/gfxlib_struct.h"

#define DIR_TEXT "textures/gui/"

//These defines are used by the image loader
#define TEXT_AREA_00 "textures/gui/button_down.png"
#define TEXT_AREA_01 "textures/gui/button_up.png"
#define TEXT_AREA_02 "textures/gui/corner_top_left.png"
#define TEXT_AREA_03 "textures/gui/corner_top_right.png"
#define TEXT_AREA_04 "textures/gui/corner_bottom_left.png"
#define TEXT_AREA_05 "textures/gui/corner_bottom_right.png"
#define TEXT_AREA_06 "textures/gui/highlight_button_up.png"
#define TEXT_AREA_07 "textures/gui/highlight_button_down.png"
#define TEXT_AREA_08 "textures/gui/horizontal.png"
#define TEXT_AREA_09 "textures/gui/left_side.png"
#define TEXT_AREA_10 "textures/gui/right_side.png"
#define TEXT_AREA_11 "textures/gui/scrollbar.png"
#define TEXT_AREA_12 "textures/gui/highlight_scrollbar.png"
#define TEXT_AREA_13 "textures/gui/horizontal_end.png"

//These defines are used by the class functions to locate the texture in the array (above)
#define IMG_BUTTON_DOWN 0
#define IMG_BUTTON_UP 1
#define IMG_CORNER_TOP_LEFT 2
#define IMG_CORNER_TOP_RIGHT 3
#define IMG_CORNER_BOTTOM_LEFT 4
#define IMG_CORNER_BOTTOM_RIGHT 5
#define IMG_HIGHLIGHT_BUTTON_UP 6
#define IMG_HIGHLIGHT_BUTTON_DOWN 7
#define IMG_HIGHLIGHT_SCROLLBAR 12
#define IMG_TOP 8
#define IMG_BOTTOM 8
#define IMG_LEFT_SIDE 9
#define IMG_RIGHT_SIDE 10
#define IMG_SCROLLBAR 11
#define IMG_END 13

class TextArea {
public:
    TextArea();
    TextArea(float x, float y, float wid, float hei, int scrollbar);
    ~TextArea();

    int GetSelectedItem() const {
        return cur_selected;
    }

    void SetSelectedItem(const int newh) {
        cur_selected = newh;
    }

    void DoHighlight(int yes);        //DoHighlight(0) to disable mouse highlighting. Also disable clicking on entries
    void DoMultiline(const int yes) {
        do_multiline = static_cast<char>(yes);
    }                                                   //DoMultiline(1) to enable multi-line entries
    void Refresh();
    void RenderText();
    void AddTextItem(const char * name,
            const char *          description,
            const char *          parent_name = nullptr,
            const GFXColor        col         = GFXColor(
                    1,
                    1,
                    1,
                    1));
    void ChangeTextItem(const char *name, const char *description, bool wrap = false);
    void ChangeTextItemColor(const char *name, const GFXColor &col);
    void SetText(const char *text);   //Sets the text. Enables Multiline and disables highlighting
    void ClearList();
//Returns the char of the currently selected item. NULL if nothing is selected
    char * GetSelectedItemName() const;
    char * GetSelectedItemDesc() const;
    void   SortList();

//Returns 1 if the click is inside the text area. 0 if it's outside. Same thing for movement
//The class requires you convert the x,y co-ordinate to a -1 to 1 float. The class doesn't know the screen dimensions
    int MouseClick(int button, int state, float x, float y);
    int MouseMove(float x, float y);
//Mouse movement while the button is clicked
    int MouseMoveClick(float x, float y);

//type: 1 is click
//2 is drag
//3 is movement
    int DoMouse(int type, float x, float y, int button, int state);

private:
//Flag that enables/disables the scrollbar (It is not shown if it is disabled). This includes the scroll buttons
    char has_scrollbar;

//Flags that enable/disable Highlighting and Multi-line text
    char do_highlight;
    char do_multiline;

//Array is as follows:
//Entire box (the border), top scroll button, bottom scroll button, entire scrollbar, active scrollbar, text area
    float xcoord[6]{};
    float ycoord[6]{};
    float width[6]{};
    float height[6]{};

//Percentage of space to allocate for the border. [0] is the thin part, [1] is the scrollbar area (right side)
    float ratio[2]{};

//Ratios for the top and button buttons (proportional x,y, height, and width values)
    float button_ratio[4]{};

//Ratios for the scroll bar area (proportional x,y, height, and width values)
    float scrollbar_ratio[4]{};

//Amount of space to display between lines
    float text_spacing;

//Amount of space to allocate to the horizontal bar for each level of text (expanding trees)
    float horizontal_per_level;

//Amount of space between the text and the top/bottom border
    float horizontal_spacer;

//Amount of space to the left of text to have the vertical bar
    float vertical_left_of_text;

//Size of the font. The float version is to prevent rounding errors in calculations
    int font_size;
    float font_size_float;

//The number of lines of text we can display at once
    float max_lines;

//0 for no button clicked, 1 for top button, 2 for bottom button, 3 for scrollbar, 4 for above scrollbar, 5 for below scrollbar
    char button_pressed;

//The currently highlighted item
    int cur_highlighted;

//The number of items in our list
    int item_count;

//The currently selected number
    int cur_selected;

//The number of the first item in the visible area
    int top_item_number;

//The number of items to jump when clicking in the passive area of the scrollbar
    int page_size;

//This stores the y coord of where the mouse was when it started dragging the scrollbar
    float scroll_start;

//This stores the current y axes of the mouse while scrolling
    float scroll_cur;

//Linked list of items that will appear if the text area is a select box (doubles as an expanding tree)
    SharedPtr<class TextAreaItem> item_tree_top;

//Check if that x,y co-ordinate is inside us
    int  Inside(float x, float y, int group) const;
    void LoadTextures();
    void RenderTextItem(SharedPtr<TextAreaItem> current, int level);

    int LocateCount(float y) const;

//Highlighted text (mouse over, selected item) will only occur at a Refresh()
    void HighlightCount(size_t count, int type) const;

    void   DisplayScrollbar();
    char * GetSelectedItem(int type) const;

//Takes a line and puts them in as separate items (line wrapping)
    void ChompIntoItems(const char *text, const char *parent);
};

//Keep everything public so the TextArea class can get faster access to the elements in this class
class TextAreaItem : public SharedFromThis<TextAreaItem> {
public:
    //TextAreaItem(void);
    //parent_class is nullptr for the master TextAreaItem
    explicit TextAreaItem(const char *new_name = "blank", const char *desc = "", SharedPtr<TextAreaItem> parent_class = nullptr);
    ~TextAreaItem();

    //A recursive function. This function will be called to all the children until one of them matches the search_name
    //If no match is found, it will use the main tree.
    SharedPtr<TextAreaItem> FindChild(const char* search_name);
    SharedPtr<TextAreaItem> FindCount(int count, size_t cur);

    void     AddChild(const char *new_name, const char *desc, const GFXColor new_col = GFXColor(1, 1, 1, 1));
    void     ExpandTree();
    void     Sort();
    GFXColor col;
    char *   name;
    char *   description;

    std::deque<SharedPtr<TextAreaItem>> children;

    WeakPtr<TextAreaItem> parent;

    size_t child_count() const {
        return children.size();
    }
};

void LoadTextures();

#endif    //VEGA_STRIKE_ENGINE_GUI_TEXT_AREA_H
