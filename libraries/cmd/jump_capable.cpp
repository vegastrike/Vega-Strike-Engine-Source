/*
 * jump_capable.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2021 Roy Falk and Stephen G. Tuggy
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


#include "jump_capable.h"
#include "ai/order.h"
#include "unit_find.h"
#include "src/universe.h"
#include "src/universe_util.h"
#include "gfx/warptrail.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_exit.h"
#include "vega_cast_utils.h"

// TODO: once implementation is refactored, deal with this too
extern QVector RealPosition(const Unit *un);
extern float getAutoRSize(Unit *orig, Unit *un, bool ignore_friend = false);
extern float globQueryShell(QVector st, QVector dir, float radius);

static QVector AutoSafeEntrancePoint(const QVector start, float rsize, const Unit *goal) {
    QVector def = UniverseUtil::SafeEntrancePoint(start, rsize);
    double bdis = (def - RealPosition(goal)).MagnitudeSquared();
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            for (int k = -1; k <= 1; k += 2) {
                QVector delta(i, j, k);
                delta.Normalize();
                QVector tmp = RealPosition(goal) + delta * (goal->rSize() + rsize);
                tmp = UniverseUtil::SafeEntrancePoint(tmp, rsize);
                double tmag = (tmp - RealPosition(goal)).MagnitudeSquared();
                if (tmag < bdis) {
                    bdis = tmag;
                    def = tmp;
                }
            }
        }
    }
    return def;
}

signed char ComputeAutoGuarantee(Unit *un) {
    Cockpit *cp;
    size_t cpnum = 0;
    if ((cp = _Universe->isPlayerStarship(un))) {
        cpnum = cp - _Universe->AccessCockpit(0);
    } else {
        return Mission::AUTO_ON;
    }
    size_t i = 0;
    for (i = 0; i < active_missions.size(); ++i) {
        if (active_missions[i]->player_num == cpnum && active_missions[i]->player_autopilot != Mission::AUTO_NORMAL) {
            return active_missions[i]->player_autopilot;
        }
    }

    for (i = 0; i < active_missions.size(); ++i) {
        if (active_missions[i]->global_autopilot != Mission::AUTO_NORMAL) {
            return active_missions[i]->global_autopilot;
        }
    }

    return Mission::AUTO_NORMAL;
}

std::string GenerateAutoError(Unit *me, Unit *targ) {
    if (UnitUtil::isAsteroid(targ)) {
        return configuration()->graphics_config.hud.asteroids_near_message;
    }
    if (targ->isPlanet()) {
        return configuration()->graphics_config.hud.planet_near_message;
    }
    if (targ->getRelation(me) < 0) {
        return configuration()->graphics_config.hud.enemy_near_message;
    }
    return configuration()->graphics_config.hud.starship_near_message;
}

///////////////////////////////////////////////

JumpCapable::JumpCapable() : activeStarSystem(nullptr) {
};

void JumpCapable::ActivateJumpDrive(int destination) {
    Unit *unit = static_cast<Unit *>(this);
    if (((unit->docked & (unit->DOCKED | unit->DOCKED_INSIDE)) == 0) && unit->jump_drive.Installed()) {
        unit->jump_drive.SetDestination(destination);
    }
}

void JumpCapable::AddDestination(const std::string &dest) {
    Unit *unit = static_cast<Unit *>(this);
    unit->pImage->destination.push_back(dest);
}

bool JumpCapable::AutoPilotTo(Unit *un, bool automaticenergyrealloc) {
    std::string tmp;
    return AutoPilotToErrorMessage(un, automaticenergyrealloc, tmp);
}

bool JumpCapable::AutoPilotToErrorMessage(const Unit *target,
        bool ignore_energy_requirements,
        std::string &failuremessage,
        int recursive_level) {
    Unit *unit = static_cast<Unit *>(this);
    const Unit *const_unit = static_cast<const Unit *>(this);

    static bool auto_valid =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "insystem_jump_or_timeless_auto-pilot", "false"));
    if (!auto_valid) {
        static std::string err = "No Insystem Jump";
        failuremessage = err;
        return false;
    }
    if (target->isUnit() == Vega_UnitType::planet) {
        const Unit *targ = *(target->viewSubUnits());
        if (targ && 0 == targ->graphicOptions.FaceCamera) {
            return AutoPilotToErrorMessage(targ, ignore_energy_requirements, failuremessage, recursive_level);
        }
    }
    if (!unit->ftl_drive.CanConsume()) {
        if (!ignore_energy_requirements) {
            return false;
        }
    }
    signed char Guaranteed = ComputeAutoGuarantee(unit);
    if (Guaranteed == Mission::AUTO_OFF) {
        return false;
    }
    static float autopilot_term_distance =
            XMLSupport::parse_float(vs_config->getVariable("physics", "auto_pilot_termination_distance", "6000"));
    static float atd_no_enemies =
            XMLSupport::parse_float(vs_config->getVariable("physics", "auto_pilot_termination_distance_no_enemies",
                    vs_config->getVariable("physics",
                            "auto_pilot_termination_distance",
                            "6000")));
    static float autopilot_no_enemies_multiplier =
            XMLSupport::parse_float(vs_config->getVariable("physics",
                    "auto_pilot_no_enemies_distance_multiplier",
                    "4"));
    if (unit->isSubUnit()) {
        static std::string err = "Return To Cockpit for Auto";
        failuremessage = err;
        return false;                            //we can't auto here;
    }
    StarSystem *ss = activeStarSystem;
    if (ss == NULL) {
        ss = _Universe->activeStarSystem();
    }
    Unit *un = NULL;
    QVector start(unit->Position());
    QVector end(RealPosition(target));
    float totallength = (start - end).Magnitude();
    bool nanspace = false;
    if (!FINITE(totallength)) {
        nanspace = true;
        start = QVector(100000000.0, 100000000.0, 10000000000000.0);
        totallength = (start - end).Magnitude();
        if (!FINITE(totallength)) {
            end = QVector(200000000.0, 100000000.0, 10000000000000.0);
            totallength = (start - end).Magnitude();
        }
    }
    QVector endne(end);

    float totpercent = 1;
    if (totallength > 1) {
        float apt =
                (target->isUnit() == Vega_UnitType::planet) ? (autopilot_term_distance + target->rSize()
                        * UniverseUtil::getPlanetRadiusPercent()) : autopilot_term_distance;
        float aptne =
                (target->isUnit() == Vega_UnitType::planet) ? (atd_no_enemies + target->rSize()
                        * UniverseUtil::getPlanetRadiusPercent()) : atd_no_enemies;
        float percent = (getAutoRSize(unit, unit) + unit->rSize() + target->rSize() + apt) / totallength;
        float percentne = (getAutoRSize(unit, unit) + unit->rSize() + target->rSize() + aptne) / totallength;
        if (percentne > 1) {
            endne = start;
        } else {
            endne = start * percentne + end * (1 - percentne);
        }
        if (percent > 1) {
            end = start;
            totpercent = 0;
        } else {
            totpercent *= (1 - percent);
            end = start * percent + end * (1 - percent);
        }
    }
    bool ok = true;

    static bool teleport_autopilot =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "teleport_autopilot", "true"));
    bool unsafe = false;
    if ((!teleport_autopilot) && (!nanspace)) {
        if (Guaranteed == Mission::AUTO_NORMAL && unit->cloak.Cloaked()) {
            bool ignore_friendlies = true;
            for (un_iter i = ss->getUnitList().createIterator(); (un = *i) != NULL; ++i) {
                static bool canflythruplanets =
                        XMLSupport::parse_bool(vs_config->getVariable("physics", "can_auto_through_planets", "true"));
                if ((!(un->isUnit() == Vega_UnitType::planet
                        && canflythruplanets)) && un->isUnit() != Vega_UnitType::nebula && (!UnitUtil::isSun(un))) {
                    if (un != this && un != target) {
                        float tdis = (start - un->Position()).Magnitude() - unit->rSize() - un->rSize();
                        float nedis = (end - un->Position()).Magnitude() - unit->rSize() - un->rSize();
                        float trad =
                                getAutoRSize(unit, un, ignore_friendlies) + getAutoRSize(unit, unit, ignore_friendlies);
                        if (tdis <= trad) {
                            failuremessage = GenerateAutoError(unit, un);
                            return false;
                        }
                        if ((nedis < trad * autopilot_no_enemies_multiplier
                                || tdis <= trad * autopilot_no_enemies_multiplier) && un->getRelation(unit) < 0) {
                            unsafe = true;
                            failuremessage = GenerateAutoError(unit, un);
                        }
                        float intersection =
                                globQueryShell(start - un->Position(), end - start, getAutoRSize(unit,
                                        un,
                                        ignore_friendlies)
                                        + un->rSize());
                        if (intersection > 0) {
                            unsafe = true;
                            end = start + (end - start) * intersection;
                            totpercent *= intersection;
                            ok = false;
                            failuremessage = GenerateAutoError(unit, un);
                        }
                    }
                }
            }
        }
    } else if (!nanspace) {
        //just make sure we aren't in an asteroid field
        Unit *un;
        for (un_iter i = ss->getUnitList().createIterator(); (un = *i) != NULL; ++i) {
            if (UnitUtil::isAsteroid(un)) {
                static float minasteroiddistance =
                        XMLSupport::parse_float(vs_config->getVariable("physics", "min_asteroid_distance", "-100"));
                if (UnitUtil::getDistance(unit, un) < minasteroiddistance) {
                    failuremessage = GenerateAutoError(unit, un);
                    return false;                     //no auto in roid field
                }
            }
        }
    }
    bool nowhere = false;
    if (this != target) {
        if ((end - start).MagnitudeSquared()
                < (static_cast<double>(unit->rSize()) * static_cast<double>(unit->rSize()))) {
            failuremessage = configuration()->graphics_config.hud.already_near_message;
            return false;
        }

        if (unsafe == false && totpercent == 0) {
            end = endne;
        }
        QVector sep(UniverseUtil::SafeEntrancePoint(end, unit->rSize()));
        if ((sep - end).MagnitudeSquared()
                > (16.0 * static_cast<double>(unit->rSize()) * static_cast<double>(unit->rSize()))) {
            //DOn't understand why rsize is so bigsep = AutoSafeEntrancePoint (end,(RealPosition(target)-end).Magnitude()-target->rSize(),target);
            sep = AutoSafeEntrancePoint(end, unit->rSize(), target);
        }
        if ((sep - RealPosition(target)).MagnitudeSquared()
                > (RealPosition(unit) - RealPosition(target)).MagnitudeSquared()) {
            sep = RealPosition(unit);
            nowhere = true;
        }
        static bool auto_turn_towards =
                XMLSupport::parse_bool(vs_config->getVariable("physics", "auto_turn_towards", "true"));
        if (auto_turn_towards) {
            for (int i = 0; i < 3; ++i) {
                Vector methem(RealPosition(target).Cast() - sep.Cast());
                methem.Normalize();
                Vector p, q, r;
                unit->GetOrientation(p, q, r);
                p = methem.Cross(r);
                float theta = p.Magnitude();
                if (theta * theta > .00001) {
                    p *= (asin(theta) / theta);
                    unit->Rotate(p);
                    unit->GetOrientation(p, q, r);
                }
                if (r.Dot(methem) < 0) {
                    unit->Rotate(p * (PI / theta));
                }
                unit->Velocity = methem * unit->Velocity.Magnitude();
            }
        }
        static string insys_jump_ani = vs_config->getVariable("graphics", "insys_jump_animation", "warp.ani");
        if (insys_jump_ani.length()) {
            static bool docache = true;
            if (docache) {
                UniverseUtil::cacheAnimation(insys_jump_ani);
                docache = false;
            }
            static float insys_jump_ani_size =
                    XMLSupport::parse_float(vs_config->getVariable("graphics", "insys_jump_animation_size", "4"));
            static float insys_jump_ani_growth =
                    XMLSupport::parse_float(vs_config->getVariable("graphics", "insys_jump_animation_growth", ".99"));
            UniverseUtil::playAnimationGrow(insys_jump_ani, RealPosition(unit),
                    unit->rSize() * insys_jump_ani_size, insys_jump_ani_growth);

            Vector v(unit->GetVelocity());
            v.Normalize();
            Vector p, q, r;
            unit->GetOrientation(p, q, r);
            static float sec =
                    XMLSupport::parse_float(vs_config->getVariable("graphics", "insys_jump_ani_second_ahead", "4"));
            UniverseUtil::playAnimationGrow(insys_jump_ani,
                    sep + unit->GetVelocity() * sec + v * unit->rSize(),
                    unit->rSize() * 8,
                    .97);
            UniverseUtil::playAnimationGrow(insys_jump_ani,
                    sep + unit->GetVelocity() * sec + 2 * v * unit->rSize()
                            + r * 4 * unit->rSize(),
                    unit->rSize() * 16,
                    .97);
        }
        static bool warptrail = XMLSupport::parse_bool(vs_config->getVariable("graphics", "warp_trail", "true"));
        if (warptrail && (!nowhere)) {
            static float warptrailtime =
                    XMLSupport::parse_float(vs_config->getVariable("graphics", "warp_trail_time", "20"));
            AddWarp(unit, RealPosition(unit), warptrailtime);
        }
        if (!nowhere) {
            unit->SetCurPosition(sep);
        }
        Cockpit *cp;
        if ((cp = _Universe->isPlayerStarship(const_unit)) != NULL) {
            std::string followermessage;
            if (unit->getFlightgroup() != NULL) {
                Unit *other = NULL;
                if (recursive_level > 0) {
                    for (un_iter ui = ss->getUnitList().createIterator(); NULL != (other = *ui); ++ui) {
                        Flightgroup *ff = other->getFlightgroup();
                        bool leadah = (ff == unit->getFlightgroup());
                        if (ff) {
                            if (ff->leader.GetUnit() == this) {
                                leadah = true;
                            }
                        }
                        Order *otherord = other->getAIState();
                        if (otherord) {
                            if (otherord->PursueTarget(unit, leadah)) {
                                other->AutoPilotToErrorMessage(unit,
                                        ignore_energy_requirements,
                                        followermessage,
                                        recursive_level - 1);
                                if (leadah) {
                                    if (NULL == _Universe->isPlayerStarship(other)) {
                                        other->SetPosition(AutoSafeEntrancePoint(unit->LocalPosition(),
                                                other->rSize() * 1.5,
                                                other));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ok;
}

float JumpCapable::CalculateNearestWarpUnit(float minmultiplier,
        Unit **nearest_unit,
        bool count_negative_warp_units) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    static float smallwarphack = XMLSupport::parse_float(vs_config->getVariable("physics", "minwarpeffectsize", "100"));
    static float bigwarphack =
            XMLSupport::parse_float(vs_config->getVariable("physics", "maxwarpeffectsize", "10000000"));
    //Boundary between multiplier regions 1&2. 2 is "high" mult
    static double warpregion1 = XMLSupport::parse_float(vs_config->getVariable("physics", "warpregion1", "5000000"));
    //Boundary between multiplier regions 0&1 0 is mult=1
    static double warpregion0 = XMLSupport::parse_float(vs_config->getVariable("physics", "warpregion0", "5000"));
    //Mult at 1-2 boundary
    static double warpcruisemult = XMLSupport::parse_float(vs_config->getVariable("physics", "warpcruisemult", "5000"));
    //degree of curve
    static double curvedegree = XMLSupport::parse_float(vs_config->getVariable("physics", "warpcurvedegree", "1.5"));
    //coefficient so as to agree with above
    static double upcurvek = warpcruisemult / pow((warpregion1 - warpregion0), curvedegree);
    //inverse fractional effect of ship vs real big object
    static float def_inv_interdiction = 1.
            / XMLSupport::parse_float(vs_config->getVariable("physics", "default_interdiction",
                    ".125"));
    Unit *planet;
    Unit *testthis = NULL;
    {
        NearestUnitLocator locatespec;
        findObjects(_Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY],
                unit->location[Unit::UNIT_ONLY],
                &locatespec);
        testthis = locatespec.retval.unit;
    }
    for (un_fiter iter = _Universe->activeStarSystem()->gravitationalUnits().fastIterator();
            (planet = *iter) || testthis;
            ++iter) {
        if (!planet || !planet->Killed()) {
            if (planet == NULL) {
                planet = testthis;
                testthis = NULL;
            }
            if (planet == this) {
                continue;
            }
            float shiphack = 1;
            if (planet->isUnit() != Vega_UnitType::planet) {
                shiphack = def_inv_interdiction;
                double spec_interdiction = planet->ship_functions.Value(Function::ftl_interdiction);
                if (spec_interdiction != 0 && planet->graphicOptions.specInterdictionOnline != 0
                        && (spec_interdiction > 0 || count_negative_warp_units)) {
                    shiphack = 1 / fabs(spec_interdiction);
                    if (unit->ship_functions.Value(Function::ftl_interdiction) != 0 && unit->graphicOptions.specInterdictionOnline != 0) {
                        //only counters artificial interdiction ... or maybe it cheap ones shouldn't counter expensive ones!? or
                        // expensive ones should counter planets...this is safe now, for gameplay
                        shiphack *= fabs(spec_interdiction);
                    }
                }
            }
            float multipliertemp = 1;
            float minsizeeffect = (planet->rSize() > smallwarphack) ? planet->rSize() : smallwarphack;
            float effectiverad = minsizeeffect * (1.0f + UniverseUtil::getPlanetRadiusPercent()) + unit->rSize();
            if (effectiverad > bigwarphack) {
                effectiverad = bigwarphack;
            }
            QVector dir = unit->Position() - planet->Position();
            double udist = dir.Magnitude();
            float sigdist = UnitUtil::getSignificantDistance(unit, planet);
            if (planet->isPlanet() && udist < (1 << 28)) {
                //If distance is viable as a float approximation and it's an actual celestial body
                udist = sigdist;
            }
            do {
                double dist = udist;
                if (dist < 0) {
                    dist = 0;
                }
                dist *= shiphack;
                if (dist > (effectiverad + warpregion0)) {
                    multipliertemp = pow((dist - effectiverad - warpregion0), curvedegree) * upcurvek;
                } else {
                    multipliertemp = 1;
                }
                if (multipliertemp < minmultiplier) {
                    minmultiplier = multipliertemp;
                    *nearest_unit = planet;
                    //eventually use new multiplier to compute
                } else {
                    break;
                }
            } while (0);
            if (!testthis) {
                break;
            } //don't want the ++
        }
    }
    return minmultiplier;
}

float JumpCapable::CourseDeviation(const Vector &OriginalCourse, const Vector &FinalCourse) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    if (unit->MaxAfterburnerSpeed() > .001) {
        return (OriginalCourse - (FinalCourse)).Magnitude() / unit->MaxAfterburnerSpeed();
    } else {
        return (FinalCourse - OriginalCourse).Magnitude();
    }
}

void JumpCapable::DeactivateJumpDrive() {
    Unit *unit = static_cast<Unit *>(this);
    unit->jump_drive.UnsetDestination();
}

const std::vector<std::string> &JumpCapable::GetDestinations() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->pImage->destination;
}

StarSystem *JumpCapable::getStarSystem() {
    Unit *unit = static_cast<Unit *>(this);
    if (activeStarSystem) {
        return activeStarSystem;
    } else {
        Cockpit *cp = _Universe->isPlayerStarship(unit);
        if (cp) {
            if (cp->activeStarSystem) {
                return cp->activeStarSystem;
            }
        }
    }
    return _Universe->activeStarSystem();
}

const StarSystem *JumpCapable::getStarSystem() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    if (activeStarSystem) {
        return activeStarSystem;
    } else {
        Cockpit *cp = _Universe->isPlayerStarship(unit);
        if (cp) {
            if (cp->activeStarSystem) {
                return cp->activeStarSystem;
            }
        }
    }
    return _Universe->activeStarSystem();
}

Vector JumpCapable::GetWarpRefVelocity() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    //Velocity
    Vector VelocityRef(0, 0, 0);
    {
        const Unit *vr = unit->computer.velocity_ref.GetConstUnit();
        if (vr) {
            VelocityRef = vr->cumulative_velocity;
        }
    }
    Vector v = unit->Velocity - VelocityRef;
    float len = v.Magnitude();
    if (len > .01) {
        //only get velocity going in DIRECTIOn of cumulative transformation for warp calc...
        v = v * (unit->cumulative_transformation_matrix.getR().Dot(v * (1. / len)));
    }
    return v;
}

Vector JumpCapable::GetWarpVelocity() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    if (unit->graphicOptions.WarpFieldStrength == 1.0) {
        // Short circuit, most ships won't be at warp, so it simplifies math a lot
        return unit->cumulative_velocity;
    } else {
        Vector VelocityRef(0, 0, 0);
        {
            Unit *vr = const_cast< UnitContainer * > (&unit->computer.velocity_ref)->GetUnit();
            if (vr) {
                VelocityRef = vr->cumulative_velocity;
            }
        }

        //return(cumulative_velocity*graphicOptions.WarpFieldStrength);
        Vector vel = unit->cumulative_velocity - VelocityRef;
        float speed = vel.Magnitude();
        //return vel*graphicOptions.WarpFieldStrength;
        if (speed > 0) {
            Vector veldir = vel * (1. / speed);
            Vector facing = unit->cumulative_transformation_matrix.getR();
            float ang = facing.Dot(veldir);
            float warpfield = unit->graphicOptions.WarpFieldStrength;
            if (ang < 0) {
                warpfield = 1. / warpfield;
            }
            return facing * (ang * speed * (warpfield - 1.)) + vel + VelocityRef;
        } else {
            return VelocityRef;
        }
    }
}

bool JumpCapable::InCorrectStarSystem(StarSystem *active) {
    return active == activeStarSystem;
}

bool JumpCapable::TransferUnitToSystem(StarSystem *Current) {
    Unit *unit = static_cast<Unit *>(this);
    if (getStarSystem()->RemoveUnit(unit)) {
        unit->RemoveFromSystem();
        unit->Target(NULL);
        Current->AddUnit(unit);

        Cockpit *an_active_cockpit = _Universe->isPlayerStarship(unit);
        if (an_active_cockpit != NULL) {
            an_active_cockpit->activeStarSystem = Current;
            an_active_cockpit->visitSystem(Current->getFileName());
        }
        activeStarSystem = Current;
        return true;
    } else {
        VS_LOG_AND_FLUSH(fatal, "Fatal Error: cannot remove starship from critical system");
        VSExit(-4);
    }
    return false;
}

bool JumpCapable::TransferUnitToSystem(unsigned int whichJumpQueue,
        class StarSystem *&previouslyActiveStarSystem,
        bool DoSightAndSound) {
    return false;
}

