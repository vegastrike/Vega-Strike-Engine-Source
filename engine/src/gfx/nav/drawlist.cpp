/*
 * drawlist.cpp
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


#include "gui/glut_support.h"

#include "gfx/hud.h"

#include "cmd/unit_generic.h"

#include "gui/glut_support.h"

#include "src/universe_util.h"

#include "src/config_xml.h"

#include "gfx/nav/drawlist.h"
#include "gfx/masks.h"

navdrawnode::navdrawnode()      //new undefined node, check for these values if wondering if assignment didnt happen.
{
    type = -1;
    size = 0.0;
    x = 0;
    y = 0;
    nextitem = NULL;
    source = NULL;
}

navdrawnode::navdrawnode(int type_, float size_, float x_, float y_, navdrawnode *nextitem_)    //new node into list
{
    type = type_;
    size = size_;
    x = x_;
    y = y_;
    nextitem = nextitem_;
    source = NULL;
}

navdrawnode::navdrawnode(int type_,
        float size_,
        float x_,
        float y_,
        Unit *source_,
        navdrawnode *nextitem_)     //new node into list
{
    type = type_;
    size = size_;
    x = x_;
    y = y_;
    nextitem = nextitem_;
    source = source_;
}

navdrawlist::navdrawlist(bool mouse,
        navscreenoccupied *screenoccupation_,
        GFXColor *factioncolours_)   //start list with a 'mouselist' flag
{
    inmouserange = mouse;
    head = NULL;
    tail = NULL;
    unselectedalpha = 0.8;
    n_contents = 0;
    screenoccupation = screenoccupation_;
    localcolours = 0;
    factioncolours = factioncolours_;
}

navdrawlist::~navdrawlist()     //destroy list
{
    inmouserange = 0;
    wipe();
    head = NULL;
    tail = NULL;
    n_contents = 0;
}

int navdrawlist::get_n_contents()       //return the amount of items in the list
{
    return n_contents;
}

void navdrawlist::insert(int type, float size, float x, float y)        //insert iteam at head of list
{
    if (head == NULL) {
        head = new navdrawnode(type, size, x, y, NULL);
        tail = head;
    } else {
        head = new navdrawnode(type, size, x, y, head);
    }
    n_contents += 1;
}

void navdrawlist::insert(int type, float size, float x, float y, Unit *source)  //insert iteam at head of list
{
    if (head == NULL) {
        head = new navdrawnode(type, size, x, y, source, NULL);
        tail = head;
    } else {
        head = new navdrawnode(type, size, x, y, source, head);
    }
    n_contents += 1;
}

void navdrawlist::wipe()        //whipe the list clean
{
    navdrawnode *temp = head;
    navdrawnode *tempdelete = NULL;
    while (temp != NULL) {
        tempdelete = temp;
        temp = temp->nextitem;
        delete tempdelete;
    }
    head = NULL;
    n_contents = 0;
}

void navdrawlist::rotate()      //take the head and stick it in the back
{
    if (head != NULL) {
        //|
        //|
        if (head->nextitem != NULL) {
            //there is something there, and its not alone
            tail->nextitem = head;
            tail = head;
            head = head->nextitem;
            tail->nextitem = NULL;
        }
    }
}

string getUnitNameAndFgNoBase(Unit *target);

void drawdescription(Unit *source,
        float x_,
        float y_,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        navscreenoccupied *screenoccupation,
        const GFXColor &col)                                                                                                                                     //take the head and stick it in the back
{
    if (source == NULL) {
        return;
    }
    drawdescription(getUnitNameAndFgNoBase(source),
            x_,
            y_,
            size_x,
            size_y,
            ignore_occupied_areas,
            screenoccupation,
            col);
}

void drawdescription(string text,
        float x_,
        float y_,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        navscreenoccupied *screenoccupation,
        const GFXColor &col)                                                                                                                                     //take the head and stick it in the back
{
    if (text.size() == 0) {
        return;
    }
    TextPlane displayname;      //will be used to display shits names

    displayname.col = col;

    int length = text.size();
    float offset = (float(length) * 0.005);
    if (ignore_occupied_areas) {
        displayname.SetPos((x_ - offset), y_);
        displayname.SetText(text);
        displayname.SetCharSize(size_x, size_y);
    } else {
        float new_y = screenoccupation->findfreesector(x_, y_);
        displayname.SetPos((x_ - offset), new_y);
        displayname.SetText(text);
        displayname.SetCharSize(size_x, size_y);
    }
    const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
    GFXColor tpbg = displayname.bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        displayname.bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    displayname.Draw(text, 0, true, false, automatte);
    displayname.bgcol = tpbg;
}

Unit *navdrawlist::gettailunit() {
    return tail->source;
}

//#define INIT_COL_ARRAY( col, r, g, b, a ) do{ col[0] = r; col[1] = g; col[2] = b; col[3] = a; }while(0)

static GFXColor getUnitTypeColor(std::string name, bool text, GFXColor def, float unselectedalpha) {
    GFXColor col = vs_config->getColor("nav", (std::string("unhighlighted_") + name) + (text ? "_text" : ""), def);
    if (col.a == 0) {
        if (name != "unit" && col.r == 0 && col.g == 0 && col.b == 0) {
            if (!text) {
                return getUnitTypeColor("unit", false, GFXColor(1, 1, .7, 1), unselectedalpha);
            } else {
                GFXColor temp = getUnitTypeColor("unit", true, GFXColor(.2, 1, .5, 0), unselectedalpha);
                temp.a = unselectedalpha;
                return temp;
            }
        }
        col.a = unselectedalpha;
    }
    return col;
}

void drawlistitem(int type,
        float size,
        float x,
        float y,
        Unit *source,
        navscreenoccupied *screenoccupation,
        bool inmouserange,
        bool currentistail,
        float unselectedalpha,
        GFXColor *factioncolours) {
    float relation = 0.0;

    //Get a color from the config
    static bool inited = false;
    static GFXColor highlighted_tail_col;
    static GFXColor highlighted_tail_text;
    static GFXColor highlighted_untail_col;
    static GFXColor unhighlighted_sun_col;
    static GFXColor unhighlighted_sun_text;
    static GFXColor unhighlighted_planet_text;
    static GFXColor unhighlighted_c_player_col;
    static GFXColor unhighlighted_c_player_text;
    static GFXColor unhighlighted_player_col;
    static GFXColor unhighlighted_player_text;
    static GFXColor unhighlighted_asteroid_col;
    static GFXColor unhighlighted_asteroid_text;
    static GFXColor unhighlighted_nebula_col;
    static GFXColor unhighlighted_nebula_text;
    static GFXColor unhighlighted_jump_col;
    static GFXColor unhighlighted_jump_text;
    static GFXColor unhighlighted_station_text;
    static GFXColor unhighlighted_fighter_text;
    static GFXColor unhighlighted_capship_text;
    static GFXColor unhighlighted_unit_text;

    if (!inited) {
        inited = true;

        highlighted_tail_col = vs_config->getColor("nav", "highlighted_unit_on_tail", GFXColor(1, .3, .3, .8));
        highlighted_tail_text = vs_config->getColor("nav", "highlighted_text_on_tail", GFXColor(1, 1, .7, 1));
        highlighted_untail_col = vs_config->getColor("nav", "highlighted_unit_off_tail", GFXColor(1, 1, 1, .8));
        unhighlighted_sun_col = getUnitTypeColor("sun", false, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_sun_text = getUnitTypeColor("sun", true, GFXColor(0, 0, 0, 0), unselectedalpha);

        //Planet color is the relation color, so is not defined here.
        unhighlighted_planet_text = getUnitTypeColor("planet", true, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_c_player_col = getUnitTypeColor("curplayer", false, GFXColor(.3, .3, 1, .8), .8);
        unhighlighted_c_player_text = getUnitTypeColor("curplayer", true, GFXColor(.3, .3, 1, 0), unselectedalpha);
        unhighlighted_player_col = getUnitTypeColor("player", false, GFXColor(.3, .3, 1, .8), .8);
        unhighlighted_player_text = getUnitTypeColor("player", true, GFXColor(.3, .3, 1, 0), unselectedalpha);
        unhighlighted_asteroid_col = getUnitTypeColor("asteroid", false, GFXColor(1, .8, .8, .6), .6);
        unhighlighted_asteroid_text = getUnitTypeColor("asteroid", true, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_nebula_col = getUnitTypeColor("nebula", false, GFXColor(1, .5, 1, .6), .6);
        unhighlighted_nebula_text = getUnitTypeColor("nebula", true, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_jump_col = getUnitTypeColor("jump", false, GFXColor(.5, .9, .9, .6), .6);
        unhighlighted_jump_text = getUnitTypeColor("jump", true, GFXColor(.3, 1, .8, 0), unselectedalpha);

        //Basic unit types:
        unhighlighted_station_text = getUnitTypeColor("station", true, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_fighter_text = getUnitTypeColor("fighter", true, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_capship_text = getUnitTypeColor("capship", true, GFXColor(0, 0, 0, 0), unselectedalpha);
        unhighlighted_unit_text = getUnitTypeColor("unit", true, GFXColor(0, 0, 0, 0), unselectedalpha);
    }

    //if(source != NULL)
    //relation =      FactionUtil::GetIntRelation( ( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) )->faction ,source->faction);
    //else
    //relation = 0;
    //the realtime relationship
    if (source != NULL) {
        relation = source->getRelation(UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()));
    } else {
        relation = 0;
    }
    relation = relation * 0.5;
    relation = relation + 0.5;

    //to avoid duplicate code
    GFXColor relColor((1.0 - relation), relation, (1.0 - (2.0 * Delta(relation, 0.5))), .7);
    //GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),1)
    if (type == navsun) {
        if (!inmouserange) {
            NavigationSystem::DrawCircle(x, y, size, unhighlighted_sun_col);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_sun_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navplanet) {
        if (!inmouserange) {
            NavigationSystem::DrawPlanet(x, y, size, relColor);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_planet_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawPlanet(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawPlanet(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navcurrentplayer) {
        if (!inmouserange) {
            NavigationSystem::DrawHalfCircleTop(x, y, size, unhighlighted_c_player_col);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_c_player_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navplayer) {
        if (!inmouserange) {
            NavigationSystem::DrawHalfCircleTop(x, y, size, unhighlighted_player_col);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_player_col);
        } else {
            if (currentistail) {
                NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navstation) {
        if (!inmouserange) {
            NavigationSystem::DrawStation(x, y, size, relColor);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_station_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawStation(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawStation(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navfighter) {
        if (!inmouserange) {
            if (factioncolours == NULL) {
                NavigationSystem::DrawHalfCircleTop(x, y, size, relColor);
                drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_fighter_text);
            } else {
                NavigationSystem::DrawHalfCircleTop(x, y, size, relColor);
                GFXColor thecolor = factioncolours[source->faction];
                thecolor.a = unselectedalpha;
                drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, thecolor);
            }
        } else {
            if (currentistail) {
                NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navcapship) {
        if (!inmouserange) {
            if (factioncolours == NULL) {
                NavigationSystem::DrawCircle(x, y, size, relColor);
                drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_capship_text);
            } else {
                NavigationSystem::DrawCircle(x, y, size, relColor);
                GFXColor thecolor = factioncolours[source->faction];

                thecolor.a = unselectedalpha;

                drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, thecolor);
            }
        } else {
            if (currentistail) {
                NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navmissile) {
        if (!inmouserange) {
            NavigationSystem::DrawMissile(x, y, size, relColor);
//drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, GFXColor(.2, 1, .5, unselectedalpha));
        }
            //NOT DRAWING NAME OF MISSILE TO MAKE ROOM FOR IMPORTANT TEXT ON SCREEN
        else {
            if (currentistail) {
                NavigationSystem::DrawMissile(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawMissile(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navasteroid) {
        if (!inmouserange) {
            NavigationSystem::DrawCircle(x, y, size, unhighlighted_asteroid_col);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_asteroid_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navnebula) {
        if (!inmouserange) {
            NavigationSystem::DrawCircle(x, y, size, unhighlighted_nebula_col);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_nebula_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
            }
        }
    } else if (type == navjump) {
        if (!inmouserange) {
            NavigationSystem::DrawJump(x, y, size, unhighlighted_jump_col);
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_jump_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawJump(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawJump(x, y, size, highlighted_untail_col);
            }
        }
    } else {
        if (!inmouserange) {
            NavigationSystem::DrawCircle(x, y, size,
                    GFXColor((1.0 - relation),
                            relation,
                            (1.0 - (2.0 * Delta(relation, 0.5))),
                            .6));
            drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_unit_text);
        } else {
            if (currentistail) {
                NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
            } else {
                NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
            }
        }
    }
    //SHOW THE NAME ALL BIG AND SHIT
    if ((currentistail) && (inmouserange == 1)) {
        //DISPLAY THE NAME
        drawdescription(source, x, y, 2.0, 2.0, false, screenoccupation, GFXColor(1, 1, .7, 1));
    }
}

void navdrawlist::draw()        //Draw the items in the list
{
    if (head == NULL) {
        return;
    } else {
        navdrawnode *current = head;
        while (current != NULL) {
            drawlistitem(current->type,
                    current->size,
                    current->x,
                    current->y,
                    current->source,
                    screenoccupation,
                    inmouserange,
                    current == tail,
                    unselectedalpha,
                    factioncolours);

            current = current->nextitem;
        }
    }
}

