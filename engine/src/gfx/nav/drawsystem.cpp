/*
 * drawsystem.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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


#include <set>
#include "vegadisk/vsfilesystem.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "src/universe.h"
#include "src/star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/collection.h"
#include "gfx/hud.h"
#include "root_generic/lin_time.h" //for fps
#include "src/config_xml.h"
#include "gui/guidefs.h"
#include "cmd/planetary_orbit.h"

// Draws the ellipse a body travels, faintly. Pieces that fall behind the camera are
// skipped, so an orbit the viewer is inside does not streak across the view.
static void DrawOrbit(Unit *unit, const NavMap &camera, float center_nav_x, float center_nav_y) {
    static const bool draw_orbits =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "draw_nav_orbits", "true"));
    if (!draw_orbits) {
        return;
    }
    PlanetaryOrbit *orbit = vega_dynamic_cast_ptr<PlanetaryOrbit>(unit->getAIState());
    if (orbit == nullptr) {
        return;      //a body that travels no orbit
    }

    ImDrawList *draw_list = ImGui::GetBackgroundDrawList();
    //Faint blue, matching the nav map's own labels rather than competing with them.
    const ImU32 colour = IM_COL32(128, 128, 255, 48);
    const int segments = 96;

    for (int i = 0; i < segments; ++i) {
        const QVector from = orbit->orbitPoint((2.0 * M_PI * i) / segments);
        const QVector to = orbit->orbitPoint((2.0 * M_PI * (i + 1)) / segments);

        float from_x = 0.0f;
        float from_y = 0.0f;
        float from_scale = 0.0f;
        float to_x = 0.0f;
        float to_y = 0.0f;
        float to_scale = 0.0f;
        if (!camera.project(from, from_x, from_y, from_scale)
                || !camera.project(to, to_x, to_y, to_scale)) {
            continue;      //this piece is behind the camera
        }

        const ImVec2 start(Coordinates::normToPixelX(center_nav_x + from_x),
                Coordinates::normToPixelY(center_nav_y + from_y));
        const ImVec2 end(Coordinates::normToPixelX(center_nav_x + to_x),
                Coordinates::normToPixelY(center_nav_y + to_y));
        draw_list->AddLine(start, end, colour, 1.0f);
    }
}

#include "src/config_xml.h"
#include "root_generic/lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "src/main_loop.h"
#include <assert.h>     //needed for assert() calls
#include "vegadisk/savegame.h"
#include "gfx/animation.h"
#include "gfx_generic/mesh.h"
#include "src/universe_util.h"
#include "src/in_mouse.h"
#include "gui/glut_support.h"
#include "cmd/unit_util.h"
#include "math.h"
#include "src/save_util.h"
#include "gfx/nav/navscreen.h"
#include "gfx/masks.h"
#include "root_generic/galaxy_gen.h"
#include "cmd/unit_util.h"

//**********************************
//Main function for drawing a CURRENT system
//works :
//scans all items, records min + max coords of the system, for relevant items
//rescans, and enlists the found items that it wants drawn
//-	items with mouse over them will go into a mouselist.
//draws the draw lists, with the mouse lists cycled 'n' times (according to kliks)
//**********************************

void NavigationSystem::DrawSystem() {
    UniverseUtil::PythonUnitIter bleh = UniverseUtil::getUnitList();
    if (!(*bleh)) {
        return;
    }
//string mystr ("3d "+XMLSupport::tostring (system_view));
//UniverseUtil::IOmessage (0,"game","all",mystr);

    //what's my name
    //***************************
    TextPlane systemname;       //will be used to display shits names
    int faction =
            FactionUtil::GetFactionIndex(UniverseUtil::GetGalaxyFaction(_Universe->activeStarSystem()->getFileName()));
    //GFXColor factioncolor = factioncolours[faction];
    string systemnamestring =
            "#ff0000Sector: #ffff00" + getStarSystemSector(_Universe->activeStarSystem()->getFileName())
                    + "  #ff0000Current System: #ffff00" + _Universe->activeStarSystem()->getName() + " ("
                    + FactionUtil::GetFactionName(faction)
                    + "#ffff00)";
    //int length = systemnamestring.size();
    //float offset = (float(length)*0.001);
    //systemname.SetPos( (((screenskipby4[0]+screenskipby4[1])/2)-offset) , screenskipby4[3]); // middle position
    systemname.SetPos(screenskipby4[0] + 0.03f, screenskipby4[3] - 0.05f);     //inset, so the text clears the top and left edges
    GFXColor temp_color(1, 1, .7, 1);
    systemname.color = static_cast<ImU32>(temp_color);
    systemname.SetText(systemnamestring);
//systemname.SetCharSize(1, 1);
    const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
    GFXColor tpbg(systemname.background_color);
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        GFXColor temp_background_color( 0, 0, 0, background_alpha );
        systemname.background_color = static_cast<ImU32>(temp_background_color);
    }
    systemname.Draw(systemnamestring, 0, true, false, automatte);
    systemname.background_color = static_cast<ImU32>(tpbg);
    //***************************

//navdrawlist mainlist(0, screenoccupation, factioncolours);		//	lists of items to draw
//mainlist.unselectedalpha = unselectedalpha;
    navdrawlist mouselist(1, screenoccupation, factioncolours);       //lists of items to draw that are in mouse range

    QVector pos;    //item position

    Adjust3dTransformation(true);
    //Set up first item to compare to + centres
    //**********************************
    while ((*bleh) && (_Universe->AccessCockpit()->GetParent() != (*bleh))
            && (UnitUtil::isSun(*bleh)
                    || !UnitUtil::isSignificant(*bleh))) {                                                                       //no sun's in initial setup
        ++bleh;
    }
    if (!(*bleh)) {      //nothing there that's significant, just do it all
        bleh = UniverseUtil::getUnitList();
    }
    //GET THE POSITION
    //*************************
    pos = (*bleh)->Position();
    //*************************

    //Modify by old rotation amount
    //*************************
//if(system_view==VIEW_3D)
//{
//}
    //*************************

    float max_x = (float) pos.i;
    float min_x = (float) pos.i;
    float max_y = (float) pos.j;
    float min_y = (float) pos.j;
    float max_z = (float) pos.k;
    float min_z = (float) pos.k;

//float themaxvalue = fabs(pos.i);
    themaxvalue = 0.0;

    // Centre the content in the free area left of the button column, which starts at
    // 0.5 in screen coordinates. The map itself fills the whole screen.
    float center_nav_x = -0.25f;
    float center_nav_y = ((screenskipby4[2] + screenskipby4[3]) / 2);
    //**********************************
    //Retrieve unit data min/max
    //**********************************
    while (*bleh) {
        //this goes through one time to get the major components locations, and scales its output appropriately
        if (UnitUtil::isSun(*bleh)) {
            ++bleh;
            continue;
        }
        string temp = (*bleh)->name;
        pos = (*bleh)->Position();
        //Modify by old rotation amount
        //*************************
//if(system_view==VIEW_3D)
//{
//}
        //*************************
        //*************************
        if ((UnitUtil::isSignificant(*bleh)) || (_Universe->AccessCockpit()->GetParent() == (*bleh))) {
            RecordMinAndMax(pos, min_x, max_x, min_y, max_y, min_z, max_z, themaxvalue);
        }
        ++bleh;
    }
    //**********************************

    //Find Centers
    //**********************************
    center_x = (min_x + max_x) / 2;
    center_y = (min_y + max_y) / 2;
    center_z = (min_z + max_z) / 2;
    //**********************************

    max_x = 2 * max_x - center_x;
    max_y = 2 * max_y - center_y;
    max_z = 2 * max_z - center_z;
    min_x = 2 * min_x - center_x;
    min_y = 2 * min_y - center_y;
    min_z = 2 * min_z - center_z;

    themaxvalue *= 2;

//#define SQRT3 1.7320508
//themaxvalue = sqrt(themaxvalue*themaxvalue + themaxvalue*themaxvalue + themaxvalue*themaxvalue);
//themaxvalue = SQRT3*themaxvalue;

    //Set Camera Distance
    //**********************************
//{
    float half_x = (max_x - min_x);
    float half_y = (max_y - min_y);
    float half_z = (max_z - min_z);

    camera_z = sqrt((half_x * half_x) + (half_y * half_y) + (half_z * half_z));

    // Frame the whole system the first time the view is drawn, and whenever it is
    // refitted. The camera's distance comes from the extent of the content and the
    // field of view, so the system fills the view rather than being scaled against
    // its own bounding box.
    if (system_needs_refit) {
        system_cam.setFraming(QVector(center_x, center_y, center_z), half_x, half_y, half_z);
        system_needs_refit = false;
    }

//float halfmax = 0.5*themaxvalue;
//camera_z = sqrt( (halfmax*halfmax) + (halfmax*halfmax) + (halfmax*halfmax) );
//camera_z = 4.0*themaxvalue;
//}

    //**********************************


/*
 *       string mystr ("max x "+XMLSupport::tostring (max_x));
 *       UniverseUtil::IOmessage (0,"game","all",mystr);
 *
 *       string mystr2 ("min x "+XMLSupport::tostring (min_x));
 *       UniverseUtil::IOmessage (0,"game","all",mystr2);
 *
 *       string mystr3 ("max y "+XMLSupport::tostring (max_y));
 *       UniverseUtil::IOmessage (0,"game","all",mystr3);
 *
 *       string mystr4 ("min y "+XMLSupport::tostring (min_y));
 *       UniverseUtil::IOmessage (0,"game","all",mystr4);
 *
 *       string mystrcx ("center x "+XMLSupport::tostring (center_x));
 *       UniverseUtil::IOmessage (0,"game","all",mystrcx);
 *
 *       string mystrcy ("center y "+XMLSupport::tostring (center_y));
 *       UniverseUtil::IOmessage (0,"game","all",mystrcy);
 */

    Unit *ThePlayer = (UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()));

    //Enlist the items and attributes
    //**********************************
    nav_near_dist = 1e30;      //reset the nearest-thing distance for this frame

    // Drawable items are collected first and drawn afterwards, so that overlapping
    // ones can be collapsed into the largest of the group before anything is drawn.
    struct NavItem {
        int type;
        float size;
        float x;
        float y;
        Unit *unit;
        double real_size;
    };
    std::vector<NavItem> drawn;

    un_iter blah = UniverseUtil::getUnitList();
    while (*blah) {
        //this draws the points

        //Retrieve unit data
        //**********************************
        string temp = (*blah)->name;

        pos = (*blah)->Position();

        float the_x = 0.0f;
        float the_y = 0.0f;
        float system_item_scale_temp = 0.0f;
        if (!system_cam.project(pos, the_x, the_y, system_item_scale_temp)) {
            ++blah;
            continue;      //behind the camera, so there is nothing to draw
        }
        the_x = center_nav_x + the_x;
        the_y = center_nav_y + the_y;

        // Keep an item within a readable size range however far away it is.
        if (system_item_scale_temp > maximumitemscaleup) {
            system_item_scale_temp = maximumitemscaleup;
        }
        if (system_item_scale_temp < minimumitemscaledown) {
            system_item_scale_temp = minimumitemscaledown;
        }

        // Remember the nearest thing in view: panning and zooming scale with it.
        const double item_distance = (pos - system_cam.position()).Magnitude();
        if (item_distance < nav_near_dist) {
            nav_near_dist = item_distance;
        }

        //IGNORE OFF SCREEN
        //**********************************
        if (!TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], the_x, the_y)) {
            ++blah;
            continue;
        }
        //**********************************

        //Now starts the test that determines the type of things and inserts
        //|
        //|
        //\/

        float insert_size = 0.0;
        int insert_type = navambiguous;
        if ((*blah)->getUnitType() == Vega_UnitType::unit) {
            //unit
            /*if(UnitUtil::isPlayerStarship(*blah) > -1)	//	is a PLAYER SHIP
             *  {
             *       if (UnitUtil::isPlayerStarship (*blah)==UniverseUtil::getCurrentPlayer()) //	is THE PLAYER
             *       {
             *               insert_type = navcurrentplayer;
             *               insert_size = navcurrentplayersize;
             *       }
             *       else	//	is A PLAYER
             *       {
             *               insert_type = navplayer;
             *               insert_size = navplayersize;
             *       }
             *  }
             *  else	//	is a non player ship
             *  {*/
            if (UnitUtil::isSignificant(*blah)) {
                //capship or station
                if ((*blah)->MaxSpeed() == 0) {
                    //is this item STATIONARY?
                    insert_type = navstation;
                    insert_size = navstationsize;
                } else {
                    //it moves = capship
                    if (ThePlayer->InRange((*blah), false, false)) {
                        //only insert if in range
                        insert_type = navcapship;
                        insert_size = navcapshipsize;
                    } else {
                        //skip unit completely if not in range
                        ++blah;
                        continue;
                    }
                }
            } else {
                //fighter
                /*if(ThePlayer->InRange((*blah),false,false))	//	only insert if in range
                 *  {
                 *       insert_type = navfighter;
                 *       insert_size = navfightersize;
                 *  }
                 *  else	// skip unit completely if not in range
                 *  {
                 * ++blah;
                 *       continue;
                 *  }*/
                if (_Universe->whichPlayerStarship(*blah) > -1) {
                    //is THE PLAYER
                    insert_type = navfighter;
                    insert_size = navfightersize;
                } else {
                    //skip unit completely if not in range
                    ++blah;
                    continue;
                }
            }
            //}
        } else if ((*blah)->getUnitType() == Vega_UnitType::planet) {
            //is it a PLANET?
            if (UnitUtil::isSun(*blah)) {
                //is this a SUN?
                insert_type = navsun;
                insert_size = navsunsize;
            } else if (!((*blah)->GetDestinations().empty())) {
                //is a jump point (has destinations)
                insert_type = navjump;
                insert_size = navjumpsize;
            } else {
                //its a planet
                insert_type = navplanet;
                insert_size = navplanetsize;
            }
        } else if ((*blah)->getUnitType() == Vega_UnitType::missile) {
            //a missile
            insert_type = navmissile;
            insert_size = navmissilesize;
        } else if ((*blah)->getUnitType() == Vega_UnitType::asteroid) {
            //an asteroid
            insert_type = navasteroid;
            insert_size = navasteroidsize;
        } else if ((*blah)->getUnitType() == Vega_UnitType::nebula) {
            //a nebula
            insert_type = navnebula;
            insert_size = navnebulasize;
        } else {
            //undefined non unit
            insert_type = navambiguous;
            insert_size = navambiguoussize;
        }
        if (system_item_scale_temp > (system_item_scale * 3)) {
            system_item_scale_temp = (system_item_scale * 3);
        }
        insert_size *= system_item_scale_temp;
        // Keep items above a minimum on-screen size, so that they stay visible when
        // the view is zoomed out to a very large system.
        if (insert_size < NavMinItemSize()) {
            insert_size = NavMinItemSize();
        }

        NavItem item;
        item.type = insert_type;
        item.size = insert_size;
        item.x = the_x;
        item.y = the_y;
        item.unit = (*blah);
        item.real_size = (*blah)->rSize();
        drawn.push_back(item);

        ++blah;
    }

    // Collapse overlapping items: where several objects land on nearly the same place,
    // keep only the largest, so a cluster draws one marker instead of a label for every
    // object in it. The player, bases, and whatever is under the mouse are always kept.
    // Ranked by real size rather than on-screen size, because the minimum size above
    // makes every distant icon measure alike.
    const float cluster_radius = 0.05f;
    auto is_keeper = [&](const NavItem &item) {
        if (item.unit != nullptr && UnitUtil::isPlayerStarship(item.unit) > -1) {
            return true;
        }
        if (item.unit != nullptr && UnitUtil::getFlightgroupNameCR(item.unit) == "Base") {
            return true;
        }
        float x = item.x;
        float y = item.y;
        return TestIfInRangeRad(x, y, item.size, mouse_x_current, mouse_y_current);
    };

    for (size_t i = 0; i < drawn.size(); ++i) {
        if (drawn[i].size < 0.0f) {
            continue;      //already collapsed into a larger neighbour
        }
        for (size_t j = i + 1; j < drawn.size(); ++j) {
            if (drawn[j].size < 0.0f) {
                continue;
            }
            const float dx = drawn[i].x - drawn[j].x;
            const float dy = drawn[i].y - drawn[j].y;
            if (((dx * dx) + (dy * dy)) >= (cluster_radius * cluster_radius)) {
                continue;
            }
            const bool keep_i = is_keeper(drawn[i]);
            const bool keep_j = is_keeper(drawn[j]);
            if (keep_i && keep_j) {
                continue;
            }
            if (drawn[j].real_size > drawn[i].real_size) {
                if (!keep_i) {
                    drawn[i].size = -1.0f;
                }
            } else if (drawn[j].real_size == drawn[i].real_size) {
                if (!keep_j) {
                    drawn[j].size = -1.0f;
                }
            } else {
                if (!keep_j) {
                    drawn[j].size = -1.0f;
                }
            }
        }
    }

    // Draw the orbits before the markers, and for every body that has one. The collapse
    // below is about markers and the names they carry, and an orbit is neither: a body
    // whose marker is merged into a larger neighbour still travels its own ellipse.
    for (size_t i = 0; i < drawn.size(); ++i) {
        if (drawn[i].unit != nullptr && ((drawn[i].type == navplanet) || (drawn[i].type == navsun))) {
            DrawOrbit(drawn[i].unit, system_cam, center_nav_x, center_nav_y);
        }
    }

    //Draw what survived the collapse.
    for (size_t i = 0; i < drawn.size(); ++i) {
        if (drawn[i].size < 0.0f) {
            continue;
        }
        NavItem &item = drawn[i];
        if (_Universe->AccessCockpit()->GetParent()->Target() == item.unit) {
            static GFXColor col = vs_config->getColor("nav", "targetted_unit", GFXColor(1, 0.3, 0.3, 0.8));
            DrawTargetCorners(item.x, item.y, item.size, col);
        }
        bool tests_in_range = false;
        if (item.type == navstation) {
            tests_in_range = TestIfInRangeBlk(item.x, item.y, item.size, mouse_x_current, mouse_y_current);
        } else {
            tests_in_range = TestIfInRangeRad(item.x, item.y, item.size, mouse_x_current, mouse_y_current);
        }
        if (tests_in_range) {
            mouselist.insert(item.type, item.size, item.x, item.y, item.unit);
        } else {
            drawlistitem(item.type,
                    item.size,
                    item.x,
                    item.y,
                    item.unit,
                    screenoccupation,
                    false,
                    false,
                    unselectedalpha,
                    factioncolours);
        }
    }
    drawn.clear();
    //**********************************	//	done enlisting items and attributes
    //Adjust mouse list for 'n' kliks
    //**********************************
    //STANDARD	: (1 3 2) ~ [0] [2] [1]
    //VS			: (1 2 3) ~ [0] [1] [2]	<-- use this
    if (mouselist.get_n_contents() > 0) {
        //mouse is over a target when this is > 0
        if (mouse_wentdown[2] == 1) {             //mouse button went down for mouse button 2(standard)
            rotations += 1;
        }
    }
    if (rotations >= mouselist.get_n_contents()) {      //dont rotate more than there is
        rotations = 0;
    }
    int r = 0;
    while (r < rotations) {
        //rotate whatver rotations, leaving n rotated items, tail on top
        mouselist.rotate();
        r += 1;
    }
    //**********************************
    //Draw the damn shit
    //**********************************
//mainlist.draw();	//	draw the items
//mainlist.wipe();	//	whipe the list
    //**********************************
    //Check for selection query
    //give back the selected tail IF there is one
    //IF given back, undo the selection state
    //**********************************
    Unit *current_player = UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer());
    if (1 || checkbit(buttonstates, 1)) {
        //button #2 is down, wanting a (selection)
        if (mouselist.get_n_contents() > 0) {
            //mouse is over a target when this is > 0
            if (mouse_wentdown[0] == 1) {
                //mouse button went down for mouse button 1
                currentselection = mouselist.gettailunit();
                unsetbit(buttonstates, 1);
                //JUST FOR NOW, target == current selection. later it'll be used for other shit, that will then set target.
                if (currentselection.GetUnit()) {
                    current_player->Target(currentselection.GetUnit());
                    current_player->radar.Lock(UnitUtil::isSignificant(current_player));
                }
            }
        }
    }
    //**********************************

    //Clear the lists
    //**********************************
    mouselist.draw();           //draw mouse over'd items
    mouselist.wipe();           //whipe mouse over'd list
    //**********************************
}
//**********************************

