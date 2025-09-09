/*
 * director.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically Alexander Rawass
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
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#include <boost/python.hpp>
#include <boost/python/class.hpp>

#include <Python.h>

#include "src/python/python_class.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif

#include <fstream>

#include "src/vegastrike.h"
#include "root_generic/lin_time.h"
#include "cmd/unit_generic.h"

#include "cmd/ai/order.h"
#include "mission.h"

#include "root_generic/vs_globals.h"
#include "vegadisk/savegame.h"
#include "msgcenter.h"
#include "cmd/briefing.h"
#include "pythonmission.h"
#include "flightgroup.h"
#include "gldrv/winsys.h"
#include "src/vs_logging.h"

/* *********************************************************** */
//ADD_FROM_PYTHON_FUNCTION(pythonMission)
void Mission::DirectorLoop() {
    double oldgametime = gametime;
    gametime += SIMULATION_ATOM;     //elapsed;
    //VS_LOG(trace, (boost::format("void Mission::DirectorLoop(): oldgametime = %1$.6f; SIMULATION_ATOM = %2$.6f; gametime = %3$.6f") % oldgametime % SIMULATION_ATOM % gametime));
    if (getTimeCompression() >= .1) {
        if (gametime <= oldgametime) {
            VS_LOG(warning, "void Mission::DirectorLoop(): gametime is before oldgametime!");
            gametime = SIMULATION_ATOM;
        }
    }
    try {
        BriefingLoop();
        if (runtime.pymissions) {
            runtime.pymissions->Execute();
        }
    }
    catch (...) {
        if (PyErr_Occurred()) {
            VS_LOG_AND_FLUSH(fatal, "void Mission::DirectorLoop(): Python error occurred");
            PyErr_Print();
            PyErr_Clear();
            VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
        }
        throw;
    }
}

void Mission::DirectorEnd() {
    if (director == NULL) {
        return;
    }
    RunDirectorScript("endgame");
}

void Mission::DirectorShipDestroyed(Unit *unit) {
    Flightgroup *fg = unit->getFlightgroup();
    if (fg == nullptr) {
        VS_LOG(info, "ship destroyed-no flightgroup");
        return;
    }
    if (fg->nr_ships_left <= 0 && fg->nr_waves_left > 0) {
        VS_LOG(info, "WARNING: nr_ships_left<=0");
        return;
    }
    fg->nr_ships_left -= 1;

    char buf[512];

    if ((fg->faction.length() + fg->type.length() + fg->name.length() + 12 + 30) < sizeof(buf)) {
        sprintf(buf, "Ship destroyed: %s:%s:%s-%d", fg->faction.c_str(), fg->type.c_str(),
                fg->name.c_str(), unit->getFgSubnumber());
    } else {
        sprintf(buf, "Ship destroyed: (ERROR)-%d", unit->getFgSubnumber());
    }

    msgcenter->add("game", "all", buf);

    if (fg->nr_ships_left == 0) {
        VS_LOG(debug, (boost::format("no ships left in fg %1%") % fg->name));
        if (fg->nr_waves_left > 0) {
            VS_LOG(info, (boost::format("Relaunching %1% wave") % fg->name));

            //launch new wave
            fg->nr_waves_left -= 1;
            fg->nr_ships_left = fg->nr_ships;

            Order *order = NULL;
            order = unit->getAIState() ? unit->getAIState()->findOrderList() : NULL;
            fg->orderlist = NULL;
            if (order) {
                fg->orderlist = order->getOrderList();
            }
            CreateFlightgroup cf{};
            cf.fg = fg;
            cf.unittype = CreateFlightgroup::UNIT;
            cf.fg->pos = unit->Position();
            cf.waves = fg->nr_waves_left;
            cf.nr_ships = fg->nr_ships;

            call_unit_launch(&cf, Vega_UnitType::unit, string(""));
        } else {
            mission->msgcenter->add("game", "all", "Flightgroup " + fg->name + " destroyed");
        }
    }
}

bool Mission::BriefingInProgress() {
    return briefing != NULL;
}

void Mission::BriefingStart() {
    if (briefing) {
        BriefingEnd();
    }
    briefing = new Briefing();
    if (runtime.pymissions) {
        runtime.pymissions->callFunction("initbriefing");
    }
}

void Mission::BriefingUpdate() {
    if (briefing) {
        briefing->Update();
    }
}

void Mission::BriefingLoop() {
    if (briefing) {
        if (runtime.pymissions) {
            runtime.pymissions->callFunction("loopbriefing");
        }
    }
}

class TextPlane *Mission::BriefingRender() {
    if (briefing) {
        vector<std::string> who;
        who.push_back("briefing");
        string str1;
        gameMessage g1, g2;
        if (msgcenter->last(0, g1, who)) {
            str1 = g1.message;
        }
        if (msgcenter->last(1, g2, who)) {
            str1 = str1 + string("\n") + g2.message;
        }
        briefing->tp.SetText(str1);
        briefing->Render();
        return &briefing->tp;
    }
    return NULL;
}

void Mission::BriefingEnd() {
    if (briefing) {
        if (runtime.pymissions) {
            runtime.pymissions->callFunction("endbriefing");
        }
        delete briefing;
        briefing = NULL;
    }
}

void Mission::DirectorBenchmark() {
    total_nr_frames++;
    if (benchmark > 0.0 && benchmark < gametime) {
        VS_LOG_AND_FLUSH(trace,
                (boost::format("Game was running for %1% secs,   av. framerate %2%") % gametime
                        % (((double) total_nr_frames) / gametime)));
        winsys_exit(0);
    }
}
