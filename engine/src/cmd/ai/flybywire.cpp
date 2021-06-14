/**
 * flybywire.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
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


#include "flybywire.h"
#include "vegastrike.h"
#include <math.h>
#include <stdio.h>
#include "cmd/unit_generic.h"
#include "lin_time.h"
#include "gfx/cockpit_generic.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "universe.h"

#define VELTHRESHOLD .1
#define ANGVELTHRESHOLD .01

using Orders::MatchLinearVelocity;

using Orders::MatchVelocity;

using Orders::MatchAngularVelocity;

//Careful with this macro!!! I can't wrap it in do{...}while(0) because it declares variables that are then
//used by code that follows. Be sure it's never instantiated like a single statement body of a conditional
//or loop, or the formatter might remove the braces and then you'll be in a heap of trouble looking for the
//bug... --chuck_starchaser.
#define MATCHLINVELSETUP()                                                                                                      \
        Unit *match = parent->VelocityReference(); Vector desired( desired_velocity );  Vector FrameOfRef( 0,                   \
                                                                                                           0,                   \
                                                                                                           0 );                 \
        if (match != NULL) {float dif1, dif2; match->GetVelocityDifficultyMult( dif1 ); dif1 *=                                 \
                                match->graphicOptions.WarpFieldStrength;                                                        \
                            parent->GetVelocityDifficultyMult( dif2 );                                                          \
                            if (match->graphicOptions.WarpFieldStrength > 1) {dif2 *= parent->graphicOptions.WarpFieldStrength; \
                            }                                                                                                   \
                            FrameOfRef =                                                                                        \
                                parent->ToLocalCoordinates( match->GetWarpVelocity()*dif1                                       \
                                                            /dif2 ); }                                                          \
        if (!LocalVelocity) {desired = parent->ToLocalCoordinates(                                                              \
                                 desired ); }                                                                                   \
        Vector velocity(                                                                                                        \
            parent->UpCoordinateLevel( parent->GetVelocity() ) )

#define MATCHLINVELEXECUTE()                                                                         \
    do {                                                                                             \
        parent->Thrust( (parent->Mass                                                           \
                         *(parent->ClampVelocity( desired,                                           \
                                                  afterburn )+FrameOfRef-velocity)/simulation_atom_var), \
                       afterburn );                                                                  \
    }                                                                                                \
    while (0)

/**
 *
 * don't need to clamp thrust since the Thrust does it for you
 *
 * caution might change
 *
 */

void MatchLinearVelocity::Execute()
{
    if ( !suborders.empty() ) {
        static int i = 0;
        if (i++ % 1000 == 0) {
            BOOST_LOG_TRIVIAL(warning) << "cannot execute suborders as Linear Velocity Matcher";                                // error printout just in case
        }
    }
    MATCHLINVELSETUP();
    if (willfinish) {
        if ( (done = fabs( desired.i+FrameOfRef.i-velocity.i ) < VELTHRESHOLD && fabs( desired.j+FrameOfRef.j-velocity.j )
                     < VELTHRESHOLD && fabs( desired.k+FrameOfRef.k-velocity.k ) < VELTHRESHOLD) )
            return;
    }
    MATCHLINVELEXECUTE();
}

MatchLinearVelocity::~MatchLinearVelocity()
{
#ifdef ORDERDEBUG
    BOOST_LOG_TRIVIAL(debug) << boost::format("mlv%1$x") % this;
    VSFileSystem::flushLogs();
#endif
}

void Orders::MatchRoll::Execute()
{
    bool   temp = done;
    Order::Execute();
    done = temp;
    Vector angvel( parent->UpCoordinateLevel( parent->GetAngularVelocity() ) );
    if (willfinish)
        if (fabs( desired_roll-angvel.k ) < ANGVELTHRESHOLD)
            return;
    //prevent matchangvel from resetting this (kinda a hack)
    parent->ApplyLocalTorque( parent->GetMoment()*Vector( 0, 0, desired_roll-angvel.k )/simulation_atom_var );
    parent->ApplyLocalTorque( parent->GetMoment()*Vector( 0, 0, desired_roll-angvel.k )/simulation_atom_var );
}

void MatchAngularVelocity::Execute()
{
    bool   temp = done;
    Order::Execute();
    done = temp;
    Vector desired( desired_ang_velocity );

    Vector angvel( parent->UpCoordinateLevel( parent->GetAngularVelocity() ) );
    if (!LocalAng)
        desired = parent->ToLocalCoordinates( desired );
    if (willfinish) {
        if ( (done = fabs( desired.i-angvel.i ) < ANGVELTHRESHOLD && fabs( desired.j-angvel.j ) < ANGVELTHRESHOLD
                     && fabs( desired.k-angvel.k ) < ANGVELTHRESHOLD) )
            return;
    }
    parent->ApplyLocalTorque( parent->GetMoment()*( desired-parent->UpCoordinateLevel(
                                                       parent->GetAngularVelocity() ) )/simulation_atom_var );
}

MatchAngularVelocity::~MatchAngularVelocity()
{
#ifdef ORDERDEBUG
    BOOST_LOG_TRIVIAL(debug) << boost::format("mav%1$x") % this;
    VSFileSystem::flushLogs();
#endif
}

void MatchVelocity::Execute()
{
    MatchAngularVelocity::Execute();

    MATCHLINVELSETUP();
    if (willfinish) {
        if ( (done = done && fabs( desired.i-velocity.i ) < VELTHRESHOLD && fabs( desired.j-velocity.j ) < VELTHRESHOLD
                     && fabs( desired.k-velocity.k ) < VELTHRESHOLD) )
            return;
    }
    MATCHLINVELEXECUTE();
}

MatchVelocity::~MatchVelocity()
{
#ifdef ORDERDEBUG
    BOOST_LOG_TRIVIAL(debug) << boost::format("mv%1$x") % this;
    VSFileSystem::flushLogs();
#endif
}

static bool getControlType()
{
    static bool control = XMLSupport::parse_bool( vs_config->getVariable( "physics", "CarControl",
                                                                          "false"
                                                                        ) );
    return control;
}

FlyByWire::FlyByWire() : MatchVelocity( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ), true, false, false )
    , sheltonslide( false )
    , controltype( !getControlType() )
{
    DesiredShiftVelocity  = Vector( 0, 0, 0 );
    DirectThrust = Vector( 0, 0, 0 );
    stolen_setspeed = false;
    stolen_setspeed_value = 0;

    static bool static_inertial_flight_model  =
        XMLSupport::parse_bool( vs_config->getVariable( "flight", "inertial::initial", "false" ) );
    static bool static_inertial_flight_enable =
        XMLSupport::parse_bool( vs_config->getVariable( "flight", "inertial::enable", "true" ) );
    inertial_flight_model  = static_inertial_flight_model;
    inertial_flight_enable = static_inertial_flight_enable;
}

void FlyByWire::Stop( float per )
{
    SetDesiredVelocity( Vector( 0, 0, per*parent->GetComputerData().max_speed() ), true );

    parent->GetComputerData().set_speed = per*parent->GetComputerData().max_speed();
}

void FlyByWire::Right( float per )
{
    desired_ang_velocity +=
        ( -per
         *(per
           > 0 ? parent->GetComputerData().max_yaw_left : parent->GetComputerData().max_yaw_right)
         /getTimeCompression() )*Vector(
            0,
            1,
            0 );
}

void FlyByWire::Up( float per )
{
    desired_ang_velocity +=
        ( -per
         *(per
           > 0 ? parent->GetComputerData().max_pitch_down : parent->GetComputerData().max_pitch_up)
         /getTimeCompression() )*Vector(
            1,
            0,
            0 );
}

void FlyByWire::RollRight( float per )
{
    desired_ang_velocity +=
        ( -per
         *(per
           > 0 ? parent->GetComputerData().max_roll_left : parent->GetComputerData().max_roll_right)
         /getTimeCompression() )*Vector(
            0,
            0,
            1 );
}

void FlyByWire::Afterburn( float per )
{
    Computer *cpu = &parent->GetComputerData();

    afterburn = (per > .1);
    if (!sheltonslide && !inertial_flight_model)
        desired_velocity = Vector( 0, 0, cpu->set_speed+per*(cpu->max_ab_speed()-cpu->set_speed) );
    else if (inertial_flight_model)
        DirectThrust += Vector( 0, 0, parent->limits.afterburn*per );
    if ( parent == _Universe->AccessCockpit()->GetParent() ) {
        //printf("afterburn is %d\n",afterburn); // DELETEME WTF all this force feedback code and its unused.
        //COMMENTED BECAUSE OF SERVER -- NEED TO REINTEGRATE IT IN ANOTHER WAY
        //forcefeedback->playAfterburner(afterburn);
    }
}

void FlyByWire::SheltonSlide( bool onoff )
{
    sheltonslide = onoff;
}

void FlyByWire::MatchSpeed( const Vector &vec )
{
    Computer *cpu = &parent->GetComputerData();

    cpu->set_speed = (vec).Magnitude();
    if ( cpu->set_speed > cpu->max_speed() )
        cpu->set_speed = cpu->max_speed();
}

void FlyByWire::Accel( float per )
{
    Computer *cpu = &parent->GetComputerData();

    cpu->set_speed += per*cpu->max_speed()*simulation_atom_var; //SIMULATION_ATOM?
    if ( cpu->set_speed > cpu->max_speed() )
        cpu->set_speed = cpu->max_speed();
    static float reverse_speed_limit = XMLSupport::parse_float( vs_config->getVariable( "physics", "reverse_speed_limit", "1.0" ) );
    if (cpu->set_speed < -cpu->max_speed()*reverse_speed_limit)
        cpu->set_speed = -cpu->max_speed()*reverse_speed_limit;
    afterburn = false;

    desired_velocity = Vector( 0, 0, cpu->set_speed );
}

#define FBWABS( m ) (m >= 0 ? m : -m)

void FlyByWire::ThrustRight( float percent )
{
    DesiredShiftVelocity.i = parent->GetComputerData().max_speed()*percent;
}

void FlyByWire::ThrustUp( float percent )
{
    DesiredShiftVelocity.j = parent->GetComputerData().max_speed()*percent;
}

void FlyByWire::ThrustFront( float percent )
{
    DesiredShiftVelocity.k = parent->GetComputerData().max_speed()*percent;
}

void FlyByWire::DirectThrustRight( float percent )
{
    DirectThrust.i = parent->limits.lateral*percent;
}

void FlyByWire::DirectThrustUp( float percent )
{
    DirectThrust.j = parent->limits.vertical*percent;
}

void FlyByWire::DirectThrustFront( float percent )
{
    if (percent > 0)
        DirectThrust.k = parent->limits.forward*percent;
    else
        DirectThrust.k = parent->limits.retro*percent;
}

void FlyByWire::Execute()
{
    bool   desireThrust = false;
    Vector des_vel_bak( desired_velocity );
    if (!inertial_flight_model) {
        //Must translate the thrust values to velocities, which is somewhat cumbersome.
        Vector Limit(
            parent->limits.lateral, parent->limits.vertical,
            ( (DirectThrust.k > 0) ? parent->limits.forward : parent->limits.retro )
                    );
        if (Limit.i <= 1) Limit.i = 1;
        if (Limit.j <= 1) Limit.j = 1;
        if (Limit.k <= 1) Limit.k = 1;
        Vector DesiredDrift(
            DirectThrust.i/Limit.i,
            DirectThrust.j/Limit.j,
            DirectThrust.k/Limit.k
                           );
        //Now, scale so that maximum shift velocity is max_speed
        DesiredDrift *= parent->GetComputerData().max_speed();
        //And apply
        DesiredShiftVelocity += DesiredDrift;
    }
    if (DesiredShiftVelocity.i || DesiredShiftVelocity.j || DesiredShiftVelocity.k) {
        if (!stolen_setspeed) {
            stolen_setspeed = true;
            stolen_setspeed_value = parent->GetComputerData().set_speed;
        }
        desired_velocity = Vector( 0, 0, stolen_setspeed_value )+DesiredShiftVelocity;
        parent->GetComputerData().set_speed = desired_velocity.Magnitude();

        desireThrust     = true;
        if (!controltype) {
            if (desired_velocity.k < 0) {
                desired_velocity.k = 0;
                parent->GetComputerData().set_speed = 0;
            }
        }
    } else if (stolen_setspeed) {
        parent->GetComputerData().set_speed = stolen_setspeed_value;
        stolen_setspeed = false;
    }
    static double collidepanic = XMLSupport::parse_float( vs_config->getVariable( "physics", "collision_inertial_time", "1.25" ) );
    Cockpit *tempcp = _Universe->isPlayerStarship( parent );
    if ( ( (sheltonslide || inertial_flight_model
            || !controltype)
          && (!desireThrust) )
        || ( tempcp && ( (getNewTime()-tempcp->TimeOfLastCollision) < collidepanic ) ) ) {
        MatchAngularVelocity::Execute();         //only match turning
        if (inertial_flight_model)
            parent->Thrust( DirectThrust, afterburn );
    } else {
        MatchVelocity::Execute();
    }
    DesiredShiftVelocity.Set( 0, 0, 0 );
    DirectThrust.Set( 0, 0, 0 );
    desired_velocity = des_vel_bak;
}

FlyByWire::~FlyByWire()
{
#ifdef ORDERDEBUG
    BOOST_LOG_TRIVIAL(debug) << boost::format("fbw%1$x") % this;
    VSFileSystem::flushLogs();
#endif
}

void FlyByWire::InertialFlight( bool onoff )
{
    inertial_flight_model = onoff;
    parent->inertialmode  = onoff;
}

bool FlyByWire::InertialFlight() const
{
    return inertial_flight_model;
}

bool FlyByWire::InertialFlightEnable() const
{
    return inertial_flight_enable;
}
