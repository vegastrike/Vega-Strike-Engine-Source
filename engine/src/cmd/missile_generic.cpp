#include "universe_util.h"
#include "missile_generic.h"
#include "unit_generic.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "configxml.h"
#include "images.h"
#include "collection.h"
#include "star_system_generic.h"
#include "role_bitmask.h"
#include "ai/order.h"
#include "faction_generic.h"
#include "unit_util.h"
#include "vsfilesystem.h"


void StarSystem::UpdateMissiles()
{
    //if false, missiles collide with rocks as units, but not harm them with explosions
    //FIXME that's how it's used now, but not really correct, as there could be separate AsteroidWeaponDamage for this
    static bool collideroids = XMLSupport::parse_bool( vs_config->getVariable( "physics", "AsteroidWeaponCollision", "false" ) );

    //WARNING: This is a big performance problem...
    //...responsible for many hiccups.
    //TODO: Make it use the collidemap to only iterate through potential hits...
    //PROBLEM: The current collidemap does not allow this efficiently (no way of
    //taking the other unit's rSize() into account).
    if ( !dischargedMissiles.empty() ) {
        if ( dischargedMissiles.back()->GetRadius() > 0 ) {           //we can avoid this iterated check for kinetic projectiles even if they "discharge" on hit
            Unit *un;
            for ( un_iter ui = getUnitList().createIterator(); 
                  NULL != ( un = (*ui) ); 
                  ++ui ) {
                enum clsptr type = un->isUnit();
                if (collideroids || type != ASTEROIDPTR )           // could check for more, unless someone wants planet-killer missiles, but what it would change?
                    dischargedMissiles.back()->ApplyDamage( un );
            }
        }
        delete dischargedMissiles.back();
        dischargedMissiles.pop_back();
    }
}

void MissileEffect::DoApplyDamage(Unit *parent, Unit *un, float distance, float damage_fraction) { 
    QVector norm = pos-un->Position();
    norm.Normalize();
    float damage_left = 1.f;
    if (un->hasSubUnits()) {
        /*
         * Compute damage aspect ratio of each subunit with their apparent size ( (radius/distance)^2 )
         * and spread damage across affected subunits based on their apparent size vs total spread surface
         */
        double total_area = 0.0f; {
            un_kiter ki = un->viewSubUnits();
            for (const Unit *subun; (subun = *ki); ++ki) {
                if (subun->Killed()) continue;
                double r = subun->rSize();
                double d = (pos - subun->Position()).Magnitude() - r;
                if (d > radius) continue;
                if (d < 0.01) d = 0.01;
                total_area += (r*r) / (d*d);
            }
        }
        if (total_area > 0) {
            BOOST_LOG_TRIVIAL(info) << boost::format("Missile subunit damage of %1$.3f%%") % (total_area * (100.0 / 4.0*M_PI));
        }
        if (total_area < 4.0*M_PI) total_area = 4.0*M_PI;

        un_iter i = un->getSubUnits();
        for (Unit *subun; (subun = *i); ++i) {
            if (subun->Killed()) continue;
            double r = subun->rSize();
            double d = (pos - subun->Position()).Magnitude() - r;
            if (d > radius) continue;
            if (d < 0.01) d = 0.01;
            double a = (r*r) / (d*d*total_area);
            DoApplyDamage(parent, subun, d, a * damage_fraction);
            damage_left -= a;
        }
    }
    if (damage_left > 0) {
        BOOST_LOG_TRIVIAL(info)
                << boost::format("Missile damaging %1%/%2% (dist=%3$.3f r=%4$.3f dmg=%5$.3f)")
                % parent->name.get()
                % ((un == parent) ? "." : un->name.get())
                % distance
                % radius
                % (damage*damage_fraction*damage_left);
        parent->ApplyDamage( pos.Cast(), norm, damage*damage_fraction*damage_left, un, GFXColor( 1,1,1,1 ),
                             ownerDoNotDereference, phasedamage*damage_fraction*damage_left );
    }
}

void MissileEffect::ApplyDamage( Unit *smaller )
{
    QVector norm = pos-smaller->Position();
    float smaller_rsize = smaller->rSize();
    float distance = norm.Magnitude()-smaller_rsize;            // no better check than the bounding sphere for now
    if ( distance < radius) {                                   // "smaller->isUnit() != MISSILEPTR &&" was removed - why disable antimissiles?
        if ( distance < 0)
            distance = 0.f;                                     //it's inside the bounding sphere, so we'll not reduce the effect
        if (radialmultiplier < .001) radialmultiplier = .001;
        float dist_part=distance/radialmultiplier;              //radialmultiplier is radius of the set damage
        float damage_mul;
        if ( dist_part > 1.f) {                                  // there can be something else, such as different eye- and ear- candy
            damage_mul = 1/(dist_part*dist_part);
        }
        else {
            damage_mul = 2.f - dist_part*dist_part;
        }
        /*
         *  contrived formula to create paraboloid falloff rather than quadratic peaking at 2x damage at origin
         *  k = 2-distance^2/radmul^2
         * 
         * if the explosion itself was a weapon, it would have double the base damage, longrange=0.5 (counted at Rm) and more generic form:
         * Kclose = 1-(1-longrange)*(R/Rm)^2
         * Kfar   = longrange/(R/Rm)^2
         * or Kapprox = longrange/(longrange-(R/Rm)^3*(1-longrange)) ; obviously, with more checks preventing /0
         */
        DoApplyDamage(smaller, smaller, distance, damage_mul);
    }
}

float Missile::ExplosionRadius()
{
    static float missile_multiplier =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "missile_explosion_radius_mult", "1" ) );
    
    return radial_effect*(missile_multiplier);
}

void StarSystem::AddMissileToQueue( MissileEffect *me )
{
    dischargedMissiles.push_back( me );
}

void Missile::Discharge() {
    if ( (damage != 0 || phasedamage != 0) && !discharged ) {
        Unit *targ = Unit::Target();
        BOOST_LOG_TRIVIAL(info) << boost::format("Missile discharged (target %1%)") % (targ != NULL) ? targ->name.get().c_str() : "NULL";
        _Universe->activeStarSystem()->AddMissileToQueue(
            new MissileEffect( Position(), damage, phasedamage,
            radial_effect, radial_multiplier, owner ) );
    discharged = true;
  }
}

void Missile::Kill( bool erase ) {
    Unit *targ = Unit::Target();
    BOOST_LOG_TRIVIAL(info)
            << boost::format("Missile killed (target %1%)")
            % (targ != NULL) ? targ->name.get().c_str() : "NULL";
    Discharge();
    Unit::Kill( erase );
}

void Missile::reactToCollision( Unit *smaller,
                                const QVector &biglocation,
                                const Vector &bignormal,
                                const QVector &smalllocation,
                                const Vector &smallnormal,
                                float dist ) {
    static bool doesmissilebounce = XMLSupport::parse_bool( vs_config->getVariable( "physics", "missile_bounce", "false" ) );
    if (doesmissilebounce)
        Unit::reactToCollision( smaller, biglocation, bignormal, smalllocation, smallnormal, dist );
    BOOST_LOG_TRIVIAL(info) << boost::format("Missile collided with %1%") % smaller->name.get();
    if (smaller->isUnit() != MISSILEPTR) {
        //2 missiles in a row can't hit each other
        this->Velocity = smaller->Velocity;
        Velocity = smaller->Velocity;
        Discharge();
        if (!killed)
            DealDamageToHull( smalllocation.Cast(), hull+1 );              //should kill, applying addmissile effect
    }
}

Unit * getNearestTarget( Unit *me )
{
    return NULL;     //THIS FUNCTION IS TOO SLOW__AND ECM SHOULD WORK DIFFERENTLY ANYHOW...WILL SAVE FIXING IT FOR LATER

    QVector pos( me->Position() );
    Unit   *un = NULL;
    Unit   *targ     = NULL;
    double  minrange = FLT_MAX;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
         ( un = (*i) );
         ++i) {
        if (un == me)
            continue;
        if (un->isUnit() != UNITPTR)
            continue;
        if (un->hull < 0)
            continue;
        if (UnitUtil::getFactionRelation( me, un ) >= 0)
            continue;
        double temp = (un->Position()-pos).Magnitude()-un->rSize();
        if (targ == NULL) {
            targ     = un;
            minrange = temp;
        } else if (temp < minrange) {
            targ = un;
        }
    }
    if (targ == NULL) {
        for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
             ( un = (*i) );
             ++i)
            if ( UnitUtil::isSun( un ) ) {
                targ = un;
                break;
            }
    }
    return targ;
}
void Missile::UpdatePhysics2( const Transformation &trans,
                              const Transformation &old_physical_state,
                              const Vector &accel,
                              float difficulty,
                              const Matrix &transmat,
                              const Vector &CumulativeVelocity,
                              bool ResolveLast,
                              UnitCollection *uc )
{
    Unit *targ;
    if ( ( targ = ( Unit::Target() ) ) ) {
        had_target = true;
        if (targ->hull < 0) {
            targ = NULL;
        } else {
            static size_t max_ecm = (size_t)XMLSupport::parse_int( vs_config->getVariable( "physics", "max_ecm", "4" ) );
            size_t missile_hash = ( (size_t) this ) / 16383;
            if ( (int)(missile_hash%max_ecm) < UnitUtil::getECM(targ)) {
                Target( NULL );                 //go wild
            } else if (hull > 0) {
                static unsigned int pointdef = ROLES::getRole( "POINTDEF" );
                targ->graphicOptions.missilelock = true;
                un_iter i = targ->getSubUnits();

                Unit   *su;
                for (; (su = *i) != NULL; ++i)
                    if (su->attackPreference() == pointdef) {
                        if (su->Target() == NULL) {
                            float speed, range, mrange;
                            su->getAverageGunSpeed( speed, range, mrange );
                            if ( ( Position()-su->Position() ).MagnitudeSquared() < range*range ) {
                                su->Target( this );
                                su->TargetTurret( this );
                            }
                        }
                    }
            }
        }
    }
    if (retarget == -1) {
        if (targ)
            retarget = 1;
        else
            retarget = 0;
    }
    if (retarget && targ == NULL)
        Target( NULL );          //BROKEN
    if ( had_target && !( Unit::Target() ) ) {
        static float max_lost_target_live_time =
            XMLSupport::parse_float( vs_config->getVariable( "physics", "max_lost_target_live_time", "30" ) );
        if (time > max_lost_target_live_time)
            time = max_lost_target_live_time;
    }
    Unit::UpdatePhysics2( trans, old_physical_state, accel, difficulty, transmat, CumulativeVelocity, ResolveLast, uc );
    this->time -= SIMULATION_ATOM;
    if (NULL != targ && !discharged) {
        QVector endpos = Position();
        QVector startpos = endpos-( SIMULATION_ATOM*GetVelocity() );
        float checker = targ->querySphere( startpos, endpos, rSize() );
        if ( checker && detonation_radius >= 0 ) {
            // Set position to the collision point
            SetPosition(startpos + (endpos - startpos) * checker);
        }
        if ( checker && detonation_radius >= 0 ) {
            //spiritplumber assumes that the missile is hitting a much larger object than itself
            static float percent_missile_match_target_velocity =
                XMLSupport::parse_float( vs_config->getVariable( "physics", "percent_missile_match_target_velocity", "1.0" ) );

            this->Velocity += percent_missile_match_target_velocity*(targ->Velocity-this->Velocity);

            BOOST_LOG_TRIVIAL(info)
                    << boost::format("Missile hit target %1% (time %2$.3f)")
                    % targ->name.get()
                    % time;
            Discharge();
            time = -1;
            //Vector norm;
            //float dist;
            /*** WARNING COLLISION STUFF... TO FIX FOR SERVER SIDE SOMEDAY ***
             *    if ((targ)->queryBoundingBox (Position(),detonation_radius+rSize())) {
             *    Discharge();
             *    time=-1;
             *    }
             */
        }
    }
    if (time < 0)
        DealDamageToHull( Vector( .1, .1, .1 ), hull+1 );
}

