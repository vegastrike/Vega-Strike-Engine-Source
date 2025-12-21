/*
 * navscreen.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * hellcatv, ace123, surfdargent, klaussfreire, jacks, pyramid3d
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


#include <set>
#include "src/vs_logging.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "src/universe.h"
#include "src/star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/collection.h"
#include "gfx/hud.h"
#include "gfx/vdu.h"
#include "root_generic/lin_time.h" //for fps
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
#include "gfx/vdu.h"
#include "gfx/nav/navscreen.h"
#include "gfx/masks.h"
#include "gfx/nav/navgetxmldata.h"
#include "gfx/nav/navitemstodraw.h"
#include "gfx/nav/navparse.h"
#include "gfx/nav/navcomputer.h"
#include "gfx/nav/navpath.h"
#include "gldrv/winsys.h"

//This sets up the items in the navscreen
//**********************************

NavigationSystem::NavigationSystem() {
    draw = -1;
    whattodraw = (1 | 2);
    pathman = new PathManager();
    navcomp = new NavComputer(this);
    for (int i = 0; i < NAVTOTALMESHCOUNT; i++) {
        mesh[i] = NULL;
    }
    factioncolours = NULL;
}

NavigationSystem::~NavigationSystem() {
    draw = 0;
    //delete mesh;
    delete screenoccupation;
    delete mesh[0];
    delete mesh[1];
    delete mesh[2];
    delete mesh[3];
    delete mesh[4];
    delete mesh[5];
    delete mesh[6];
    delete mesh[7];
    delete factioncolours;
}

void NavigationSystem::mouseDrag(int x, int y) {
    mousex = x;
    mousey = y;
}

void NavigationSystem::mouseMotion(int x, int y) {
    mousex = x;
    mousey = y;
}

void NavigationSystem::mouseClick(int button, int state, int x, int y) {
    // For fullscreen, convert real coordinates (native resolution) to 
    // relative coordinates
    if(native_resolution_x != configuration().graphics.resolution_x ||
       native_resolution_y != configuration().graphics.resolution_y) {
        double native_resolution_x_dbl = native_resolution_x;
        double native_resolution_y_dbl = native_resolution_y;
        double resolution_x_dbl = configuration().graphics.resolution_x;
        double resolution_y_dbl = configuration().graphics.resolution_y;
        double x_factor = resolution_x_dbl / native_resolution_x_dbl;
        double y_factor = resolution_y_dbl / native_resolution_y_dbl;
        x *= x_factor;
        y *= y_factor;
    }

    mousex = x;
    mousey = y;
    if (state == WS_MOUSE_DOWN) {
        mousestat |= (1 << lookupMouseButton(button));
    } else if (button != WS_WHEEL_UP && button != WS_WHEEL_DOWN) {
        mousestat &= (~(1 << lookupMouseButton(button)));
    }
}

void NavigationSystem::Setup() {
    _Universe->AccessCockpit()->visitSystem(_Universe->activeStarSystem()->getFileName());

    configmode = 0;

    rotations = 0;

    minimumitemscaledown = 0.2;
    maximumitemscaleup = 3.0;

    axis = 3;

    rx = -0.5;              //galaxy mode settings
    ry = 0.5;
    rz = 0.0;
    zoom = 1.8;

    rx_s = -0.5;              //system mode settings
    ry_s = 1.5;
    rz_s = 0.0;
    zoom_s = 1.8;

    scrolloffset = 0;

    camera_z = 1.0;     //updated after a pass
    center_x = 0.0;     //updated after a pass
    center_y = 0.0;     //updated after a pass
    center_z = 0.0;     //updated after a pass

    path_view = PATH_ON;
    const bool start_sys_ortho = configuration().graphics.system_map_ortho_view;
    const bool start_sec_ortho = configuration().graphics.sector_map_ortho_view;
    system_view = start_sys_ortho ? VIEW_ORTHO : VIEW_2D;
    galaxy_view = start_sec_ortho ? VIEW_ORTHO : VIEW_2D;
    system_multi_dimensional = 1;
    galaxy_multi_dimensional = 1;

    zshiftmultiplier = 2.5;     //shrink the output
    item_zscalefactor = 1.0;            //camera distance prespective multiplier for affecting item sizes
    system_item_scale = 1.0;
    mouse_previous_state[0] =
            0;        //could have used a loop, but this way the system uses immediate instead of R type.
    mouse_previous_state[1] = 0;
    mouse_previous_state[2] = 0;
    mouse_previous_state[3] = 0;
    mouse_previous_state[4] = 0;
    mouse_wentup[0] = 0;
    mouse_wentup[1] = 0;
    mouse_wentup[2] = 0;
    mouse_wentup[3] = 0;
    mouse_wentup[4] = 0;
    mouse_wentdown[0] = 0;
    mouse_wentdown[1] = 0;
    mouse_wentdown[2] = 0;
    mouse_wentdown[3] = 0;
    mouse_wentdown[4] = 0;
    mouse_x_previous = (-1 + float(mousex) / (.5 * configuration().graphics.resolution_x));
    mouse_y_previous = (1 + float(-1 * mousey) / (.5 * configuration().graphics.resolution_y));

    const int max_map_nodes = configuration().graphics.max_map_nodes;
    systemIter.init(UniverseUtil::getSystemFile(), max_map_nodes);
    sectorIter.init(systemIter);
    systemselectionindex = 0;
    sectorselectionindex = 0;
    destinationsystemindex = 0;
    currentsystemindex = 0;
    setFocusedSystemIndex(0);

    const int time_to_helpscreen = configuration().general.times_to_show_help_screen;
    buttonstates = 0;
    if (getSaveData(0, "436457r1K3574r7uP71m35", 0) <= time_to_helpscreen) {
        whattodraw = 0;
    } else {
        whattodraw = (1 | 2);
    }
    currentselection = NULL;
    factioncolours = new GFXColor[FactionUtil::GetNumFactions()];
    unselectedalpha = 1.0;

    sectorOffset = systemOffset = 0;

    unsigned int p;
    for (p = 0; p < FactionUtil::GetNumFactions(); p++) {
        factioncolours[p].r = 1;
        factioncolours[p].g = 1;
        factioncolours[p].b = 1;
        factioncolours[p].a = 1;
    }
    for (p = 0; p < NAVTOTALMESHCOUNT; p++) {
        meshcoordinate_x[p] = 0.0;
    }
    for (p = 0; p < NAVTOTALMESHCOUNT; p++) {
        meshcoordinate_y[p] = 0.0;
    }
    for (p = 0; p < NAVTOTALMESHCOUNT; p++) {
        meshcoordinate_z[p] = 0.0;
    }
    for (p = 0; p < NAVTOTALMESHCOUNT; p++) {
        meshcoordinate_z_delta[p] = 0.0;
    }
    //select target
    //NAV/MISSION toggle
    //

//HERE GOES THE PARSING

//*************************
    screenskipby4[0] = .3;
    screenskipby4[1] = .7;
    screenskipby4[2] = .3;
    screenskipby4[3] = .7;

    buttonskipby4_1[0] = .75;
    buttonskipby4_1[1] = .95;
    buttonskipby4_1[2] = .85;
    buttonskipby4_1[3] = .90;

    buttonskipby4_2[0] = .75;
    buttonskipby4_2[1] = .95;
    buttonskipby4_2[2] = .75;
    buttonskipby4_2[3] = .80;

    buttonskipby4_3[0] = .75;
    buttonskipby4_3[1] = .95;
    buttonskipby4_3[2] = .65;
    buttonskipby4_3[3] = .70;

    buttonskipby4_4[0] = .75;
    buttonskipby4_4[1] = .95;
    buttonskipby4_4[2] = .55;
    buttonskipby4_4[3] = .60;

    buttonskipby4_5[0] = .75;
    buttonskipby4_5[1] = .95;
    buttonskipby4_5[2] = .45;
    buttonskipby4_5[3] = .50;

    buttonskipby4_6[0] = .75;
    buttonskipby4_6[1] = .95;
    buttonskipby4_6[2] = .35;
    buttonskipby4_6[3] = .40;

    buttonskipby4_7[0] = .75;
    buttonskipby4_7[1] = .95;
    buttonskipby4_7[2] = .25;
    buttonskipby4_7[3] = .30;
    if (!ParseFile("navdata.xml")) {
        //start DUMMP VARS
        screenskipby4[0] = .3;
        screenskipby4[1] = .7;
        screenskipby4[2] = .3;
        screenskipby4[3] = .7;

        buttonskipby4_1[0] = .75;
        buttonskipby4_1[1] = .95;
        buttonskipby4_1[2] = .85;
        buttonskipby4_1[3] = .90;

        buttonskipby4_2[0] = .75;
        buttonskipby4_2[1] = .95;
        buttonskipby4_2[2] = .75;
        buttonskipby4_2[3] = .80;

        buttonskipby4_3[0] = .75;
        buttonskipby4_3[1] = .95;
        buttonskipby4_3[2] = .65;
        buttonskipby4_3[3] = .70;

        buttonskipby4_4[0] = .75;
        buttonskipby4_4[1] = .95;
        buttonskipby4_4[2] = .55;
        buttonskipby4_4[3] = .60;

        buttonskipby4_5[0] = .75;
        buttonskipby4_5[1] = .95;
        buttonskipby4_5[2] = .45;
        buttonskipby4_5[3] = .50;

        buttonskipby4_6[0] = .75;
        buttonskipby4_6[1] = .95;
        buttonskipby4_6[2] = .35;
        buttonskipby4_6[3] = .40;

        buttonskipby4_7[0] = .75;
        buttonskipby4_7[1] = .95;
        buttonskipby4_7[2] = .25;
        buttonskipby4_7[3] = .30;

        unsetbit(whattodraw, 4);
        for (int i = 0; i < NAVTOTALMESHCOUNT; i++) {
            mesh[i] = NULL;
        }
        VS_LOG(error, "ERROR: Map mesh file not found!!! Using default: blank mesh.");
        //end DUMMY VARS
    }
    ScreenToCoord(screenskipby4[0]);
    ScreenToCoord(screenskipby4[1]);
    ScreenToCoord(screenskipby4[2]);
    ScreenToCoord(screenskipby4[3]);

    ScreenToCoord(buttonskipby4_1[0]);
    ScreenToCoord(buttonskipby4_1[1]);
    ScreenToCoord(buttonskipby4_1[2]);
    ScreenToCoord(buttonskipby4_1[3]);

    ScreenToCoord(buttonskipby4_2[0]);
    ScreenToCoord(buttonskipby4_2[1]);
    ScreenToCoord(buttonskipby4_2[2]);
    ScreenToCoord(buttonskipby4_2[3]);

    ScreenToCoord(buttonskipby4_3[0]);
    ScreenToCoord(buttonskipby4_3[1]);
    ScreenToCoord(buttonskipby4_3[2]);
    ScreenToCoord(buttonskipby4_3[3]);

    ScreenToCoord(buttonskipby4_4[0]);
    ScreenToCoord(buttonskipby4_4[1]);
    ScreenToCoord(buttonskipby4_4[2]);
    ScreenToCoord(buttonskipby4_4[3]);

    ScreenToCoord(buttonskipby4_5[0]);
    ScreenToCoord(buttonskipby4_5[1]);
    ScreenToCoord(buttonskipby4_5[2]);
    ScreenToCoord(buttonskipby4_5[3]);

    ScreenToCoord(buttonskipby4_6[0]);
    ScreenToCoord(buttonskipby4_6[1]);
    ScreenToCoord(buttonskipby4_6[2]);
    ScreenToCoord(buttonskipby4_6[3]);

    ScreenToCoord(buttonskipby4_7[0]);
    ScreenToCoord(buttonskipby4_7[1]);
    ScreenToCoord(buttonskipby4_7[2]);
    ScreenToCoord(buttonskipby4_7[3]);

    reverse = configuration().joystick.reverse_mouse_spr ? 1 : -1;

    // reverse = -1;
    if ((screenskipby4[1] - screenskipby4[0]) < (screenskipby4[3] - screenskipby4[2])) {
        system_item_scale *= (screenskipby4[1] - screenskipby4[0]);            //is actually over 1, which is itself
    } else {
        system_item_scale *= (screenskipby4[3] - screenskipby4[2]);
    }
    screenoccupation = new navscreenoccupied(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], true);

    //Get special colors from the config
    currentcol = vs_config->getColor("nav", "current_system",
            GFXColor(1, 0.3, 0.3, 1.0));
    destinationcol = vs_config->getColor("nav", "destination_system",
            GFXColor(1, 0.77, 0.3, 1.0));
    selectcol = vs_config->getColor("nav", "selection_system",
            GFXColor(0.3, 1, 0.3, 1.0));
    pathcol = vs_config->getColor("nav", "path_system",
            GFXColor(1, 0.3, 0.3, 1.0));
    navcomp->init();
}

//**********************************

//This is the main draw loop for the nav screen
//**********************************
void NavigationSystem::Draw() {
    if (!CheckDraw()) {
        return;
    }
    if (_Universe->AccessCockpit()->GetParent() == NULL) {
        return;
    }

    //DRAW THE SCREEN MODEL
    //**********************************
    Vector p, q, r;
    const float zrange = configuration().graphics.cockpit_nav_zrange_flt;
    const float zfloor = configuration().graphics.cockpit_nav_zfloor_flt;
    _Universe->AccessCamera()->GetOrientation(p, q, r);
    _Universe->AccessCamera()->UpdateGFX(GFXTRUE,
            GFXTRUE,
            GFXFALSE,
            GFXTRUE,
            zfloor,
            zfloor + zrange);

    _Universe->activateLightMap();
    for (int i = 0; i < NAVTOTALMESHCOUNT; i++) {
        float screen_x = 0.0;
        float screen_y = 0.0;
        float screen_z = 0.0;

        screen_x = meshcoordinate_x[i];
        screen_y = meshcoordinate_y[i];
        screen_z = meshcoordinate_z[i];
        if (checkbit(buttonstates, (i - 1))) {          //button1 = 0, starts at -1, returning 0, no addition done
            screen_z += meshcoordinate_z_delta[i];
        }
        QVector pos = _Universe->AccessCamera()->GetPosition();

        //offset horizontal
        //***************
        pos = (p.Cast() * screen_x) + pos;
        //***************

        //offset vertical
        //***************
        pos = (q.Cast() * screen_y) + pos;
        //***************

        //offset sink
        //***************
        pos = (r.Cast() * screen_z) + pos;
        //***************

        Matrix mat(p, q, r, pos);
        if (mesh[i]) {
            mesh[i]->Draw(FLT_MAX, mat);
        }
    }
    Mesh::ProcessZFarMeshes(true);
    Mesh::ProcessUndrawnMeshes(false, true);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXColor4f(1, 1, 1, 1);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(LIGHTING);

    GFXHudMode(true);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    //**********************************

    screenoccupation->reset();

    //Save current mouse location
    //**********************************
    mouse_x_current = (-1 + float(mousex) / (.5 * configuration().graphics.resolution_x));
    mouse_y_current = (1 + float(-1 * mousey) / (.5 * configuration().graphics.resolution_y));
    //**********************************

    //Set Mouse
    //**********************************
    SetMouseFlipStatus();       //define bools 'mouse_wentdown[]' 'mouse_wentup[]'
    //**********************************
    //Draw the Navscreen Functions
    //**********************************
    if (checkbit(whattodraw, 1)) {
        if (checkbit(whattodraw, 2)) {
            if (galaxy_view == VIEW_3D) {
                DrawNavCircle(((screenskipby4[0] + screenskipby4[1]) / 2.0),
                        ((screenskipby4[2] + screenskipby4[3]) / 2.0),
                        0.6,
                        rx,
                        ry,
                        GFXColor(1, 1, 1,
                                0.2));
            } else {
                DrawGrid(screenskipby4[0],
                        screenskipby4[1],
                        screenskipby4[2],
                        screenskipby4[3],
                        GFXColor(1, 1, 1, 0.2));
            }
            DrawGalaxy();
        } else {
            if (system_view == VIEW_3D) {
                DrawNavCircle(((screenskipby4[0] + screenskipby4[1]) / 2.0),
                        ((screenskipby4[2] + screenskipby4[3]) / 2.0),
                        0.6,
                        rx_s,
                        ry_s,
                        GFXColor(1, 1, 1,
                                0.2));
            } else {
                DrawGrid(screenskipby4[0],
                        screenskipby4[1],
                        screenskipby4[2],
                        screenskipby4[3],
                        GFXColor(1, 1, 1, 0.2));
            }
            DrawSystem();
        }
    } else {
        if (checkbit(whattodraw, 3)) {
            DrawSectorList();
        } else if (checkbit(whattodraw, 2)) {
            DrawShip();
        } else {
            DrawMission();
        }
    }
    //**********************************

    DrawObjectives();

    //Draw Button Outlines
    //**********************************
    bool outlinebuttons = 0;
    if (configmode > 0) {
        outlinebuttons = 1;
    }
    DrawButton(buttonskipby4_1[0], buttonskipby4_1[1], buttonskipby4_1[2], buttonskipby4_1[3], 1, outlinebuttons);
    DrawButton(buttonskipby4_2[0], buttonskipby4_2[1], buttonskipby4_2[2], buttonskipby4_2[3], 2, outlinebuttons);
    DrawButton(buttonskipby4_3[0], buttonskipby4_3[1], buttonskipby4_3[2], buttonskipby4_3[3], 3, outlinebuttons);
    DrawButton(buttonskipby4_4[0], buttonskipby4_4[1], buttonskipby4_4[2], buttonskipby4_4[3], 4, outlinebuttons);
    DrawButton(buttonskipby4_5[0], buttonskipby4_5[1], buttonskipby4_5[2], buttonskipby4_5[3], 5, outlinebuttons);
    DrawButton(buttonskipby4_6[0], buttonskipby4_6[1], buttonskipby4_6[2], buttonskipby4_6[3], 6, outlinebuttons);
    DrawButton(buttonskipby4_7[0], buttonskipby4_7[1], buttonskipby4_7[2], buttonskipby4_7[3], 7, outlinebuttons);
    //**********************************

    //Save current mouse location as previous for next cycle
    //**********************************
    mouse_x_previous = (-1 + float(mousex) / (.5 * configuration().graphics.resolution_x));
    mouse_y_previous = (1 + float(-1 * mousey) / (.5 * configuration().graphics.resolution_y));
    //**********************************

    GFXEnable(TEXTURE0);
    GFXHudMode(false);
}
//**********************************

//This is the mission info screen
//**********************************
void NavigationSystem::DrawMission() {
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    navdrawlist factionlist(0, screenoccupation, factioncolours);

    float deltax = screenskipby4[1] - screenskipby4[0];
    float deltay = screenskipby4[3] - screenskipby4[2];
    float originx = screenskipby4[0];    //left
    float originy = screenskipby4[3];    //top
    vector<float> *killlist = &_Universe->AccessCockpit()->savegame->getMissionData(string("kills"));
    string relationskills = "Relations";
    if (killlist->size() > 0) {
        relationskills += " | Kills";
    }
    drawdescription(relationskills,
            (originx + (0.1 * deltax)),
            (originy),
            1,
            1,
            0,
            screenoccupation,
            GFXColor(.3, 1, .3, 1));
    drawdescription(" ", (originx + (0.1 * deltax)), (originy), 1, 1, 0, screenoccupation, GFXColor(.3, 1, .3, 1));

    drawdescription(" ", (originx + (0.3 * deltax)), (originy), 1, 1, 0, screenoccupation, GFXColor(.3, 1, .3, 1));
    drawdescription(" ", (originx + (0.3 * deltax)), (originy), 1, 1, 0, screenoccupation, GFXColor(.3, 1, .3, 1));

    size_t numfactions = FactionUtil::GetNumFactions();
    size_t i = 0;
    string factionname = "factionname";
    float relation = 0.0;
    const string disallowedFactions = configuration().graphics.unprintable_factions;
    const string disallowedExtension = configuration().graphics.unprintable_faction_extension;
    int totkills = 0;
    size_t fac_loc_before = 0, fac_loc = 0, fac_loc_after = 0;
    for (; i < numfactions; ++i) {
        factionname = FactionUtil::GetFactionName(i);
        if (factionname != "neutral" && factionname != "privateer" && factionname != "planets"
                && factionname != "upgrades") {
            if (i < killlist->size()) {
                totkills += (int) (*killlist)[i];
            }
            if (factionname.find(disallowedExtension) != string::npos) {
                continue;
            }
            fac_loc_after = 0;
            fac_loc = disallowedFactions.find(factionname, fac_loc_after);
            while (fac_loc != string::npos) {
                if (fac_loc > 0) {
                    fac_loc_before = fac_loc - 1;
                } else {
                    fac_loc_before = 0;
                }
                fac_loc_after = fac_loc + factionname.size();
                if ((fac_loc == 0 || disallowedFactions[fac_loc_before] == ' ' || disallowedFactions[fac_loc_before]
                        == '\t')
                        && (disallowedFactions[fac_loc_after] == ' ' || disallowedFactions[fac_loc_after] == '\t'
                                || disallowedFactions[fac_loc_after] == '\0')) {
                    break;
                }
                fac_loc = disallowedFactions.find(factionname, fac_loc_after);
            }
            if (fac_loc != string::npos) {
                continue;
            }
            relation = UnitUtil::getRelationFromFaction(UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()), i);

            //draw faction name
            const float *colors = FactionUtil::GetSparkColor(i);
            drawdescription(FactionUtil::GetFactionName(
                            i), (originx + (0.1 * deltax)), (originy), 1, 1, 0, screenoccupation,
                    GFXColor(colors[0], colors[1], colors[2], 1.));

            float relation01 = relation * 0.5 + 0.5;
            relation = ((relation > 1 ? 1 : relation) < -1 ? -1 : relation);
            int percent = (int) (relation * 100.0);
            string relationtext(XMLSupport::tostring(percent));
            if (i < killlist->size()) {
                relationtext += " | ";
                relationtext += XMLSupport::tostring((int) (*killlist)[i]);
            }
            drawdescription(relationtext, (originx + (0.3 * deltax)), (originy), 1, 1, 0, screenoccupation,
                    GFXColor((1.0 - relation01), (relation01), (1.0 - (2.0 * Delta(relation01, 0.5))), 1));
        }
    }
    string relationtext("Total Kills: ");
    relation = 1;

    relationtext += XMLSupport::tostring(totkills);
    drawdescription(relationtext, (originx + (0.2 * deltax)), (originy - (0.95 * deltay)), 1, 1, 0, screenoccupation,
            GFXColor((1.0 - relation), relation, (1.0 - (2.0 * Delta(relation, 0.5))), 1));

//drawdescription(" Terran : ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, screenoccupation, GFXColor(.3,1,.3,1));
//drawdescription(" Rlaan : ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, screenoccupation, GFXColor(1,.3,.3,1));
//drawdescription(" Aera : ", (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 0, screenoccupation, GFXColor(.3,.3,1,1));

//float love_from_terran = FactionUtil::getRelation(1);
//float love_from_rlaan = FactionUtil::getRelation(2);
//float love_from_aera = FactionUtil::getRelation(3);

    TextPlane displayname;
    displayname.color = GFXColor(1, 1, 1, 1);
    displayname.SetSize(.42, -.7);
    displayname.SetPos(originx + (.1 * deltax) + .37, originy /*+(1*deltay)*/ );
    std::string text;
    if (active_missions.size() > 1) {
        for (unsigned int i = 1; i < active_missions.size(); ++i) {
            text += active_missions[i]->mission_name + ":\n";
            for (unsigned int j = 0; j < active_missions[i]->objectives.size(); ++j) {
                text += active_missions[i]->objectives[j].objective + ": "
                        + XMLSupport::tostring((int) (active_missions[i]->objectives[j].completeness * 100)) + "%\n";
            }
        }
        text += "\n";
    }
    text +=
            "#FFA000     PRESS SHIFT-M TO TOGGLE THIS MENU    \n\n\n\n#000000*******#00a6FFVega Strike 0.7#000000*********\nWelcome to VS. Your ship undocks stopped; #8080FFArrow keys/mouse/joystick#000000 steer your ship. Use #8080FF+#000000 & #8080FF-#000000 to adjust cruise control, or #8080FF/#000000 & #8080FF[backspace]#000000 to go to max governor setting or full-stop, respectively. Use #8080FFy#000000 to toggle between maneuver and travel settings for your relative velocity governors. Use #8080ff[home]#000000 & #8080FF[end]#000000 to set and unset velocity reference point to the current target (non-hostile targets only). Use #8080FFTab#000000 to activate Overdrive(if present).\n\nPress #8080FFn#000000 to cycle nav points, #8080FFt#000000 to cycle targets, and #8080FFp#000000 to target objects in front of you.\n\n#8080FF[space]#000000 fires guns, and #8080ff[Enter]#000000 fires missiles.\n\nThe #8080FFa#000000 key activates SPEC drive for insystem FTL.\nInterstellar Travel requires a #FFBB11 jump drive#000000 and #FFBB11FTL Capacitors#000000 to be installed. To jump, fly into the green wireframe nav-marker; hit #8080FFj#000000 to jump to the linked system.\n\nTo dock, target a base, planet or large vessel and hail with #8080FF0#000000 to request docking clearance. When you get close, a green box will appear. Fly to the box. When inside the box, #8080FFd#000000 will dock.\n\n#FF0000If Vega Strike halts or acts oddly,#000000\n#FFFF00immediately#000000 post the latest log\nfile from $HOME/.vegastrike/logs/\nto https://forums.vega-strike.org/\nbefore you restart Vega Strike.\n";
    displayname.SetText(text);
    displayname.SetCharSize(1, 1);
    displayname.Draw();
/*
 *       string exitinfo("To exit help press #8080FFshift-M#000000\n#8080FFShift-M#000000 will bring up this\nhelp menu any time.\nThe right buttons access the galaxy and system maps");
 *
 *       displayname.SetSize (.6,-.8);
 *       displayname.SetPos(originx-.02,   originy-1.2);
 *       displayname.SetText (exitinfo);
 *       displayname.SetCharSize (1,1);
 *       displayname.Draw();*/
    GFXEnable(TEXTURE0);
}
//**********************************

//This is the mission info screen
//**********************************
extern string MakeUnitXMLPretty(string str, Unit *un);

void NavigationSystem::DrawShip() {
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    navdrawlist factionlist(0, screenoccupation, factioncolours);

    float deltax = screenskipby4[1] - screenskipby4[0];
    float originx = screenskipby4[0]; //left
    float originy = screenskipby4[3]; //top
    string writethis;
    Unit *par;
    if ((par = _Universe->AccessCockpit()->GetParent())) {
        writethis = MakeUnitXMLPretty(par->WriteUnitString(), par);
    }
    TextPlane displayname;
    displayname.color = GFXColor(.3, 1, .3, 1);
    displayname.SetSize(.7, -.8);
    displayname.SetPos(originx - (.1 * deltax), originy /*+(1*deltay)*/ );
    displayname.SetText(writethis);
    displayname.SetCharSize(1, 1);
    const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
    GFXColor tpbg = displayname.background_color;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        displayname.background_color = GFXColor(0, 0, 0, background_alpha);
    }
    displayname.Draw(writethis, 0, true, false, automatte);
    displayname.background_color = tpbg;

//factionlist.drawdescription(writethis, (originx + (0.1*deltax)),(originy - (0.1*deltay)), 1, 1, 1, GFXColor(1,1,1,1));

    GFXEnable(TEXTURE0);
}

void NavigationSystem::DrawSectorList() {
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    float deltax = screenskipby4[1] - screenskipby4[0];
    float deltay = screenskipby4[3] - screenskipby4[2];
    float originx = screenskipby4[0];  //left
    float originy = screenskipby4[3];  //top
    float width = (deltax / 6);
    float height = (0.031 * deltay);
    const unsigned numRows = 26;
    float the_x, the_y, the_x1, the_y1, the_x2, the_y2;
    GFXColor color;
    unsigned count, index, row;

    //Draw Title of Column
    drawdescription("Sectors", originx + (0.5 * width), originy - (0.0 * deltay), 1, 1, 1, screenoccupation,
            GFXColor(.3, 1, .3, 1));

    //Draw Scroll Pieces
    color = GFXColor(0.7, 0.3, 0.3, 1.0);

    the_x = width * (0.5) + originx;
    the_y = originy - (0.05 * deltay);
    the_x1 = the_x - width / 2;
    the_y1 = the_y - height;
    the_x2 = the_x + width / 2;
    the_y2 = the_y;
    if (TestIfInRange(the_x1, the_x2, the_y1, the_y2, mouse_x_current, mouse_y_current)) {
        if (mouse_wentdown[0] == 1) {             //mouse button went down for mouse button 1
            if (sectorOffset > 0) {
                --sectorOffset;
            }
        }
    }
    drawdescription("Up", the_x, the_y, 1, 1, 1, screenoccupation, color);

    the_x = width * (0.5) + originx;
    the_y = originy - (0.05 * deltay) - height * (29);
    the_x1 = the_x - width / 2;
    the_y1 = the_y - height;
    the_x2 = the_x + width / 2;
    the_y2 = the_y;
    if (TestIfInRange(the_x1, the_x2, the_y1, the_y2, mouse_x_current, mouse_y_current)) {
        if (mouse_wentdown[0] == 1) {             //mouse button went down for mouse button 1
            if (sectorOffset < (sectorIter.size() - numRows)) {
                ++sectorOffset;
            }
        }
    }
    drawdescription("Down", the_x, the_y, 1, 1, 1, screenoccupation, color);

    count = 0;
    for (sectorIter.seek(); !sectorIter.done(); ++sectorIter) {
        bool drawable = false;
        for (unsigned i = 0; i < sectorIter->GetSubsystemSize(); i++) {
            if (systemIter[sectorIter->GetSubsystemIndex(i)].isDrawable()) {
                drawable = true;
                break;
            }
        }
        if (!drawable) {
            continue;
        }
        if ((count < sectorOffset) || (count >= (numRows + sectorOffset))) {
            ++count;
            continue;
        }
        row = (count - sectorOffset) % numRows;
        the_x = width * (0.5) + originx;
        the_y = originy - (0.05 * deltay) - height * (row + 2);
        the_x1 = the_x - width / 2;
        the_y1 = the_y - height;
        the_x2 = the_x + width / 2;
        the_y2 = the_y;
        if (TestIfInRange(the_x1, the_x2, the_y1, the_y2, mouse_x_current, mouse_y_current)) {
            if (mouse_wentdown[0] == 1) {
                //mouse button went down for mouse button 1
                sectorselectionindex = sectorIter.getIndex();
                systemOffset = 0;
            }
        }
        if (sectorIter.getIndex() == sectorselectionindex) {
            color = selectcol;
        } else {
            color = GFXColor(0.7, 0.3, 0.3, 1.0);
        }
        drawdescription(sectorIter->GetName(), the_x, the_y, 1, 1, 1, screenoccupation, color);
        ++count;
    }
    drawdescription("Systems", originx + (1.5) * width, originy - (0.0 * deltay), 1, 1, 1, screenoccupation,
            GFXColor(.3, 1, .3, 1));

    //Draw Scroll Pieces
    color = GFXColor(0.7, 0.3, 0.3, 1.0);

    the_x = width * (1.5) + originx;
    the_y = originy - (0.05 * deltay);
    the_x1 = the_x - width / 2;
    the_y1 = the_y - height;
    the_x2 = the_x + width / 2;
    the_y2 = the_y;
    if (TestIfInRange(the_x1, the_x2, the_y1, the_y2, mouse_x_current, mouse_y_current)) {
        if (mouse_wentdown[0] == 1) {             //mouse button went down for mouse button 1
            if (systemOffset > 0) {
                --systemOffset;
            }
        }
    }
    drawdescription("Up", the_x, the_y, 1, 1, 1, screenoccupation, color);

    the_x = width * (1.5) + originx;
    the_y = originy - (0.05 * deltay) - height * (29);
    the_x1 = the_x - width / 2;
    the_y1 = the_y - height;
    the_x2 = the_x + width / 2;
    the_y2 = the_y;
    if (TestIfInRange(the_x1, the_x2, the_y1, the_y2, mouse_x_current, mouse_y_current)) {
        if (mouse_wentdown[0] == 1) {             //mouse button went down for mouse button 1
            if (systemOffset < (sectorIter[sectorselectionindex].GetSubsystemSize() - numRows)) {
                ++systemOffset;
            }
        }
    }
    drawdescription("Down", the_x, the_y, 1, 1, 1, screenoccupation, color);

    count = 0;
    sectorIter.seek(sectorselectionindex);
    for (unsigned i = 0; i < sectorIter->GetSubsystemSize(); ++i) {
        index = sectorIter->GetSubsystemIndex(i);
        if (!systemIter[index].isDrawable()) {
            continue;
        }
        if ((count < systemOffset) || (count >= (numRows + systemOffset))) {
            ++count;
            continue;
        }
        row = (count - systemOffset) % numRows;
        the_x = width * (1.5) + originx;
        the_y = originy - (0.05 * deltay) - height * (row + 2);
        the_x1 = the_x - width / 2;
        the_y1 = the_y - height;
        the_x2 = the_x + width / 2;
        the_y2 = the_y;
        if (TestIfInRange(the_x1, the_x2, the_y1, the_y2, mouse_x_current, mouse_y_current)) {
            if (mouse_wentdown[0] == 1) {
                //mouse button went down for mouse button 1
                unsigned oldselection = systemselectionindex;
                systemselectionindex = index;
                if (systemselectionindex == oldselection) {
                    setFocusedSystemIndex(systemselectionindex);
                }
            }
        }
        if (index == destinationsystemindex) {
            color = destinationcol;
        } else if (index == focusedsystemindex) {
            color = currentcol;
        } else if (index == systemselectionindex) {
            color = selectcol;
        } else {
            color = GFXColor(0.7, 0.3, 0.3, 1.0);
        }
        string csector, csystem;
        Beautify(systemIter[index].GetName(), csector, csystem);

        drawdescription(csystem, the_x, the_y, 1, 1, 1, screenoccupation, color);
        ++count;
    }
}

void NavigationSystem::DrawObjectives() {
    if (checkbit(whattodraw, 4)) {
        //Draw the objectives screen!
        DrawObjectivesTextPlane(&screen_objectives, scrolloffset, _Universe->AccessCockpit()->GetParent());
    }
}

//this sets weather to draw the screen or not
//**********************************
void NavigationSystem::SetDraw(bool n) {
    if (draw == -1) {
        Setup();
        draw = 0;
    }
    if (n != (draw == 1)) {
        ClearPriorities();
        scrolloffset = 0;
        draw = n ? 1 : 0;
    }
}
//**********************************

//this gets rid of states that could be damaging
//**********************************
void NavigationSystem::ClearPriorities() {
    unsetbit(buttonstates, 1);
    currentselection = NULL;
//rx = 1.0;		//	resetting rotations is up to hitting the 2d/3d button
//ry = 1.0;
//rz = 0.0;
//rx_s = 1.0;
//ry_s = 1.0;
//rz_s = 0.0;
}
//**********************************

//This will set a wentdown and wentup flag just for the event of mouse button going down or up
//this is an FF test. not a state test.
//**********************************
void NavigationSystem::SetMouseFlipStatus() {
//getMouseButtonStatus()&1 = mouse button 1 standard = button 1 VS
//getMouseButtonStatus()&2 = mouse button 3 standard = button 2 VS
//getMouseButtonStatus()&4 = mouse button 2 standard = button 3 VS
//getMouseButtonStatus()&8 = mouse wheel up
//getMouseButtonStatus()&16 = mouse wheel down

    //use the VS scheme, (1 2 3 4 5) , instead of standard (1 3 2 4 5)
    //state 0 = up
    //state 1 = down

    bool status = 0;
    int i;
    for (i = 0; i < 5; i++) {
        status = (getMouseButtonStatus() & (1 << i)) ? 1 : 0;
        if ((status == 1) && (mouse_previous_state[i] == 0)) {
            mouse_wentdown[i] = 1;
            mouse_wentup[i] = 0;
        } else if ((status == 0) && (mouse_previous_state[i] == 1)) {
            mouse_wentup[i] = 1;
            mouse_wentdown[i] = 0;
        } else {
            mouse_wentup[i] = 0;
            mouse_wentdown[i] = 0;
            if (i == 3 || i == 4) {
                mousestat &= (~(1 << i));
            }
        }
    }
    for (i = 0; i < 5; i++) {
        mouse_previous_state[i] = (getMouseButtonStatus() & (1 << i));
    }            //button 'i+1' state VS
}

//**********************************

//returns a modified vector rotated by x y z radians
//**********************************
QVector NavigationSystem::dxyz(QVector vector, double x_, double y_, double z_) {
/*
 *         void Roll (QFLOAT rad){
 *               QFLOAT theta = atan2 (j,i)+rad;
 *               QFloat len = XSQRT (j*j+i*i);
 *               j = sin (theta)*len;
 *               i = cos (theta)*len;
 *         }
 */
    if (x_ != 0) {
        double distance_yz = sqrt((vector.j * vector.j) + (vector.k * vector.k));
        double current_angle = atan2(vector.k, vector.j);
        current_angle += x_;
        vector.j = cos(current_angle) * distance_yz;
        vector.k = sin(current_angle) * distance_yz;
    }
    if (y_ != 0) {
        double distance_xz = sqrt((vector.i * vector.i) + (vector.k * vector.k));
        double current_angle = atan2(vector.k, vector.i);
        current_angle += y_;
        vector.i = cos(current_angle) * distance_xz;
        vector.k = sin(current_angle) * distance_xz;
    }
    if (z_ != 0) {
        double distance_xy = sqrt((vector.i * vector.i) + (vector.j * vector.j));
        double current_angle = atan2(vector.j, vector.i);
        current_angle += z_;
        vector.i = cos(current_angle) * distance_xy;
        vector.j = sin(current_angle) * distance_xy;
    }
    return vector;
}

//**********************************

void NavigationSystem::setCurrentSystem(string newSystem) {
    for (unsigned i = 0; i < systemIter.size(); ++i) {
        if (systemIter[i].GetName() == newSystem) {
            setCurrentSystemIndex(i);
            break;
        }
    }
}

void NavigationSystem::setFocusedSystemIndex(unsigned newSystemIndex) {
    focusedsystemindex = newSystemIndex;
    themaxvalue = 0;
    if (galaxy_view != VIEW_3D) {
        //This resets the panning position when not in 3d view.
        //Otehrwise, the focused system may end up off screen which will cause a lot of confusion.
        rx = -0.5;                      //galaxy mode settings
        ry = 0.5;
        rz = 0.0;
    }
    camera_z = 0;     //calculate camera distance again... it may have changed.
}

void NavigationSystem::setCurrentSystemIndex(unsigned newSystemIndex) {
    currentsystemindex = newSystemIndex;
    //causes occasional crash--only may have tracked it down
    const bool AlwaysUpdateNavMap = configuration().graphics.update_nav_after_jump;
    if (AlwaysUpdateNavMap) {
        pathman->updatePaths(PathManager::CURRENT);
    }
}

void NavigationSystem::setDestinationSystemIndex(unsigned newSystemIndex) {
    destinationsystemindex = newSystemIndex;
    pathman->updatePaths(PathManager::TARGET);
}

std::string NavigationSystem::getCurrentSystem() {
    if (factioncolours == NULL || focusedsystemindex >= systemIter.size()) {
        return _Universe->activeStarSystem()->getFileName();
    }
    return systemIter[currentsystemindex].GetName();
}

std::string NavigationSystem::getSelectedSystem() {
    if (factioncolours == NULL || focusedsystemindex >= systemIter.size()) {
        return _Universe->activeStarSystem()->getFileName();
    }
    return systemIter[systemselectionindex].GetName();
}

std::string NavigationSystem::getDestinationSystem() {
    if (factioncolours == NULL || focusedsystemindex >= systemIter.size()) {
        return _Universe->activeStarSystem()->getFileName();
    }
    return systemIter[destinationsystemindex].GetName();
}

std::string NavigationSystem::getFocusedSystem() {
    if (factioncolours == NULL || focusedsystemindex >= systemIter.size()) {
        return _Universe->activeStarSystem()->getFileName();
    }
    return systemIter[focusedsystemindex].GetName();
}

//Passes a draw button command, with colour
//Tests for a mouse over, to set colour
//**********************************
//1 = nav/mission
//2 = select currentselection
//3 = up
//4 = down
//5 = toggle prespective rezoom
//6 = toggle 2d/3d mode
int NavigationSystem::mousey = 0;
int NavigationSystem::mousex = 0;
int NavigationSystem::mousestat;

void NavigationSystem::DrawButton(float &x1, float &x2, float &y1, float &y2, int button_number, bool outline) {
    float mx = mouse_x_current;
    float my = mouse_y_current;
    bool inrange = TestIfInRange(x1, x2, y1, y2, mx, my);

    string label;
    if (button_number == 1) {
        label = "Nav/Info";
    } else if (button_number == 3) {
        label = "Target Selected";
    } else if (button_number == 7) {
        label = "2D/Ortho/3D";
    } else if (checkbit(whattodraw, 1)) {
        if (button_number == 2) {
            label = "Path On/Off/Only";
        } else if (button_number == 4) {
            label = "Up";
        } else if (button_number == 5) {
            label = "Down";
        } else if (button_number == 6) {
            label = "Axis Swap";
        }
    } else {
        if (button_number == 2) {
            label = "Sectors";
        } else if (button_number == 4) {
            label = "Ship";
        } else if (button_number == 5) {
            label = "Mission";
        } else if (button_number == 6) {
            label = "Nav Comp";
        }
    }
    TextPlane a_label;
    a_label.color = GFXColor(1, 1, 1, 1);
    int length = label.size();
    float offset = (float(length) * 0.0065);
    float xl = (x1 + x2) / 2.0;
    float yl = (y1 + y2) / 2.0;
    a_label.SetPos((xl - offset) - (checkbit(buttonstates, button_number - 1) ? 0.006 : 0), (yl + 0.025));
    a_label.SetText(label);
    const bool nav_button_labels = configuration().graphics.draw_nav_button_labels;
    if (nav_button_labels) {
        const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
        GFXColor tpbg = a_label.background_color;
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            a_label.background_color = GFXColor(0, 0, 0, background_alpha);
        }
        a_label.Draw(label, 0, true, false, automatte);
        a_label.background_color= tpbg;
    }
    //!!! DEPRESS !!!
    if ((inrange == 1) && (mouse_wentdown[0] == 1)) {
        currentselection =
                NULL;                //any new button depression means no depression on map, no selection made

        //******************************************************
        //**                 DEPRESS FUNCTION                 **	DEPRESS ALL
        //******************************************************

        dosetbit(buttonstates, (button_number - 1));            //all buttons go down

        //******************************************************
    }
    //!!! RELEASE !!!
    if ((inrange == 1) && (checkbit(buttonstates, (button_number - 1))) && (mouse_wentup[0])) {
        //******************************************************
        //**                 MISSION MODE	                  **	UNSET BITS WHEN ENTERING MISSION MODE
        //******************************************************
        if (!checkbit(whattodraw, 1)) {
            unsetbit(buttonstates, (button_number - 1));                //all are up in mission mode
        } else {
            unsetbit(buttonstates, (button_number - 1));
        }                //all are up in navigation mode
        //******************************************************
        //******************************************************
        //**                 BUTTON 1 FUNCTION                **	NAV-INFO vs STATUS-INFO
        //******************************************************
        if (button_number == 1) {          //releasing #1, toggle the draw (nav / mission)
            flipbit(whattodraw, 1);
        }
        //******************************************************
        //******************************************************
        //**                 BUTTON 2 FUNCTION                **	PATH options
        //******************************************************
        if (button_number == 2) {
            //releasing #2, toggle the path viewing settings(off/on/only)
            if ((checkbit(whattodraw, 1)) && (checkbit(whattodraw, 2))) {
                path_view = (path_view + 1) % PATH_MAXIMUM;
            } else if (!checkbit(whattodraw, 1)) {
                dosetbit(whattodraw, 3);
            }
        }
        //******************************************************
        //******************************************************
        //**                 BUTTON 3 FUNCTION                **	TARGET SELECTED SYSTEM
        //******************************************************
        if (button_number == 3) {
            //hit --TARGET--
            if (((checkbit(whattodraw, 1)) && (checkbit(whattodraw, 2)))             //Nav-Galaxy Mode
                    || ((!checkbit(whattodraw, 1)) && (checkbit(whattodraw, 3)))) {              //Mission-Sector Mode
                setDestinationSystemIndex(systemselectionindex);
            }
        }
        //******************************************************
        //******************************************************
        //**                 BUTTON 4 FUNCTION                **	UP
        //******************************************************
        if (button_number == 4) {
            //hit --UP--
            if (checkbit(whattodraw, 1)) {
                //if in nav system NOT mission
                dosetbit(whattodraw, 2);                      //draw galaxy
                setFocusedSystemIndex(currentsystemindex);
                systemselectionindex = currentsystemindex;
            } else {
                //if in mission mode
                unsetbit(whattodraw, 3);
                dosetbit(whattodraw, 2);                      //draw shipstats
            }
        }
        //******************************************************
        //******************************************************
        //**                 BUTTON 5 FUNCTION                **	DOWN
        //******************************************************
        if (button_number == 5) {
            //hit --DOWN--
            if (checkbit(whattodraw, 1)) {
                //if in nav system NOT mission

                unsetbit(whattodraw, 2);                      //draw system
            } else {
                //if in mission mode
                unsetbit(whattodraw, 3);
                unsetbit(whattodraw, 2);                      //draw mission
            }
        }
        //******************************************************
        //******************************************************
        //**                 BUTTON 6 FUNCTION                **	AXIS
        //******************************************************
        if (button_number == 6) {
            //releasing #1, toggle the draw (nav / mission)
            if (checkbit(whattodraw, 1)) {
                //if in nav system NOT mission
                zoom = 1.8;
                zoom_s = 1.8;

                axis = axis - 1;
                if (axis == 0) {
                    axis = 3;
                }
                camera_z = 0;
            } else {
                //if in mission mode

                navcomp->run();
            }
        }
        //******************************************************
        //******************************************************
        //**                 BUTTON 7 FUNCTION                **	2D/3D
        //******************************************************
        if (button_number == 7) {
            if ((checkbit(whattodraw, 1)) && (checkbit(whattodraw, 2)) && galaxy_multi_dimensional) {
                galaxy_view = (galaxy_view + 1) % VIEW_MAX;
                rx = -0.5;
                ry = 0.5;
                rz = 0.0;
            }
            if ((checkbit(whattodraw, 1)) && (!checkbit(whattodraw, 2)) && system_multi_dimensional) {
                system_view = (system_view + 1) % VIEW_MAX;
                rx_s = -0.5;
                ry_s = 0.5;
                rz_s = 0.0;
            }
            camera_z = 0;
        }
        //******************************************************
    }
    //!!! OUT OF BOUNDS !!!
    //******************************************************
    //**                 OUT OF RANGE	                  **	ALL DIE
    //******************************************************
    if (inrange == 0) {
        unsetbit(buttonstates, (button_number - 1));
    }
    //******************************************************
    //******************************************************
    //**             TRACE OUTLINES FOR EZ SETUP          **	ARTIST DEV UTIL
    //******************************************************
    if (outline == 1) {
        if (inrange == 1) {
            if (checkbit(buttonstates, (button_number - 1))) {
                DrawButtonOutline(x1, x2, y1, y2, GFXColor(1, 0, 0, 1));
            } else {
                DrawButtonOutline(x1, x2, y1, y2, GFXColor(1, 1, 0, 1));
            }
        } else {
            if (checkbit(buttonstates, (button_number - 1))) {
                DrawButtonOutline(x1, x2, y1, y2, GFXColor(1, 0, 0, 1));
            } else {
                DrawButtonOutline(x1, x2, y1, y2, GFXColor(1, 1, 1, 1));
            }
        }
    }
    //******************************************************
}
//**********************************

//Draws the actual button outline
//**********************************
void NavigationSystem::DrawButtonOutline(float &x1, float &x2, float &y1, float &y2, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    const float verts[8 * 3] = {
            x1, y1, 0,
            x1, y2, 0,
            x2, y1, 0,
            x2, y2, 0,
            x1, y1, 0,
            x2, y1, 0,
            x1, y2, 0,
            x2, y2, 0,
    };
    GFXDraw(GFXLINE, verts, 8);

    GFXEnable(TEXTURE0);
}
//**********************************

template<class T>
static inline bool intersect(T x0, T y0, T x1, T y1, T sx0, T sy0, T sx1, T sy1, T &ansx, T &ansy) {
    bool fxy = false;
    if (((x1 == x0) && (sx1 == sx0)) || ((x1 == x0) && (y1 == y0)) || ((sx1 == sx0) && (sy1 == sy0))) {
        //If both lines are vertical, then act as if they don't intersect.
        //If either one is a point, then for all practical purposes they do not intersect.
        return false;
    }
    if ((x1 == x0) && (sy1 == sy0)) {
        //Line 1 vertical, line 2 horizontal.
        ansx = x1;
        ansy = sy1;
        return ((sx0 <= x1
                && x1 <= sx1) || (sx1 <= x1 && x1 <= sx0)) && ((y0 <= sy1 && sy1 <= y1) || (y1 <= sy1 && sy1 <= y0));
    }
    if ((sx1 == sx0) && (y1 == y0)) {
        //line 1 horizontal, Line 2 vertical.
        ansx = sx1;
        ansy = y1;
        return ((x0 <= sx1
                && sx1 <= x1) || (x1 <= sx1 && sx1 <= x0)) && ((sy0 <= y1 && y1 <= sy1) || (sy1 <= y1 && y1 <= sy0));
    }
    //If either line is vertical (both was handled above), then flip the coordinate plane to prevent division by zero.
    if ((x1 == x0) || (sx1 == sx0)) {
        T temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
        fxy = true;
        temp = sx0;
        sx0 = sy0;
        sy0 = temp;
        temp = sx1;
        sx1 = sy1;
        sy1 = temp;
        fxy = true;
    }
    //Now we can be sure that no vertical lines exist.
    //Proceed with the operation.
    T m = (y1 - y0) / (x1 - x0);
    T sm = (sy1 - sy0) / (sx1 - sx0);
    if (m == sm) {
        //Parallel Lines
        return false;
    }
    ansx = (m * x1 - sm * sx1 - y1 + sy1) / (m - sm);
    ansy = (y1 - m * x1 + m * ansx);
    if (((x0 <= ansx
            && ansx <= x1)
            || (x1 <= ansx && ansx <= x0)) && ((sx0 <= ansx && ansx <= sx1) || (sx1 <= ansx && ansx <= sx0))) {
        //Inside the line segment.
        if (fxy) {
            //Deswapify them!
            T temp = ansx;
            ansx = ansy;
            ansy = temp;
        }
        return true;
    }
    //Too bad. They are outside the line segment
    return false;
}

void NavigationSystem::IntersectBorder(float &x, float &y, const float &x1, const float &y1) const {
    float ansx;
    float ansy;
    if (intersect(x, y, x1, y1, screenskipby4[1], screenskipby4[3], screenskipby4[0], screenskipby4[3], ansx, ansy)
            || intersect(x,
                    y,
                    x1,
                    y1,
                    screenskipby4[0],
                    screenskipby4[2],
                    screenskipby4[0],
                    screenskipby4[3],
                    ansx,
                    ansy)
            || intersect(x,
                    y,
                    x1,
                    y1,
                    screenskipby4[0],
                    screenskipby4[2],
                    screenskipby4[1],
                    screenskipby4[2],
                    ansx,
                    ansy)
            || intersect(x,
                    y,
                    x1,
                    y1,
                    screenskipby4[1],
                    screenskipby4[3],
                    screenskipby4[1],
                    screenskipby4[2],
                    ansx,
                    ansy)) {
        x = ansx;
        y = ansy;
    }
}

//tests if given are in the range
//**********************************
bool NavigationSystem::TestIfInRange(float &x1, float &x2, float &y1, float &y2, float tx, float ty) {
    if (((tx < x2) && (tx > x1)) && ((ty < y2) && (ty > y1))) {
        return 1;
    } else {
        return 0;
    }
}
//**********************************

//tests if given are in the circle range
//**********************************
bool NavigationSystem::TestIfInRangeRad(float &x, float &y, float size, float tx, float ty) {
    if ((((x - tx) * (x - tx)) + ((y - ty) * (y - ty))) < ((0.5 * size) * (0.5 * size))) {
        return 1;
    } else {
        return 0;
    }
}

//**********************************

//Tests if given are in block range
//**********************************
bool NavigationSystem::TestIfInRangeBlk(float &x, float &y, float size, float tx, float ty) {
    if ((Delta(tx, x) < (0.5 * size)) && (Delta(ty, y) < (0.5 * size))) {
        return 1;
    } else {
        return 0;
    }
}
//**********************************

/*
 *  //	Gived the delta of 2 items
 *  //	**********************************
 *  float NavigationSystem::Delta(float a, float b)
 *  {
 *
 *       float ans = a-b;
 *       if(ans < 0)
 *               return (-1.0 * ans);
 *       else
 *               return ans;
 *  }
 *  //	**********************************
 */

//converts the % of screen system to 0-center system
//**********************************
void NavigationSystem::ScreenToCoord(float &x) {
    x -= .5;
    x *= 2;
}
//**********************************

//checks if the draw flag is 1
//**********************************
bool NavigationSystem::CheckDraw() {
    return draw == 1;
}
//**********************************

void NavigationSystem::Adjust3dTransformation(bool three_d, bool system_vs_galaxy) {
    //Adjust transformation
    //**********************************
    if ((mouse_previous_state[0] == 1)
            && TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], mouse_x_current,
                    mouse_y_current)) {
        if (system_vs_galaxy) {
            if (three_d) {
                float ndx = -1.0 * (mouse_y_current - mouse_y_previous);
                float ndy = -4.0 * (mouse_x_current - mouse_x_previous);
                float ndz = 0.0;

                rx_s += ndx;
                ry_s += ndy;
                rz_s += ndz;
                if (rx_s > 0.0 / 2) {
                    rx_s = 0.0 / 2;
                }
                if (rx_s < -6.28 / 2) {
                    rx_s = -6.28 / 2;
                }
                if (ry_s >= 6.28) {
                    ry_s -= 6.28;
                }
                if (ry_s <= -6.28) {
                    ry_s += 6.28;
                }
                if (rz_s >= 6.28) {
                    rz_s -= 6.28;
                }
                if (rz_s <= -6.28) {
                    rz_s += 6.28;
                }
            } else {
                //rotation switches to panning
                float ndy = -1.0 * (mouse_y_current - mouse_y_previous);
                float ndx = -1.0 * (mouse_x_current - mouse_x_previous);
                float ndz = 0.0;

                //shift less when zoomed in more
                //float zoom_modifier = ( (1-(((zoom_s-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / 1 );
//float _l2 = log(2.0);
                float zoom_modifier = 1.;                 //(log(zoom_s)/_l2);

                rx_s -= ((ndx * camera_z) / zoom_modifier);
                ry_s -= ((ndy * camera_z) / zoom_modifier);
                rz_s -= ((ndz * camera_z) / zoom_modifier);
            }
        } else {
            //galaxy
            if (three_d) {
                float ndx = -1.0 * (mouse_y_current - mouse_y_previous);
                float ndy = -4.0 * (mouse_x_current - mouse_x_previous);
                float ndz = 0.0;

                rx += ndx;
                ry += ndy;
                rz += ndz;
                if (rx > 0.0 / 2) {
                    rx = 0.0 / 2;
                }
                if (rx < -6.28 / 2) {
                    rx = -6.28 / 2;
                }
                if (ry >= 6.28) {
                    ry -= 6.28;
                }
                if (ry <= -6.28) {
                    ry += 6.28;
                }
                if (rz >= 6.28) {
                    rz -= 6.28;
                }
                if (rz <= -6.28) {
                    rz += 6.28;
                }
            } else {
                //rotation switches to panning
                float ndy = -1.0 * (mouse_y_current - mouse_y_previous);
                float ndx = -1.0 * (mouse_x_current - mouse_x_previous);
                float ndz = 0.0;

                //shift less when zoomed in more
                //float zoom_modifier = ( (1-(((zoom-0.5*MAXZOOM)/MAXZOOM)*(0.85))) / 1 );
//float _l2 = log(2.0);
                float zoom_modifier = 1.;                 //(log(zoom)/_l2);

                rx -= ((ndx * camera_z) / zoom_modifier);
                ry -= ((ndy * camera_z) / zoom_modifier);
                rz -= ((ndz * camera_z) / zoom_modifier);
            }
        }
    }
    //**********************************
    //Set the prespective zoom level
    //**********************************
    if (((mouse_previous_state[1] == 1)
            && TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], mouse_x_current,
                    mouse_y_current)) || (mouse_wentdown[3] || mouse_wentdown[4])) {
        const float wheel_zoom_level = configuration().graphics.wheel_zoom_amount_flt;
        if (system_vs_galaxy) {
            if (mouse_wentdown[3]) {
                zoom_s += wheel_zoom_level;
            } else if (mouse_wentdown[4]) {
                zoom_s -= wheel_zoom_level;
            } else {
                zoom_s = zoom_s + ( /*1.0 +*/ 8 * (mouse_y_current - mouse_y_previous));
            }
            if (zoom_s < 1.2) {
                zoom_s = 1.2;
            }
            if (zoom_s > MAXZOOM) {
                zoom_s = MAXZOOM;
            }
        } else {
            if (mouse_wentdown[3]) {
                zoom += wheel_zoom_level;
            } else if (mouse_wentdown[4]) {
                zoom -= wheel_zoom_level;
            } else {
                zoom = zoom + ( /*1.0 +*/ 8 * (mouse_y_current - mouse_y_previous));
            }
            if (zoom < .5) {
                zoom = .5;
            }
            if (zoom > MAXZOOM / 2) {
                zoom = MAXZOOM / 2;
            }
        }
    }
    //**********************************
}

void NavigationSystem::ReplaceAxes(QVector &pos) {
    //replace axes
    //*************************
    if (axis != 3) {
        //3 == z == default
        if (axis == 2) {
            float old_i = pos.i;
            float old_j = pos.j;
            float old_k = pos.k;
            pos.i = old_i;
            pos.j = -old_k;
            pos.k = old_j;
        } else {
            //(axis == 1)
            float old_i = pos.i;
            float old_j = pos.j;
            float old_k = pos.k;
            pos.i = old_j;
            pos.j = -old_k;
            pos.k = old_i;
        }
    }
    //*************************
}

void NavigationSystem::RecordMinAndMax(const QVector &pos,
        float &min_x,
        float &max_x,
        float &min_y,
        float &max_y,
        float &min_z,
        float &max_z,
        float &max_all) {
    //Record min and max
    //**********************************
    if ((float) pos.i > max_x) {
        max_x = (float) pos.i;
    }
    if ((float) pos.i < min_x) {
        min_x = (float) pos.i;
    }
//if( fabs((float)pos.i) > max_all )
//max_all = fabs((float)pos.i);
    if ((fabs(max_x - min_x)) > max_all) {
        max_all = 0.5 * (fabs(max_x - min_x));
    }
    if ((float) pos.j > max_y) {
        max_y = (float) pos.j;
    }
    if ((float) pos.j < min_y) {
        min_y = (float) pos.j;
    }
//if( fabs((float)pos.j) > max_all )
//max_all = fabs((float)pos.j);
    if ((fabs(max_y - min_y)) > max_all) {
        max_all = 0.5 * (fabs(max_y - min_y));
    }
    if ((float) pos.k > max_z) {
        max_z = (float) pos.k;
    }
    if ((float) pos.k < min_z) {
        min_z = (float) pos.k;
    }
//if( fabs((float)pos.k) > max_all )
//max_all = fabs((float)pos.k);
    if ((fabs(max_z - min_z)) > max_all) {
        max_all = 0.5 * (fabs(max_z - min_z));
    }
    //**********************************
}

void NavigationSystem::DrawOriginOrientationTri(float center_nav_x, float center_nav_y, bool system_not_galaxy) {
    //Draw Origin Orientation Tri
    //**********************************
    QVector directionx;
    QVector directiony;
    QVector directionz;
    if (axis == 2) {
        directionx.i = 0.1;
        directionx.j = 0.0;
        directionx.k = 0.0;

        directionz.i = 0.0;
        directionz.j = 0.1;
        directionz.k = 0.0;

        directiony.i = 0.0;
        directiony.j = 0.0;
        directiony.k = 0.1;
    } else if (axis == 1) {
        directiony.i = 0.1;
        directiony.j = 0.0;
        directiony.k = 0.0;

        directionz.i = 0.0;
        directionz.j = 0.1;
        directionz.k = 0.0;

        directionx.i = 0.0;
        directionx.j = 0.0;
        directionx.k = 0.1;
    } else {
        //(axis == 3)
        directionx.i = 0.1;
        directionx.j = 0.0;
        directionx.k = 0.0;

        directiony.i = 0.0;
        directiony.j = 0.1;
        directiony.k = 0.0;

        directionz.i = 0.0;
        directionz.j = 0.0;
        directionz.k = 0.1;
    }
    if (system_not_galaxy) {
        if (system_view == VIEW_3D) {
            directionx = dxyz(directionx, 0, 0, ry_s);
            directionx = dxyz(directionx, rx_s, 0, 0);

            directiony = dxyz(directiony, 0, 0, ry_s);
            directiony = dxyz(directiony, rx_s, 0, 0);

            directionz = dxyz(directionz, 0, 0, ry_s);
            directionz = dxyz(directionz, rx_s, 0, 0);
        }
    } else if (galaxy_view == VIEW_3D) {
        directionx = dxyz(directionx, 0, 0, ry);
        directionx = dxyz(directionx, rx, 0, 0);

        directiony = dxyz(directiony, 0, 0, ry);
        directiony = dxyz(directiony, rx, 0, 0);

        directionz = dxyz(directionz, 0, 0, ry);
        directionz = dxyz(directionz, rx, 0, 0);
    }
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    float x0 = center_nav_x - 0.8 * ((screenskipby4[1] - screenskipby4[0]) / 2);
    float y0 = center_nav_y - 0.8 * ((screenskipby4[3] - screenskipby4[2]) / 2);
    float x1 = x0 + (directionx.i * (0.3 / (0.3 - directionx.k)));
    float y1 = y0 + (directionx.j * (0.3 / (0.3 - directionx.k)));
    float x2 = x0 + (directiony.i * (0.3 / (0.3 - directiony.k)));
    float y2 = y0 + (directiony.j * (0.3 / (0.3 - directiony.k)));
    float x3 = x0 + (directionz.i * (0.3 / (0.3 - directionz.k)));
    float y3 = y0 + (directionz.j * (0.3 / (0.3 - directionz.k)));

    const float verts[6 * (3 + 4)] = {
            x0, y0, 0, 1, 0, 0, 0.5,
            x1, y1, 0, 1, 0, 0, 0.5,
            x0, y0, 0, 0, 1, 0, 0.5,
            x2, y2, 0, 0, 1, 0, 0.5,
            x0, y0, 0, 0, 0, 1, 0.5,
            x3, y3, 0, 0, 0, 1, 0.5,
    };
    GFXDraw(GFXLINE, verts, 6, 3, 4);

    GFXEnable(TEXTURE0);
    //**********************************
}

float NavigationSystem::CalculatePerspectiveAdjustment(float &zscale,
        float &zdistance,
        QVector &pos,
        QVector &pos_flat,
        float &system_item_scale_temp,
        bool system_not_galaxy) {
    pos_flat.i = pos.i;
    pos_flat.j = pos.j;
    pos_flat.k = center_z;

    //Modify by rotation amount
    //*************************

    pos.i -= center_x;
    pos.j -= center_y;
    pos.k -= center_z;

    pos_flat.i -= center_x;
    pos_flat.j -= center_y;
    pos_flat.k -= center_z;
    if (system_not_galaxy) {
        if (system_view == VIEW_3D) {
            //3d = rotate
            pos = dxyz(pos, 0, 0, ry_s);
            pos = dxyz(pos, rx_s, 0, 0);

            pos_flat = dxyz(pos_flat, 0, 0, ry_s);
            pos_flat = dxyz(pos_flat, rx_s, 0, 0);
        } else {
            //2d = pan
            pos.i += rx_s;
            pos.j += ry_s;

            pos_flat.i += rx_s;
            pos_flat.j += ry_s;
        }
    } else {
        if (galaxy_view == VIEW_3D) {
            //3d = rotate
            pos = dxyz(pos, 0, 0, ry);
            pos = dxyz(pos, rx, 0, 0);

            pos_flat = dxyz(pos_flat, 0, 0, ry);
            pos_flat = dxyz(pos_flat, rx, 0, 0);
        } else {
            //2d = pan
            pos.i += rx;
            pos.j += ry;

            pos_flat.i += rx;
            pos_flat.j += ry;
        }
    }
    //*************************

    //CALCULATE PRESPECTIVE ADJUSTMENT
    //**********************************

    float standard_unit = 0.25 * camera_z;    //maxvalue=X, camera_z=4X

    zdistance = (camera_z - pos.k);       //3-5 standard_unit
    double zdistance_flat = (camera_z - pos_flat.k);

    zscale = standard_unit / zdistance;        //1 / (zdistance/standard_unit)
    double zscale_flat = standard_unit / zdistance_flat;

    double real_zoom = 0.0;
    double real_zoom_flat = 0.0;
//float _l2 = log(2.0f);
    if (system_not_galaxy) {
        const double zoom_s_tmp = zoom_s;
        real_zoom = zoom_s_tmp * zoom_s_tmp * zscale;
        real_zoom_flat = zoom_s_tmp * zoom_s_tmp * zscale_flat;
//real_zoom = zoom_s*zscale;
//real_zoom_flat = zoom_s*zscale_flat;
///		real_zoom = (log(zoom_s)/_l2)*zscale;
///		real_zoom_flat = (log(zoom_s)/_l2)*zscale_flat;
    } else {
        const double zoom_tmp = zoom;
        real_zoom = zoom_tmp * zoom_tmp * zscale;
        real_zoom_flat = zoom_tmp * zoom_tmp * zscale_flat;
//real_zoom = (log(zoom)/_l2)*zscale;
//real_zoom_flat = (log(zoom)/_l2)*zscale_flat;
    }
    pos.i *= real_zoom;
    pos.j *= real_zoom;
    pos.k *= real_zoom;

    pos_flat.i *= real_zoom_flat;
    pos_flat.j *= real_zoom_flat;
    pos_flat.k *= real_zoom_flat;

    float itemscale = real_zoom * item_zscalefactor;
    if (itemscale < minimumitemscaledown) {       //dont shrink into nonexistance
        itemscale = minimumitemscaledown;
    }
    if (itemscale > maximumitemscaleup) {         //dont expand too far
        itemscale = maximumitemscaleup;
    }
    system_item_scale_temp = system_item_scale * itemscale;
    //**********************************
    return itemscale;
}

void NavigationSystem::TranslateCoordinates(QVector &pos,
        QVector &pos_flat,
        float center_nav_x,
        float center_nav_y,
        float themaxvalue,
        float &zscale,
        float &zdistance,
        float &the_x,
        float &the_y,
        float &the_x_flat,
        float &the_y_flat,
        float &system_item_scale_temp,
        bool system_not_galaxy) {

    // This code is necessary to pan and zoom the nav star map
    // as well as to display system names and make systems
    // highlight their names on mouse hover
    // and also make them clickable for setting jump routes
    CalculatePerspectiveAdjustment(
            zscale,
            zdistance,
            pos,
            pos_flat,
            system_item_scale_temp,
            system_not_galaxy);

    //TRANSLATE INTO SCREEN DISPLAY COORDINATES
    //**********************************
    the_x = (float) pos.i;
    the_y = (float) pos.j;
    the_x_flat = (float) pos_flat.i;
    the_y_flat = (float) pos_flat.j;

    the_x = (the_x / (themaxvalue));
    the_y = (the_y / (themaxvalue));

    the_x_flat = (the_x_flat / (themaxvalue));
    the_y_flat = (the_y_flat / (themaxvalue));

    float navscreen_width_delta = (screenskipby4[1] - screenskipby4[0]);
    float navscreen_height_delta = (screenskipby4[3] - screenskipby4[2]);
    float navscreen_small_delta = std::min(navscreen_width_delta, navscreen_height_delta);

    the_x = (the_x * navscreen_small_delta);
    the_x = the_x + center_nav_x;
    the_y = (the_y * navscreen_small_delta);
    the_y = the_y + center_nav_y;

    the_x_flat = (the_x_flat * navscreen_small_delta);
    the_x_flat = the_x_flat + center_nav_x;
    the_y_flat = (the_y_flat * navscreen_small_delta);
    the_y_flat = the_y_flat + center_nav_y;
    //**********************************
    if ((system_not_galaxy ? system_view : galaxy_view) == VIEW_ORTHO) {
        the_x = the_x_flat;
        the_y = the_y_flat;
        pos = pos_flat;
    }
}

void NavigationSystem::TranslateAndDisplay(QVector &pos,
        QVector &pos_flat,
        float center_nav_x,
        float center_nav_y,
        float themaxvalue,
        float &zscale,
        float &zdistance,
        float &the_x,
        float &the_y,
        float &system_item_scale_temp,
        bool system_not_galaxy) {
    float the_x_flat;
    float the_y_flat;
    if ((system_not_galaxy ? system_view : galaxy_view) == VIEW_ORTHO) {
        TranslateCoordinates(pos, pos_flat, center_nav_x, center_nav_y, themaxvalue, zscale, zdistance,
                the_x, the_y, the_x_flat, the_y_flat, system_item_scale_temp, system_not_galaxy);
        return;
    } else {
        TranslateCoordinates(pos, pos_flat, center_nav_x, center_nav_y, themaxvalue, zscale, zdistance,
                the_x, the_y, the_x_flat, the_y_flat, system_item_scale_temp, system_not_galaxy);
    }
    DisplayOrientationLines(the_x, the_y, the_x_flat, the_y_flat, system_not_galaxy);
}

void NavigationSystem::DisplayOrientationLines(float the_x,
        float the_y,
        float the_x_flat,
        float the_y_flat,
        bool system_not_galaxy) {
    if ((system_not_galaxy ? system_view : galaxy_view) == VIEW_ORTHO) {
        return;
    }
    //Draw orientation lines
    //**********************************
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXColorf(GFXColor(0.5, 0.5, 0.5, .15));

    bool display_flat_circle = true;
    if ((the_y_flat > screenskipby4[3])
            || (the_y_flat < screenskipby4[2])
            || (the_x_flat > screenskipby4[1])
            || (the_x_flat < screenskipby4[0])) {
        GFXColorf(GFXColor(0, 1, 1, .05));
        display_flat_circle = false;
    }
    bool display_flat = true;
    if ((the_x > screenskipby4[1])
            || (the_x < screenskipby4[0])
            || (the_y > screenskipby4[3])
            || (the_y < screenskipby4[2])) {
        GFXColorf(GFXColor(1, 1, 0, .05));
        display_flat = false;
    }
    if (display_flat) {
        IntersectBorder(the_x_flat, the_y_flat, the_x, the_y);
        const float verts[2 * 3] = {
                the_x_flat, the_y_flat, 0,
                the_x, the_y, 0,
        };
        GFXDraw(GFXLINE, verts, 2);
        if (display_flat_circle) {
            DrawCircle(the_x_flat, the_y_flat, (.005 * system_item_scale), GFXColor(1, 1, 1, .2));
        }
    }

    GFXEnable(TEXTURE0);
    //**********************************
}

void Beautify(string systemfile, string &sector, string &system) {
    string::size_type slash = systemfile.find("/");
    if (slash == string::npos) {
        sector = "";
        system = systemfile;
    } else {
        sector = systemfile.substr(0, slash);
        system = systemfile.substr(slash + 1);
    }
    if (sector.size()) {
        sector[0] = toupper(sector[0]);
    }
    if (system.size()) {
        system[0] = toupper(system[0]);
    }
}

