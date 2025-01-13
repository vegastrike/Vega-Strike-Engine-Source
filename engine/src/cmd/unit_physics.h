/*
 * unit_physics.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_PHYSICS_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_PHYSICS_H

#include "gfx/mesh.h"
#include "lin_time.h"
#include "beam.h"
#include "planet.h"
#include "audiolib.h"
#include "configxml.h"
#include "vs_globals.h"
#include "vs_math.h"

#ifdef FIX_TERRAIN
#include "gfx/planetary_transform.h"
#endif
#include "gfx/cockpit.h"
#include "unit_util.h"
#include "universe_util.h"
#include "cmd/script/mission.h"

extern unsigned int apply_float_to_unsigned_int(float tmp); //short fix


void Unit::UpdatePhysics2(const Transformation &trans,
        const Transformation &old_physical_state,
        const Vector &accel,
        float difficulty,
        const Matrix &transmat,
        const Vector &cum_vel,
        bool lastframe,
        UnitCollection *uc) {
    Movable::UpdatePhysics2(trans, old_physical_state, accel, difficulty, transmat, cum_vel, lastframe, uc);

    this->AddVelocity(difficulty);

#ifdef DEPRECATEDPLANETSTUFF
    if (planet) {
        Matrix basis;
        curr_physical_state.to_matrix( this->cumulative_transformation_matrix );
        Vector p, q, r, c;
        MatrixToVectors( this->cumulative_transformation_matrix, p, q, r, c );
        planet->trans->InvTransformBasis( this->cumulative_transformation_matrix, p, q, r, c );
        planet->cps = Transformation::from_matrix( this->cumulative_transformation_matrix );
    }
#endif
    this->cumulative_transformation = this->curr_physical_state;
    this->cumulative_transformation.Compose(trans, transmat);
    this->cumulative_transformation.to_matrix(this->cumulative_transformation_matrix);
    this->cumulative_velocity = TransformNormal(transmat, this->Velocity) + cum_vel;
    unsigned int i, n;
    if (lastframe) {
        char tmp = 0;
        //double blah = queryTime();
        for (i = 0, n = this->meshdata.size(); i < n; i++) {
            if (!this->meshdata[i]) {
                continue;
            }
            if (!this->meshdata[i]->HasBeenDrawn()) {
                this->meshdata[i]->UpdateFX(simulation_atom_var /*SIMULATION_ATOM?*/ );
            } else {
                this->meshdata[i]->UnDraw();
                tmp = 1;
            }
        }
        //double blah1 = queryTime();
        if (!tmp && this->Destroyed()) {
            Explode(false, simulation_atom_var /*SIMULATION_ATOM?*/ );
        }
        //double blah2 = queryTime();
    }
}

/****************************** ONLY SOUND/GFX STUFF LEFT IN THOSE FUNCTIONS *********************************/


void GameUnit::Thrust(const Vector &amt1, bool afterburn) {
    if (this->afterburntype == 0) {
        afterburn = afterburn && this->energy > this->afterburnenergy * simulation_atom_var;
    } //SIMULATION_ATOM; ?
    if (this->afterburntype == 1) {
        afterburn = afterburn && this->fuel > 0;
    }
    if (this->afterburntype == 2) {
        afterburn = afterburn && this->warpenergy > 0;
    }
    Unit::Thrust(amt1, afterburn);

    static bool must_afterburn_to_buzz =
            XMLSupport::parse_bool(vs_config->getVariable("audio", "buzzing_needs_afterburner", "false"));
    if (_Universe->isPlayerStarship(this) != NULL) {
        static int playerengine = AUDCreateSound(vs_config->getVariable("unitaudio",
                "player_afterburner",
                "sfx10.wav"), true);
        static float enginegain = XMLSupport::parse_float(vs_config->getVariable("audio", "afterburner_gain", ".5"));
        if (afterburn != AUDIsPlaying(playerengine)) {
            if (afterburn) {
                AUDPlay(playerengine, QVector(0, 0, 0), Vector(0, 0, 0), enginegain);
            } else {
                AUDStopPlaying(playerengine);
            }
        }
    } else if (afterburn || !must_afterburn_to_buzz) {
        static float buzzingtime = XMLSupport::parse_float(vs_config->getVariable("audio", "buzzing_time", "5"));
        static float
                buzzingdistance = XMLSupport::parse_float(vs_config->getVariable("audio", "buzzing_distance", "5"));
        static float lastbuzz = getNewTime();
        Unit *playa = _Universe->AccessCockpit()->GetParent();
        if (playa) {
            if (UnitUtil::getDistance(this,
                    playa) < buzzingdistance && playa->owner != this && this->owner != playa
                    && this->owner != playa->owner) {
                float ttime = getNewTime();
                if (ttime - lastbuzz > buzzingtime) {
                    Vector pvel = playa->GetVelocity();
                    Vector vel = this->GetVelocity();
                    pvel.Normalize();
                    vel.Normalize();
                    float dotprod = vel.Dot(pvel);
                    if (dotprod < .86) {
                        lastbuzz = ttime;
                        //AUDPlay( this->sound->engine, this->Position(), this->GetVelocity(), 1 );
                        playEngineSound();
                    } else {
                    }
                }
            }
        }
    }
}

Vector Unit::ResolveForces(const Transformation &trans, const Matrix &transmat) {
#ifndef PERFRAMESOUND
    //AUDAdjustSound( this->sound->engine, this->cumulative_transformation.position, this->cumulative_velocity );
    adjustSound(SoundType::engine);
#endif
    return Movable::ResolveForces(trans, transmat);
}

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_PHYSICS_H
