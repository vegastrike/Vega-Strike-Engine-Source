/*
 * missionscript.cpp
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


/*
 *  AI for mission scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include "script.h"
#include "navigation.h"
#include "xml_support.h"
#include "flybywire.h"
#include <stdio.h>
#include <vector>
#include <stack>
#include "vs_logging.h"
#include "tactics.h"
#include "cmd/unit_generic.h"

#include "missionscript.h"
#include "cmd/script/mission.h"

AImissionScript::AImissionScript(string modname) {
    actionstring = "";

    modulename = modname;

    classid = mission->createClassInstance(modulename);

    first_run = true;
}

AImissionScript::~AImissionScript() {
    VS_LOG(info, "destructor\n");
    VS_LOG(info, parent->getFullAIDescription().c_str());

    mission->runScript(modulename, "quitai", classid);

    mission->destroyClassInstance(modulename, classid);
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(debug, (boost::format("aims%1$x") % this));
#endif
}

void AImissionScript::Execute() {
    desired_ang_velocity = Vector(0, 0, 0);
    desired_velocity = Vector(0, 0, 0);

    mission->setCurrentAIUnit(parent);
    mission->setCurrentAIOrder(this);
    if (first_run) {
        for (unsigned int i = 0;
                (i < active_missions.size())
                        && (!(mission->runScript(modulename, "initai", classid)));
                i++) {
        }
        first_run = false;
    }
    mission->runScript(modulename, "executeai", classid);

    varInst *vi = mission->lookupClassVariable(modulename, "aistyle", classid);
    if (vi == NULL || vi->type != VAR_INT) {
        Order::Execute();
    } else {
        if (vi->int_val == 0) {
            Order::Execute();
        } else if (vi->int_val == 1) {
            FlyByWire::Execute();
        }
    }
    mission->deleteVarInst(vi);
    done = false;

    varInst *done_vi = mission->lookupClassVariable(modulename, "_done", classid);
    if (done_vi != NULL && done_vi->type == VAR_BOOL && done_vi->bool_val == true) {
        done = true;
    }
    mission->deleteVarInst(done_vi);
}

AIFlyToWaypoint::AIFlyToWaypoint(const QVector &wp, float velo, bool afburn,
        float rng) : AImissionScript("ai_flyto_waypoint") {
    waypoint = wp;
    vel = velo;
    range = rng;
    aburn = afburn;

    varInst *vi_wp = mission->lookupClassVariable(modulename, "waypoint", classid);
    mission->call_vector_into_olist(vi_wp, waypoint);

    varInst *vi_range = mission->lookupClassVariable(modulename, "abort_range", classid);
    vi_range->float_val = range;

    varInst *vi_vel = mission->lookupClassVariable(modulename, "vel", classid);
    vi_vel->float_val = vel;

    varInst *vi_aburn = mission->lookupClassVariable(modulename, "afterburner", classid);
    vi_aburn->bool_val = aburn;
}

AIFlyToWaypointDefend::AIFlyToWaypointDefend(const QVector &wp, float velo, bool afburn, float rng,
        float defend_range) : AImissionScript("ai_flyto_waypoint_defend") {
    waypoint = wp;
    vel = velo;
    range = rng;
    aburn = afburn;

    varInst *vi_wp = mission->lookupClassVariable(modulename, "waypoint", classid);
    mission->call_vector_into_olist(vi_wp, waypoint);

    varInst *vi_range = mission->lookupClassVariable(modulename, "abort_range", classid);
    vi_range->float_val = range;

    varInst *vi_defrange = mission->lookupClassVariable(modulename, "defend_range", classid);
    vi_defrange->float_val = defend_range;

    varInst *vi_vel = mission->lookupClassVariable(modulename, "vel", classid);
    vi_vel->float_val = vel;

    varInst *vi_aburn = mission->lookupClassVariable(modulename, "afterburner", classid);
    vi_aburn->bool_val = aburn;
}

AISuperiority::AISuperiority() : AImissionScript("ai_superiority") {
}

AIFlyToJumppoint::AIFlyToJumppoint(Unit *jumppoint_unit, float fly_speed, bool aft) : AImissionScript(
        "ai_flyto_jumppoint") {
    varInst *vi_speed = mission->lookupClassVariable(modulename, "fly_speed", classid);
    vi_speed->float_val = fly_speed;

    varInst *vi_aft = mission->lookupClassVariable(modulename, "afterburner", classid);
    vi_aft->bool_val = aft;

    varInst *vi_unit = mission->lookupClassVariable(modulename, "jumppoint_unit", classid);
    vi_unit->objectname = "unit";
    vi_unit->object = jumppoint_unit;
}

AIPatrol::AIPatrol(int mode, const QVector &area, float range, Unit *around_unit,
        float patrol_speed) : AImissionScript("ai_patrol") {
    varInst *vi_wp = mission->lookupClassVariable(modulename, "area", classid);
    mission->call_vector_into_olist(vi_wp, area);

    varInst *vi_range = mission->lookupClassVariable(modulename, "range", classid);
    vi_range->float_val = range;

    varInst *vi_speed = mission->lookupClassVariable(modulename, "patrol_speed", classid);
    vi_speed->float_val = patrol_speed;

    varInst *vi_mode = mission->lookupClassVariable(modulename, "patrol_mode", classid);
    vi_mode->int_val = mode;

    varInst *vi_unit = mission->lookupClassVariable(modulename, "around_unit", classid);
    vi_unit->objectname = "unit";
    vi_unit->object = around_unit;
}

AIPatrolDefend::AIPatrolDefend(int mode, const QVector &area, float range, Unit *around_unit,
        float patrol_speed) : AImissionScript("ai_patrol_defend") {
    varInst *vi_wp = mission->lookupClassVariable(modulename, "area", classid);
    mission->call_vector_into_olist(vi_wp, area);

    varInst *vi_range = mission->lookupClassVariable(modulename, "range", classid);
    vi_range->float_val = range;

    varInst *vi_speed = mission->lookupClassVariable(modulename, "patrol_speed", classid);
    vi_speed->float_val = patrol_speed;

    varInst *vi_mode = mission->lookupClassVariable(modulename, "patrol_mode", classid);
    vi_mode->int_val = mode;

    varInst *vi_unit = mission->lookupClassVariable(modulename, "around_unit", classid);
    vi_unit->objectname = "unit";
    vi_unit->object = around_unit;
}

AIOrderList::AIOrderList(olist_t *orderlist) : AImissionScript("ai_orderlist") {
    varInst *vi_unit = mission->lookupClassVariable(modulename, "my_order_list", classid);
    vi_unit->objectname = "olist";
    vi_unit->object = orderlist;

    my_orderlist = orderlist;
}

