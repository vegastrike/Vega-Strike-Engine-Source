/*
 * mission.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alexander Rawass
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


/*
 *  xml Mission written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>

#include <Python.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <assert.h>
#include "cmd/unit_generic.h"
#include "mission.h"
#include "flightgroup.h"

#include "src/python/python_class.h"
#include "vegadisk/savegame.h"
#include "src/universe.h"
#include "src/vs_logging.h"
#include "src/vs_exit.h"

/* *********************************************************** */
Mission::~Mission() {
    VS_LOG(info, "Mission Cleanup Not Yet Implemented");
    //do not delete msgcenter...could be vital
}

double Mission::gametime = 0.0;
int Mission::number_of_ships = 0;
int Mission::number_of_flightgroups = 0;
int Mission::total_nr_frames = 0;
vector<Flightgroup *>Mission::flightgroups;

Mission::Mission(const char *filename, const std::string &script, bool loadscripts) {
    ConstructMission(filename, script, loadscripts);
}

Mission::Mission(const char *filename, bool loadscripts) {
    ConstructMission(filename, string(""), loadscripts);
}

void Mission::ConstructMission(const char *configfile, const std::string &script, bool loadscripts) {
    player_autopilot = global_autopilot = AUTO_NORMAL;
    player_num = 0;
    briefing = NULL;
    director = NULL;
    runtime.pymissions = NULL;
    nextpythonmission = NULL;
    if (script.length() > 0) {
        nextpythonmission = new char[script.length() + 2];
        nextpythonmission[script.length() + 1] = 0;
        nextpythonmission[script.length()] = 0;
        strcpy(nextpythonmission, script.c_str());
    }
    easyDomFactory<missionNode> domf;
    top = domf.LoadXML(configfile);
    static bool dontpanic = false;
    if (top == NULL && !dontpanic) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Panic exit - mission file %1% not found") % configfile));
        VSExit(0);
    } else {
        dontpanic = true;
    }
    if (top == NULL) {
        return;
    }

    variables = NULL;
    origin_node = NULL;

#ifndef VS_MIS_SEL
    director = NULL;
    if (loadscripts) {
        initTagMap();
        initCallbackMaps();

        top->Tag(&tagmap);
    }
#endif
}

Unit *Mission::Objective::getOwner() {
    Unit *Nawl = NULL;
    if (Owner != Nawl) {
        Unit *ret = Owner.GetUnit();
        if (ret == NULL) {
            objective = "";
        }              //unit died
        return ret;
    }
    return Owner.GetUnit();
}

MessageCenter *Mission::msgcenter = NULL;

void Mission::initMission(bool loadscripts) {
    if (!top) {
        return;
    }
    if (msgcenter == NULL) {
        msgcenter = new MessageCenter();
    }
    checkMission(top, loadscripts);
    mission_name = getVariable("mission_name", "");
}

/* *********************************************************** */

bool Mission::checkMission(easyDomNode *node, bool loadscripts) {
    if (node->Name() != "mission") {
        VS_LOG(warning, "this is no Vegastrike mission file");
        return false;
    }
    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        if ((*siter)->Name() == "variables") {
            doVariables(*siter);
        } else if (((*siter)->Name() == "flightgroups")) {
            doFlightgroups(*siter);
        } else if (((*siter)->Name() == "settings")) {
            doSettings(*siter);
        } else if (((*siter)->Name() == "module")) {
            if (loadscripts) {
                DirectorStart((missionNode *) *siter);
            }
        } else if (((*siter)->Name() == "python") && (!this->nextpythonmission)) {
            //I need to get rid of an extra whitespace at the end that expat may have added... Python is VERY strict about that... :(
            string locals = (*siter)->attr_value(textAttr);
            const char *constdumbstr = locals.c_str();             //get the text XML attribute
            int i = strlen(constdumbstr);          //constdumbstr is the string I wish to copy... i is its length.
            char *dumbstr = new char[i + 2];             //allocate 2 extra bytes for a double-null-terminated string.
            strncpy(dumbstr, constdumbstr, i);             //i copy constdumbstr to dumbstr.
            dumbstr[i] = '\0';             //I make sure that it has 2 null bytes at the end.
            dumbstr[i + 1] = '\0';             //I am allowed to use i+1 because I allocated 2 extra bytes
            for (i -= 1; i >= 0; i--) {
                //start from the end-1, or i-1 and set i to that value(i-=1)
                if (dumbstr[i] == '\t' || dumbstr[i] == ' ' || dumbstr[i] == '\r' || dumbstr[i] == '\n') {
                    //I check if dumbstr[i] contains whitespace
                    dumbstr[i] = '\0';                     //if so, I truncate the string
                } else {
                    dumbstr[i + 1] =
                            '\n';                     //otherwise I add a new line (python sometimes gets mad...)
                    dumbstr[i + 2] =
                            '\0';                     //and add a null byte (If i is at the end of the allocated memory, I will use the extra byte
                    break;                     //get out of the loop so that it doesn't endlessly delete the newlines that I added.
                }
            }
            this->nextpythonmission = dumbstr;
        } else {
            VS_LOG(warning, (boost::format("warning: Unknown tag: %1%") % ((*siter)->Name())));
        }
    }
    return true;
}

static std::vector<Mission *> Mission_delqueue;

void Mission::wipeDeletedMissions() {
    while (!Mission_delqueue.empty()) {
        delete Mission_delqueue.back();
        Mission_delqueue.pop_back();
    }
}

int Mission::getPlayerMissionNumber() {
    int num = 0;

    vector<Mission *> *active_missions = ::active_missions.Get();
    vector<Mission *>::iterator pl = active_missions->begin();

    if (pl == active_missions->end()) {
        return -1;
    }

    for (; pl != active_missions->end(); ++pl) {
        if ((*pl)->player_num == this->player_num) {
            if (*pl == this) {
                return (int) num;
            } else {
                num++;
            }
        }
    }

    return -1;
}

Mission *Mission::getNthPlayerMission(int cp, int missionnum) {
    vector<Mission *> *active_missions = ::active_missions.Get();
    Mission *activeMis = NULL;
    if (missionnum >= 0) {
        int num = -1;
        vector<Mission *>::iterator pl = active_missions->begin();
        if (pl == active_missions->end()) {
            return NULL;
        }
        for (; pl != active_missions->end(); ++pl) {
            if ((*pl)->player_num == (unsigned int) cp) {
                num++;
            }
            if (num == missionnum) {
                //Found it!
                activeMis = (*pl);
                break;
            }
        }
    }
    return activeMis;
}

void Mission::terminateMission() {
    vector<Mission *> *active_missions = ::active_missions.Get();
    vector<Mission *>::iterator f;

    f = std::find(Mission_delqueue.begin(), Mission_delqueue.end(), this);
    if (f != Mission_delqueue.end()) {
        VS_LOG(info, (boost::format("Not deleting mission twice: %1%") % this->mission_name));
    }

    f = std::find(active_missions->begin(), active_missions->end(), this);

    // Debugging aid for persistent missions bug
    if (g_game.vsdebug >= 1) {
        int misnum = -1;
        for (vector<Mission *>::iterator i = active_missions->begin(); i != active_missions->end(); ++i) {
            if ((*i)->player_num == player_num) {
                ++misnum;
                VS_LOG(info, (boost::format("   Mission #%1%: %2%") % misnum % (*i)->mission_name));
            }
        }
    }

    int queuenum = -1;
    if (f != active_missions->end()) {
        queuenum = getPlayerMissionNumber();          //-1 used as error code, 0 is first player mission

        active_missions->erase(f);
    }
    if (this != (*active_missions)[0]) {        //Shouldn't this always be true?
        Mission_delqueue.push_back(this);
    }          //only delete if we arent' the base mission
    //NETFIXME: This routine does not work properly yet.
    VS_LOG(info, (boost::format("Terminating mission %1% #%2%") % this->mission_name % queuenum));
    if (queuenum >= 0) {
        // queuenum - 1 since mission #0 is the base mission (main_menu) and is persisted
        // in savegame.cpp:LoadSavedMissions, and it has no correspondin active_scripts/active_missions entry,
        // meaning the actual active_scripts index is offset by 1.
        unsigned int num = queuenum - 1;

        vector<std::string> *scripts = &_Universe->AccessCockpit(player_num)->savegame->getMissionStringData(
                "active_scripts");
        VS_LOG(info, (boost::format("Terminating mission #%1% - got %2% scripts") % queuenum % scripts->size()));
        if (num < scripts->size()) {
            scripts->erase(scripts->begin() + num);
        }
        vector<std::string> *missions = &_Universe->AccessCockpit(player_num)->savegame->getMissionStringData(
                "active_missions");
        VS_LOG(info, (boost::format("Terminating mission #%1% - got %2% missions") % queuenum % missions->size()));
        if (num < missions->size()) {
            missions->erase(missions->begin() + num);
        }
        VS_LOG(info, (boost::format("Terminating mission #%1% - %2% scripts remain") % queuenum % scripts->size()));
        VS_LOG(info, (boost::format("Terminating mission #%1% - %2% missions remain") % queuenum % missions->size()));
    }
    if (runtime.pymissions) {
        runtime.pymissions->Destroy();
    }
    runtime.pymissions = NULL;
}

/* *********************************************************** */

void Mission::doOrigin(easyDomNode *node) {
    origin_node = node;
}

/* *********************************************************** */

#ifndef VS_MIS_SEL

void Mission::GetOrigin(QVector &pos, string &planetname) {
    if (origin_node == NULL) {
        pos.i = pos.j = pos.k = 0.0;
        planetname = string();
        return;
    }
    bool ok = doPosition(origin_node, &pos.i, NULL);
    if (!ok) {
        pos.i = pos.j = pos.k = 0.0;
    }
    planetname = origin_node->attr_value("planet");
}

#endif

/* *********************************************************** */

void Mission::doSettings(easyDomNode *node) {
    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        easyDomNode *mnode = *siter;
        if (mnode->Name() == "origin") {
            doOrigin(mnode);
        }
    }
}

/* *********************************************************** */

void Mission::doVariables(easyDomNode *node) {
    if (variables != NULL) {
        VS_LOG(warning, "only one variable section allowed");
        return;
    }
    variables = node;

    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        checkVar(*siter);
    }
}

/* *********************************************************** */

void Mission::checkVar(easyDomNode *node) {
    if (node->Name() != "var") {
        VS_LOG(warning, "not a variable");
        return;
    }
    string name = node->attr_value("name");
    string value = node->attr_value("value");
}

/* *********************************************************** */

void Mission::doFlightgroups(easyDomNode *node) {
    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        checkFlightgroup(*siter);
    }
}

void Mission::AddFlightgroup(Flightgroup *fg) {
    fg->flightgroup_nr = flightgroups.size();
    flightgroups.push_back(fg);
    number_of_flightgroups = flightgroups.size();
}

/* *********************************************************** */

void Mission::checkFlightgroup(easyDomNode *node) {
    if (node->Name() != "flightgroup") {
        VS_LOG(warning, "not a flightgroup");
        return;
    }
    //nothing yet
    string texture = node->attr_value("logo");
    string texture_alpha = node->attr_value("logo_alpha");
    string name = node->attr_value("name");
    string faction = node->attr_value("faction");
    string type = node->attr_value("type");
    string ainame = node->attr_value("ainame");
    string waves = node->attr_value("waves");
    string nr_ships = node->attr_value("nr_ships");
    string terrain_nr = node->attr_value("terrain_nr");
    string unittype = node->attr_value("unit_type");
    if (name.empty() || faction.empty() || type.empty() || ainame.empty() || waves.empty() || nr_ships.empty()) {
        VS_LOG(warning, "no valid flightgroup decsription");
        return;
    }
    if (unittype.empty()) {
        unittype = string("unit");
    }
    int waves_i = atoi(waves.c_str());
    int nr_ships_i = atoi(nr_ships.c_str());

    bool have_pos = false;

    double pos[3];
    float rot[3];

    rot[0] = rot[1] = rot[2] = 0.0;
    CreateFlightgroup cf;
    cf.fg = Flightgroup::newFlightgroup(name, type, faction, ainame, nr_ships_i, waves_i, texture, texture_alpha, this);
    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        if ((*siter)->Name() == "pos") {
            have_pos = doPosition(*siter, pos, &cf);
        }
//        else if ( (*siter)->Name() == "rot" )
//            doRotation( *siter, rot, &cf );  This function isn't implemented yet
        else if ((*siter)->Name() == "order") {
            doOrder(*siter, cf.fg);
        }
    }
    if (!have_pos) {
        VS_LOG(warning, (boost::format("don't have a position in flightgroup %1%") % name));
    }
    if (terrain_nr.empty()) {
        cf.terrain_nr = -1;
    } else {
        if (terrain_nr == "mission") {
            cf.terrain_nr = -2;
        } else {
            cf.terrain_nr = atoi(terrain_nr.c_str());
        }
    }
    cf.unittype = CreateFlightgroup::UNIT;
    if (unittype == "vehicle") {
        cf.unittype = CreateFlightgroup::VEHICLE;
    }
    if (unittype == "building") {
        cf.unittype = CreateFlightgroup::BUILDING;
    }
    cf.nr_ships = nr_ships_i;
    cf.domnode = (node);       //don't hijack node

    cf.fg->pos.i = pos[0];
    cf.fg->pos.j = pos[1];
    cf.fg->pos.k = pos[2];
    for (int i = 0; i < 3; i++) {
        cf.rot[i] = rot[i];
    }
    cf.nr_ships = nr_ships_i;
    if (ainame[0] == '_') {
#ifndef VS_MIS_SEL
        addModule(ainame.substr(1));
#endif
    }
    number_of_ships += nr_ships_i;
}

/* *********************************************************** */

bool Mission::doPosition(easyDomNode *node, double pos[3], CreateFlightgroup *cf) {
    string x = node->attr_value("x");
    string y = node->attr_value("y");
    string z = node->attr_value("z");
    if (x.empty() || y.empty() || z.empty()) {
        VS_LOG(warning, "no valid position");
        return false;
    }
    pos[0] = strtod(x.c_str(), NULL);
    pos[1] = strtod(y.c_str(), NULL);
    pos[2] = strtod(z.c_str(), NULL);
    if (cf != NULL) {
        pos[0] += cf->fg->pos.i;
        pos[1] += cf->fg->pos.j;
        pos[2] += cf->fg->pos.k;
    }
    return true;
}

/* *********************************************************** */

Flightgroup *Mission::findFlightgroup(const string &offset_name, const string &fac) {
    vector<Flightgroup *>::const_iterator siter;
    for (siter = flightgroups.begin(); siter != flightgroups.end(); siter++) {
        if ((*siter)->name == offset_name && (fac.empty() || (*siter)->faction == fac)) {
            return *siter;
        }
    }
    return NULL;
}

/* *********************************************************** */

/* void Mission::doRotation( easyDomNode *node, float rot[3], class CreateFlightgroup* )
{
    //not yet
    //return true;
}
*/
/* *********************************************************** */

void Mission::doOrder(easyDomNode *node, Flightgroup *fg) {
    //nothing yet
    string order = node->attr_value("order");
    string target = node->attr_value("target");
    if (order.empty() || target.empty()) {
        VS_LOG(warning, "you have to give an order and a target");
        return;
    }
    //the tmptarget is evaluated later
    //because the target may be a flightgroup that's not yet defined
    fg->ordermap[order] = target;
}

/* *********************************************************** */

string Mission::getVariable(string name, string defaultval) {
    vector<easyDomNode *>::const_iterator siter;
    for (siter = variables->subnodes.begin(); siter != variables->subnodes.end(); siter++) {
        string scan_name = (*siter)->attr_value("name");
        if (scan_name == name) {
            return (*siter)->attr_value("value");
        }
    }
    return defaultval;
}
