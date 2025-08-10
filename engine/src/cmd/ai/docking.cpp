/*
 * docking.cpp
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


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "src/python/python_compile.h"
#include "docking.h"
#include "root_generic/xml_support.h"
#include "src/config_xml.h"
#include "cmd/unit_generic.h"
#include "warpto.h"
#include "src/universe_util.h"
#include <string>

static void DockedScript(Unit *docker, Unit *base) {
    static string script = vs_config->getVariable("AI", "DockedToScript", "");
    if (script.length() > 0) {
        Unit *targ = docker->Target();
        docker->Target(base);
        UniverseUtil::setScratchUnit(docker);
        CompileRunPython(script);
        UniverseUtil::setScratchUnit(NULL);
        docker->Target(targ);         //should be NULL;
    }
}

namespace Orders {
DockingOps::DockingOps(Unit *unitToDockWith, Order *ai, bool physically_dock, bool keeptrying) : MoveTo(QVector(0,
                0,
                1),
        false,
        10, false),
        docking(unitToDockWith),
        state(GETCLEARENCE),
        oldstate(ai) {
    formerOwnerDoNotDereference = NULL;
    this->keeptrying = keeptrying;
    facedtarget = false;
    physicallyDock = true;
    port = -1;
    const float temptimer = configuration()->physics.docking_time_flt;
    timer = temptimer;
}

void DockingOps::SetParent(Unit *par) {
    MoveTo::SetParent(par);
    if (parent) {
        formerOwnerDoNotDereference = parent->owner;
        parent->SetOwner(docking.GetUnit());
    }
}

void DockingOps::Execute() {
    Unit *utdw = docking.GetUnit();
    if (parent == utdw || utdw == NULL) {
        RestoreOldAI();
        Destroy();
        return;
    }
    switch (state) {
        case GETCLEARENCE:
            if (!RequestClearence(utdw)) {
                if (!keeptrying) {
                    RestoreOldAI();
                    Destroy();
                    return;
                }
            } else {
                state = DOCKING;
                //no break
            }
        case DOCKING:
            if (DockToTarget(utdw)) {
                state = DOCKED;
            }
            break;
        case DOCKED:
            if (PerformDockingOperations(utdw)) {
                state = UNDOCKING;
            }
            break;
        case UNDOCKING:
            if (Undock(utdw)) {
                RestoreOldAI();
                Destroy();
                return;
            }
            break;
    }
    parent->SetAngularVelocity(Vector(0, 0, 0));     //FIXME if you want it to turn to dock point
    done = false;
}

void DockingOps::Destroy() {
    if (parent) {
        if (oldstate) {
            oldstate->Destroy();
        }
        oldstate = NULL;
        if (formerOwnerDoNotDereference) {
            parent->SetOwner((Unit *) formerOwnerDoNotDereference);             //set owner will not deref
            formerOwnerDoNotDereference = NULL;
        }
    }
    docking.SetUnit(NULL);
}

void DockingOps::RestoreOldAI() {
    if (parent) {
        parent->aistate = oldstate;         //that's me!
        if (formerOwnerDoNotDereference) {
            parent->SetOwner((Unit *) formerOwnerDoNotDereference);
            formerOwnerDoNotDereference = NULL;
        }
        oldstate = NULL;
    }
}

int SelectDockPort(Unit *utdw, Unit *parent) {
    const vector<DockingPorts> &dp = utdw->DockingPortLocations();
    float dist = FLT_MAX;
    int num = -1;
    for (unsigned int i = 0; i < dp.size(); ++i) {
        if (!dp[i].IsOccupied()) {
            Vector rez = Transform(utdw->GetTransformation(), dp[i].GetPosition());
            float wdist = (rez - parent->Position()).MagnitudeSquared();
            if (wdist < dist) {
                num = i;
                dist = wdist;
            }
        }
    }
    return num;
}

bool DockingOps::RequestClearence(Unit *utdw) {
    if (physicallyDock && !utdw->RequestClearance(parent)) {
        return false;
    }
    port = SelectDockPort(utdw, parent);
    if (port == -1) {
        return false;
    }
    return true;
}

QVector DockingOps::Movement(Unit *utdw) {
    const QVector loc(Transform(utdw->GetTransformation(), utdw->DockingPortLocations()[port].GetPosition().Cast()));
    SetDest(loc);

    SetAfterburn(DistanceWarrantsTravelTo(parent, (loc - parent->Position()).Magnitude(), true));
    if (!facedtarget) {
        facedtarget = true;
        EnqueueOrder(new ChangeHeading(loc, 4, 1, true));
    }
    MoveTo::Execute();
    if (rand() % 256 == 0) {
        WarpToP(parent, utdw, true);
    }
    return loc;
}


bool DockingOps::DockToTarget(Unit *utdw) {
    if (utdw->DockingPortLocations()[port].IsOccupied()) {
        if (keeptrying) {
            state = GETCLEARENCE;
            return false;
        } else {
            docking.SetUnit(NULL);
            state = GETCLEARENCE;
            return false;
        }
    }
    QVector loc = Movement(utdw);
    float rad = utdw->DockingPortLocations()[port].GetRadius() + parent->rSize();
    float diss = (parent->Position() - loc).MagnitudeSquared() - .1;
    bool isplanet = utdw->getUnitType() == Vega_UnitType::planet;

    if (diss <= (isplanet ? rad * rad : parent->rSize() * parent->rSize())) {
        DockedScript(parent, utdw);
        if (physicallyDock) {
            return parent->Dock(utdw);
        } else {
            rechargeShip(parent, 0);
            return true;
        }
    } else if (diss <= 1.2 * rad * rad) {
        timer += SIMULATION_ATOM;
        const float tmp = configuration()->physics.docking_time_flt;
        if (timer >= 1.5 * tmp) {
            if (physicallyDock) {
                return parent->Dock(utdw);
            } else {
                rechargeShip(parent, 0);
                return true;
            }
        }
    }
    return false;
}

bool DockingOps::PerformDockingOperations(Unit *utdw) {
    timer -= SIMULATION_ATOM;
    bool isplanet = utdw->getUnitType() == Vega_UnitType::planet;
    if (timer < 0) {
        const float tmp = configuration()->physics.un_docking_time_flt;
        timer = tmp;
        EnqueueOrder(new ChangeHeading(parent->Position() * 2 - utdw->Position(), 4, 1, true));
        if (physicallyDock) {
            return parent->UnDock(utdw);
        } else {
            return true;
        }
    } else if (!physicallyDock) {
        if (isplanet) {
            //orbit;
            QVector cur = utdw->Position() - parent->Position();
            QVector up = QVector(0, 1, 0);
            if (up.i == cur.i && up.j == cur.j && up.k == cur.k) {
                up = QVector(0, 0, 1);
            }
            SetDest(cur.Cross(up) * 10000);
            MoveTo::Execute();
        } else {
            Movement(utdw);
        }
    }
    return false;
}

bool DockingOps::Undock(Unit *utdw) {
    //this is a good heuristic... find the location where you are.compare with center...then fly the fuck away
    QVector awaydir = parent->Position() - utdw->Position();
    float len = ((utdw->rSize() + parent->rSize() * 2) / awaydir.Magnitude());
    awaydir *= len;
    SetDest(awaydir + utdw->Position());
    MoveTo::Execute();
    timer -= SIMULATION_ATOM;
    return (len < 1) || done || timer < 0;
}

DockingOps *DONOTUSEAI = NULL;
}

