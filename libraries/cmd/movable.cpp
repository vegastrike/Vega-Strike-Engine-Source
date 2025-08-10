/*
 * movable.cpp
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


#include "cmd/movable.h"
#include "gfx_generic/vec.h"
#include "cmd/unit_generic.h"
#include "src/universe_util.h"
#include "cmd/unit_find.h"
#include "src/star_system.h"
#include "src/universe.h"
#include "configuration/game_config.h"
#include "src/vs_logging.h"
#include "src/audiolib.h"
#include "cmd/audible.h"
#include "root_generic/configxml.h"


#include <string>
#include "src/vega_cast_utils.h"
#include <climits>
#include <utility>

#include "resource/random_utils.h"

float accelStarHandler(float &input) {
    return input / (configuration()->physics.game_speed_flt * configuration()->physics.game_accel_flt);
}

float speedStarHandler(float &input) {
    return input / configuration()->physics.game_speed_flt;
}

Movable::Movable() : sim_atom_multiplier(1),
        predicted_priority(1),
        last_processed_sqs(0),
        docked(NOT_DOCKED),
        cumulative_transformation_matrix(identity_matrix),
        corner_min(Vector(FLT_MAX, FLT_MAX, FLT_MAX)),
        corner_max(Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX)),
        radial_size(0),
        Momentofinertia(0.01) {
    cur_sim_queue_slot = randomInt(SIM_QUEUE_SIZE);
    const Vector default_angular_velocity(configuration()->general.pitch_flt,
            configuration()->general.yaw_flt,
            configuration()->general.roll_flt);
    cutsqr = configuration()->warp.warp_stretch_cutoff_flt * configuration()->warp.warp_stretch_cutoff_flt;
    outcutsqr = configuration()->warp.warp_stretch_decel_cutoff_flt * configuration()->warp.warp_stretch_decel_cutoff_flt;

    Identity(cumulative_transformation_matrix);
    cumulative_transformation = identity_transformation;
    curr_physical_state = prev_physical_state = identity_transformation;
    resolveforces = true;
    AngularVelocity = default_angular_velocity;
}

Movable::graphic_options::graphic_options() {
    FaceCamera = Animating = missilelock = unused1 = WarpRamping = NoDamageParticles = 0;
    specInterdictionOnline = 1;
    NumAnimationPoints = 0;
    RampCounter = 0;
    MinWarpMultiplier = MaxWarpMultiplier = 1;

    // Added implementation to make var false
    // I don't like it, because it's true by default and false by default
    // TODO: figure out which it actually is
    RecurseIntoSubUnitsOnCollision = false;
    SubUnit = 0; // Also this
}

void Movable::SetPosition(const QVector &pos) {
    prev_physical_state.position = curr_physical_state.position = pos;
}

void Movable::GetOrientation(Vector &p, Vector &q, Vector &r) const {
    Matrix m;
    curr_physical_state.to_matrix(m);
    p = m.getP();
    q = m.getQ();
    r = m.getR();
}

Vector Movable::GetNetAcceleration() const {
    Vector p, q, r;
    GetOrientation(p, q, r);
    Vector res(NetLocalForce.i * p + NetLocalForce.j * q + NetLocalForce.k * r);
    res += NetForce;
    return res / Mass;
}

Vector Movable::GetNetAngularAcceleration() const {
    Vector p, q, r;
    GetOrientation(p, q, r);
    Vector res(NetLocalTorque.i * p + NetLocalTorque.j * q + NetLocalTorque.k * r);
    res += NetTorque;
    return res / GetMoment();
}

float Movable::GetMaxAccelerationInDirectionOf(const Vector &ref, bool afterburn) const {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    Vector p, q, r;
    GetOrientation(p, q, r);
    const Vector lref(ref * p, ref * q, ref * r);
    const float tp = (lref.i == 0) ? 0 : fabs(unit->drive.lateral.Value() / lref.i);
    const float tq = (lref.j == 0) ? 0 : fabs(unit->drive.vertical.Value() / lref.j);
    const float tr = (lref.k == 0) ? 0 : fabs(((lref.k > 0) ? unit->drive.forward.Value() : unit->drive.retro.Value()) / lref.k);
    const float trqmin = (tr < tq) ? tr : tq;
    const float tm = tp < trqmin ? tp : trqmin;
    return lref.Magnitude() * tm / Mass;
}

void Movable::SetVelocity(const Vector &v) {
    Velocity = v;
}

void Movable::SetAngularVelocity(const Vector &v) {
    AngularVelocity = v;
}

//FIXME Daughter units should be able to be turrets (have y/p/r)
void Movable::SetResolveForces(bool ys) {
    resolveforces = ys;
}

void Movable::UpdatePhysics(const Transformation& trans,
                            const Matrix& transmat,
                            const Vector& cum_vel,
                            bool lastframe,
                            UnitCollection* uc,
                            Unit* superunit) {
    //Save information about when this happened
    const unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
    //Well, wasn't skipped actually, but...
    this->last_processed_sqs = cur_sim_frame;
    this->cur_sim_queue_slot = (cur_sim_frame + this->sim_atom_multiplier) % SIM_QUEUE_SIZE;
    const Transformation old_physical_state = curr_physical_state;

#if defined(LOG_TIME_TAKEN_DETAILS)
    const double b4_update_physics_3 = realTime();
#endif
    UpdatePhysics3(trans, transmat, lastframe, uc, superunit);
#if defined(LOG_TIME_TAKEN_DETAILS)
    const double after_update_physics_3 = realTime();
    VS_LOG(trace,
           (boost::format("%1%: Time taken by UpdatePhysics3: %2%") % __FUNCTION__ % (after_update_physics_3 -
               b4_update_physics_3)));
#endif

    if (resolveforces) {
        //clamp velocity
        // TODO: use resource class to do this more elegantly
        ResolveForces(trans, transmat);
        const float velocity_max = configuration()->physics.velocity_max_flt;
        if (Velocity.i > velocity_max) {
            Velocity.i = velocity_max;
        }
        else if (Velocity.i < -velocity_max) {
            Velocity.i = -velocity_max;
        }
        if (Velocity.j > velocity_max) {
            Velocity.j = velocity_max;
        }
        else if (Velocity.j < -velocity_max) {
            Velocity.j = -velocity_max;
        }
        if (Velocity.k > velocity_max) {
            Velocity.k = velocity_max;
        }
        else if (Velocity.k < -velocity_max) {
            Velocity.k = -velocity_max;
        }
    }

#if defined(LOG_TIME_TAKEN_DETAILS)
    const double after_resolve_forces = realTime();
    VS_LOG(trace,
           (boost::format("%1%: Time taken by ResolveForces: %2%") % __FUNCTION__ % (after_resolve_forces -
               after_update_physics_3)));
#endif

    // The 1.0 difficulty is a hack based on the hack in GetVelocityDifficultyMult
    this->UpdatePhysics2(trans, old_physical_state, Vector(), 1.0, transmat, cum_vel, lastframe, uc);

#if defined(LOG_TIME_TAKEN_DETAILS)
    const double after_update_physics_2 = realTime();
    VS_LOG(trace,
           (boost::format("%1%: Time taken by UpdatePhysics2: %2%") % __FUNCTION__ % (after_update_physics_2 -
               after_resolve_forces)));
#endif
}

void Movable::AddVelocity(float difficulty) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);
    float lastWarpField = graphicOptions.WarpFieldStrength;

    bool playa = isPlayerShip();

    float warprampuptime = playa ? configuration()->warp.warp_ramp_up_time_flt : configuration()->warp.computer_warp_ramp_up_time_flt;
    //Warp Turning on/off
    if (graphicOptions.WarpRamping) {
        float oldrampcounter = graphicOptions.RampCounter;
        if (unit->ftl_drive.Enabled()) {             //Warp Turning on
            graphicOptions.RampCounter = warprampuptime;
        } else {                                        //Warp Turning off
            graphicOptions.RampCounter = configuration()->warp.warp_ramp_down_time_flt;
        }
        //switched mid - ramp time; we also know old mode's ramptime != 0, or there won't be ramping
        if (oldrampcounter != 0 && graphicOptions.RampCounter != 0) {
            if (unit->ftl_drive.Enabled()) {             //Warp is turning on before it turned off
                graphicOptions.RampCounter *= (1 - oldrampcounter / configuration()->warp.warp_ramp_down_time_flt);
            } else {                                        //Warp is turning off before it turned on
                graphicOptions.RampCounter *= (1 - oldrampcounter / warprampuptime);
            }
        }
        graphicOptions.WarpRamping = 0;
    }
    if (unit->ftl_drive.Enabled() || graphicOptions.RampCounter != 0) {
        float rampmult = 1.f;
        if (graphicOptions.RampCounter != 0) {
            graphicOptions.RampCounter -= simulation_atom_var;
            if (graphicOptions.RampCounter <= 0) {
                graphicOptions.RampCounter = 0;
            }
            if (!unit->ftl_drive.Enabled() && graphicOptions.RampCounter > configuration()->warp.warp_ramp_down_time_flt) {
                graphicOptions.RampCounter = (1 - graphicOptions.RampCounter / warprampuptime) * configuration()->warp.warp_ramp_down_time_flt;
            }
            if (unit->ftl_drive.Enabled() && graphicOptions.RampCounter > warprampuptime) {
                graphicOptions.RampCounter = warprampuptime;
            }
            rampmult = (unit->ftl_drive.Enabled()) ? 1.0F
                    - ((graphicOptions.RampCounter
                            / warprampuptime)
                            * (graphicOptions.RampCounter
                                    / warprampuptime)) : (graphicOptions.RampCounter
                    / configuration()->warp.warp_ramp_down_time_flt) * (graphicOptions.RampCounter / configuration()->warp.warp_ramp_down_time_flt);
        }
        graphicOptions.WarpFieldStrength = GetMaxWarpFieldStrength(rampmult);
    } else {
        graphicOptions.WarpFieldStrength = 1;
    }
    //not anymore? lastWarpField=1;
    Vector v;
    if (graphicOptions.WarpFieldStrength != 1.0) {
        v = unit->GetWarpVelocity();
    } else {
        v = Velocity;
    }

    graphicOptions.WarpFieldStrength =
            lastWarpField * configuration()->warp.warp_memory_effect_flt + (1.0 - configuration()->warp.warp_memory_effect_flt) * graphicOptions.WarpFieldStrength;
    curr_physical_state.position = curr_physical_state.position + (v * simulation_atom_var * difficulty).Cast();
    //now we do this later in update physics
    //I guess you have to, to be robust
}

void Movable::UpdatePhysics2(const Transformation &trans,
        const Transformation &old_physical_state,
        const Vector &accel,
        float difficulty,
        const Matrix &transmat,
        const Vector &cum_vel,
        bool lastframe,
        UnitCollection *uc) {
    //Only in non-networking OR networking && is a player OR SERVER && not a player
    if (AngularVelocity.i || AngularVelocity.j || AngularVelocity.k) {
        Rotate(simulation_atom_var * (AngularVelocity));
    }

}

void Movable::Rotate(const Vector &axis) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);
    double theta = axis.Magnitude();
    double ootheta = 0;
    if (theta == 0) {
        return;
    }
    ootheta = 1 / theta;
    float s = cos(theta * .5);
    Quaternion rot = Quaternion(s, axis * (sinf(theta * .5) * ootheta));
    if (theta < 0.0001) {
        rot = identity_quaternion;
    }
    curr_physical_state.orientation *= rot;
    if (unit->limit_min > -1) {
        Matrix mat;
        curr_physical_state.orientation.to_matrix(mat);
        if (unit->structure_limits.Dot(mat.getR()) < unit->limit_min) {
            curr_physical_state.orientation = prev_physical_state.orientation;
        }
    }
}

Vector Movable::ResolveForces(const Transformation &trans, const Matrix &transmat) {
    //First, save theoretical instantaneous acceleration (not time-quantized) for GetAcceleration()
    SavedAccel = GetNetAcceleration();
    SavedAngAccel = GetNetAngularAcceleration();

    Vector p, q, r;
    GetOrientation(p, q, r);
    Vector temp1(NetLocalTorque.i * p + NetLocalTorque.j * q + NetLocalTorque.k * r);
    if (NetTorque.i || NetTorque.j || NetTorque.k) {
        temp1 += InvTransformNormal(transmat, NetTorque);
    }
    if (GetMoment()) {
        temp1 = temp1 / GetMoment();
    }

    // TODO: restore this with the unit name
    //    else
    //        VSFileSystem::vs_fprintf( stderr, "zero moment of inertia %s\n", name.get().c_str() );
    Vector temp(temp1 * simulation_atom_var);
    AngularVelocity += temp;

    float caprate;
    if (isPlayerShip()) {         //clamp to avoid vomit-comet effects
        caprate = configuration()->physics.max_player_rotation_rate_flt;
    } else {
        caprate = configuration()->physics.max_non_player_rotation_rate_flt;
    }
    if (AngularVelocity.MagnitudeSquared() > caprate * caprate) {
        AngularVelocity = AngularVelocity.Normalize() * caprate;
    }
    //acceleration
    Vector temp2 = (NetLocalForce.i * p + NetLocalForce.j * q + NetLocalForce.k * r);
    if (!(FINITE(NetForce.i) && FINITE(NetForce.j) && FINITE(NetForce.k))) {
        VS_LOG(info, "NetForce skrewed");
    }
    if (NetForce.i || NetForce.j || NetForce.k) {
        temp2 += InvTransformNormal(transmat, NetForce);
    }
    temp2 = temp2 / Mass;
    temp = temp2 * simulation_atom_var;
    if (!(FINITE(temp2.i) && FINITE(temp2.j) && FINITE(temp2.k))) {
        VS_LOG(info, "NetForce transform skrewed");
    }
    float oldmagsquared = Velocity.MagnitudeSquared();
    Velocity += temp;
    //}

    float newmagsquared = Velocity.MagnitudeSquared();

    bool oldbig = oldmagsquared > cutsqr;
    bool newbig = newmagsquared > cutsqr;
    bool oldoutbig = oldmagsquared > outcutsqr;
    bool newoutbig = newmagsquared > outcutsqr;
    if ((newbig && !oldbig) || (oldoutbig && !newoutbig)) {
        static bool docache = true;
        if (docache && !configuration()->graphics.in_system_jump_animation.empty()) {
            UniverseUtil::cacheAnimation(configuration()->graphics.in_system_jump_animation);
            docache = false;
        }
        Vector v(GetVelocity());
        v.Normalize();
        Vector p, q, r;
        GetOrientation(p, q, r);

        float tmpsec = oldbig ? configuration()->warp.warp_stretch_decel_cutoff_flt : configuration()->warp.warp_stretch_cutoff_flt;
        UniverseUtil::playAnimationGrow(configuration()->graphics.in_system_jump_animation,
                realPosition().Cast() + Velocity * tmpsec + v * radial_size,
                radial_size * 8,
                1);
    }

    // stephengtuggy 2020-10-17: These need to be initialized here, because they depend on having an active mission.
    air_res_coef = XMLSupport::parse_floatf(active_missions[0]->getVariable("air_resistance", "0"));
    lateral_air_res_coef = XMLSupport::parse_floatf(active_missions[0]->getVariable("lateral_air_resistance", "0"));

    if (air_res_coef != 0.0F || lateral_air_res_coef != 0.0F) {
        float velmag = Velocity.Magnitude();
        Vector AirResistance = Velocity
                * (air_res_coef * velmag / Mass) * (corner_max.i - corner_min.i) * (corner_max.j - corner_min.j);
        if (AirResistance.Magnitude() > velmag) {
            Velocity.Set(0, 0, 0);
        } else {
            Velocity = Velocity - AirResistance;
            if (lateral_air_res_coef != 0.0F) {
                Vector p, q, r;
                GetOrientation(p, q, r);
                Vector lateralVel = p * Velocity.Dot(p) + q * Velocity.Dot(q);
                AirResistance = lateralVel
                        * (lateral_air_res_coef * velmag
                                / Mass) * (corner_max.i - corner_min.i) * (corner_max.j - corner_min.j);
                if (AirResistance.Magnitude() > lateralVel.Magnitude()) {
                    Velocity = r * Velocity.Dot(r);
                } else {
                    Velocity = Velocity - AirResistance;
                }
            }
        }
    }
    NetForce = NetLocalForce = NetTorque = NetLocalTorque = Vector(0, 0, 0);

    return temp2;
}

void Movable::SetOrientation(QVector q, QVector r) {
    q.Normalize();
    r.Normalize();
    QVector p;
    CrossProduct(q, r, p);
    CrossProduct(r, p, q);
    curr_physical_state.orientation = Quaternion::from_vectors(p.Cast(), q.Cast(), r.Cast());
}

void Movable::SetOrientation(QVector p, QVector q, QVector r) {
    q.Normalize();
    r.Normalize();
    p.Normalize();
    curr_physical_state.orientation = Quaternion::from_vectors(p.Cast(), q.Cast(), r.Cast());
}

void Movable::SetOrientation(Quaternion Q) {
    curr_physical_state.orientation = std::move(Q);
}

#define MM(A, B) m.r[B*3+A]

Vector Movable::UpCoordinateLevel(const Vector &v) const {
    Matrix m;
    curr_physical_state.to_matrix(m);
    return Vector(v.i * MM(0, 0) + v.j * MM(1, 0) + v.k * MM(2, 0),
            v.i * MM(0, 1) + v.j * MM(1, 1) + v.k * MM(2, 1),
            v.i * MM(0, 2) + v.j * MM(1, 2) + v.k * MM(2, 2));
}

#undef MM

Vector Movable::DownCoordinateLevel(const Vector &v) const {
    Matrix m;
    curr_physical_state.to_matrix(m);
    return TransformNormal(m, v);
}

#define MM(A, B) ( (cumulative_transformation_matrix.r[B*3+A]) )

Vector Movable::ToLocalCoordinates(const Vector &v) const {
    //Matrix m;
    //062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);
    return Vector(v.i * MM(0, 0) + v.j * MM(1, 0) + v.k * MM(2, 0),
            v.i * MM(0, 1) + v.j * MM(1, 1) + v.k * MM(2, 1),
            v.i * MM(0, 2) + v.j * MM(1, 2) + v.k * MM(2, 2));
}

#undef MM

Vector Movable::ToWorldCoordinates(const Vector &v) const {
    return TransformNormal(cumulative_transformation_matrix, v);
}

// TODO: move this to JumpCapable
double Movable::GetMaxWarpFieldStrength(float rampmult) const {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);
    Vector v = unit->GetWarpRefVelocity();
//    QVector qv = v.Cast();

    //inverse fractional effect of ship vs real big object
    float minimum_multiplier = configuration()->warp.warp_multiplier_max_flt * graphicOptions.MaxWarpMultiplier;
    Unit *nearest_unit = nullptr;
    minimum_multiplier = unit->CalculateNearestWarpUnit(minimum_multiplier, &nearest_unit, true);
    float minWarp = configuration()->warp.warp_multiplier_min_flt * graphicOptions.MinWarpMultiplier;
    float maxWarp = configuration()->warp.warp_multiplier_max_flt * graphicOptions.MaxWarpMultiplier;
    if (minimum_multiplier < minWarp) {
        minimum_multiplier = minWarp;
    }
    if (minimum_multiplier > maxWarp) {
        minimum_multiplier = maxWarp;
    } //SOFT LIMIT
    minimum_multiplier *= rampmult;
    if (minimum_multiplier < 1) {
        minimum_multiplier = 1;
    }
    v *= minimum_multiplier;
    float vmag = sqrt(v.i * v.i + v.j * v.j + v.k * v.k);
//    static float default_max_warp_effective_velocity = static_cast<float>(M_PI * M_PI * 300000000.0);
//    const float warp_max_effective_velocity = vega_config::GetGameConfig().GetFloat("physics.warpMaxEfVel", default_max_warp_effective_velocity);
    const float warp_max_effective_velocity = configuration()->warp.max_effective_velocity_flt;
    if (vmag > warp_max_effective_velocity) {
        v *= warp_max_effective_velocity / vmag; //HARD LIMIT
        minimum_multiplier *= warp_max_effective_velocity / vmag;
    }
    return minimum_multiplier;
}


void Movable::FireEngines(const Vector &Direction /*unit vector... might default to "r"*/,
        float FuelSpeed,
        float FMass) {
    NetForce += Direction * (static_cast<double>(FuelSpeed) * static_cast<double>(FMass) / GetElapsedTime());
}

//applies a force for the whole gameturn upon the center of mass
void Movable::ApplyForce(const Vector &Vforce) {
    if (FINITE(Vforce.i) && FINITE(Vforce.j) && FINITE(Vforce.k)) {
        NetForce += Vforce;
    } else {
        VS_LOG(error, "fatal force");
    }
}

//applies a force for the whole gameturn upon the center of mass
void Movable::ApplyLocalForce(const Vector &Vforce) {
    if (FINITE(Vforce.i) && FINITE(Vforce.j) && FINITE(Vforce.k)) {
        NetLocalForce += Vforce;
    } else {
        VS_LOG(error, "fatal local force");
    }
}

void Movable::Accelerate(const Vector &Vforce) {
    if (FINITE(Vforce.i) && FINITE(Vforce.j) && FINITE(Vforce.k)) {
        NetForce += Vforce * Mass;
    } else {
        VS_LOG(error, "fatal force");
    }
}

void Movable::ApplyTorque(const Vector &Vforce, const QVector &Location) {
    //Not completely correct
    NetForce += Vforce;
    NetTorque += Vforce.Cross((Location - curr_physical_state.position).Cast());
}

void Movable::ApplyLocalTorque(const Vector &Vforce, const Vector &Location) {
    NetForce += Vforce;
    NetTorque += Vforce.Cross(Location);
}

//usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
void Movable::ApplyBalancedLocalTorque(const Vector &Vforce, const Vector &Location) {
    NetTorque += Vforce.Cross(Location);
}

void Movable::ApplyLocalTorque(const Vector &torque) {
    NetLocalTorque += ClampTorque(torque);
}

Vector Movable::MaxTorque(const Vector &torque) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    //torque is a normal
    return torque * (Vector(copysign(unit->drive.pitch.Value(), torque.i),
            copysign(unit->drive.yaw.Value(), torque.j),
            copysign(unit->drive.roll.Value(), torque.k)) * torque);
}

Vector Movable::ClampTorque(const Vector &amt1) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    Vector Res = amt1;

    float fuelclamp = (unit->fuel.Level() <= 0) ? configuration()->components.fuel.no_fuel_thrust_flt : 1;
    if (fabs(amt1.i) > fuelclamp * unit->drive.pitch) {
        Res.i = copysign(fuelclamp * unit->drive.pitch, amt1.i);
    }
    if (fabs(amt1.j) > fuelclamp * unit->drive.yaw) {
        Res.j = copysign(fuelclamp * unit->drive.yaw, amt1.j);
    }
    if (fabs(amt1.k) > fuelclamp * unit->drive.roll) {
        Res.k = copysign(fuelclamp * unit->drive.roll, amt1.k);
    }
    //1/5,000,000 m/s

    return Res;
}


Vector Movable::ClampVelocity(const Vector &velocity, const bool afterburn) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    double max_speed;
    double magnitude = velocity.Magnitude();

    // If we're using afterburn and have enough energy
    // TODO: Need to make sure somewhere that damage to Afterburner.speed does not
    // reduce it below Drive.speed
    if(afterburn && (unit->afterburner.CanConsume() || configuration()->components.fuel.no_fuel_afterburn_flt)) {
        max_speed = unit->MaxAfterburnerSpeed();
    } else if(unit->drive.CanConsume() ) { //|| configuration()->components.fuel.no_fuel_thrust) {
        max_speed = unit->MaxSpeed();
    } else {
        max_speed = 0;
    }

    if(magnitude > max_speed) {
        return velocity * (max_speed / magnitude);
    }

    return velocity;
}


// TODO: move somewhere (drive?) or do something more elegant.
// Would this be fixed by simply setting the Resource and getting the value back?
// Yes. If we use the drive resources, we don't need this.
Vector Movable::ClampAngVel(const Vector &velocity) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    Vector res(velocity);
    if (res.i >= 0) {
        if (res.i > unit->drive.max_pitch_down.Value()) {
            res.i = unit->drive.max_pitch_down.Value();
        }
    } else if (-res.i > unit->drive.max_pitch_up.Value()) {
        res.i = -unit->drive.max_pitch_up.Value();
    }
    if (res.j >= 0) {
        if (res.j > unit->drive.max_yaw_left.Value()) {
            res.j = unit->drive.max_yaw_left.Value();
        }
    } else if (-res.j > unit->drive.max_yaw_right.Value()) {
        res.j = -unit->drive.max_yaw_right.Value();
    }
    if (res.k >= 0) {
        if (res.k > unit->drive.max_roll_left.Value()) {
            res.k = unit->drive.max_roll_left.Value();
        }
    } else if (-res.k > unit->drive.max_roll_right.Value()) {
        res.k = -unit->drive.max_roll_right.Value();
    }
    return res;
}

Vector Movable::MaxThrust(const Vector &amt1) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    //amt1 is a normal
    return amt1 * (Vector(copysign(unit->drive.lateral.Value(), amt1.i),
            copysign(unit->drive.vertical.Value(), amt1.j),
            amt1.k > 0 ? unit->drive.forward.Value() : -unit->drive.retro.Value()) * amt1);
}

//CMD_FLYBYWIRE depends on new version of Clampthrust... don't change without resolving it
// TODO: refactor soon. Especially access to the fuel variable
Vector Movable::ClampThrust(const Vector &amt1, bool afterburn) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    // TODO: Figure out why this constant isn't used, and either use it or delete it
    const bool finegrainedFuelEfficiency = configuration()->components.fuel.variable_fuel_consumption;


    if(!unit->afterburner.CanConsume()) {
        afterburn = false;
    }


    Vector Res = amt1;

    float fuelclamp = (unit->fuel.Level() <= 0) ? configuration()->components.fuel.no_fuel_thrust_flt : 1;
    float abfuelclamp = (unit->fuel.Level() <= 0) ? configuration()->components.fuel.no_fuel_afterburn_flt : 1;
    if (fabs(amt1.i) > fabs(fuelclamp * unit->drive.lateral)) {
        Res.i = copysign(fuelclamp * unit->drive.lateral, amt1.i);
    }
    if (fabs(amt1.j) > fabs(fuelclamp * unit->drive.vertical)) {
        Res.j = copysign(fuelclamp * unit->drive.vertical, amt1.j);
    }
    float ablimit =
            afterburn
                    ? ((unit->afterburner.thrust - unit->drive.forward.Value()) * abfuelclamp + unit->drive.forward.Value() * fuelclamp)
                    : unit->drive.forward.Value();
    if (amt1.k > ablimit) {
        Res.k = ablimit;
    }
    if (amt1.k < -unit->drive.retro) {
        Res.k = -unit->drive.retro;
    }

    if (afterburn) {
        unit->afterburner.Consume();
    }

    return Res;
}

void Movable::LateralThrust(float amt) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (amt > 1.0) {
        amt = 1.0;
    }
    if (amt < -1.0) {
        amt = -1.0;
    }
    ApplyLocalForce(amt * unit->drive.lateral.Value() * Vector(1, 0, 0));
}

void Movable::VerticalThrust(float amt) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (amt > 1.0) {
        amt = 1.0;
    }
    if (amt < -1.0) {
        amt = -1.0;
    }
    ApplyLocalForce(amt * unit->drive.vertical.Value() * Vector(0, 1, 0));
}

void Movable::LongitudinalThrust(float amt) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (amt > 1.0) {
        amt = 1.0;
    }
    if (amt < -1.0) {
        amt = -1.0;
    }
    ApplyLocalForce(amt * unit->drive.forward.Value() * Vector(0, 0, 1));
}

void Movable::YawTorque(float amt) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (amt > unit->drive.yaw.Value()) {
        amt = unit->drive.yaw.Value();
    } else if (amt < -unit->drive.yaw.Value()) {
        amt = -unit->drive.yaw.Value();
    }
    ApplyLocalTorque(amt * Vector(0, 1, 0));
}

void Movable::PitchTorque(float amt) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (amt > unit->drive.pitch.Value()) {
        amt = unit->drive.pitch.Value();
    } else if (amt < -unit->drive.pitch.Value()) {
        amt = -unit->drive.pitch.Value();
    }
    ApplyLocalTorque(amt * Vector(1, 0, 0));
}

void Movable::RollTorque(float amt) {
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    if (amt > unit->drive.roll.Value()) {
        amt = unit->drive.roll.Value();
    } else if (amt < -unit->drive.roll.Value()) {
        amt = -unit->drive.roll.Value();
    }
    ApplyLocalTorque(amt * Vector(0, 0, 1));
}

void Movable::Thrust(const Vector &amt1, bool afterburn) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    afterburn = afterburn && unit->afterburner.CanConsume();

    //Unit::Thrust( amt1, afterburn );
    {
        Vector amt = ClampThrust(amt1, afterburn);
        ApplyLocalForce(amt);
    }

    static bool must_afterburn_to_buzz =
            XMLSupport::parse_bool(vs_config->getVariable("audio", "buzzing_needs_afterburner", "false"));
    if (_Universe->isPlayerStarship(unit) != nullptr) {
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
            if (UnitUtil::getDistance(unit,
                    playa) < buzzingdistance && playa->owner != this && unit->owner != playa
                    && unit->owner != playa->owner) {
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
                        unit->playEngineSound();
                    } else {
                    }
                }
            }
        }
    }
}

// If in Travel mode (non-combat), speed limit is multiplied by a power of 10.
double Movable::MaxSpeed() const {
    const double combat_mode_multiplier = configuration()->components.drive.non_combat_mode_multiplier; // configuration()->physics.combat_mode_multiplier
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);
    return (unit->computer.combat_mode) ? unit->drive.speed.AdjustedValue() : combat_mode_multiplier * unit->drive.speed.AdjustedValue();
}

// Same as comment above. It makes less sense to limit travel speed with afterburners to afterburner speed multiplied by a power of 10.
double Movable::MaxAfterburnerSpeed() const {
    const double combat_mode_multiplier = configuration()->components.drive.non_combat_mode_multiplier; // configuration()->physics.combat_mode_multiplier
    const Unit *unit = vega_dynamic_const_cast_ptr<const Unit>(this);

    //same capped big speed as combat...else different
    return (unit->computer.combat_mode) ? unit->afterburner.speed.AdjustedValue() : combat_mode_multiplier * unit->drive.speed.AdjustedValue();
}
