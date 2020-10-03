#include "collision.h"

#include "unit_generic.h"
#include "universe.h"
#include "universe_util.h"
#include "game_config.h"
#include "missile.h"
#include "enhancement.h"
#include <typeinfo>
#include <boost/log/trivial.hpp>

// TODO: convert all float to double and all Vector to QVector.

static const float kilojoules_per_damage = GameConfig::GetVariable( "physics", "kilojoules_per_unit_damage", 5400 );
static const float collision_scale_factor =
        GameConfig::GetVariable( "physics", "collision_damage_scale", 1.0f );
static const float inelastic_scale = GameConfig::GetVariable( "physics", "inelastic_scale", 0.8f);
static const float min_time =
        GameConfig::GetVariable( "physics", "minimum_time_between_recorded_player_collisions", 0.1f);
static const float minimum_mass = 1e-6f;

//Collision force caps primarily for AI-AI collisions. Once the AIs get a real collision avoidance system, we can
// turn damage for AI-AI collisions back on, and then we can remove these caps.
//value, in seconds of desired maximum recovery time
static const float max_torque_multiplier =
        GameConfig::GetVariable( "physics", "maxCollisionTorqueMultiplier", 0.67f);
//value, in seconds of desired maximum recovery time
static const float max_force_multiplier  =
        GameConfig::GetVariable( "physics", "maxCollisionForceMultiplier", 5);

static const int upgrade_faction =
        GameConfig::GetVariable( "physics", "cargo_deals_collide_damage",
                                                        false) ? -1 : FactionUtil::GetUpgradeFaction();

static const float collision_hack_distance =
        GameConfig::GetVariable( "physics", "collision_avoidance_hack_distance", 10000);
static const float front_collision_hack_distance =
        GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_distance", 200000);

static float front_collision_hack_angle = cos( 3.1415926536f * GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_angle", 40)/180.0f );

static const bool collision_damage_to_ai = GameConfig::GetVariable( "physics", "collisionDamageToAI", false);

static const bool crash_dock_unit = GameConfig::GetVariable( "physics", "unit_collision_docks", false);

static const bool crash_dock_hangar = GameConfig::GetVariable( "physics", "only_hangar_collision_docks", false);

// Disabled bouncing missile option. Missiles always explode when colliding with something.
//static const bool does_missile_bounce = GameConfig::GetVariable( "physics", "missile_bounce", false);



Collision::Collision(Unit* unit, const QVector& location, const Vector& normal):
    unit(unit), location(location), normal(normal)
{
    cockpit = _Universe->isPlayerStarship( unit ); // smcp/thcp
    unit_type = unit->isUnit();
    is_player_ship = _Universe->isPlayerStarship(unit);
    mass = std::max(unit->GetMass(), minimum_mass);
    position = unit->Position();
    velocity = unit->GetVelocity();
}




// This function handles the initial reaction of the unit to hitting the other unit
// Note: I'm changing the expected behavior here
// Return value indicates whether to continue processing
/*UNITPTR,
PLANETPTR,
BUILDINGPTR,
NEBULAPTR,
ASTEROIDPTR,
ENHANCEMENTPTR,
MISSILEPTR*/
void Collision::shouldApplyForceAndDealDamage(Unit* other_unit)
{
    // Collision with a nebula does nothing
    if(other_unit->isUnit() == NEBULAPTR)
    {
        return;
    }

    // Collision with a enhancement improves your shield apparently
    if(other_unit->isUnit() == ENHANCEMENTPTR)
    {
        apply_force = true;
        return;
    }

    // Collision with a jump point does nothing
    if(other_unit->isJumppoint())
    {
        other_unit->jumpReactToCollision(unit);
        return;
    }

    if(unit->isJumppoint())
    {
        return;
    }

    switch(unit_type)
    {
    // Missiles and asteroids always explode on impact with anything except Nebula and Enhancement.
    case MISSILEPTR:
        // Missile should explode when killed
        // If not, uncomment this
        //((Missile*)unit)->Discharge();
        unit->Kill();
        return;

    case ASTEROIDPTR:
        unit->Kill();
        return;

    // Planets and Nebulas can't be killed right now
    case PLANETPTR:
    case NEBULAPTR:
        return;

    // Buildings should not calculate actual damage
    case BUILDINGPTR:
        return;

    // Units (ships) should calculate actual damage
    case UNITPTR:
        apply_force = true;
        deal_damage = true;
        return;

    // Not sure what an enhancement is, but it looks like it's something that can increase the shields of the unit it collided with.
    // TODO: refactor this.
    case ENHANCEMENTPTR:
        if (other_unit->isUnit() == ASTEROIDPTR)
        {
            apply_force = true;
            return;
        }

        double percent;
        char tempdata[sizeof(Shield)];
        memcpy( tempdata, &unit->shield, sizeof(Shield));
        unit->shield.number = 0;     //don't want them getting our boosted shields!
        unit->shield.shield2fb.front = unit->shield.shield2fb.back =
                unit->shield.shield2fb.frontmax = unit->shield.shield2fb.backmax = 0;
        other_unit->Upgrade( unit, 0, 0, true, true, percent );
        memcpy( &unit->shield, tempdata, sizeof (Shield) );
        string fn( unit->filename );
        string fac( FactionUtil::GetFaction( unit->faction ) );
        unit->Kill();
        _Universe->AccessCockpit()->savegame->AddUnitToSave( fn.c_str(), ENHANCEMENTPTR, fac.c_str(), reinterpret_cast<long>(unit));
        apply_force = true;
        return;
    }
}

// This stops the unit from going through the other unit
/*void Collision::applyForce(double elasticity, float& m2, Vector& v2)
{
    //for torque... location -- approximation hack of MR^2 for rotational inertia (moment of inertia currently just M)
    Vector torque = force/(unit->radial_size*unit->radial_size);
    Vector force  = this->force-torque; // Note the variable shadowing

    float  max_force  = max_force_multiplier*(unit->limits.forward + unit->limits.retro + unit->limits.lateral + unit->limits.vertical);
    float  max_torque = max_torque_multiplier*(unit->limits.yaw + unit->limits.pitch+unit->limits.roll);

    //Convert from frames to seconds, so that the specified value is meaningful
    max_force  = max_force / (unit->sim_atom_multiplier * simulation_atom_var);
    max_torque = max_torque / (unit->sim_atom_multiplier * simulation_atom_var);
    float torque_magnitude = torque.Magnitude();
    float force_magnitude = force.Magnitude();
    if (torque_magnitude > max_torque) {
        torque *= (max_torque/torque_magnitude);
    }
    if (force_magnitude > max_force) {
        force *= (max_force/force_magnitude);
    }
    unit->ApplyTorque( torque, location );
    unit->ApplyForce( force-torque );
}*/




void Collision::dealDamage(Collision other_collision, float factor)
{
    if(!deal_damage)
    {
        return;
    }

    // Shorten variables for better visibility
    float& m1 = mass;
    float& m2 = other_collision.mass;
    Vector& v1 = velocity;
    Vector& v2 = other_collision.velocity;

    float relative_velocity = (v1-v2).Magnitude();
    float damage = relative_velocity * relative_velocity *
            (m2)/(m1+m2) * factor;

    unit->ApplyDamage(other_collision.location.Cast(),
                      other_collision.normal,
                      damage, unit, GFXColor( 1,1,1,2 ), other_collision.unit->owner
                      != nullptr ? other_collision.unit->owner : this );
}

void Collision::applyForce(double elasticity, float& m2, Vector& v2)
{
    /*
    The current implementation is of an imperfectly elastic collision.
    An elastic collision is an encounter between two bodies in which the total kinetic energy of the two bodies remains the same. In an ideal, perfectly elastic collision, there is no net conversion of kinetic energy into other forms such as heat, noise, or potential energy.
    https://en.wikipedia.org/wiki/Elastic_collision

    This function adds an elasticity ratio to reduce velocity, due to heat, damage, etc.
    */

    // elasticity must be between 0 and 1
    if(elasticity >1 || elasticity <0)
    {
        BOOST_LOG_TRIVIAL(warning) << "Collision::applyForce Expected an elasticity value between 0 and 1 but got " << elasticity;
        elasticity = std::max(elasticity, 0.0);
        elasticity = std::min(elasticity, 1.0);
    }

    if(!apply_force)
    {
        return;
    }

    // Shorten variables for better visibility
    float& m1 = mass;
    Vector& v1 = velocity;

    // Perfectly elastic result
    Vector new_velocity = v1*(m1-m2)/(m1+m2) + v2*2*m2/(m1+m2);

    // Apply ratio
    new_velocity = new_velocity * elasticity;

    // Apply force
    // Disabling - it messes with the thrusters
    //unit->ApplyForce(new_velocity);
}


// Discussion - the original code ran this once for both units. This required a comprison of the units to find out which is smaller.
void Collision::collide( Unit* unit1,
                                   const QVector &location1,
                                   const Vector &normal1,
                                   Unit* unit2,
                                   const QVector &location2,
                                   const Vector &normal2,
                                   float distance)
{
    Collision collision1 = Collision(unit1, location1, normal1);
    Collision collision2 = Collision(unit2, location2, normal2);

    // TODO: Try "crash" landing both ways
    // See commit a66bdcfa1e00bf039183603913567d48e52c7a8e method crashLand for an example
    // If configured, the game should behave like privateer and land if close enough to a dock

    // TODO: Try "auto jump" both ways
    // See commit a66bdcfa1e00bf039183603913567d48e52c7a8e method Unit->jumpReactToCollision for an example
    // I assume this is for "always open" jump gates that you pass through

    collision1.shouldApplyForceAndDealDamage(unit2);
    collision2.shouldApplyForceAndDealDamage(unit1);

    float elasticity = 0.8f;
    collision1.applyForce(elasticity, collision2.mass, collision2.velocity);
    collision2.applyForce(elasticity, collision1.mass, collision1.velocity);

    collision1.dealDamage(collision2);
    collision2.dealDamage(collision1);

    // TODO: add torque
}
