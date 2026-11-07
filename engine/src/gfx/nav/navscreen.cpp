/*
 * navscreen.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: hellcatv, ace123, surfdargent, klaussfreire, jacks, pyramid3d
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
#include "gfx/nav/navscreen.h"
#include "gfx/masks.h"
#include "gfx/nav/navgetxmldata.h"
#include "gfx/nav/navitemstodraw.h"
#include "gfx/nav/navparse.h"
#include "gfx/nav/navcomputer.h"
#include "gfx/nav/navpath.h"
#include "gldrv/winsys.h"
#include "gui/imgui_support.h"

// The nav buttons are rounded rectangles, measured in pixels.
static const float kNavButtonRounding = 3.0f;
static const float kNavButtonOutlineThickness = 1.5f;

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

    rotations = 0;

    minimumitemscaledown = 0.2;
    maximumitemscaleup = 3.0;


    //Both cameras are framed to their content the first time they are drawn, and
    //then keep whatever position and orientation the player gives them.
    system_needs_refit = true;
    galaxy_needs_refit = true;
    nav_near_dist = 1e30;         //replaced with the real distance as the view draws

    rx = -0.5;              //galaxy mode settings
    ry = 0.5;
    rz = 0.0;
    zoom = 1.0;             //open zoomed out far enough that the whole map fits

    rx_s = -0.5;              //system mode settings
    ry_s = 1.5;
    rz_s = 0.0;
    zoom_s = 1.0;             //as above, for the system map

    scrolloffset = 0;

    camera_z = 1.0;     //updated after a pass
    center_x = 0.0;     //updated after a pass
    center_y = 0.0;     //updated after a pass
    center_z = 0.0;     //updated after a pass

    path_view = PATH_ON;

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
    // The map fills the whole screen. The button column is drawn over its right edge.
    screenskipby4[0] = 0;
    screenskipby4[1] = 1;
    screenskipby4[2] = 0;
    screenskipby4[3] = 1;

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
        // Without the file there are no system item scaling parameters.
        unsetbit(whattodraw, 4);
        VS_LOG(error, "ERROR: navdata.xml not found. Nav system items will not be scaled.");
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

    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXColor4f(1, 1, 1, 1);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(LIGHTING);

    GFXHudMode(true);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    StartGUIFrame();
 
    // The map is drawn with the same field of view as the world around it, so that it
    // reads as a view of the same place rather than a differently-lensed one. It stays
    // fixed: moving closer or further away is the camera moving, not the lens changing.
    const float nav_fov = static_cast<float>(configuration().graphics.fov_flt * M_PI / 180.0);
    system_cam.setFov(nav_fov);
    galaxy_cam.setFov(nav_fov);

    // The nav computer is a flat interface drawn over the game, so hide the game completely.
    const ImVec2 start_position(0,0);
    const ImVec2 end_position(configuration().graphics.resolution_x,
                              configuration().graphics.resolution_y);
    const ImU32 background_color = IM_COL32(0,0,0,255);
    ImGui::GetBackgroundDrawList()->AddRectFilled(start_position, end_position, background_color,
                    0.0f // No rounded borders
    );

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
        DrawGrid(screenskipby4[0],
                screenskipby4[1],
                screenskipby4[2],
                screenskipby4[3],
                GFXColor(1, 1, 1, 0.2));
        if (checkbit(whattodraw, 2)) {
            DrawGalaxy();
        } else {
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
    const bool outlinebuttons = true;
    DrawButton(buttonskipby4_1[0], buttonskipby4_1[1], buttonskipby4_1[2], buttonskipby4_1[3], 1, outlinebuttons);
    DrawButton(buttonskipby4_2[0], buttonskipby4_2[1], buttonskipby4_2[2], buttonskipby4_2[3], 2, outlinebuttons);
    DrawButton(buttonskipby4_3[0], buttonskipby4_3[1], buttonskipby4_3[2], buttonskipby4_3[3], 3, outlinebuttons);
    DrawButton(buttonskipby4_4[0], buttonskipby4_4[1], buttonskipby4_4[2], buttonskipby4_4[3], 4, outlinebuttons);
    DrawButton(buttonskipby4_5[0], buttonskipby4_5[1], buttonskipby4_5[2], buttonskipby4_5[3], 5, outlinebuttons);
    //**********************************

    // A short reminder of the controls, along the bottom of the screen.
    static const bool draw_nav_help =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "draw_nav_help", "true"));
    if (draw_nav_help) {
        // drawdescription() centres its text on the position it is given, so these are
        // placed around the middle of the screen rather than at the left edge, and far
        // enough up that neither line is cut off by the bottom.
        const float help_y = screenskipby4[2] + 0.10f;
        const float help_x = (screenskipby4[0] + screenskipby4[1]) * 0.5f;
        const GFXColor helpcol(0.7f, 0.7f, 0.7f, 0.85f);
        drawdescription("Mouse:  click selects    left-drag circles the target    middle-drag moves the map    "
                        "right-drag looks around    wheel moves in/out",
                help_x, help_y, 0.6f, 0.6f, true, screenoccupation, helpcol);
        drawdescription("Keys:   arrows move the map    Shift+arrows look around    Alt+arrows move in/out and sideways",
                help_x, help_y + 0.05f, 0.6f, 0.6f, true, screenoccupation, helpcol);
    }

    //Save current mouse location as previous for next cycle
    //**********************************
    mouse_x_previous = (-1 + float(mousex) / (.5 * configuration().graphics.resolution_x));
    mouse_y_previous = (1 + float(-1 * mousey) / (.5 * configuration().graphics.resolution_y));
    //**********************************

    GFXEnable(TEXTURE0);
    EndGUIFrame(MOUSE_POINTER_NORMAL);
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

    drawdescription(" ", (originx + (0.2 * deltax)), (originy), 1, 1, 0, screenoccupation, GFXColor(.3, 1, .3, 1));
    drawdescription(" ", (originx + (0.2 * deltax)), (originy), 1, 1, 0, screenoccupation, GFXColor(.3, 1, .3, 1));

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
            drawdescription(relationtext, (originx + (0.2 * deltax)), (originy), 1, 1, 0, screenoccupation,
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
    GFXColor temp_color(1, 1, 1, 1);
    displayname.color = static_cast<ImU32>(temp_color);
    displayname.SetSize(.62, -.7);
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
            "#FFA000     PRESS SHIFT-M TO TOGGLE THIS MENU    \n\n\n\n#000000*******#00a6FFVega Strike 0.10.0#000000*********\nWelcome to VS. Your ship undocks stopped; #8080FFArrow keys/mouse/joystick#000000 steer your ship. Use #8080FF+#000000 & #8080FF-#000000 to adjust cruise control, or #8080FF/#000000 & #8080FF[backspace]#000000 to go to max governor setting or full-stop, respectively. Use #8080FFy#000000 to toggle between maneuver and travel settings for your relative velocity governors. Use #8080ff[home]#000000 & #8080FF[end]#000000 to set and unset velocity reference point to the current target (non-hostile targets only). Use #8080FFTab#000000 to activate Overdrive(if present).\n\nPress #8080FFn#000000 to cycle nav points, #8080FFt#000000 to cycle targets, and #8080FFp#000000 to target objects in front of you.\n\n#8080FF[space]#000000 fires guns, and #8080ff[Enter]#000000 fires missiles.\n\nThe #8080FFa#000000 key activates SPEC drive for insystem FTL.\nInterstellar Travel requires a #FFBB11 jump drive#000000 and #FFBB11FTL Capacitors#000000 to be installed. To jump, fly into the green wireframe nav-marker; hit #8080FFj#000000 to jump to the linked system.\n\nTo dock, target a base, planet or large vessel and hail with #8080FF0#000000 to request docking clearance. When you get close, a green box will appear. Fly to the box. When inside the box, #8080FFd#000000 will dock.\n\n#FF0000If Vega Strike halts or acts oddly,#000000\n#FFFF00immediately#000000 post the latest log\nfile from $HOME/.vegastrike/logs/\nto https://forums.vega-strike.org/\nbefore you restart Vega Strike.\n";
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
    GFXColor temp_color(.3, 1, .3, 1);
    displayname.color = static_cast<ImU32>(temp_color);
    displayname.SetSize(.7, -.8);
    displayname.SetPos(originx - (.1 * deltax), originy /*+(1*deltay)*/ );
    displayname.SetText(writethis);
    displayname.SetCharSize(1, 1);
    const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
    GFXColor tpbg(displayname.background_color);
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        GFXColor temp_background_color( 0, 0, 0, background_alpha );
        displayname.background_color = static_cast<ImU32>(temp_background_color);
    }
    displayname.Draw(writethis, 0, true, false, automatte);
    displayname.background_color = static_cast<ImU32>(tpbg);

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
//getMouseButtonStatus()&1 = left button
//getMouseButtonStatus()&2 = middle button
//getMouseButtonStatus()&4 = right button
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
    // Frame the galaxy on the newly focused system, which is what resetting the old pan
    // position used to achieve.
    galaxy_needs_refit = true;
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
    } else if (checkbit(whattodraw, 1)) {
        if (button_number == 2) {
            label = "Path On/Off/Only";
        } else if (button_number == 4) {
            label = "Up";
        } else if (button_number == 5) {
            label = "Down";
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
    GFXColor temp_color(1, 1, 1, 1);
    a_label.color = static_cast<ImU32>(temp_color);
    int length = label.size();
    float offset = (float(length) * 0.0065);
    float xl = (x1 + x2) / 2.0;
    float yl = (y1 + y2) / 2.0;
    a_label.SetPos((xl - offset) - (checkbit(buttonstates, button_number - 1) ? 0.006 : 0), (yl + 0.025));
    a_label.SetText(label);

    // A subtle dark fill so the button reads as a button rather than as bare text.
    ImDrawList *draw_list = GetNavDrawList();
    draw_list->AddRectFilled(NormToPixel(x1, y2), NormToPixel(x2, y1), IM_COL32(0, 0, 0, 153), kNavButtonRounding);

    const bool nav_button_labels = configuration().graphics.draw_nav_button_labels;
    if (nav_button_labels) {
        const float background_alpha = configuration().graphics.hud.text_background_alpha_flt;
        GFXColor tpbg(a_label.background_color);
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            GFXColor temp_background_color( 0, 0, 0, background_alpha );
            a_label.background_color = static_cast<ImU32>(temp_background_color);
        }
        a_label.Draw(label, 0, true, false, automatte);
        a_label.background_color = static_cast<ImU32>(tpbg);
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
            if (!checkbit(whattodraw, 1)) {
                //if in mission mode

                flipbit(whattodraw, 1);
            }
        }
        //******************************************************
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
    GetNavDrawList()->AddRect(NormToPixel(x1, y2), NormToPixel(x2, y1), ToImColor(col), kNavButtonRounding, 0,
            kNavButtonOutlineThickness);
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

void NavigationSystem::Adjust3dTransformation(bool is_system_not_galaxy) {
    // Drives the camera of whichever view is showing, with the same bindings in both:
    // the right button looks around, the left and middle buttons move the map, and the
    // wheel moves in towards it or back out.
    //
    // The indices are the bits lookupMouseButton() sets: 0 is the left button, 1 the
    // middle one and 2 the right (see in_mouse.cpp).
    NavMap &camera = is_system_not_galaxy ? system_cam : galaxy_cam;
    if (!TestIfInRange(screenskipby4[0], screenskipby4[1], screenskipby4[2], screenskipby4[3], mouse_x_current,
            mouse_y_current)) {
        return;
    }

    if (mouse_previous_state[2] == 1) {
        // Right-drag looks around from where the camera is.
        const float ndx = mouse_x_current - mouse_x_previous;
        const float ndy = mouse_y_current - mouse_y_previous;
        camera.orbitBy(ndx * 0.6f, -ndy * 0.6f);      //y flipped, so that dragging up looks up
    }

    // Panning, and moving in and out, scale with the distance to the nearest thing in
    // view, which is only known once the view has been drawn: fall back to the framing
    // distance for the first frame, or when there was nothing in view.
    const double scale = (nav_near_dist < 1e30) ? nav_near_dist : camera.nominalDistance();

    if (mouse_previous_state[0] == 1) {
        // Left-drag turns the map about the selected object, which is what the player is
        // pointing at when they do this. The object keeps the place it has on the screen
        // and everything else swings around it. With nothing selected there is no pivot to
        // work about, so turn about the point the camera is looking at.
        QVector pivot = camera.focusPoint();
        if (is_system_not_galaxy) {
            Unit *target = _Universe->AccessCockpit()->GetParent()->Target();
            if (target != nullptr) {
                pivot = target->Position();
            }
        } else if (systemselectionindex < systemIter.size()) {
            // The system selected on the map. The focused one only changes when a
            // selection is clicked twice, so it lags behind.
            pivot = systemIter[systemselectionindex].Position();
        }
        const float ndx = mouse_x_current - mouse_x_previous;
        const float ndy = mouse_y_current - mouse_y_previous;
        camera.orbitAround(pivot, ndx * 0.6f, -ndy * 0.6f);
    }

    if (mouse_previous_state[1] == 1) {
        // Middle-drag moves the map.
        const float ndx = mouse_x_current - mouse_x_previous;
        const float ndy = mouse_y_current - mouse_y_previous;
        const double step = scale * 0.5;
        camera.panBy(-ndx * step, -ndy * step);
    }

    const float wheel_zoom_level = configuration().graphics.wheel_zoom_amount_flt;
    if (mouse_wentdown[3] || mouse_wentdown[4]) {
        // The wheel moves the camera in and out. The lens does not change.
        const double step = scale * wheel_zoom_level;
        camera.zoomBy(mouse_wentdown[3] ? step : -step);
    }
}

void NavigationSystem::arrowKey(int dir, unsigned int mods) {
    // Keyboard camera control, for while the nav computer is open: the caller gates
    // the ship's own arrow-key handlers, so these never steer the ship.
    //   arrows         = move the map
    //   Shift+arrows   = look around
    //   Alt+up/down    = move in towards the map, or back out
    //   Alt+left/right = move the map sideways
    NavMap &camera = checkbit(whattodraw, 2) ? galaxy_cam : system_cam;

    const bool shift = (mods & KB_MOD_SHIFT) != 0;
    const bool alt = (mods & KB_MOD_ALT) != 0;

    // As with the mouse, steps scale with the distance to the nearest object in view.
    const double scale = (nav_near_dist < 1e30) ? nav_near_dist : camera.nominalDistance();
    const double step = scale * 0.5;

    if (shift) {
        const float amount = 0.05f;
        if (dir == 0) {
            camera.orbitBy(0.0f, amount);
        } else if (dir == 1) {
            camera.orbitBy(0.0f, -amount);
        } else if (dir == 2) {
            camera.orbitBy(amount, 0.0f);
        } else {
            camera.orbitBy(-amount, 0.0f);
        }
    } else if (alt) {
        if (dir == 0) {
            camera.zoomBy(step);
        } else if (dir == 1) {
            camera.zoomBy(-step);
        } else if (dir == 2) {
            camera.panBy(-step, 0.0);
        } else {
            camera.panBy(step, 0.0);
        }
    } else {
        if (dir == 0) {
            camera.panBy(0.0, step);
        } else if (dir == 1) {
            camera.panBy(0.0, -step);
        } else if (dir == 2) {
            camera.panBy(-step, 0.0);
        } else {
            camera.panBy(step, 0.0);
        }
    }
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


/*
 * Display orientation projection lines using ImGui DrawList and pixel projection.
 */

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

