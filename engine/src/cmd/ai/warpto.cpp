/**
 * warpto.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2025 Stephen G. Tuggy, and other Vega Strike Contributors
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "src/universe_util.h"
#include "src/config_xml.h"

float max_allowable_travel_time() {
    const float mat = vega_config::config->ai.max_allowable_travel_time;
    return mat;
}

bool DistanceWarrantsWarpTo(Unit *parent, float dist, bool following) {
    //first let us decide whether the target is far enough to warrant using warp
    //double dist =UnitUtil::getSignificantDistance(parent,target);
    const float tooclose = vega_config::config->ai.too_close_for_warp_tactic;
    const float tooclosefollowing = vega_config::config->ai.too_close_for_warp_in_formation;
    float toodamnclose = following ? tooclosefollowing : tooclose;
    float diff = 1;
    parent->GetVelocityDifficultyMult(diff);
    float timetolive = dist / (diff * parent->drive.speed);
    if (timetolive > (5 * max_allowable_travel_time())) {
        return true;
    } else if (timetolive > (max_allowable_travel_time())) {
        float mytime = SIMULATION_ATOM * 1.5;
        const bool rampdown = vega_config::config->physics.autopilot_ramp_warp_down;
        if (rampdown == false) {
            const float warprampdowntime = vega_config::config->physics.warprampdowntime;
            mytime = warprampdowntime;
        }
        if (dist - parent->GetWarpVelocity().Magnitude() * mytime < toodamnclose) {
            return false;
        }              //avoid nasty jitter-jumping behavior should eventually have "running away check"

        return true;
    }
    return false;
}

bool DistanceWarrantsTravelTo(Unit *parent, float dist, bool following) {
    //first let us decide whether the target is far enough to warrant using warp
    float diff = 1;
    parent->GetVelocityDifficultyMult(diff);
    float timetolive = dist / (diff * parent->drive.speed);
    if (timetolive > max_allowable_travel_time()) {
        return true;
    }
    return false;
}

bool TargetWorthPursuing(Unit *parent, Unit *target) {
    return true;
}

static void ActuallyWarpTo(Unit *parent, const QVector &tarpos, Vector tarvel, Unit *MatchSpeed = NULL) {
    Vector vel = parent->GetVelocity();
    const float mindirveldot = vega_config::config->ai.warp_cone;
    const float mintarveldot = vega_config::config->ai.match_velocity_cone;
    tarvel.Normalize();
    vel.Normalize();
    Vector dir = tarpos - parent->Position();
    dir.Normalize();
    float dirveldot = dir.Dot(parent->cumulative_transformation_matrix.getR());
    dir *= -1;
    float chasedot = dir.Dot(tarvel);
    if (dirveldot > mindirveldot) {
        const float min_energy_to_enter_warp = vega_config::config->ai.min_energy_to_enter_warp;
        const float min_warpfield_to_enter_warp = vega_config::config->ai.min_warp_to_try;
        if ((parent->ftl_energy.Percent() > min_energy_to_enter_warp)
                && (parent->GetMaxWarpFieldStrength(1) > min_warpfield_to_enter_warp)) {
            if (!parent->ftl_drive.Enabled()) {
                parent->ftl_drive.Enable();                 //don't want the AI thrashing
                parent->graphicOptions.WarpRamping = 1;                 //don't want the AI thrashing
            }
        }
    } else {
        parent->ftl_drive.Disable();
    }
    const bool domatch = vega_config::config->ai.match_velocity_of_pursuant;
    if (chasedot > mintarveldot || !domatch) {
        parent->computer.velocity_ref.SetUnit(NULL);
    } else {
        parent->computer.velocity_ref.SetUnit(MatchSpeed);
    }
}

void WarpToP(Unit *parent, Unit *target, bool following) {
    float dist = UnitUtil::getSignificantDistance(parent, target);
    if (DistanceWarrantsWarpTo(parent, dist, following)) {
        if (TargetWorthPursuing(parent, target)) {
            const bool auto_valid = vega_config::config->physics.in_system_jump_or_timeless_auto_pilot; // Default: false
            if (auto_valid) {
                std::string tmp;
                parent->AutoPilotTo(target, false);
            } else {
                ActuallyWarpTo(parent, target->Position(), target->GetVelocity(), target);
            }
        }
    } else {
        parent->ftl_drive.Disable();
    }
}

void WarpToP(Unit *parent, const QVector &target, float radius, bool following) {
    float dist = (parent->Position() - target).Magnitude() - radius - parent->rSize();
    if (DistanceWarrantsWarpTo(parent, dist, following)) {
        const bool auto_valid = vega_config::config->physics.in_system_jump_or_timeless_auto_pilot /* default: false */
        if (!auto_valid) {
            ActuallyWarpTo(parent, target, QVector(0, 0, .00001));
        }
    } else {
        parent->ftl_drive.Disable();
    }
}

