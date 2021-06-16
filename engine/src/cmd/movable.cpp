/**
 * movable.cpp
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
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


#include "movable.h"
#include "gfx/vec.h"
#include "unit_generic.h"
#include "universe_util.h"
#include "unit_find.h"
#include "star_system.h"
#include "universe.h"
#include "game_config.h"

#include <iostream>
#include <string>


float accelStarHandler( float &input )
{
    static float game_speed = GameConfig::GetVariable( "physics", "game_speed", 1.0f );
    static float game_accel = GameConfig::GetVariable( "physics", "game_accel", 1.0f );
    return input/(game_speed*game_accel);
}

float speedStarHandler( float &input)
{
    static float game_speed = GameConfig::GetVariable( "physics", "game_speed", 1.0f );
    return input/game_speed;
}

static Vector default_angular_velocity( GameConfig::GetVariable( "general", "pitch", 0.0f ),
                                        GameConfig::GetVariable( "general", "yaw", 0.0f ),
                                        GameConfig::GetVariable( "general", "roll", 0.0f));


bool Movable::configLoaded = false;
float Movable::VELOCITY_MAX = 0.0f;
//for the heck of it.
float Movable::humanwarprampuptime = 0.0f;
//for the heck of it.
float Movable::compwarprampuptime = 0.0f;
float Movable::warprampdowntime = 0.0f;
float Movable::WARPMEMORYEFFECT = 0.0f;
float Movable::maxplayerrotationrate = 0.0f;
float Movable::maxnonplayerrotationrate = 0.0f;
float Movable::warpstretchcutoff = 0.0f;
float Movable::warpstretchoutcutoff = 0.0f;
float Movable::sec = 0.0f;
float Movable::endsec = 0.0f;
float Movable::warpMultiplierMin = 0.0f;
float Movable::warpMultiplierMax = 0.0f;
float Movable::warpMaxEfVel = 0.0f;
float Movable::cutsqr = 0.0f;
float Movable::outcutsqr = 0.0f;
std::string Movable::insys_jump_ani = "";
float Movable::air_res_coef = 0.0f;
float Movable::lateral_air_res_coef = 0.0f;

Movable::Movable() : cumulative_transformation_matrix( identity_matrix ) {
    if (!configLoaded) {
        VELOCITY_MAX = GameConfig::GetVariable( "physics", "velocity_max", 10000);
        humanwarprampuptime = GameConfig::GetVariable( "physics", "warprampuptime", 5);
        compwarprampuptime = GameConfig::GetVariable( "physics", "computerwarprampuptime", 10);
        warprampdowntime = GameConfig::GetVariable( "physics", "warprampdowntime", 0.5f);
        WARPMEMORYEFFECT = GameConfig::GetVariable( "physics", "WarpMemoryEffect", 0.9f);
        maxplayerrotationrate = GameConfig::GetVariable( "physics", "maxplayerrot", 24);
        maxnonplayerrotationrate = GameConfig::GetVariable( "physics", "maxNPCrot", 360);
        warpstretchcutoff = GameConfig::GetVariable( "graphics", "warp_stretch_cutoff", 500000) * GameConfig::GetVariable( "physics", "game_speed", 1);
        warpstretchoutcutoff = GameConfig::GetVariable( "graphics", "warp_stretch_decel_cutoff", 500000) * GameConfig::GetVariable( "physics", "game_speed", 1);
        sec = GameConfig::GetVariable( "graphics", "insys_jump_ani_second_ahead", 4) / (GameConfig::GetVariable( "physics", "game_speed", 1) * GameConfig::GetVariable( "physics", "game_accel", 1));
        endsec = GameConfig::GetVariable( "graphics", "insys_jump_ani_second_ahead_end",0.03f) /( GameConfig::GetVariable( "physics", "game_speed",1.0f) *GameConfig::GetVariable( "physics", "game_accel", 1.0f) );
        //Pi^2
        warpMultiplierMin = GameConfig::GetVariable( "physics", "warpMultiplierMin", 9.86960440109f);
        //C
        warpMultiplierMax = GameConfig::GetVariable( "physics", "warpMultiplierMax", 300000000 );
        //Pi^2 * C
        warpMaxEfVel = GameConfig::GetVariable( "physics", "warpMaxEfVel", 2960881320.0f);
        cutsqr    = warpstretchcutoff * warpstretchcutoff;
        outcutsqr = warpstretchoutcutoff * warpstretchoutcutoff;
        insys_jump_ani = vs_config->getVariable( "graphics", "insys_jump_animation", "warp.ani" );
        configLoaded = true;
    }

    Identity( cumulative_transformation_matrix );
    cumulative_transformation = identity_transformation;
    curr_physical_state = prev_physical_state = identity_transformation;

    AngularVelocity = default_angular_velocity;
}



Movable::graphic_options::graphic_options()
{
    FaceCamera  = Animating = missilelock = InWarp = unused1 = WarpRamping = NoDamageParticles = 0;
    specInterdictionOnline = 1;
    NumAnimationPoints = 0;
    RampCounter = 0;
    MinWarpMultiplier = MaxWarpMultiplier = 1;

    // Added implementation to make var false
    // I don't like it, because it's true by default and false by default
    // TODO: figure out which it actually is
    RecurseIntoSubUnitsOnCollision = false;
    SubUnit    = 0; // Also this
}


void Movable::SetPosition( const QVector &pos )
{
    prev_physical_state.position = curr_physical_state.position = pos;
}

void Movable::GetOrientation( Vector &p, Vector &q, Vector &r ) const
{
    Matrix m;
    curr_physical_state.to_matrix( m );
    p = m.getP();
    q = m.getQ();
    r = m.getR();
}

Vector Movable::GetNetAcceleration() const
{
    Vector p, q, r;
    GetOrientation( p, q, r );
    Vector res( NetLocalForce.i*p+NetLocalForce.j*q+NetLocalForce.k*r );
    res += NetForce;
    return res/Mass;
}

Vector Movable::GetNetAngularAcceleration() const
{
    Vector p, q, r;
    GetOrientation( p, q, r );
    Vector res( NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k*r );
    res += NetTorque;
    return res/GetMoment();
}

float Movable::GetMaxAccelerationInDirectionOf( const Vector &ref, bool afterburn ) const
{
    Vector p, q, r;
    GetOrientation( p, q, r );
    Vector lref( ref*p, ref*q, ref*r );
    float  tp     = (lref.i == 0) ? 0 : fabs( Limits().lateral/lref.i );
    float  tq     = (lref.j == 0) ? 0 : fabs( Limits().vertical/lref.j );
    float  tr     = (lref.k == 0) ? 0 : fabs( ( (lref.k > 0) ? Limits().forward : Limits().retro )/lref.k );
    float  trqmin = (tr < tq) ? tr : tq;
    float  tm     = tp < trqmin ? tp : trqmin;
    return lref.Magnitude()*tm/Mass;
}

void Movable::SetVelocity( const Vector &v )
{
    Velocity = v;
}

void Movable::SetAngularVelocity( const Vector &v )
{
    AngularVelocity = v;
}

//FIXME Daughter units should be able to be turrets (have y/p/r)
void Movable::SetResolveForces( bool ys )
{
    resolveforces = ys;
}



void Movable::UpdatePhysics( const Transformation &trans,
                          const Matrix &transmat,
                          const Vector &cum_vel,
                          bool lastframe,
                          UnitCollection *uc,
                          Unit *superunit )
{
    //Save information about when this happened
    unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
    //Well, wasn't skipped actually, but...
    this->last_processed_sqs = cur_sim_frame;
    this->cur_sim_queue_slot = (cur_sim_frame+this->sim_atom_multiplier)%SIM_QUEUE_SIZE;
    Transformation old_physical_state = curr_physical_state;

    UpdatePhysics3(trans, transmat, lastframe, uc, superunit);




    if (resolveforces) {
        //clamp velocity
        ResolveForces(trans, transmat);
        if (Velocity.i > VELOCITY_MAX)
            Velocity.i = VELOCITY_MAX;

        else if (Velocity.i < -VELOCITY_MAX)
            Velocity.i = -VELOCITY_MAX;
        if (Velocity.j > VELOCITY_MAX)
            Velocity.j = VELOCITY_MAX;

        else if (Velocity.j < -VELOCITY_MAX)
            Velocity.j = -VELOCITY_MAX;
        if (Velocity.k > VELOCITY_MAX)
            Velocity.k = VELOCITY_MAX;

        else if (Velocity.k < -VELOCITY_MAX)
            Velocity.k = -VELOCITY_MAX;
    }

    // The 1.0 difficulty is a hack based on the hack in GetVelocityDifficultyMult
    this->UpdatePhysics2( trans, old_physical_state, Vector(), 1.0, transmat, cum_vel, lastframe, uc );


}

void Movable::AddVelocity( float difficulty )
{
    Unit *unit = static_cast<Unit*>(this);
    float  lastWarpField = graphicOptions.WarpFieldStrength;

    bool   playa = isPlayerShip();

    float  warprampuptime = playa ? humanwarprampuptime : compwarprampuptime;
    //Warp Turning on/off
    if (graphicOptions.WarpRamping) {
        float  oldrampcounter = graphicOptions.RampCounter;
        if (graphicOptions.InWarp == 1)             //Warp Turning on
            graphicOptions.RampCounter = warprampuptime;
        else                                        //Warp Turning off
            graphicOptions.RampCounter = warprampdowntime;
        //switched mid - ramp time; we also know old mode's ramptime != 0, or there won't be ramping
        if (oldrampcounter != 0 && graphicOptions.RampCounter != 0 ) {
            if (graphicOptions.InWarp == 1)             //Warp is turning on before it turned off
                graphicOptions.RampCounter *= (1 - oldrampcounter/warprampdowntime);
            else                                        //Warp is turning off before it turned on
                graphicOptions.RampCounter *= (1 - oldrampcounter/warprampuptime);
        }
        graphicOptions.WarpRamping = 0;
    }
    if (graphicOptions.InWarp == 1 || graphicOptions.RampCounter != 0) {
        float rampmult = 1.f;
        if (graphicOptions.RampCounter != 0) {
            graphicOptions.RampCounter -= simulation_atom_var;
            if (graphicOptions.RampCounter <= 0)
                graphicOptions.RampCounter = 0;
            if (graphicOptions.InWarp == 0 && graphicOptions.RampCounter > warprampdowntime)
                graphicOptions.RampCounter = (1-graphicOptions.RampCounter/warprampuptime)*warprampdowntime;
            if (graphicOptions.InWarp == 1 && graphicOptions.RampCounter > warprampuptime)
                graphicOptions.RampCounter = warprampuptime;
            rampmult = (graphicOptions.InWarp) ? 1.0
                        -( (graphicOptions.RampCounter
                            /warprampuptime)
                            *(graphicOptions.RampCounter
                            /warprampuptime) ) : (graphicOptions.RampCounter
                                                    /warprampdowntime)*(graphicOptions.RampCounter/warprampdowntime);
        }
        graphicOptions.WarpFieldStrength = GetMaxWarpFieldStrength(rampmult);
    } else {
        graphicOptions.WarpFieldStrength = 1;
    }
    //not any more? lastWarpField=1;
    Vector v;
    if (graphicOptions.WarpFieldStrength != 1.0)
        v = unit->GetWarpVelocity();
    else
        v = Velocity;

    graphicOptions.WarpFieldStrength = lastWarpField*WARPMEMORYEFFECT+(1.0-WARPMEMORYEFFECT)*graphicOptions.WarpFieldStrength;
    curr_physical_state.position     = curr_physical_state.position+(v*simulation_atom_var*difficulty).Cast();
    //now we do this later in update physics
    //I guess you have to, to be robust}
}

void Movable::UpdatePhysics2( const Transformation &trans,
                           const Transformation &old_physical_state,
                           const Vector &accel,
                           float difficulty,
                           const Matrix &transmat,
                           const Vector &cum_vel,
                           bool lastframe,
                           UnitCollection *uc ) {
    //Only in non-networking OR networking && is a player OR SERVER && not a player
    if (AngularVelocity.i || AngularVelocity.j || AngularVelocity.k)
      Rotate( simulation_atom_var*(AngularVelocity) );

}

void Movable::Rotate( const Vector &axis )
{
    double     theta   = axis.Magnitude();
    double     ootheta = 0;
    if (theta == 0) return;
    ootheta = 1/theta;
    float      s   = cos( theta*.5 );
    Quaternion rot = Quaternion( s, axis*(sinf( theta*.5 )*ootheta) );
    if (theta < 0.0001)
        rot = identity_quaternion;
    curr_physical_state.orientation *= rot;
    if (limits.limitmin > -1) {
        Matrix mat;
        curr_physical_state.orientation.to_matrix( mat );
        if (limits.structurelimits.Dot( mat.getR() ) < limits.limitmin)
            curr_physical_state.orientation = prev_physical_state.orientation;
    }
}


Vector Movable::ResolveForces( const Transformation &trans, const Matrix &transmat )
{
    //First, save theoretical instantaneous acceleration (not time-quantized) for GetAcceleration()
    SavedAccel = GetNetAcceleration();
    SavedAngAccel = GetNetAngularAcceleration();

    Vector p, q, r;
    GetOrientation( p, q, r );
    Vector temp1( NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k*r );
    if (NetTorque.i || NetTorque.j || NetTorque.k)
        temp1 += InvTransformNormal( transmat, NetTorque );
    if ( GetMoment() )
        temp1 = temp1/GetMoment();

    // TODO: restore this with the unit name
    //    else
    //        VSFileSystem::vs_fprintf( stderr, "zero moment of inertia %s\n", name.get().c_str() );
    Vector temp( temp1*simulation_atom_var );
    AngularVelocity += temp;

    float caprate;
    if ( isPlayerShip() )         //clamp to avoid vomit-comet effects
        caprate = maxplayerrotationrate;
    else
        caprate = maxnonplayerrotationrate;
    if (AngularVelocity.MagnitudeSquared() > caprate*caprate)
        AngularVelocity = AngularVelocity.Normalize()*caprate;
    //acceleration
    Vector temp2 = (NetLocalForce.i*p+NetLocalForce.j*q+NetLocalForce.k*r);
    if ( !( FINITE( NetForce.i ) && FINITE( NetForce.j ) && FINITE( NetForce.k ) ) ) {
        BOOST_LOG_TRIVIAL(info) << "NetForce skrewed";
    }
    if (NetForce.i || NetForce.j || NetForce.k)
        temp2 += InvTransformNormal( transmat, NetForce );
    temp2 = temp2/Mass;
    temp  = temp2*simulation_atom_var;
    if ( !( FINITE( temp2.i ) && FINITE( temp2.j ) && FINITE( temp2.k ) ) ) {
        BOOST_LOG_TRIVIAL(info) << "NetForce transform skrewed";
    }
    float oldmagsquared = Velocity.MagnitudeSquared();
        Velocity += temp;
    //}

    float newmagsquared = Velocity.MagnitudeSquared();

    bool oldbig    = oldmagsquared > cutsqr;
    bool newbig    = newmagsquared > cutsqr;
    bool oldoutbig = oldmagsquared > outcutsqr;
    bool newoutbig = newmagsquared > outcutsqr;
    if ( (newbig && !oldbig) || (oldoutbig && !newoutbig) ) {
        static bool   docache = true;
        if (docache) {
            UniverseUtil::cacheAnimation( insys_jump_ani );
            docache = false;
        }
        Vector v( GetVelocity() );
        v.Normalize();
        Vector p, q, r;
        GetOrientation( p, q, r );

        float tmpsec = oldbig ? endsec : sec;
        UniverseUtil::playAnimationGrow( insys_jump_ani, realPosition().Cast()+Velocity*tmpsec+v*radial_size, radial_size*8, 1 );
    }

    // stephengtuggy 2020-10-17: These need to be initialized here, because they depend on having an active mission.
    air_res_coef = XMLSupport::parse_float( active_missions[0]->getVariable( "air_resistance", "0" ) );
    lateral_air_res_coef = XMLSupport::parse_float( active_missions[0]->getVariable( "lateral_air_resistance", "0"));

    if (air_res_coef || lateral_air_res_coef) {
        float  velmag = Velocity.Magnitude();
        Vector AirResistance = Velocity
                               *( air_res_coef*velmag/Mass )*(corner_max.i-corner_min.i)*(corner_max.j-corner_min.j);
        if (AirResistance.Magnitude() > velmag) {
            Velocity.Set( 0, 0, 0 );
        } else {
            Velocity = Velocity-AirResistance;
            if (lateral_air_res_coef) {
                Vector p, q, r;
                GetOrientation( p, q, r );
                Vector lateralVel = p*Velocity.Dot( p )+q*Velocity.Dot( q );
                AirResistance = lateralVel
                                *( lateral_air_res_coef*velmag
                                  /Mass )*(corner_max.i-corner_min.i)*(corner_max.j-corner_min.j);
                if ( AirResistance.Magnitude() > lateralVel.Magnitude() )
                    Velocity = r*Velocity.Dot( r );
                else
                    Velocity = Velocity-AirResistance;
            }
        }
    }
    NetForce = NetLocalForce = NetTorque = NetLocalTorque = Vector( 0, 0, 0 );

    return temp2;
}

void Movable::SetOrientation( QVector q, QVector r )
{
    q.Normalize();
    r.Normalize();
    QVector p;
    CrossProduct( q, r, p );
    CrossProduct( r, p, q );
    curr_physical_state.orientation = Quaternion::from_vectors( p.Cast(), q.Cast(), r.Cast() );
}

void Movable::SetOrientation( QVector p, QVector q, QVector r )
{
    q.Normalize();
    r.Normalize();
    p.Normalize();
    curr_physical_state.orientation = Quaternion::from_vectors( p.Cast(), q.Cast(), r.Cast() );
}

void Movable::SetOrientation( Quaternion Q )
{
    curr_physical_state.orientation = Q;
}

#define MM( A, B ) m.r[B*3+A]

Vector Movable::UpCoordinateLevel( const Vector &v ) const
{
    Matrix m;
    curr_physical_state.to_matrix( m );
    return Vector( v.i*MM( 0, 0 )+v.j*MM( 1, 0 )+v.k*MM( 2, 0 ),
                  v.i*MM( 0, 1 )+v.j*MM( 1, 1 )+v.k*MM( 2, 1 ),
                  v.i*MM( 0, 2 )+v.j*MM( 1, 2 )+v.k*MM( 2, 2 ) );
}

#undef MM

Vector Movable::DownCoordinateLevel( const Vector &v ) const
{
    Matrix m;
    curr_physical_state.to_matrix( m );
    return TransformNormal( m, v );
}

#define MM( A, B ) ( (cumulative_transformation_matrix.r[B*3+A]) )

Vector Movable::ToLocalCoordinates( const Vector &v ) const
{
    //Matrix m;
    //062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);
    return Vector( v.i*MM( 0, 0 )+v.j*MM( 1, 0 )+v.k*MM( 2, 0 ),
                  v.i*MM( 0, 1 )+v.j*MM( 1, 1 )+v.k*MM( 2, 1 ),
                  v.i*MM( 0, 2 )+v.j*MM( 1, 2 )+v.k*MM( 2, 2 ) );
}

#undef MM

Vector Movable::ToWorldCoordinates( const Vector &v ) const
{
    return TransformNormal( cumulative_transformation_matrix, v );
}

// TODO: move this to JumpCapable
float Movable::GetMaxWarpFieldStrength( float rampmult ) const
{
    const Unit *unit = static_cast<const Unit*>(this);
    Vector v = unit->GetWarpRefVelocity();


    //inverse fractional effect of ship vs real big object
    float minmultiplier = warpMultiplierMax*graphicOptions.MaxWarpMultiplier;
    Unit *nearest_unit  = NULL;
    minmultiplier = unit->CalculateNearestWarpUnit( minmultiplier, &nearest_unit, true );
    float minWarp = warpMultiplierMin*graphicOptions.MinWarpMultiplier;
    float maxWarp = warpMultiplierMax*graphicOptions.MaxWarpMultiplier;
    if (minmultiplier < minWarp)
        minmultiplier = minWarp;
    if (minmultiplier > maxWarp)
        minmultiplier = maxWarp; //SOFT LIMIT
    minmultiplier *= rampmult;
    if (minmultiplier < 1)
        minmultiplier = 1;
    v *= minmultiplier;
    float vmag = sqrt( v.i*v.i+v.j*v.j+v.k*v.k );
    if (vmag > warpMaxEfVel) {
        v *= warpMaxEfVel/vmag; //HARD LIMIT
        minmultiplier *= warpMaxEfVel/vmag;
    }
    return minmultiplier;
}

