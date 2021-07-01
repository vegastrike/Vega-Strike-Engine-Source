/**
 * collision.cpp
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

static float kilojoules_per_damage = GameConfig::GetVariable( "physics", "kilojoules_per_unit_damage", 5400 );
static float collision_scale_factor =
        GameConfig::GetVariable( "physics", "collision_damage_scale", 1.0f );
static float inelastic_scale = GameConfig::GetVariable( "physics", "inelastic_scale", 0.2f); // THIS DOES NOT SEEM TO BE LOADING THE VARIABLE FROM THE CONFIG FILE?!?
static float min_time =
        GameConfig::GetVariable( "physics", "minimum_time_between_recorded_player_collisions", 0.1f);
static const float minimum_mass = 1e-6f;

//Collision force caps primarily for AI-AI collisions. Once the AIs get a real collision avoidance system, we can
// turn damage for AI-AI collisions back on, and then we can remove these caps.
//value, in seconds of desired maximum recovery time
static float max_torque_multiplier =
        GameConfig::GetVariable( "physics", "maxCollisionTorqueMultiplier", 0.67f);
//value, in seconds of desired maximum recovery time
static float max_force_multiplier  =
        GameConfig::GetVariable( "physics", "maxCollisionForceMultiplier", 5);

static int upgrade_faction =
        GameConfig::GetVariable( "physics", "cargo_deals_collide_damage",
                                                        false) ? -1 : FactionUtil::GetUpgradeFaction();

static float collision_hack_distance =
        GameConfig::GetVariable( "physics", "collision_avoidance_hack_distance", 10000);
static float front_collision_hack_distance =
        GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_distance", 200000);

static float front_collision_hack_angle = cos( 3.1415926536f * GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_angle", 40)/180.0f );

static bool collision_damage_to_ai = GameConfig::GetVariable( "physics", "collisionDamageToAI", false);

static bool crash_dock_unit = GameConfig::GetVariable( "physics", "unit_collision_docks", false);

static bool crash_dock_hangar = GameConfig::GetVariable( "physics", "only_hangar_collision_docks", false);

// Disabled bouncing missile option. Missiles always explode when colliding with something.
//static bool does_missile_bounce = GameConfig::GetVariable( "physics", "missile_bounce", false);



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
/*_UnitType::unit,
_UnitType::planet,
_UnitType::building,
_UnitType::nebula,
_UnitType::asteroid,
_UnitType::enhancement,
_UnitType::missile*/
void Collision::shouldApplyForceAndDealDamage(Unit* other_unit)
{
    // Collision with a nebula does nothing
    if(other_unit->isUnit() == _UnitType::nebula)
    {
        return;
    }

    // Collision with a enhancement improves your shield apparently
    if(other_unit->isUnit() == _UnitType::enhancement)
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
    case _UnitType::missile:
        // Missile should explode when killed
        // If not, uncomment this
        //((Missile*)unit)->Discharge();
        unit->Kill();
        return;

    case _UnitType::asteroid:
        apply_force = true;
        deal_damage = true;
        return;

    // Planets and Nebulas can't be killed right now
    case _UnitType::planet:
    case _UnitType::nebula:
        return;

    // Buildings should not calculate actual damage
    case _UnitType::building:
        return;

    // Units (ships) should calculate actual damage
    case _UnitType::unit:
        apply_force = true;
        deal_damage = true;
        return;

    // Not sure what an enhancement is, but it looks like it's something that can increase the shields of the unit it collided with.
    // TODO: refactor this.
    case _UnitType::enhancement:
        if (other_unit->isUnit() == _UnitType::asteroid)
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

        _Universe->AccessCockpit()->savegame->AddUnitToSave( fn.c_str(), _UnitType::enhancement, fac.c_str(), reinterpret_cast<long>(unit));
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




void Collision::dealDamage(Collision other_collision, double deltaKE_linear, double deltaKE_angular)
{
    if(!deal_damage)
    {
        return;
    }

    float damage = 0.5 * ( deltaKE_linear + deltaKE_angular) /* deltaKE is in KiloJoules, due to mass being in units of 1000 Kg, not Kg - so convert accordingly */ / kilojoules_per_damage; // assign half the change in energy to this unit, convert from KJ to VSD
    
    unit->ApplyDamage(other_collision.location.Cast(),
                      other_collision.normal,
                      damage, unit, GFXColor( 1,1,1,2 ), other_collision.unit->owner
                      != nullptr ? other_collision.unit->owner : this );
}

void Collision::applyForce(QVector &force, QVector &location_local, QVector &new_velocity, QVector& new_angular_velocity, const Vector& normal)
{
    
    if(!apply_force)
    {
        return;
    }
    //Attempt to work around interpenetration issues [KLUDGE]
    double magnitude = new_velocity.Magnitude();
    QVector velocity_norm = new_velocity.Normalize();
    QVector movementKludgeAmount = 2 * (new_angular_velocity.Dot(velocity_norm)*unit->radial_size / PI + magnitude) *velocity_norm;
    unit->SetPosition(unit->Position() + (movementKludgeAmount) * SIMULATION_ATOM); // move by one physics frame at F() resultant linear velocity + component of angular velocity along linear velocity vector before applying force -- this is NOT REMOTELY correct, but will often be sufficient

    // Apply force at location, should just be one call to ApplyLocalTorque.... but there's a catch - our current moment of inertia units(as in SI units, not unit class) are a bit b0rked, so we need to do a workaround :-(
    Vector torqueForce = force * (1 / (0.667 * unit->radial_size * unit->radial_size)); // use shell approximation
    Vector remainingLinearforce = force - torqueForce;
    unit->ApplyLocalTorque(torqueForce, location_local);
    unit->ApplyForce(remainingLinearforce);
}


// Discussion - the original code ran this once for both units. This required a comparison of the units to find out which is smaller.
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

    // The below code for impulse calculation is placed here for minimum disturbance of interfaces, not because it is a good location/organization

    /* Using impulse calculation from https://en.wikipedia.org/wiki/Collision_response#Impulse-based_reaction_model
    *  Without loss of generality, we will always use the normal for the larger object (normal1) in computing the impulse value [ in the common case, the bigger object has bigger surfaces that you're hitting with clearer normals, but it should work either way ]
    * Requires:
    * 1. Computing the impulse magnitude j in terms of v_relative(includes angular contribution), mass1, mass2, I1, I2, local contact point1 (r1 in wiki), local contact point 2 (r2 in wiki), normal1 and elasticity [also, unit_radius1 and unit_radius2, see caveats below]
    * 1a: Caveats -     i. I1 and I2 should be 3x3 matrices, currently, they are scalars [equivalent to the following two assumptions - I matrices are both diagonal and uniform in value, which is true for some simple shape approximations of moment of inertia]
    *                   ii. Currently, the value for I in the dataset is always equal to M. Below, we will adjust this using the hollow sphere model to incorporate the unit's radius.
    *                   iii. Whenever we get around to using actual moment of inertia matrices, we will need to change this to adjust the math operators, but not the logic
    *                   iv. Computing I^-1 for the 3x3 MoI is modestly computationally intensive compared to other operations. If we switch from scalar to matrix, I recommend pre-computing the inverse of moment of inertia, as it will rarely change (presumably, only when subunits or their future equivalent are altered)
    * 2. Compute impulse vector = j * normal1
    * 3. Compute new linear and angular velocities
    * 4. Compute changes in kinetic energy deltaE_k1 and deltaE_k2 (for damage )
    */

    // impulse calculations:
    QVector location1_local = location1 - unit1->Position(); // vector from center of mass to contact point for unit1
    QVector location2_local = location2 - unit2->Position(); // vector from center of mass to contact point for unit2
    QVector velocity_of_contact_point1 = unit1->GetVelocity() + unit1->GetAngularVelocity().Cross(location1_local); // compute velocity of point of contact for unit 1 - note that "location1_local" is a relative vector from unit1's coordinate system, with origin assumed to be at center of mass. Placing the origin off center is considered a data set error
    QVector velocity_of_contact_point2 = unit2->GetVelocity() + unit2->GetAngularVelocity().Cross(location2_local); // compute velocity of point of contact for unit 2
    QVector relative_velocity = velocity_of_contact_point2 - velocity_of_contact_point1;
    // CAVEAT - next two variables (I1, I2) should be 3x3 matrices; using current data set (scalar) and assuming hollow shell (0.667 MR^2); avoiding divide by zero in case of data set omissions -- should probably just put some asserts here?
    // 2/3 constant from shell approximation -- this will disappear when moment of inertia is actually turned into a 3x3 matrix OR getter is adjusted to fix dataside issues 
    // should assert: mass >0; radial_size !=0; moment !=0; -- may require data set cleaning if asserted
    double I1 = std::max(unit1->GetMoment(), minimum_mass) * unit1->radial_size * unit1->radial_size * 0.667; // deriving scalar moment of inertia for unit 1
    double I2 = std::max(unit2->GetMoment(), minimum_mass) * unit2->radial_size * unit2->radial_size * 0.667; // deriving scalar moment of inertia for unit 2
    double I1_inverse = 1 / I1; // Matrix inverse for matrix version of momentof inertia I1 is computable, but probably still better to have precomputed and fetched -- not an issue when I is still scalar
    double I2_inverse = 1 / I2;
    double mass1 = std::max(unit1->GetMass(), minimum_mass); // avoid subsequent divides by 0 - again, should probably just check all the invariants and yell at the dataset with an assert here OR change the getter for mass and moment and [add getter for] radial_size that do the data cleaning/logging/yelling
    double mass2 = std::max(unit2->GetMass(), minimum_mass); // avoid subsequent divides by 0
    double mass1_inverse = 1 / mass1;
    double mass2_inverse = 1 / mass2;
    // impulse magnitude, as per equation 5 in wiki -- note that  e = 1 - inelastic_scale, so 1+e = 1 + 1 -inelastic_scale = 2 - inelastic_scale
    double impulse_magnitude = -1 * ((2 - inelastic_scale) * relative_velocity).Dot(normal1) /
        (mass1_inverse + mass2_inverse +
            ((I1_inverse * (location1_local.Cross(normal1))).Cross(location1_local) + (I2_inverse * (location2_local.Cross(normal1))).Cross(location2_local)).Dot(normal1)
            );
    QVector impulse = impulse_magnitude * normal1;

    QVector v1_new = unit1->GetVelocity() - impulse * mass1_inverse;
    QVector v2_new = unit2->GetVelocity() + impulse * mass2_inverse; // difference in sign is intentional and important
    QVector w1_new = unit1->GetAngularVelocity() - impulse_magnitude * I1_inverse * (location1_local.Cross(normal1));
    QVector w2_new = unit2->GetAngularVelocity() + impulse_magnitude * I2_inverse * (location2_local.Cross(normal1)); // again, repeated use of normal1 and lack of use of normal2 is intentional; difference in sign is intentional and important

    /* Should be able to run the following, since they should be equal, but it's asserting occasionally O_o  -- but only sometimes
    * // assertion check for conservation of momentum (equation 4)
    * double RestorativeVelAlongNormal = -1 * (1 - inelastic_scale) * relative_velocity.Dot(normal1);
    * double ResultantVelAlongNormal = ((v2_new + w2_new.Cross(location2_local)) - (v1_new + w1_new.Cross(location1_local))).Dot(normal1);
    * BOOST_ASSERT_MSG((ResultantVelAlongNormal > 0.95 * RestorativeVelAlongNormal) && (ResultantVelAlongNormal < 1.05 * RestorativeVelAlongNormal), "Computed error between equation sides for collision is beyond acceptable error bounds");
    */

    // Kinetic energy including rotational, from https://en.wikipedia.org/wiki/Moment_of_inertia#Kinetic_energy_2 -- kinetic energy should probably be refactored to be a [new method] getKineticEnergy on units OR a helper function
    double kinetic_energy_system_initial_linear = 0.5 * mass1 * unit1->GetVelocity().MagnitudeSquared() + 0.5 * mass2 * unit2->GetVelocity().MagnitudeSquared();
    double kinetic_energy_system_initial_angular = 0.5 * unit1->GetAngularVelocity().Dot(I1 * unit1->GetAngularVelocity()) + 0.5 * unit2->GetAngularVelocity().Dot(I2 * unit2->GetAngularVelocity());
    
    // compute changes in kinetic energy for linear and rotational
    double kinetic_energy_system_linear = 0.5 * mass1 * v1_new.MagnitudeSquared() + 0.5 * mass2 * v2_new.MagnitudeSquared();
    double kinetic_energy_system_angular = (0.5 * w1_new.Dot(I1 * w1_new)) + ( 0.5 * w2_new.Dot(I2 * w2_new));
    double delta_kinetic_energy_linear = kinetic_energy_system_initial_linear - kinetic_energy_system_linear;
    double delta_kinetic_energy_angular = kinetic_energy_system_initial_angular - kinetic_energy_system_angular;
    

    //we have an impulse [ Force * Time ] and our physics interfaces operate on forces and torques [Force * Meters]
    QVector force_on_1 = -1 * impulse * (1.0f / (unit1->sim_atom_multiplier * SIMULATION_ATOM)); // divide impulse by time over which it will be applied to derive force -- allow different units to have different physics fidelity
    QVector force_on_2 = impulse * (1.0f / (unit2->sim_atom_multiplier * SIMULATION_ATOM)); // divide impulse by time over which it will be applied to derive force 


    collision1.applyForce(force_on_1, location1_local, v1_new, w1_new, normal1);
    collision2.applyForce(force_on_2, location2_local, v2_new, w2_new, normal1);

    collision1.dealDamage(collision2, delta_kinetic_energy_linear, delta_kinetic_energy_angular);
    collision2.dealDamage(collision1, delta_kinetic_energy_linear, delta_kinetic_energy_angular);

    // TODO: add torque
}
