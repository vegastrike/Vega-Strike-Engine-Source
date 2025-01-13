/*
 * intelligent.cpp
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


/* This class provides all AI related methods to intelligent unit type -
 * ships, space installations, missiles, drones, etc. */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "intelligent.h"

#include "ai/order.h"
#include "python/python_class.h"
#include "script/mission.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/ikarus.h"
#include "cmd/ai/missionscript.h"
#include "unit_generic.h"
#include "pilot.h"
#include "gfx/mesh.h"
#include "ai/turretai.h"
#include "collide2/CSopcodecollider.h"

#include <string>

using std::string;

// TODO: really. This should be properly defined
extern bool CheckAccessory(Unit *tur);

// TODO: remove
using namespace Orders;

Intelligent::Intelligent() {

}

void Intelligent::LoadAIScript(const std::string &s) {
    if (s.find(".py") != string::npos) {
        Order *ai = PythonClass<FireAt>::Factory(s);
        PrimeOrders(ai);
        return;
    } else {
        if (s.length() > 0) {
            if (*s.begin() == '_') {
                mission->addModule(s.substr(1));
                PrimeOrders(new AImissionScript(s.substr(1)));
            } else {
                if (s == "ikarus") {
                    PrimeOrders(new Orders::Ikarus());
                } else {
                    string ai_agg = s + ".agg.xml";
                    PrimeOrders(new Orders::AggressiveAI(ai_agg.c_str()));
                }
            }
        } else {
            PrimeOrders();
        }
    }
}

void Intelligent::eraseOrderType(unsigned int type) {
    if (aistate) {
        aistate->eraseType(type);
    }
}

bool Intelligent::LoadLastPythonAIScript() {
    Order *pyai = PythonClass<Orders::FireAt>::LastPythonClass();
    if (pyai) {
        PrimeOrders(pyai);
    } else if (!aistate) {
        PrimeOrders();
        return false;
    }
    return true;
}

bool Intelligent::EnqueueLastPythonAIScript() {
    Order *pyai = PythonClass<Orders::FireAt>::LastPythonClass();
    if (pyai) {
        EnqueueAI(pyai);
    } else if (!aistate) {
        return false;
    }
    return true;
}

void Intelligent::PrimeOrders(Order *newAI) {
    Unit *unit = static_cast<Unit *>(this);

    if (newAI) {
        if (aistate) {
            aistate->Destroy();
        }
        aistate = newAI;
        newAI->SetParent(unit);
    } else {
        PrimeOrders();
    }
}

void Intelligent::PrimeOrders() {
    Unit *unit = static_cast<Unit *>(this);

    if (aistate) {
        aistate->Destroy();
        aistate = nullptr;
    }
    aistate = new Order;                 //get 'er ready for enqueueing
    aistate->SetParent(unit);
}

void Intelligent::PrimeOrdersLaunched() {
    Unit *unit = static_cast<Unit *>(this);

    if (aistate) {
        aistate->Destroy();
        aistate = nullptr;
    }
    Vector vec(0, 0, 10000);
    aistate = new ExecuteFor(new Orders::MatchVelocity(unit->ClampVelocity(vec, true), Vector(0,
                    0,
                    0), true, true,
            false), 4.0f);
    aistate->SetParent(unit);
}

void Intelligent::SetAI(Order *newAI) {
    Unit *unit = static_cast<Unit *>(this);

    newAI->SetParent(unit);
    if (aistate) {
        aistate->ReplaceOrder(newAI);
    } else {
        aistate = newAI;
    }
}

void Intelligent::EnqueueAI(Order *newAI) {
    Unit *unit = static_cast<Unit *>(this);

    newAI->SetParent(unit);
    if (aistate) {
        aistate->EnqueueOrder(newAI);
    } else {
        aistate = newAI;
    }
}

void Intelligent::EnqueueAIFirst(Order *newAI) {
    Unit *unit = static_cast<Unit *>(this);

    newAI->SetParent(unit);
    if (aistate) {
        aistate->EnqueueOrderFirst(newAI);
    } else {
        aistate = newAI;
    }
}

void Intelligent::ExecuteAI() {
    Unit *unit = static_cast<Unit *>(this);
    Flightgroup *flightgroup = unit->flightgroup;
    if (flightgroup) {
        Unit *leader = flightgroup->leader.GetUnit();
        //no heirarchy in flight group
        if (leader ? (flightgroup->leader_decision > -1) && (leader->getFgSubnumber() >= unit->getFgSubnumber())
                : true) {
            if (!leader) {
                flightgroup->leader_decision = flightgroup->nr_ships;
            }
            flightgroup->leader.SetUnit(unit);
        }
        flightgroup->leader_decision--;
    }
    if (aistate) {
        aistate->Execute();
    }
    if (!unit->SubUnits.empty()) {
        un_iter iter = unit->getSubUnits();
        Unit *un;
        while ((un = *iter)) {
            un->ExecuteAI();                     //like dubya
            ++iter;
        }
    }
}

string Intelligent::getFullAIDescription() {
    Unit *unit = static_cast<Unit *>(this);

    if (getAIState()) {
        return unit->getFgID() + ":" + getAIState()->createFullOrderDescription(0).c_str();
    } else {
        return "no order";
    }
}

float Intelligent::getRelation(const Unit *targ) const {
    const Unit *unit = static_cast<const Unit *>(this);

    return unit->pilot->GetEffectiveRelationship(unit, targ);
}

double Intelligent::getMinDis(const QVector &pnt) const {
    const Unit *unit = static_cast<const Unit *>(this);

    float minsofar = 1e+10;
    float tmpvar;
    unsigned int i;
    Vector TargetPoint(unit->cumulative_transformation_matrix.getP());

#ifdef VARIABLE_LENGTH_PQR
    //the scale factor of the current UNIT
    float SizeScaleFactor = sqrtf( TargetPoint.Dot( TargetPoint ) );
#endif
    for (i = 0; i < unit->nummesh(); ++i) {
        TargetPoint =
                (Transform(unit->cumulative_transformation_matrix, unit->meshdata[i]->Position()).Cast() - pnt).Cast();
        tmpvar = sqrtf(TargetPoint.Dot(TargetPoint)) - unit->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
            *SizeScaleFactor
#endif
                ;
        if (tmpvar < minsofar) {
            minsofar = tmpvar;
        }
    }
    for (un_kiter ui = unit->viewSubUnits(); !ui.isDone(); ++ui) {
        tmpvar = (*ui)->getMinDis(pnt);
        if (tmpvar < minsofar) {
            minsofar = tmpvar;
        }
    }
    return minsofar;
}

void Intelligent::SetTurretAI() {
    Unit *unit = static_cast<Unit *>(this);

    unit->turretstatus = 2;
    static bool talkinturrets = XMLSupport::parse_bool(vs_config->getVariable("AI", "independent_turrets", "false"));
    if (talkinturrets) {
        Unit *un;
        for (un_iter iter = unit->getSubUnits(); (un = *iter); ++iter) {
            if (!CheckAccessory(un)) {
                un->EnqueueAIFirst(new Orders::FireAt(configuration()->ai.firing_config.aggressivity));
                un->EnqueueAIFirst(new Orders::FaceTarget(false, 3));
            }
            un->SetTurretAI();
        }
    } else {
        Unit *un;
        for (un_iter iter = unit->getSubUnits(); (un = *iter); ++iter) {
            if (!CheckAccessory(un)) {
                if (un->aistate) {
                    un->aistate->Destroy();
                }
                un->aistate = (new Orders::TurretAI());
                un->aistate->SetParent(un);
            }
            un->SetTurretAI();
        }
    }
}

void Intelligent::DisableTurretAI() {
    Unit *unit = static_cast<Unit *>(this);

    unit->turretstatus = 1;
    Unit *un;
    for (un_iter iter = unit->getSubUnits(); (un = *iter); ++iter) {
        if (un->aistate) {
            un->aistate->Destroy();
        }
        un->aistate = new Order;         //get 'er ready for enqueueing
        un->aistate->SetParent(un);
        un->UnFire();
        un->DisableTurretAI();
    }
}

csOPCODECollider *Intelligent::getCollideTree(const Vector &RESTRICT scale, std::vector<mesh_polygon> *RESTRICT pol) {
    Unit *unit = static_cast<Unit *>(this);

    if (!pol) {
        vector<mesh_polygon> polies;
        for (unsigned int j = 0; j < unit->nummesh(); j++) {
            unit->meshdata[j]->GetPolys(polies);
        }
        if (scale.i != 1 || scale.j != 1 || scale.k != 1) {
            for (unsigned int i = 0; i < polies.size(); ++i) {
                for (unsigned int j = 0; j < polies[i].v.size(); ++j) {
                    polies[i].v[j].i *= scale.i;
                    polies[i].v[j].j *= scale.j;
                    polies[i].v[j].k *= scale.k;
                }
            }
        }
        return new csOPCODECollider(polies);
    }
    if (scale.i != 1 || scale.j != 1 || scale.k != 1) {
        for (unsigned int i = 0; i < pol->size(); ++i) {
            for (unsigned int j = 0; j < (*pol)[i].v.size(); ++j) {
                (*pol)[i].v[j].i *= scale.i;
                (*pol)[i].v[j].j *= scale.j;
                (*pol)[i].v[j].k *= scale.k;
            }
        }
    }
    return new csOPCODECollider(*pol);
}
