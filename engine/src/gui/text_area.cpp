/*
 * Copyright (C) 2001-2022 Daniel Horn, David Ranger, pyramid3d,
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

#include "text_area.h"
#include "gldrv/winsys.h"
#include <cstdlib>
#include <cstring>

//Array of textures for the text area
//GUITexture *Images;

//List of images to load. Last element must always be NULL
/*
 *  char *LoadImages[] = {  TEXT_AREA_00, TEXT_AREA_01, TEXT_AREA_02, TEXT_AREA_03, TEXT_AREA_04,
 *                       TEXT_AREA_05, TEXT_AREA_06, TEXT_AREA_07, TEXT_AREA_08, TEXT_AREA_09,
 *                       TEXT_AREA_10, TEXT_AREA_11, TEXT_AREA_12, TEXT_AREA_13,
 *                       NULL
 *  };
 */

static char EMPTY_STR[] = "";

TextArea::~TextArea() {
}

TextArea::TextArea() {
    TextArea(0, 0, 1, 1, 1);
}

//Currently, corners overlap the horizontal and vertical bars. It's only noticable with transparency
TextArea::TextArea(float x, float y, float wid, float hei, int scrollbar) {
    #ifdef DEBUG
    // stephengtuggy 2020-10-30: Leaving these here, since this library is supposed to be
    // self-sufficient, hence not depend on Vega Strike libraries
    cout<<"Displaying border at "<<x<<","<<y<<"\n";
    cout<<"with the dimensions of "<<wid<<","<<y<<"\n";
    cout<<"Scrollbar: "<<scrollbar<<endl;
    cout<<"------------\n";
    #endif

    LoadTextures();

    //Initialize the variables
    item_count = 0;
    cur_selected = 0;
    top_item_number = 0;
    button_pressed = 0;
    cur_highlighted = 0;
    page_size = 5;
    scroll_start = 0;
    scroll_cur = 0;
    do_highlight = 1;
    do_multiline = 0;

    ratio[0] = 0.06;
    if (scrollbar == 0) {
        ratio[1] = ratio[0];
        has_scrollbar = 0;
    } else {
        ratio[1] = 0.12;
        has_scrollbar = 1;
    }
    //Top and bottom button ratios. Only change these if you change the ratio[1] and/or the image
    //If you change these, uncomment the appropriate ShowColor() in the Refresh to verify the alignment
    button_ratio[0] = 0.91096;       //left x, both buttons
    button_ratio[1] = 0.0345;        //Top button, top left corner y
    button_ratio[2] = 0.0585;        //Height and width of the buttons

    scrollbar_ratio[0] = 0.91096;       //x axis
    scrollbar_ratio[1] = 0.1;           //y axis
    scrollbar_ratio[2] = 0.0585;        //Width
    //scrollbar_ratio[3] = 0.89;	// Height
    scrollbar_ratio[3] = 0.73;          //Height (how much the bar doesn't take of the buttons)

    //Set the variables to control where and how text will be displayed
    font_size = 4;
    font_size_float = 4;
    horizontal_per_level = 0.05;
    horizontal_spacer = 0.01;
    vertical_left_of_text = 0.02;
    text_spacing = (font_size_float / 100) + (horizontal_spacer * 2);

    //The parent TextAreaItem. This is the only link where parent == NULL. It is not displayed and handled only internally
    ItemList = new TextAreaItem("", "", NULL);
    if (wid < 0 || hei < 0) {
        // stephengtuggy 2020-10-30: Leaving this here, since comment at top of
        // file says that this library is supposed to be self-sufficient, so it
        // shouldn't rely on Vega Strike stuff presumably
        printf("Cannot draw border with negative height or width\n");
        return;
    }
    if (x + wid > 1) {
        wid = 1 - x;
    }
    if (y - hei < -1) {
        hei = -1 + y;
    }
    //Set the class variables to remember the dimensions and locations of the text area
    xcoord[0] = x;
    ycoord[0] = y;
    width[0] = wid;
    height[0] = hei;

    //Displayable text area
    //Some parts of y and height are based on the width, not the height, of the text area
    //If you want to see the area created by this, there's a commented ShowColor() at the end of TextArea::Refresh
    xcoord[5] = x + (wid * ratio[0]) + (wid * vertical_left_of_text);
    ycoord[5] = y - (wid * ratio[0]) - (wid * horizontal_spacer);
    width[5] = (wid * (1 - ratio[0] - ratio[1])) - (wid * vertical_left_of_text * 2);
    height[5] = (hei * (1 - ratio[0])) - (wid * horizontal_spacer * 2);

    //Top scroll button
    xcoord[1] = x + (wid * button_ratio[0]);
    ycoord[1] = y - (wid * button_ratio[1]);
    width[1] = wid * button_ratio[2];
    height[1] = width[1];

    //Bottom scroll button
    xcoord[2] = xcoord[1];
    ycoord[2] = y - hei + (wid * (button_ratio[2] + button_ratio[1]));
    //The original non-working patched formula. I spent so much time fixing this, that this line is to remember it. God knows why.
    //ycoord[2] = y - (hei * (1 - button_ratio[1] - button_ratio[3]));
    width[2] = width[1];
    height[2] = width[1];

    //Scrollbar Area
    xcoord[3] = x + (wid * scrollbar_ratio[0]);
    ycoord[3] = y - (wid * scrollbar_ratio[1]);
    width[3] = wid * scrollbar_ratio[2];
    //height[3] = hei * scrollbar_ratio[3];
    height[3] = hei - (height[1] + height[2]) - ((height[2] * scrollbar_ratio[3]) * 2);

    //Static portion of the active scrollbar
    xcoord[4] = xcoord[3];
    width[4] = width[3];

    max_lines = (height[5] / text_spacing) - 1;

    Refresh();
}

void TextArea::Refresh(void) {
    // Obscene amounts of commented code removed, check r13553 for content

    //Draw the bars to run across the length

    if (has_scrollbar != 0) {
        DisplayScrollbar();
    }
    RenderText();
    if (cur_highlighted > 0) {
        HighlightCount(cur_highlighted, 1);
    }
    if (cur_selected > 0) {
        HighlightCount(cur_selected, 2);
    }
}

void TextArea::RenderText(void) {
    if (item_count == 0) {
        return;
    }
    //There's a bug in glut_support. Can't show a color then text. Have to render an image between colors and text
    //ShowImage(0,0,0,0, Images[0], 0, 0);
    RenderTextItem(ItemList, 0);
}

void TextArea::RenderTextItem(TextAreaItem *current, int level) {
    static int count = 0;
    float new_y = 0, new_x = 0;
    int max = 0;
    int cur = 0;
    if (current == 0) {
        return;
    }
    if (level == 0) {
        count = 0;
    }
    if ((count - top_item_number) >= max_lines) {
        return;
    }
    if (level > 0) {
        if (count < top_item_number) {
            count++;
        } else {
            new_y = ycoord[5] - (text_spacing * (count + 1 - top_item_number)) + horizontal_spacer;
            new_x = xcoord[5] + (horizontal_per_level * (level - 1));
            GFXColorf(current->col);
            ShowText(new_x, new_y, width[5], font_size, current->description, do_multiline);
            count++;
        }
    }
    max = current->child_count;
    if (max <= 0) {
        return;
    }
    for (cur = 0; cur < max; cur++) {
        RenderTextItem(current->child[cur], level + 1);
    }
}

void TextArea::AddTextItem(const char *name, const char *description, const char *parent_name, const GFXColor col) {
    TextAreaItem *master;
    master = ItemList->FindChild(parent_name);
    item_count++;
    if (master == NULL) {
        ItemList->AddChild(name, description, col);
    } else {
        master->AddChild(name, description, col);
    }
}

void TextArea::ChangeTextItem(const char *name, const char *description, bool wrap) {
    TextAreaItem *search;
    search = ItemList->FindChild(name);
    if (search == 0) {
        return;
    }
    if (search->description != 0) {
        free(search->description);
    }
    search->description = strdup(description);
}

void TextArea::ChangeTextItemColor(const char *name, const GFXColor &col) {
    TextAreaItem *search;
    search = ItemList->FindChild(name);
    if (search == 0) {
        return;
    }
    search->col = col;
}

void TextArea::ClearList(void) {
    //Wipe the list clean
    if (ItemList != nullptr) {
        delete ItemList;
        ItemList = nullptr;
    }
    item_count = 0;
    cur_selected = 0;
    top_item_number = 0;
    ItemList = new TextAreaItem("", "", NULL);
}

void TextArea::SetText(const char *text) {
    do_highlight = 0;
    do_multiline = 1;
    ClearList();
    ChompIntoItems(text, NULL);
}

char *TextArea::GetSelectedItemName(void) {
    return GetSelectedItem(1);
}

char *TextArea::GetSelectedItemDesc(void) {
    return GetSelectedItem(2);
}

char *TextArea::GetSelectedItem(int type) {
    TextAreaItem *search;
    search = ItemList->FindCount(cur_selected, 0);
    if (search == 0) {
        return EMPTY_STR;
    }
    if (type == 1) {
        return search->name;
    } else {
        return search->description;
    }
}

void TextArea::SortList(void) {
    ItemList->Sort();
}

//The button checks assume that the scroll buttons and scrollbar are on the same x axis
//If you change the position of the buttons, you'll need to add more checks here
int TextArea::MouseClick(int button, int state, float x, float y) {
    if (state == WS_MOUSE_UP && scroll_start != 0) {
        scroll_cur = 0;
        scroll_start = 0;
        return 1;
    }
    if (Inside(x, y, 0) == 0) {
        return 0;
    }
    if (button != WS_LEFT_BUTTON) {
        return 1;
    }                                        //Don't have anything to do with the middle and right button
    //Check to see if the cursor is in the same x axis as the buttons and scrollbar
    if (x > xcoord[1] && x < (xcoord[1] + width[1])) {
        //Find out if the click is on a button, the scrollbar, or nowhere
        if (y < ycoord[1] && y > (ycoord[1] - height[1])) {
            if (state == WS_MOUSE_UP) {
                //ShowImage(xcoord[1], ycoord[1], width[1], height[1], Images[IMG_BUTTON_UP], 0, 0);
                button_pressed = 0;
            } else {
                //ShowImage(xcoord[1], ycoord[1], width[1], height[1], Images[IMG_HIGHLIGHT_BUTTON_UP], 0, 0);
                button_pressed = 1;

                top_item_number--;
                if (top_item_number < 0) {
                    top_item_number = 0;
                }
                if (cur_selected < 0) {
                    cur_selected = 0;
                }
            }
        }
        if (y < ycoord[2] && y > (ycoord[2] - height[2])) {
            if (state == WS_MOUSE_UP) {
                //ShowImage(xcoord[2], ycoord[2], width[2], height[2], Images[IMG_BUTTON_DOWN], 0, 0);
                button_pressed = 0;
            } else {
                //ShowImage(xcoord[2], ycoord[2], width[2], height[2], Images[IMG_HIGHLIGHT_BUTTON_DOWN], 0, 0);
                button_pressed = 2;

                top_item_number++;
                if (top_item_number >= (item_count - max_lines)) {
                    //This is to avoid a compile time warning.
                    #ifdef NO_WARNINGS
                    float tmp = item_count - max_lines;
                    char LINE[10];
                    sprintf(LINE, "%.0f", tmp);
                    top_item_number = atoi(LINE);
                    #else
                    top_item_number = item_count-max_lines;
                    #endif    //NO_WARNINGS
                }
            }
        }
    }
    if (Inside(x, y, 5) != 0 && do_highlight > 0) {
        cur_selected = LocateCount(y);
    }
    if (Inside(x, y, 3) != 0 && Inside(x, y, 4) == 0) {
        if (state != WS_MOUSE_UP) {
            return 1;
        }           //We're outside the active scroll bar, but in the passive area. The active scrollbar is only for MouseMoveClick
        if (y > ycoord[4]) {
            //Top area
            top_item_number -= page_size;
            if (top_item_number < 0) {
                top_item_number = 0;
            }
        } else {
            //Bottom area
            top_item_number += page_size;
            if (top_item_number >= (item_count - max_lines)) {
                #ifdef NO_WARNINGS
                float tmp = item_count - max_lines;
                char LINE[10];
                sprintf(LINE, "%.0f", tmp);
                top_item_number = atoi(LINE);
                #else
                top_item_number = item_count-max_lines;
                #endif    //NO_WARNINGS
            }
        }
    }
    if (Inside(x, y, 4)) {
        scroll_start = y;
    }
    return 1;
}

int TextArea::MouseMove(float x, float y) {
    if (do_highlight == 0) {
        return 0;
    }
    if (Inside(x, y, 5) == 0) {
        return 0;
    }
    cur_highlighted = LocateCount(y);
    return 1;
}

int TextArea::MouseMoveClick(float x, float y) {
    if (scroll_start == 0 && Inside(x, y, 4) == 0) {
        return 0;
    }
    if (scroll_start == 0) {
        scroll_start = y;
    }
    scroll_cur = y;
    return 1;
}

int TextArea::DoMouse(int type, float x, float y, int button, int state) {
    if (type == 1) {
        return MouseClick(button, state, x, y);
    }
    if (type == 2) {
        return MouseMoveClick(x, y);
    }
    if (type == 3) {
        return MouseMove(x, y);
    }
    return 0;
}

int TextArea::Inside(float x, float y, int group) {
    if (x < xcoord[group] || y > ycoord[group]) {
        return 0;
    }
    if (x > (xcoord[group] + width[group])) {
        return 0;
    }
    if (y < (ycoord[group] - height[group])) {
        return 0;
    }
    return 1;
}

void TextArea::LoadTextures(void) {
    /*
     *  int cur, max;
     *  static int images_loaded = 0;
     *  if (images_loaded == 1) { return; }	// We've already loaded the images from a previous TextArea class
     *  else { images_loaded = 1; }
     *
     *  max = 0;
     *  while (LoadImages[max] != NULL) { max++; }
     *  Images = new GUITexture [max];
     * #ifdef DEBUG
     *  cout << "Loading " << max << " images\n";
     * #endif
     *  for (cur = 0; cur < max; cur++) {
     *       Images[cur] = ReadTex(LoadImages[cur]);
     * #ifdef DEBUG
     *       cout << "\tLoading: #" << cur << ": " << LoadImages[cur] << endl;
     * #endif
     *  }
     */
}

//Assumes the mouse is in the text area
int TextArea::LocateCount(float y) {
    float base = ycoord[5] - y - horizontal_spacer;
    if (base < 0) {
        return 0;
    }
    base /= text_spacing;
    base += 0.5;
    #ifdef NO_WARNINGS
    char LINE[1024];
    sprintf(LINE, "%.0f", base);
    return atoi(LINE);

    #else
    return base;
    #endif    //NO_WARNINGS
}

void TextArea::HighlightCount(int count, int type) {
    float x = 0, y = 0;
    if (count <= 0 || count > max_lines + 1) {
        return;
    }
    y = ycoord[5] - (text_spacing * (count - 1)) - horizontal_spacer + ((text_spacing - (font_size_float / 100)) / 2);
    x = xcoord[5];
    if (count <= this->ItemList->child_count) {
        if (type == 1) {
            ShowColor(x, y, width[5], text_spacing, 1, 1, 1, 0.25);
        }
        if (type == 2) {
            ShowColor(x, y, width[5], text_spacing, 0.2, 0.2, 0.4, 0.5);
        }
    }
}

void TextArea::DisplayScrollbar(void) {
    float new_y = 0, new_height = 0, item_perc = 0, page_perc = 0, y_dist = 0;
    ShowColor(xcoord[3], ycoord[3], width[3], height[3], 0.51, 0.47, 0.79, 1);
    if (item_count <= max_lines) {
        ShowColor(xcoord[3], ycoord[3], width[3], height[3], 0.66, 0.6, 1, 1);
        ShowColor(xcoord[3], ycoord[3], width[3], height[3], 0, 1, 1, 0.05);
        return;
    }
    //The percentage that each item consists of the entire list
    item_perc = item_count;
    item_perc = 1 / item_perc;

    //The percentage that the visible list takes of the entire list
    page_perc = max_lines;
    page_perc /= item_count;
    //If this isn't 0, the scrollbar is being moved
    if (scroll_cur != 0) {
        float move = ((scroll_cur - scroll_start) / height[3]) / item_perc;
        int change = 0;
        #ifdef NO_WARNINGS
        char LINE[10];
        sprintf(LINE, "%.0f", move);
        change = atoi(LINE);
        #else
        change = move;
        #endif    //NO_WARNINGS
        if (move < 0) {
            change *= -1;
            top_item_number += change;
            if (top_item_number >= (item_count - max_lines)) {
                #ifdef NO_WARNINGS
                sprintf(LINE, "%.0f", item_count - max_lines);
                top_item_number = atoi(LINE);
                #else
                top_item_number = item_count-max_lines;
                #endif    //NO_WARNINGS
            }
        } else {
            top_item_number -= change;
            if (top_item_number < 0) {
                top_item_number = 0;
            }
        }
        if (change != 0) {
            scroll_start = scroll_cur;
        }
    }
    //How much is scrolled up past the view
    y_dist = item_perc * top_item_number;

    new_y = ycoord[3] - (y_dist * height[3]);
    new_height = page_perc * height[3];

    ycoord[4] = new_y;
    height[4] = new_height;

    ShowColor(xcoord[3], new_y, width[3], new_height, 0.66, 0.6, 1, 1);
    if (scroll_start == 0) {
        ShowColor(xcoord[3], new_y, width[3], new_height, 0, 1, 1, 0.05);
    } else {
        ShowColor(xcoord[3], new_y, width[3], new_height, 1, 0, 0, 0.05);
    }
}

void TextArea::ChompIntoItems(const char *text, const char *parent) {
    char *temp = strdup(text);
    char *cur = temp, chr = '\0';
    int i = 0, max = strlen(temp);
    float cur_width = 0, wid = 0, end = glutStrokeWidth(GLUT_STROKE_ROMAN, 'A');
    end /= 2500;
    wid = end;
    for (i = 0; i <= max; i++) {
        if (temp[i] == '\r') {
            continue;
        }
        cur_width = WidthOfChar(temp[i]);
        if (wid + cur_width > width[5] || temp[i] == '\n') {
            chr = temp[i];
            temp[i] = '\0';
            AddTextItem("content", cur, parent);
            temp[i] = chr;
            cur = &temp[i];
            if (cur[0] == '\n') {
                cur++;
            }
            wid = end;
        } else {
            wid += cur_width;
        }
    }
    if (temp[i] != '\0') {
        AddTextItem("content", cur, parent);
    }
    free(temp);
}

/*TextAreaItem::TextAreaItem(void) {
 *       TextAreaItem("blank","", NULL);
 *  }*/
//#include <stdlib.h>
//#define rnd (((float)rand())/((float)RAND_MAX))
TextAreaItem::TextAreaItem(const char *new_name, const char *desc, TextAreaItem *parent_class) :
        col(1, 1, 1, 1) {
//{	col = GFXColor (rnd,rnd,rnd,1);
    if (new_name != 0) {
        name = strdup(new_name);
    } else {
        name = 0;
    }
    if (desc != 0) {
        description = strdup(desc);
    } else {
        description = 0;
    }
    child_count_multiplier = 0;
    child_count = 0;
    child = NULL;
    parent = NULL;
//if (parent == NULL) { expanded = 1; }
}

TextAreaItem::~TextAreaItem(void) {
    if (name != nullptr) {
        free(name);
        name = nullptr;
    }
    if (description != nullptr) {
        free(description);
        description = nullptr;
    }
    //if there are no children, it won't run through this for
    for (int cur = 0; cur < child_count; cur++) {
        if (child[cur] != nullptr) {
            delete child[cur];
            child[cur] = nullptr;
        }
    }
    if (child != nullptr) {
        delete child;
        child = nullptr;
    }
}

TextAreaItem *TextAreaItem::FindChild(const char *search_name) {
    int cur = 0;
    //int max = child_count_multiplier * 10;
    TextAreaItem *match;
    if (search_name == NULL) {
        return this;
    }
    if (strcmp(name, search_name) == 0) {
        return this;
    }
    for (cur = 0; cur < child_count; cur++) {
        match = child[cur]->FindChild(search_name);
        if (match != NULL) {
            return match;
        }
    }
    return NULL;
}

void TextAreaItem::Sort(void) {
    int cur, cur2;
    TextAreaItem *temp;
    for (cur = 0; cur < child_count; cur++) {
        child[cur]->Sort();
        for (cur2 = 0; cur2 < child_count - 1; cur2++) {
            if (strcmp(child[cur2]->description, child[cur2 + 1]->description) > 0) {
                temp = child[cur2];
                child[cur2] = child[cur2 + 1];
                child[cur2 + 1] = temp;
            }
        }
    }
}

TextAreaItem *TextAreaItem::FindCount(int count, int cur) {
    static int current = 0;
    if (cur == 0) {
        current = 0;
    }       //int max = child_count_multiplier * 10;
    TextAreaItem *match;
    if (count == current) {
        return this;
    }
    current++;
    for (cur = 0; cur < child_count; cur++) {
        match = child[cur]->FindCount(count, cur + 1);
        if (match != NULL) {
            return match;
        }
    }
    return NULL;
}

typedef TextAreaItem *TextAreaItemStr;

void TextAreaItem::AddChild(const char *new_name, const char *desc, const GFXColor col) {
    TextAreaItem **newlist;
    int cur = 0;
    child_count++;
    if (child_count > child_count_multiplier * 10) {
        child_count_multiplier++;
        newlist = new TextAreaItemStr[child_count_multiplier * 10];
        if (child != NULL) {
            for (cur = 0; cur < child_count - 1; cur++) {
                newlist[cur] = child[cur];
            }
            delete child;
        }
        child = newlist;
    }
    child[child_count - 1] = new TextAreaItem(new_name, desc, NULL);
    child[child_count - 1]->col = col;
}

