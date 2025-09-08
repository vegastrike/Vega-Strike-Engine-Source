/*
 * turretai.cpp
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


#include "src/config_xml.h"
#include "root_generic/vs_globals.h"
#include "turretai.h"
#include "cmd/unit_generic.h"
#include "cmd/role_bitmask.h"
#include "src/universe.h"

using namespace Orders;

TurretAI::TurretAI() : FaceTargetITTS(false) {
    type |= WEAPON;
    range = -1;
    speed = 1;
    mrange = 1;
    hadFired = false;
}

void TurretAI::getAverageGunSpeed(float &speed, float &range, float &mrange) const {
    speed = this->speed;
    range = this->range;
    mrange = this->mrange;
}

extern unsigned int FireBitmask(Unit *parent, bool shouldfire, bool firemissile);

void TurretAI::Execute() {
    Unit *targ = parent->Target();
    if (range == -1) {
        range = mrange = speed = 0;
        parent->getAverageGunSpeed(speed, range, mrange);
        float tspeed, trange, tmrange;
        Unit *gun;
        if (parent->getNumMounts() == 0) {
            speed = 1;
            range = 1;
            mrange = 1;                             //not much
        }
        for (un_iter i = parent->getSubUnits(); (gun = *i) != NULL; ++i) {
            (*i)->getAverageGunSpeed(tspeed, trange, tmrange);
            if (trange > range) {
                speed = tspeed;
                range = trange;
                mrange = tmrange;
            }
        }
        if (range == 0) {
            range = mrange;
            speed = FLT_MAX;
        }
    }
    if (targ) {
        const float dot_cutoff = configuration().ai.firing.turret_dot_cutoff_flt;
        const float missile_prob = configuration().ai.firing.turret_missile_probability_flt;
        FaceTargetITTS::Execute();
        if (parent->getNumMounts() > 0) {
            Vector R(parent->GetTransformation().getR());
            QVector Pos(targ->Position() - parent->Position());
            double mag = Pos.Magnitude();
            Pos = Pos / mag;
            float dot = R.Dot(Pos.Cast());
            int neu = FactionUtil::GetNeutralFaction();
            int upg = FactionUtil::GetUpgradeFaction();
            bool isplayerstarship = _Universe->isPlayerStarshipVoid(parent->owner) != NULL;

            bool shouldfire =
                    ((mag - targ->rSize() - parent->rSize() < range
                            && dot > dot_cutoff)
                            && (isplayerstarship == false || targ->faction == upg
                                    || (isplayerstarship
                                            && (targ->getRelation((Unit *) parent->owner)
                                                    < 0 /*now that it is a player, we know it's dereferencable*/
                                                    || targ->Target() == (Unit *) parent->owner)))
                            && targ->faction != neu);

            //FIXME - rand() is not going to be in the expected range here - stephengtuggy 2020-07-25
            parent->Fire(FireBitmask(parent, shouldfire, rand() < missile_prob * RAND_MAX * SIMULATION_ATOM), true);
            if (!shouldfire) {
                parent->UnFire();
            }
            hadFired = shouldfire;
        }
        if (targ->Destroyed()) {
            parent->Target(nullptr);
        }
    } else if (hadFired && parent->getNumMounts() > 0) {
        // When we get a kill, we must stop firing
        parent->UnFire();
        hadFired = false;
    }
}

