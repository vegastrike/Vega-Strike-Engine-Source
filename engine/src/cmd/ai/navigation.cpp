/*
 * navigation.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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


#include "navigation.h"
#include "root_generic/macosx_math.h"
#include <math.h>
#include <vector>
#include "cmd/unit_find.h"
#include "src/universe.h"
#ifndef _WIN32
#include <assert.h>
#endif
#include "cmd/unit_generic.h"
#include "root_generic/lin_time.h"
#include "cmd/script/flightgroup.h"
#include "src/config_xml.h"
#include "root_generic/vs_globals.h"
#include "src/vs_logging.h"
#include "warpto.h"
#include "flybywire.h"
#include "cmd/unit_util.h"
#include "root_generic/vega_random.h"
#include "src/star_system.h"
#include "src/universe.h"

using namespace Orders;

constexpr float M_PI_FLT = M_PI;

// Collects up to a bounded number of units (other than ourselves) in range for the
// SPEC clear-space steering -- used as the action for UnitWithinRangeLocator over
// the UNIT_ONLY map. Capping the count keeps the per-frame cost bounded even with
// hundreds of ships in the vicinity.
struct ClearSpaceCollector {
    std::vector<Unit *> *units;
    std::size_t max_units;
    ClearSpaceCollector() : units(nullptr), max_units(0) {}
    void init(std::vector<Unit *> &u, std::size_t maxn) {
        units = &u;
        max_units = maxn;
    }
    bool acquire(Unit *unit, float /*distance*/) {
        if (units == nullptr || units->size() >= max_units) {
            return false;
        }
        units->push_back(unit);
        return true;
    }
};

/**
 * the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
 * vslowdown - decel * t = 0               t = vslowdown/decel
 * finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = .5 * v^2 / decel + slowdownx
 * slowdownx =  .5 accel * t^2 + v0 * t + initx
 * finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx
 * Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t
 *
 * balanced thrust equation
 * Length = accel * t^2 +  2*t*v0 + .5*v0^2/accel
 * t = ( -2v0 (+/-) sqrtf (4*v0^2 - 4*(.5*v0^2 - accel*Length) ) / (2*accel))
 * t = -v0/accel (+/-) sqrtf (.5*v0^2 + Length*accel)/accel;
 *
 * 8/15/05 Patched Calulate BalancedDecel time: our previous quantization factor ignored the quantization during ACCEL phase and also ignored the fact that we overestimated the integral rather than underestimated
 *         new quantization factor is .5*accel*simulation_atom_var*simulation_atom_var-.5*initialVelocity*simulation_atom_var
 *            also this threshold idea is silly--accelerate if t>SIM_ATOM decel if t<0  still havent fixed t between 0 and SIM_ATOM...have decent approx for now.
 * 3/2/02  Patched CalculateBalancedDecel time with the fact that length should be more by a
 * quantity of .5*initialVelocity*simulation_atom_var
 *
 */

static float CalculateBalancedDecelTime(float l, float v, float &F, float mass) {
    float accel = F / mass;
    if (accel <= 0) {
        return 0;
    }
    if (l < 0) {
        l = -l;
        v = -v;
        F = -F;
    }
    double temp = .5 * v * v
            + (l - v * simulation_atom_var * (.5) + .5 * simulation_atom_var * simulation_atom_var * accel) * accel;
    if (temp < 0) {
        temp = 0;
    }
    return (-v + sqrtf(temp)) / accel;
}

/**
 * the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
 * vslowdown - decel * t = 0               t = vslowdown/decel
 * finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = .5 * v^2 / decel + slowdownx
 * slowdownx =  .5 accel * t^2 + v0 * t + initx
 * finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx
 * Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t
 *
 * imbalanced thrust equation
 * Length = .5*(accel+accel*accel/decel) * t^2 +  t*v0(1+accel/decel) + .5*v0^2/decel
 * t = ( -v0*(1+accel/decel) (+/-) sqrtf (v0^2*(1+accel/decel)^2 - 2*(accel+accel*accel/decel)*(.5*v0^2/decel-Length)))/2*.5*(accel+accel*accel/decel);
 * t = (-v0 (+/-) sqrtf (v0^2 - 2*(accel/(1+accel/decel))*(.5*v0^2/decel-Length)))/accel
 */

static float CalculateDecelTime(float l, float v, float &F, float D, float mass) {
    float accel = F / mass;
    float decel = D / mass;
    if (l < 0) {
        l = -l;
        v = -v;
        accel = decel;
        decel = F / mass;
        F = -D;
    }
    float vsqr = v * v;
    float fourac = 2 * accel * ((.5 * v * v / decel) - v * simulation_atom_var * .5 - l) / (1 + accel / decel);
    if (fourac > vsqr) {
        return FLT_MAX;
    }       //FIXME avoid sqrt negative  not sure if this is right

    return (-v + sqrtf(vsqr - fourac)) / accel;
}

//failed attempt below
/**
 * The Time We need to start slowing down from the now calculation.
 * k = num simulation atoms before we slow down. m = num sim atoms till we stop
 * R = P = 1    We know  v + S*k*a-S*m*d=0  (stopped at end)  m = v/(d*S)+k*a/d
 * L = a*S^2 (k*(k+R)*.5) -  d*S^2 (m*(m+P)*.5) + v*(k+m)*S
 * L = .5*a*k^2*S^2+.5*a*k*R*s^2 - .5*d*S^2 (k*a/d + v/(d*S))*(k*a/d+v/(d*S)+P) + S*v*(v/(d*S) + k*a/d + k)
 * L = k^2 (.5*a*S^2) +.5*a*k*R*S^2 - .5*d*S^2 (k^2 a^2 / d^2 + 2*(k*a/d)*v/(dS) + v^2/(d^2 s^2) + Pk*a/d + P * v/(d*S)) + v^2/d + S*v*k (a/d + 1)
 * 0 = k^2 (.5*a*S*S*(1-a/d)) + k*S*( .5*R*a*S - .5*P*a*S- .5*2*a*v/d+v(1+a/d)) + v*v/d-.5*v*v/d- P*v*S*.5 - L
 * 0 = (k*S)^2 *(.5*a*(1-a/d)) + k*S*v + v^2/(2d) - vS/2 -L
 * kS = -v (+/-) sqrtf (v*v-a*(1-a/d)*(v*v/d-vS-L))/(a*(1-a/d))
 */
//end failed attempt

void MoveTo::SetDest(const QVector &target) {
    targetlocation = target;
    done = false;
}

bool MoveToParent::OptimizeSpeed(Unit *parent, float v, float &a, float max_speed) {
    v += (a / parent->GetMass()) * simulation_atom_var;
    if ((!max_speed) || fabs(v) <= max_speed) {
        return true;
    }
    float deltaa =
            parent->GetMass() * (fabs(v) - max_speed) / simulation_atom_var;       //clamping should take care of it
    a += (v > 0) ? -deltaa : deltaa;
    return false;
}

float MOVETHRESHOLD = simulation_atom_var / 1.9;

bool MoveToParent::Done(const Vector &ang_vel) {
    if (fabs(ang_vel.i) < THRESHOLD
            && fabs(ang_vel.j) < THRESHOLD
            && fabs(ang_vel.k) < THRESHOLD) {      //if velocity is lower than threshold
        return true;
    }
    return false;
}

void MoveTo::Execute() {
    done = done || m.Execute(parent, targetlocation);
}

bool MoveToParent::Execute(Unit *parent, const QVector &targetlocation) {
    bool done = false;
    Vector local_vel(parent->UpCoordinateLevel(parent->GetVelocity()));
    //local location is ued for storing the last velocity;
    terminatingX += ((local_vel.i > 0) != (last_velocity.i > 0) || (!local_vel.i));
    terminatingY += ((local_vel.j > 0) != (last_velocity.j > 0) || (!local_vel.j));
    terminatingZ += ((local_vel.k > 0) != (last_velocity.k > 0) || (!local_vel.k));

    last_velocity = local_vel;
    Vector heading = parent->ToLocalCoordinates((targetlocation - parent->Position()).Cast());
    Vector thrust(parent->drive.lateral, parent->drive.vertical,
            afterburn ? parent->afterburner.thrust : parent->drive.forward);
    float max_speed =
            (afterburn ? parent->MaxAfterburnerSpeed() : parent->MaxSpeed());
    Vector normheading = heading;
    normheading.Normalize();
    Vector max_velocity = max_speed * normheading;
    max_velocity.Set(fabs(max_velocity.i),
            fabs(max_velocity.j),
            fabs(max_velocity.k));
    if (done) {
        return done;
    }       //unreachable

    if (terminatingX > switchbacks
            && terminatingY > switchbacks
            && terminatingZ > switchbacks) {
        if (Done(last_velocity)) {
            if (selfterminating) {
                done = true;
            } else {
                terminatingX = 0;
                terminatingY = 0;
                terminatingZ = 0;
            }
            return done;
        }
        thrust = (-parent->GetMass() / simulation_atom_var) * last_velocity;
    } else {
        float div = 1.0f;
        float vdiv = 1.0f;
        if (selfterminating && terminatingX > 8 && terminatingY > 8 && terminatingZ > 8) {
            int tmp = (terminatingX - 4);
            if (terminatingY < terminatingX) {
                tmp = terminatingY - 4;
            }
            if (terminatingZ < terminatingX && terminatingZ < terminatingY) {
                tmp = terminatingZ - 4;
            }
            tmp /= 4;
            if (tmp > 30) {
                tmp = 30;
            }
            vdiv = (float) (1 << tmp);
            div = vdiv;
            thrust.i /= div;
            thrust.j /= div;
            thrust.k /= div;
        }
        //start with Forward/Reverse:
        float t =
                CalculateDecelTime(heading.k, last_velocity.k, thrust.k, parent->drive.retro / div, parent->GetMass());
        if (t < THRESHOLD) {
            thrust.k =
                    (thrust.k > 0 ? -parent->drive.retro
                            / div : (afterburn ? parent->afterburner.thrust / div : parent->drive.forward / div));
        } else if (t < simulation_atom_var) {
            thrust.k *= t / simulation_atom_var;
            thrust.k +=
                    (simulation_atom_var
                            - t)
                            * (thrust.k > 0 ? -parent->drive.retro
                                    / div : (afterburn ? parent->afterburner.thrust / div : parent->drive.forward / div))
                            / simulation_atom_var;
        }
        OptimizeSpeed(parent, last_velocity.k, thrust.k, max_velocity.k / vdiv);
        t = CalculateBalancedDecelTime(heading.i, last_velocity.i, thrust.i, parent->GetMass());
        if (t < THRESHOLD) {
            thrust.i = -thrust.i;
        } else if (t < simulation_atom_var) {
            thrust.i *= (t - (simulation_atom_var - t)) / simulation_atom_var;
        }
        OptimizeSpeed(parent, last_velocity.i, thrust.i, max_velocity.i / vdiv);
        t = CalculateBalancedDecelTime(heading.j, last_velocity.j, thrust.j, parent->GetMass());
        if (t < THRESHOLD) {
            thrust.j = -thrust.j;
        } else if (t < simulation_atom_var) {
            thrust.j *= (t - (simulation_atom_var - t)) / simulation_atom_var;
        }
        OptimizeSpeed(parent, last_velocity.j, thrust.j, max_velocity.j / vdiv);
    }
    parent->ApplyLocalForce(thrust);

    return done;
}

MoveTo::~MoveTo() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("mt%1$x") % this));
#endif
}

bool ChangeHeading::OptimizeAngSpeed(float optimal_speed_pos, float optimal_speed_neg, float v, float &a) {
    v += (a / parent->GetMoment()) * simulation_atom_var;
    if ((optimal_speed_pos == 0 && optimal_speed_neg == 0) || (v >= -optimal_speed_neg && v <= optimal_speed_pos)) {
        return true;
    }
    if (v > 0) {
        float deltaa = parent->GetMoment() * (v - optimal_speed_pos)
                / simulation_atom_var;           //clamping should take care of it
        a -= deltaa;
    } else {
        float deltaa = parent->GetMoment() * (-v - optimal_speed_neg)
                / simulation_atom_var;           //clamping should take care of it
        a += deltaa;
    }
    return false;
}

/**
 * uses CalculateBalancedDecelTime to figure out which way (left or righT) is best to aim for.
 * works for both pitch and yaw axis if you pass in the -ang_vel.j for the y
 */
void ChangeHeading::TurnToward(float atancalc, float ang_veli, float &torquei) {
    //We need to end up at destination with positive velocity, but no more than we can decelerate from in a single simulation_atom_var
    if (1) {
        float mass = parent->GetMoment();
        float max_arrival_speed = torquei * simulation_atom_var / mass;
        float accel_needed = (atancalc / simulation_atom_var - ang_veli) / simulation_atom_var;
        float arrival_velocity = accel_needed * simulation_atom_var + ang_veli;
        if (fabs(arrival_velocity) <= max_arrival_speed && fabs(accel_needed) < torquei / mass) {
            torquei = accel_needed * mass;
            return;
        }
    }
    float t = CalculateBalancedDecelTime(atancalc,
            ang_veli,
            torquei,
            parent->GetMoment());     //calculate when we should decel
    if (t < 0) {
        //if it can't make it: try the other way
        torquei = fabs(torquei);         //copy sign again
        t = CalculateBalancedDecelTime(atancalc > 0 ? atancalc - 2 * PI : atancalc + 2 * PI,
                ang_veli,
                torquei,
                parent->GetMoment());
    }
    if (t > 0) {
        if (t < simulation_atom_var) {
            torquei *= ((t / simulation_atom_var) - ((simulation_atom_var - t) / simulation_atom_var));
        }
    } else {
        torquei = -parent->GetMoment() * ang_veli / simulation_atom_var;         //clamping should take care of it
    }
}

void ChangeHeading::SetDest(const QVector &target) {
    final_heading = target;
    ResetDone();
}

float TURNTHRESHOLD = simulation_atom_var / 1.9;

///if velocity is lower than threshold
bool ChangeHeading::Done(const Vector &ang_vel) {
    if (fabs(ang_vel.i) < THRESHOLD
            && fabs(ang_vel.j) < THRESHOLD
            && fabs(ang_vel.k) < THRESHOLD) {
        return true;
    }
    return false;
}

void ChangeHeading::Execute() {
    bool temp = done;
    Order::Execute();
    done = temp;
    Vector ang_vel = parent->GetAngularVelocity();
    Vector local_velocity(parent->UpCoordinateLevel(ang_vel));
    Vector local_heading(parent->ToLocalCoordinates((final_heading - parent->Position()).Cast()));
    char xswitch =
            ((local_heading.i > 0) != (last_velocity.i > 0) || (!local_heading.i)) && last_velocity.i != 0 ? 1 : 0;
    char yswitch =
            ((local_heading.j > 0) != (last_velocity.j > 0) || (!local_heading.j)) && last_velocity.j != 0 ? 1 : 0;
    const bool AICheat = configuration().ai.turn_cheat;
    bool cheater = false;
    const float min_for_no_oversteer = configuration().ai.min_angular_accel_cheat_flt;
    if (AICheat && ((parent->drive.yaw + parent->drive.pitch) * 180 / (PI * parent->GetMass()) > min_for_no_oversteer)
            && !parent->isSubUnit()) {
        if (xswitch || yswitch) {
            Vector P, Q, R;
            parent->GetOrientation(P, Q, R);
            Vector desiredR = (final_heading - parent->Position()).Cast();
            desiredR.Normalize();
            const float cheatpercent = configuration().ai.ai_cheat_dot_flt;
            if (desiredR.Dot(R) > cheatpercent) {
                P = Q.Cross(desiredR);
                Q = desiredR.Cross(P);
                parent->SetOrientation(Q, desiredR);
                xswitch = yswitch = 1;
                if (xswitch) {
                    if (yswitch) {
                        local_velocity.j = .0f;
                        local_velocity.i = .0f;
                        ang_vel.i = .0f;
                        ang_vel.j = .0f;
                    } else {
                        local_velocity.i = .0f;
                        ang_vel.i = .0f;
                    }
                } else if (yswitch) {
                    local_velocity.j = .0f;
                    ang_vel.j = .0f;
                }
                cheater = true;
                ang_vel.k = local_velocity.k = 0;
                parent->SetAngularVelocity(ang_vel);
            }
        }
    }
    terminatingX += xswitch;
    terminatingY += yswitch;
    last_velocity = local_velocity;
    if (done /*||(xswitch&&yswitch)*/) {
        return;
    }
    Vector torque(parent->drive.pitch, parent->drive.yaw, 0);     //set torque to max accel in any direction
    if (terminatingX > switchbacks && terminatingY > switchbacks) {
        if (Done(local_velocity)) {
            if (this->terminating) {
                done = true;
            } else {
                terminatingX = 0;
                terminatingY = 0;
            }
            return;
        }
        torque = (-parent->GetMoment() / simulation_atom_var) * local_velocity;
    } else {
        TurnToward(atan2(local_heading.j, local_heading.k),
                local_velocity.i,
                torque.i);         //find angle away from axis 0,0,1 in yz plane
        OptimizeAngSpeed(turningspeed * parent->drive.max_pitch_down,
                turningspeed * parent->drive.max_pitch_up,
                local_velocity.i,
                torque.i);
        TurnToward(atan2(local_heading.i, local_heading.k), -local_velocity.j, torque.j);
        torque.j = -torque.j;
        OptimizeAngSpeed(turningspeed * parent->drive.max_yaw_left,
                turningspeed * parent->drive.max_yaw_right,
                local_velocity.j,
                torque.j);
        torque.k = -parent->GetMoment() * local_velocity.k / simulation_atom_var;         //try to counteract roll;
    }
    if (!cheater) {
        parent->ApplyLocalTorque(torque);
    }
}

ChangeHeading::~ChangeHeading() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("ch%1$x") % this));
#endif
}

FaceTargetITTS::FaceTargetITTS(bool fini, int accuracy) : ChangeHeading(QVector(0, 0, 1), accuracy), finish(fini) {
    type = FACING;
    subtype = STARGET;
    speed = float(.00001);
    useitts = true;
    const bool alwaysuseitts = configuration().ai.always_use_itts;
    if (!alwaysuseitts) {
        if (VegaRandom::Instance().GenRandUInt32() >= g_game.difficulty * static_cast<double>(RAND_MAX)) {
            useitts = false;
        }
    }
}

FaceTargetITTS::~FaceTargetITTS() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("fti%1$x") % this));
#endif
}

void FaceTargetITTS::Execute() {
    Unit *target = parent->Target();
    if (target == NULL) {
        done = finish;
        return;
    }
    if (speed == float(.00001)) {
        float mrange;
        float range;
        parent->getAverageGunSpeed(speed, range, mrange);
        if (speed == float(.00001)) {
            speed = FLT_MAX;
        }
    }
    SetDest(useitts ? target->PositionITTS(parent->Position(), parent->cumulative_velocity, speed, false)
            : target->Position());
    ChangeHeading::Execute();
    if (!finish) {
        ResetDone();
    }
}

FaceTarget::FaceTarget(bool fini, int accuracy) : ChangeHeading(QVector(0, 0, 1), accuracy), finish(fini) {
    type = FACING;
    subtype = STARGET;
}

void FaceTarget::Execute() {
    Unit *target = parent->Target();
    if (target == NULL) {
        done = finish;
        return;
    }
    SetDest(target->isSubUnit() ? target->Position() : target->LocalPosition());
    ChangeHeading::Execute();
    if (!finish) {
        ResetDone();
    }
}

FaceTarget::~FaceTarget() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("ft%1$x") % this));
#endif
}

AutoLongHaul::AutoLongHaul(bool fini, int accuracy) : ChangeHeading(QVector(0, 0, 1), accuracy), finish(fini) {
    type = FACING | MOVEMENT;
    subtype = STARGET;
    deactivatewarp = false;
    StraightToTarget = true;
    inside_landing_zone = false;
}

void AutoLongHaul::MakeLinearVelocityOrder() {
    eraseType(MOVEMENT);
    const float combat_mode_mult = configuration().physics.auto_docking_speed_boost_flt;

    float speed =
            parent->computer.combat_mode ? parent->drive.speed
                    : parent->afterburner.speed /*won't do insanity flight mode + spec = ludicrous speed*/;
    if (inside_landing_zone) {
        speed *= combat_mode_mult;
    }
    MatchLinearVelocity *temp = new MatchLinearVelocity(Vector(0, 0, speed), true, false, false);
    temp->SetParent(parent);
    Order::EnqueueOrder(temp);
}

void AutoLongHaul::SetParent(Unit *parent1) {
    ChangeHeading::SetParent(parent1);
    group.SetUnit(parent1->Target());
    inside_landing_zone = false;
    MakeLinearVelocityOrder();
}

extern bool DistanceWarrantsWarpTo(Unit *parent, float dist, bool following);

static float mymin(float a, float b) {
    return a < b ? a : b;
}

// TODO: move this kludge to FtlDrive
inline void WarpRampOff(Unit *un, bool rampdown) {
    if (un->ftl_drive.Enabled()) {
        un->ftl_drive.Disable();
        if (rampdown) {
            un->graphicOptions.WarpRamping = 1;
        }
    }
}

inline void CautiousWarpRampOn(Unit *un) {
    if ((!un->ftl_drive.Enabled())
            && (un->graphicOptions.RampCounter == 0)) { // don't restart warp during ramp-down - avoid shaking
        un->ftl_drive.Enable();
        un->graphicOptions.WarpRamping = 1;
    }
}

bool useJitteryAutopilot(Unit *parent, Unit *target, float minaccel) {
    const float specInterdictionLimit = configuration().physics.min_spec_interdiction_for_jittery_autopilot_flt;
    if (target->isPlanet() == false
            && (target->graphicOptions.specInterdictionOnline == 0
                    || fabs(target->ship_functions.Value(Function::ftl_interdiction)) < specInterdictionLimit)) {
        return true;
    }
    if (parent->computer.combat_mode == false) {
        return true;
    }
    float maxspeed = parent->afterburner.speed;
    const float accel_auto_limit = configuration().physics.max_accel_for_smooth_autopilot_flt;
    const float speed_auto_limit = configuration().physics.max_over_combat_speed_for_smooth_autopilot_flt;
    if (minaccel < accel_auto_limit || parent->Velocity.MagnitudeSquared() > maxspeed * maxspeed * speed_auto_limit
            * speed_auto_limit) {
        return true;
    }
    return false;
}

bool AutoLongHaul::InsideLandingPort(const Unit *obstacle) const {
    const float landing_port_limit = configuration().physics.auto_landing_port_unclamped_seconds_flt;
    return UnitUtil::getSignificantDistance(parent,
            obstacle)
            < -landing_port_limit * parent->afterburner.speed;
}

void AutoLongHaul::Execute() {
    Unit *target = group.GetUnit();
    if (target == NULL) {
        group.SetUnit(parent->Target());
        done = finish;
        parent->autopilotactive = false;
        return;
    }
    const float max_compression_range = configuration().warp.max_effective_velocity_flt;
    QVector myposition = parent->isSubUnit() ? parent->Position() : parent->LocalPosition();     //get unit pos
    QVector destination = target->isSubUnit() ? target->Position() : target->LocalPosition();     //get destination
    QVector destinationdirection = (destination - myposition);       //find vector from us to destination
    double destinationdistance = destinationdirection.Magnitude();
    destinationdirection =
            destinationdirection * (1. / destinationdistance);       //this is a direction, so it is normalize

    // Distance to stop from the ship's current speed (including SPEC). The autopilot
    // flies straight to this braking point, winds down SPEC there and brakes to a
    // stop. Used both to gate obstacle avoidance (don't dodge objects farther away
    // than we can already stop) and as the clean disengage point.
    const double current_speed = parent->Velocity.Magnitude();
    const double ship_mass = parent->GetMass();
    double brake_distance = 0.0;
    if (ship_mass > 0.0 && parent->drive.retro > 0.0) {
        brake_distance = (current_speed * current_speed)
                / (2.0 * (parent->drive.retro / ship_mass));
    }

    StraightToTarget = true;    // free to fly

    if (parent->graphicOptions.RampCounter == 0) {
        //face target unless warp ramping is done
        Unit *obstacle = NULL;
        // The thing compressing our SPEC bubble is the nearest object in space.
        parent->GetNearestObjectSignificantDistance(&obstacle);
        bool currently_inside_landing_zone = false;
        if (obstacle) {
            currently_inside_landing_zone = InsideLandingPort(obstacle);
        }
        if (currently_inside_landing_zone != inside_landing_zone) {
            inside_landing_zone = currently_inside_landing_zone;
            MakeLinearVelocityOrder();
        }
        // Steer into clear space so SPEC works at full. Vector-sum steering: each
        // object that compresses our SPEC bubble pushes us directly away from it,
        // weighted by how close (and thus how much it interferes) it is. The
        // destination pulls us in, and that pull builds as we clear the objects --
        // once outside the interfering vectors we steer for the destination. A close
        // base dominates; ships around it add up; far objects barely matter.
        const float gather_range = max_compression_range
                * configuration().physics.warp_clearance_range_mult_flt;
        StarSystem *ss = _Universe->activeStarSystem();
        const float repel = configuration().physics.warp_clearance_repel_flt;
        const float attract = configuration().physics.warp_clearance_attract_flt;
        const float attract_gain = configuration().physics.warp_clearance_attract_gain_flt;
        const float falloff = configuration().physics.warp_clearance_falloff_flt;

        // Cull the interfering objects to only the closest handful so a crowd of
        // ships in the vicinity can't dominate or cost too much -- the nearest matter
        // most anyway.
        const unsigned int kMaxShips = 8;
        const unsigned int kMaxObjects = 5;
        std::vector<Unit *> ships;
        if (!is_null(parent->location[Unit::UNIT_ONLY])) {
            UnitWithinRangeLocator<ClearSpaceCollector> locator(gather_range, 0.0f);
            locator.action.init(ships, kMaxShips);
            findObjects(ss->collide_map[Unit::UNIT_ONLY], parent->location[Unit::UNIT_ONLY], &locator);
        }
        std::vector<std::pair<double, Unit *>> ranked;
        // gravitational bodies (few, large -- always considered)
        Unit *u;
        for (un_fiter iter = ss->gravitationalUnits().fastIterator(); (u = *iter); ++iter) {
            if (u != nullptr && !u->Killed() && u != parent) {
                ranked.emplace_back(UnitUtil::getSignificantDistance(parent, u), u);
            }
        }
        for (Unit *o : ships) {
            if (o != nullptr && o != parent) {
                ranked.emplace_back(UnitUtil::getSignificantDistance(parent, o), o);
            }
        }
        std::sort(ranked.begin(), ranked.end(),
                [](const std::pair<double, Unit *> &a, const std::pair<double, Unit *> &b) {
                    return a.first < b.first;
                });
        if (ranked.size() > kMaxObjects) {
            ranked.resize(kMaxObjects);
        }

        QVector sum(0.0f, 0.0f, 0.0f);
        bool any = false;
        // The destination must never be a repulsor -- it is where we want to go and
        // it is fine that it compresses our SPEC bubble. The autopilot target can be
        // a subunit of the station, so match the whole unit (target and its owner).
        Unit *target_root = target;
        if (target != nullptr && target->isSubUnit()) {
            target_root = UnitUtil::owner(target);
        }
        for (const auto &pr : ranked) {
            Unit *o = pr.second;
            if (o == nullptr || o == parent || o == target || o == target_root) {
                continue;
            }
            double sig = pr.first;
            if (sig >= gather_range) {
                continue;
            }
            QVector to_obj = o->LocalPosition() - myposition;
            double dist = to_obj.Magnitude();
            if (dist < 0.0001) {
                continue;
            }
            float weight = repel * (falloff / static_cast<float>(falloff + sig));
            if (weight <= 0.0f) {
                continue;
            }
            sum += (-to_obj / dist) * weight;
            any = true;
        }
        if (any) {
            StraightToTarget = false;
            // The destination pull has a long reach (it is where we want to go) and
            // grows almost logarithmically as we near it, so it overrides the
            // repulsion of ships clustered at the target instead of the autopilot
            // avoiding them. Far from the target it stays near the base attract;
            // very close it spikes.
            const double attract_strength = attract
                    + std::log1p(attract_gain / (destinationdistance + 1.0));
            // Repulsors are almost the inverse of the attractor: strong while the
            // destination is far away, then fading continuously as we get closer, so
            // they cause less and less of a detour until the destination is in range
            // -- just enough left to avoid a crash. The strength tracks the inverse
            // of the destination pull, floored at warp_clearance_arrive_damp.
            const float arrive_damp = configuration().physics.warp_clearance_arrive_damp_flt;
            double repulsion_damp = attract / attract_strength;
            if (repulsion_damp < arrive_damp) {
                repulsion_damp = arrive_damp;
            }
            if (repulsion_damp > 1.0) {
                repulsion_damp = 1.0;
            }
            sum *= static_cast<float>(repulsion_damp);
            sum += destinationdirection * static_cast<float>(attract_strength);
            QVector desired;
            double mag = sum.Magnitude();
            if (mag > 0.0001) {
                desired = sum / mag;
            } else {
                desired = destinationdirection;
            }
            double clear_distance = mymin(destinationdistance, gather_range);
            destination = myposition + desired * clear_distance;
        }
    }
    if (!parent->ftl_drive.Enabled() && parent->graphicOptions.RampCounter == 0) {
        deactivatewarp = false;
    }
    const double dis = UnitUtil::getSignificantDistance(parent, target);

    // SPEC stays on while flying toward the destination -- including during any turn
    // (lining up with the destination, or a detour) -- and only winds down once we're
    // within the braking distance (about to disengage). The old auto_pilot_spec_lining_
    // up_angle check dropped out of warp whenever the facing briefly deviated from the
    // target, which made SPEC flicker in and out whenever the ship turned -- e.g. leaving
    // a planet (now behind us) to head for a faraway object, with nothing to avoid.
    const bool rampdown = configuration().physics.auto_pilot_ramp_warp_down;
    const float min_warpfield_to_enter_warp = configuration().ai.min_warp_to_try_flt;
    if (parent->GetMaxWarpFieldStrength() < min_warpfield_to_enter_warp) {
        deactivatewarp = true;
    }
    if (dis <= brake_distance) {
        deactivatewarp = true;
    }
    if (DistanceWarrantsWarpTo(parent,
            UnitUtil::getSignificantDistance(parent, target), false) && deactivatewarp == false) {
        \
        CautiousWarpRampOn(parent);
    } else {
        WarpRampOff(parent, rampdown);
    }
    SetDest(destination);
    bool combat_mode = parent->computer.combat_mode;
    parent->computer.combat_mode = !inside_landing_zone;     //turn off limits in landing zone
    ChangeHeading::Execute();
    parent->computer.combat_mode = combat_mode;
    if (!finish) {
        ResetDone();
    }
    const float distance_to_stop = configuration().physics.auto_pilot_termination_distance_flt;
    const float enemy_distance_to_stop = configuration().physics.auto_pilot_termination_distance_enemy_flt;
    const bool do_auto_finish = configuration().physics.auto_pilot_terminate;
    // Disengage when we're within the distance it takes to stop from our current
    // (SPEC) speed -- fly straight to that point, then brake cleanly instead of
    // overshooting. distance_to_stop remains a hard floor.
    if (do_auto_finish
            && (dis <= brake_distance || dis < distance_to_stop
                    || (target->Target() == parent && dis < enemy_distance_to_stop))) {
        parent->autopilotactive = false;
        WarpRampOff(parent, rampdown);
        done = true;
    }
}

AutoLongHaul::~AutoLongHaul() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("alh%1$x") % this));    // Was "ft"
#endif
}

void FaceDirection::SetParent(Unit *un) {
    if (un->getFlightgroup()) {
        AttachSelfOrder(un->getFlightgroup()->leader.GetUnit());
    }
    ChangeHeading::SetParent(un);
}

FaceDirection::FaceDirection(float dist, bool fini, int accuracy) : ChangeHeading(QVector(0, 0, 1), accuracy),
        finish(fini) {
    type = FACING;
    subtype |= SSELF;
    this->dist = dist;
}

void FaceDirection::Execute() {
    Unit *target = group.GetUnit();
    if (target == NULL) {
        done = finish;
        return;
    }
    Vector face(target->GetTransformation().getR());
    if ((parent->Position() - target->Position()).Magnitude() - parent->rSize() - target->rSize() > dist) {
        SetDest(target->Position());
    } else {
        SetDest(parent->Position() + face.Cast());
    }
    ChangeHeading::Execute();
    if (!finish) {
        ResetDone();
    }
}

FaceDirection::~FaceDirection() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("fd%1$x") % this));     // Was "ft"
#endif
}

void FormUp::SetParent(Unit *un) {
    if (un->getFlightgroup()) {
        AttachSelfOrder(un->getFlightgroup()->leader.GetUnit());
    }
    MoveTo::SetParent(un);
}

FormUp::FormUp(const QVector &pos) : MoveTo(QVector(0, 0, 0), false, 255, false), Pos(pos) {
    subtype |= SSELF;
}

void FormUp::SetPos(const QVector &v) {
    Pos = v;
}

void FormUp::Execute() {
    Unit *targ = group.GetUnit();
    if (targ) {
        MoveTo::SetDest(Transform(targ->GetTransformation(), Pos));
        const bool can_warp_to = configuration().ai.warp_to_wingmen;
        if (VegaRandom::Instance().RandomUInt32UpTo(63) == 0 && (can_warp_to || _Universe->AccessCockpit()->autoInProgress())) {
            WarpToP(parent, targ, true);
        }
    }
    MoveTo::Execute();
}

FormUp::~FormUp() {
}

void FormUpToOwner::SetParent(Unit *un) {
    Unit *ownerDoNotDereference = NULL;
    Unit *temp;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
            (temp = *i) != NULL;
            ++i) {
        if (temp == un->owner) {
            ownerDoNotDereference = temp;
            break;
        }
    }
    if (ownerDoNotDereference != NULL) {
        AttachSelfOrder(ownerDoNotDereference);
    }
    MoveTo::SetParent(un);
}

FormUpToOwner::FormUpToOwner(const QVector &pos) : MoveTo(QVector(0, 0, 0), false, 255, false), Pos(pos) {
    subtype |= SSELF;
}

void FormUpToOwner::SetPos(const QVector &v) {
    Pos = v;
}

void FormUpToOwner::Execute() {
    Unit *targ = group.GetUnit();
    if (targ) {
        MoveTo::SetDest(Transform(targ->GetTransformation(), Pos));
        const bool can_warp_to = configuration().ai.warp_to_wingmen;
        if (VegaRandom::Instance().RandomUInt32UpTo(63) == 0 && (can_warp_to || _Universe->AccessCockpit()->autoInProgress())) {
            WarpToP(parent, targ, true);
        }
    }
    MoveTo::Execute();
}

FormUpToOwner::~FormUpToOwner() {
}

