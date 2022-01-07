/**
 * missionscript.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef _CMD_AI_MISSION_SCRIPT_H_
#define _CMD_AI_MISSION_SCRIPT_H_

#include "order.h"
#include "navigation.h"
#include "flybywire.h"

#include "cmd/script/mission.h"

class AImissionScript : public FlyByWire {
public:
///saves scriptname in the filename var
    AImissionScript(string modulename);
    ~AImissionScript();
///Loads the AI script from the hard drive, or executes if loaded
    void Execute();

    virtual string getOrderDescription()
    {
        char buffer[300];
        sprintf(buffer, "%s:%d:%s", modulename.c_str(), classid, getActionString().c_str());
        return buffer;
    }

protected:

    string modulename;
    unsigned int classid;
    bool first_run;
};

class AIFlyToWaypoint : public AImissionScript {
public:
    AIFlyToWaypoint(const QVector &waypoint, float vel, bool afburn, float range);

    QVector waypoint;
    float vel;
    float range;
    bool aburn;
};
class AIFlyToWaypointDefend : public AImissionScript {
public:
    AIFlyToWaypointDefend(const QVector &waypoint, float vel, bool afburn, float range, float defend_range);

    QVector waypoint;
    float vel;
    float range;
    bool aburn;
};

class AIFlyToJumppoint : public AImissionScript {
public:
    AIFlyToJumppoint(Unit *jumppoint, float fly_speed, bool aft);
};

class AIPatrol : public AImissionScript {
public:
    AIPatrol(int mode, const QVector &area, float range, Unit *around_unit, float patrol_speed);
};

class AIPatrolDefend : public AImissionScript {
public:
    AIPatrolDefend(int mode, const QVector &area, float range, Unit *around_unit, float patrol_speed);
};

class AISuperiority : public AImissionScript {
public:
    AISuperiority();
};

class AIOrderList : public AImissionScript {
public:
    AIOrderList(olist_t *orderlist);

    virtual olist_t *getOrderList()
    {
        return my_orderlist;
    }

private:
    olist_t *my_orderlist;
};

#endif

