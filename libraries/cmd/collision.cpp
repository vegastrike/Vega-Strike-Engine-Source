/*
 * collision.cpp
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


#include "cmd/collision.h"

#include "cmd/unit_generic.h"
#include "src/universe.h"
#include "src/universe_util.h"
#include "configuration/configuration.h"
#include "cmd/missile.h"
#include "cmd/enhancement.h"
#include "cmd/damageable.h"

#include <typeinfo>
// #include <boost/log/trivial.hpp>
#include "vega_cast_utils.h"
#include "cmd/planet.h"
#include "src/vs_logging.h"

// TODO: convert all float to double and all Vector to QVector.

Collision::Collision(Unit *unit, const QVector &location, const Vector &normal) :
        unit(unit), location(location), normal(normal) {
    cockpit = _Universe->isPlayerStarship(unit); // smcp/thcp
    unit_type = unit->getUnitType();
    is_player_ship = _Universe->isPlayerStarship(unit);
    mass = (std::max)(unit->getMass(), static_cast<float>(configuration()->physics.minimum_mass_flt));
    position = unit->Position();
    velocity = unit->GetVelocity();
}

// This function handles the initial reaction of the unit to hitting the other unit
// Note: I'm (@royfalk) changing the expected behavior here
// Return value indicates whether to continue processing
/*Vega_UnitType::unit,
Vega_UnitType::planet,
Vega_UnitType::building,
Vega_UnitType::nebula,
Vega_UnitType::asteroid,
Vega_UnitType::enhancement,
Vega_UnitType::missile*/
void Collision::shouldApplyForceAndDealDamage(Unit* other_unit) {
    Vega_UnitType other_units_type = other_unit->getUnitType();

    // Collision with a nebula does nothing
    if (other_units_type == Vega_UnitType::nebula) {
        return;
    }

    // Collision with a enhancement improves your shield apparently
    if (other_units_type == Vega_UnitType::enhancement) {
        apply_force = true;
        return;
    }

    // Collision with a jump point does nothing
    if (other_unit->isJumppoint()) {
        other_unit->jumpReactToCollision(unit);
        return;
    }

    if (unit->isJumppoint()) {
        return;
    }

    switch (unit_type) {
    // Missiles and asteroids always explode on impact with anything except Nebula and Enhancement.
    case Vega_UnitType::missile:
        // Missile should explode when killed
        // If not, uncomment this
        //((Missile*)unit)->Discharge();
        unit->Kill();
        return;

    case Vega_UnitType::asteroid:
        apply_force = true;
        deal_damage = true;
        return;

    // Planets and Nebulas can't be killed right now
    case Vega_UnitType::planet:
    case Vega_UnitType::nebula:
        return;

    // Buildings should not calculate actual damage
    case Vega_UnitType::building:
        return;

    // Units (ships) should calculate actual damage
    case Vega_UnitType::unit:
        // Handle the "Nav 8" case
        if (other_units_type == Vega_UnitType::planet) {
#if defined(LOG_TIME_TAKEN_DETAILS)
            const double nav_8_start_time = realTime();
#endif
            const auto* as_planet = vega_dynamic_const_cast_ptr<const Planet>(other_unit);
            if (as_planet->is_nav_point()) {
                VS_LOG(debug, "Can't collide with a Nav Point");
#if defined(LOG_TIME_TAKEN_DETAILS)
                const double nav_8_end_time = realTime();
                VS_LOG(trace,
                       (boost::format("%1%: Time taken by handling Nav 8 case: %2%") % __FUNCTION__ % (nav_8_end_time -
                           nav_8_start_time)));
#endif
                return;
            }
#if defined(LOG_TIME_TAKEN_DETAILS)
            const double nav_8_end_time = realTime();
            VS_LOG(trace,
                   (boost::format("%1%: Time taken by handling Nav 8 case: %2%") % __FUNCTION__ % (nav_8_end_time -
                       nav_8_start_time)));
#endif
        }
        apply_force = true;
        deal_damage = true;
        return;

    // An enhancement upgrades the shields of the unit it collided with.
    // TODO: refactor this.
    case Vega_UnitType::enhancement:
        // We can't enhance rocks
        if (other_units_type == Vega_UnitType::asteroid ||
            other_units_type == Vega_UnitType::planet) {
            apply_force = true;
            return;
        }

        // disabled for now.
        // TODO: someone from the "product" team needs to define the
        // exact behavior. Preferably after we sort the upgrade
        // code.


        /*double percent;
        char tempdata[sizeof(Shield)];
        memcpy( tempdata, &unit->shield, sizeof(Shield));
        unit->shield.number = 0;     //don't want them getting our boosted shields!
        unit->shield.shield2fb.front = unit->shield.shield2fb.back =
                unit->shield.shield2fb.frontmax = unit->shield.shield2fb.backmax = 0;
        other_unit->Upgrade( unit, 0, 0, true, true, percent );
        memcpy( &unit->shield, tempdata, sizeof (Shield) );
        string fn( unit->filename );
        string fac( FactionUtil::GetFaction( unit->faction ) );*/
        unit->Kill();

        //_Universe->AccessCockpit()->savegame->AddUnitToSave( fn.c_str(), Vega_UnitType::enhancement, fac.c_str(), reinterpret_cast<long>(unit));
        apply_force = true;
        return;
    }
}

// Apply damage, in VS Damage units (a unit of energy), based on change in energy from collision outcome
void Collision::dealDamage(Collision other_collision, double deltaKE_linear, double deltaKE_angular) {
    if (!deal_damage) {
        return;
    }

    // deltaKE is in KiloJoules, due to mass being in units of 1000 Kg, not Kg -
    // so convert accordingly
    // assign half the change in energy to this unit, convert from KJ to VSD
    Damage damage(0.5 * (deltaKE_linear + deltaKE_angular) /
            configuration()->constants.kj_per_unit_damage);

    unit->ApplyDamage(other_collision.location.Cast(),
            other_collision.normal,
            damage, unit, GFXColor(1, 1, 1, 2), other_collision.unit->owner
                    != nullptr ? other_collision.unit->owner : this);
}

/*
* Attempt to work around interpenetration issues[KLUDGE]
* Specifically, impulse model for rigid body collisions assumes that, being rigid bodies, the two geometries have zero interpenetration,
* but VS physics granularity + object relative velocity means that non-trivial interpenetration is common
*/
void Collision::adjustInterpenetration(QVector &new_velocity, QVector &new_angular_velocity, const Vector &normal) {

    if (!apply_force) {
        return;
    }

    double magnitude = new_velocity.Magnitude();
    QVector velocity_norm =
            new_velocity.Normalize(); // want to perform positional displacement in direction of new velocity vector
    double movementKludgeMagnitude = 2 * (new_angular_velocity.Dot(velocity_norm) * unit->radial_size / PI + magnitude);
    QVector movementKludgeAmount = movementKludgeMagnitude * velocity_norm + abs(movementKludgeMagnitude) * normal;
    unit->SetPosition(unit->Position() + (movementKludgeAmount)
            * SIMULATION_ATOM); // move by one physics frame at F() resultant linear velocity + component of angular velocity along linear velocity vector + normal before applying force -- this is NOT REMOTELY correct, but will often be sufficient
}

// apply force and torque, enforce clamping
void Collision::applyForce(QVector &force, QVector &location_local) {
    if (!apply_force) {
        return;
    }

    /*
    Disabling clamping for the time being - will revisit this later; will be somewhat cleaner once moment of inertia implementation is upgraded.
    May also want to do clamping in a configurable fashion AI vs. player, etc.

    //Collision force caps primarily for AI-AI collisions. Once the AIs get a real collision avoidance system, we can
    // turn damage for AI-AI collisions back on, and then we can remove these caps.
    //value, in seconds of desired maximum recovery time



    //scale max applicable force by unit's ability to restore orientation and velocity, then convert from frames to seconds
    float  max_force = max_force_multiplier * (unit->limits.forward + unit->limits.retro + unit->limits.lateral + unit->limits.vertical) / (unit->sim_atom_multiplier * SIMULATION_ATOM);
    float  max_torque = max_torque_multiplier * (unit->limits.yaw + unit->limits.pitch + unit->limits.roll) / (unit->sim_atom_multiplier * SIMULATION_ATOM);
    float force_magnitude = force.Magnitude();
    if (force_magnitude > max_force) {
        force *= (max_force / force_magnitude);
    }
    if (torque_magnitude > max_torque) {
        torque *= (max_torque / torque_magnitude);
    }

    */

    // Apply force at location, should just be one call to ApplyLocalTorque.... but there's a catch - our current moment of inertia units(as in SI units, not unit class) are a bit b0rked, so we need to do a workaround :-(
    Vector torqueForce = force * (1 / (0.667 * unit->radial_size * unit->radial_size)); // use shell approximation
    Vector remainingLinearforce = force - torqueForce;
    unit->ApplyLocalTorque(torqueForce, location_local);
    unit->ApplyForce(remainingLinearforce);
}

/*
* Correctness check to validate collision math - should be eventually moved to testing, rather than on critical path
*/
void Collision::validateCollision(const QVector &relative_velocity,
        const Vector &normal1,
        const QVector &location1_local,
        const QVector &location2_local,
        const QVector &v1_new,
        const QVector &v2_new,
        const QVector &w1_new,
        const QVector &w2_new) {
    //following two values should be identical. Due to FP math, should be nearly identical. Checks for both absolute and relative error, the former to shield slightly larger errors on very small values - can set tighter/looser bounds later
    double RestorativeVelAlongNormal =
            -1.0 * (1.0 - configuration()->physics.inelastic_scale_dbl) * relative_velocity.Dot(normal1);
    double ResultantVelAlongNormal =
            ((v2_new + w2_new.Cross(location2_local)) - (v1_new + w1_new.Cross(location1_local))).Dot(normal1);
    double normalizedError = abs(1.0 - ResultantVelAlongNormal / RestorativeVelAlongNormal);
    double absoluteError = abs(ResultantVelAlongNormal - RestorativeVelAlongNormal);
    // absolute error > 1 milimeter/second along collision normal AND normalized error > 1%
    if (absoluteError > 0.001 && normalizedError > 0.01) {
        VS_LOG(warning,
                (boost::format(
                        "Computed error between equation sides for collision is beyond acceptable error bounds:%1%:%2%:%3%:%4%")
                        % absoluteError % normalizedError % ResultantVelAlongNormal % RestorativeVelAlongNormal));
    }
}

// Discussion - the original code ran this once for both units. This required a comparison of the units to find out which is smaller.
// The history on this is as follows - one of the normals is picked to compute the force along; while this is arbitrary in the ideal case, given interpenetration, we use the heuristic that the larger object's normal is likely to better characterize the collision angles
void Collision::collide(Unit *unit1,
        const QVector &location1,
        const Vector &normal1,
        Unit *unit2,
        const QVector &location2,
        const Vector &normal2,
        float distance) {

    Collision collision1 = Collision(unit1, location1, normal1);
    Collision collision2 = Collision(unit2, location2, normal2);

    // TODO: Try "crash" landing both ways
    // See commit a66bdcfa1e00bf039183603913567d48e52c7a8e method crashLand for an example
    // If configured, the game should behave like privateer and land if close enough to a dock

    // Try "auto jump" if either unit is a jump point
    if (unit1->isJumppoint()) {
        VS_LOG(debug, "unit1->jumpReactToCollision(unit2) being called");
        unit1->jumpReactToCollision(unit2);
        return;
    } else if (unit2->isJumppoint()) {
        VS_LOG(debug, "unit2->jumpReactToCollision(unit1) being called");
        unit2->jumpReactToCollision(unit1);
        return;
    }

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
    QVector velocity_of_contact_point1 = unit1->GetVelocity() + unit1->GetAngularVelocity()
            .Cross(location1_local); // compute velocity of point of contact for unit 1 - note that "location1_local" is a relative vector from unit1's coordinate system, with origin assumed to be at center of mass. Placing the origin off center is considered a data set error
    QVector velocity_of_contact_point2 = unit2->GetVelocity()
            + unit2->GetAngularVelocity().Cross(location2_local); // compute velocity of point of contact for unit 2
    QVector relative_velocity = velocity_of_contact_point2 - velocity_of_contact_point1;
    // CAVEAT - next two variables (I1, I2) should be 3x3 matrices; using current data set (scalar) and assuming hollow shell (0.667 MR^2); avoiding divide by zero in case of data set omissions -- should probably just put some asserts here?
    // 2/3 constant from shell approximation -- this will disappear when moment of inertia is actually turned into a 3x3 matrix OR getter is adjusted to fix dataside issues
    // should assert: mass >0; radial_size !=0; moment !=0; -- may require data set cleaning if asserted
    double I1 =
            std::max(static_cast<double>(unit1->GetMoment()), configuration()->physics.minimum_mass_dbl) * unit1->radial_size * unit1->radial_size
                    * 0.667; // deriving scalar moment of inertia for unit 1
    double I2 =
            std::max(static_cast<double>(unit2->GetMoment()), configuration()->physics.minimum_mass_dbl) * unit2->radial_size * unit2->radial_size
                    * 0.667; // deriving scalar moment of inertia for unit 2
    double I1_inverse = 1.0
            / I1; // Matrix inverse for matrix version of momentof inertia I1 is computable, but probably still better to have precomputed and fetched -- not an issue when I is still scalar
    double I2_inverse = 1.0 / I2;
    double mass1 = std::max(static_cast<double>(unit1->GetMass()),
            configuration()->physics
                    .minimum_mass_dbl); // avoid subsequent divides by 0 - again, should probably just check all the invariants and yell at the dataset with an assert here OR change the getter for mass and moment and [add getter for] radial_size that do the data cleaning/logging/yelling
    double mass2 = std::max(static_cast<double>(unit2->GetMass()), configuration()->physics.minimum_mass_dbl); // avoid subsequent divides by 0
    double mass1_inverse = 1.0 / mass1;
    double mass2_inverse = 1.0 / mass2;
    // impulse magnitude, as per equation 5 in wiki -- note that  e = 1 - inelastic_scale, so 1+e = 1 + 1 -inelastic_scale = 2 - inelastic_scale
    double impulse_magnitude = -1 * ((2 - configuration()->physics.inelastic_scale_dbl) * relative_velocity).Dot(normal1) /
            (mass1_inverse + mass2_inverse +
                    ((I1_inverse * (location1_local.Cross(normal1))).Cross(location1_local)
                            + (I2_inverse * (location2_local.Cross(normal1))).Cross(location2_local)).Dot(normal1)
            );
    QVector impulse = impulse_magnitude * normal1;

    QVector v1_new = unit1->GetVelocity() - impulse * mass1_inverse;
    QVector v2_new = unit2->GetVelocity() + impulse * mass2_inverse; // difference in sign is intentional and important
    QVector w1_new = unit1->GetAngularVelocity() - impulse_magnitude * I1_inverse * (location1_local.Cross(normal1));
    QVector w2_new = unit2->GetAngularVelocity() + impulse_magnitude * I2_inverse
            * (location2_local.Cross(normal1)); // again, repeated use of normal1 and lack of use of normal2 is intentional; difference in sign is intentional and important

    /*
    * Can factor the following out into testing code later
    * BEGIN TESTING CODE
    */
    validateCollision(relative_velocity, normal1, location1_local, location2_local, v1_new, v2_new, w1_new, w2_new);
    /*
    * END TESTING CODE
    */

    // Kinetic energy including rotational, from https://en.wikipedia.org/wiki/Moment_of_inertia#Kinetic_energy_2
    double kinetic_energy_system_initial_linear = 0.5 * mass1 * unit1->GetVelocity().MagnitudeSquared()
            + 0.5 * mass2 * unit2->GetVelocity().MagnitudeSquared();
    double kinetic_energy_system_initial_angular =
            0.5 * unit1->GetAngularVelocity().Dot(I1 * unit1->GetAngularVelocity())
                    + 0.5 * unit2->GetAngularVelocity().Dot(I2 * unit2->GetAngularVelocity());

    // compute changes in kinetic energy for linear and rotational
    double kinetic_energy_system_linear =
            0.5 * mass1 * v1_new.MagnitudeSquared() + 0.5 * mass2 * v2_new.MagnitudeSquared();
    double kinetic_energy_system_angular = (0.5 * w1_new.Dot(I1 * w1_new)) + (0.5 * w2_new.Dot(I2 * w2_new));
    double delta_kinetic_energy_linear = kinetic_energy_system_initial_linear - kinetic_energy_system_linear;
    double delta_kinetic_energy_angular = kinetic_energy_system_initial_angular - kinetic_energy_system_angular;

    //we have an impulse [ Force * Time ] and our physics interfaces operate on forces and torques [Force * Meters]
    QVector force_on_1 = -1 * impulse * (1.0f / (unit1->sim_atom_multiplier
            * SIMULATION_ATOM)); // divide impulse by time over which it will be applied to derive force -- allow different units to have different physics fidelity
    QVector force_on_2 = impulse * (1.0f / (unit2->sim_atom_multiplier
            * SIMULATION_ATOM)); // divide impulse by time over which it will be applied to derive force

    collision1.adjustInterpenetration(v1_new, w1_new, normal1); // Call this BEFORE applying force
    collision1.applyForce(force_on_1, location1_local); // handles both force-on-center-of-mass and torque
    collision2.adjustInterpenetration(v2_new, w2_new, normal1); // uses normal1, not normal2
    collision2.applyForce(force_on_2, location2_local);

    collision1.dealDamage(collision2, delta_kinetic_energy_linear, delta_kinetic_energy_angular);
    collision2.dealDamage(collision1, delta_kinetic_energy_linear, delta_kinetic_energy_angular);
}
