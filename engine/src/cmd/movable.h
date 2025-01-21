/*
 * movable.h
 *
 * Copyright (C) 2020-2023 Daniel Horn, Roy Falk, Stephen G. Tuggy, Benjamen R. Meyer,
 * ministerofinformation, and other Vega Strike contributors
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
#ifndef VEGA_STRIKE_ENGINE_CMD_MOVABLE_H
#define VEGA_STRIKE_ENGINE_CMD_MOVABLE_H

#include "gfx/vec.h"
#include "gfx/quaternion.h"
#include "star_system.h"

#include <cfloat>

struct Transformation;
class Matrix;
class Unit;
class UnitCollection;
struct Quaternion;

class Movable {

protected:

public:
    //mass of this unit (may change with cargo)
    // TODO: subclass with return Mass+fuel;
    float Mass;

    float getMass() {
        return Mass;
    }

    float getMass() const {
        return Mass;
    }

    void setMass(float mass) {
        Mass = mass;
    }

    // Fields
    //The velocity this unit has in World Space
    Vector cumulative_velocity;
    //The force applied from outside accrued over the whole physics frame
    Vector NetForce;
    //The force applied by internal objects (thrusters)
    Vector NetLocalForce;
    //The torque applied from outside objects
    Vector NetTorque;
    //The torque applied from internal objects
    Vector NetLocalTorque;
    //the current velocities in LOCAL space (not world space)
    Vector AngularVelocity;
    Vector Velocity;

    // TODO: move enum to dockable class
    enum DOCKENUM { NOT_DOCKED = 0x0, DOCKED_INSIDE = 0x1, DOCKED = 0x2, DOCKING_UNITS = 0x4 };

    //The previous state in last physics frame to interpolate within
    Transformation prev_physical_state;
    //The state of the current physics frame to interpolate within
    Transformation curr_physical_state;

    //Should we resolve forces on this unit (is it free to fly or in orbit)
    // TODO: this should be deleted when we separate satelites from movables
    bool resolveforces;

    //The number of frames ahead this was put in the simulation queue
    unsigned int sim_atom_multiplier;
    //The number of frames ahead this is predicted to be scheduled in the next scheduling round
    unsigned int predicted_priority;
    //When will physical simulation occur
    unsigned int cur_sim_queue_slot;
    //Used with subunit scheduling, to avoid the complex ickiness of having to synchronize scattered slots
    unsigned int last_processed_sqs;

    // TODO: this should go up to ship
    unsigned char docked;

    //The cumulative (incl subunits parents' transformation)
    Matrix cumulative_transformation_matrix;
    //The cumulative (incl subunits parents' transformation)
    Transformation cumulative_transformation;

    Vector corner_min;
    Vector corner_max;
    //How big is this unit
    float radial_size;

    struct graphic_options {
        // TODO: Most of these should migrate to ftl_drive
        unsigned SubUnit;
        unsigned RecurseIntoSubUnitsOnCollision;
        unsigned missilelock;
        unsigned FaceCamera;
        unsigned Animating;
        
        unsigned WarpRamping;
        unsigned unused1;
        unsigned NoDamageParticles;
        unsigned specInterdictionOnline;
        unsigned char NumAnimationPoints;
        float WarpFieldStrength = 1;
        float RampCounter;
        float MinWarpMultiplier;
        float MaxWarpMultiplier;
        graphic_options();
    } graphicOptions;
protected:
    //Moment of intertia of this unit
    float Momentofinertia; // Was 0 but Init says 0.01
    Vector SavedAccel;
    Vector SavedAngAccel;
    float cutsqr{0.0F};
    float outcutsqr{0.0F};
    float air_res_coef{0.0F};
    float lateral_air_res_coef{0.0F};

// Methods

public:
    Movable();

protected:
    // forbidden
    Movable(const Movable &) = delete;
    // forbidden
    Movable &operator=(const Movable &) = delete;
    virtual ~Movable() = default;

public:
    void AddVelocity(float difficulty);
//Resolves forces of given unit on a physics frame
    virtual Vector ResolveForces(const Transformation &, const Matrix &);

    //Sets the unit-space position
    void SetPosition(const QVector &pos);

//Returns the pqr oritnattion of the unit in world space
    void SetOrientation(QVector q, QVector r);
    void SetOrientation(Quaternion Q);
    void SetOrientation(QVector p, QVector q, QVector r);
    void GetOrientation(Vector &p, Vector &q, Vector &r) const;
    Vector GetNetAcceleration() const;
    Vector GetNetAngularAcceleration() const;

//acceleration, retrieved from NetForce - not stable (partial during simulation), use GetAcceleration()
    Vector GetAcceleration() const {
        return SavedAccel;
    }

    Vector GetAngularAcceleration() const {
        return SavedAngAccel;
    }

//acceleration, stable over the simulation
    float GetMaxAccelerationInDirectionOf(const Vector &ref, bool afterburn) const;
//Transforms a orientation vector Up a coordinate level. Does not take position into account
    Vector UpCoordinateLevel(const Vector &v) const;
//Transforms a orientation vector Down a coordinate level. Does not take position into account
    Vector DownCoordinateLevel(const Vector &v) const;
//Transforms a orientation vector from world space to local space. Does not take position into account
    Vector ToLocalCoordinates(const Vector &v) const;
//Transforms a orientation vector to world space. Does not take position into account
    Vector ToWorldCoordinates(const Vector &v) const;

    virtual bool isPlayerShip() {
        return false;
    };

    //Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
    //Not needed here, so only in NetUnit and Unit classes
    void UpdatePhysics(const Transformation &trans,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc,
            Unit *superunit);
    virtual void UpdatePhysics2(const Transformation &trans,
            const Transformation &old_physical_state,
            const Vector &accel,
            float difficulty,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc = NULL);

    //Returns unit-space ang velocity
    const Vector &GetAngularVelocity() const {
        return AngularVelocity;
    }

    //Return unit-space velocity
    const Vector &GetVelocity() const {
        return cumulative_velocity;
    }

    void SetVelocity(const Vector &);
    void SetAngularVelocity(const Vector &);

    float GetMoment() const {
        return Momentofinertia; // TODO: subclass with return Momentofinertia+fuel;
    }

    float GetMass() const {
        return Mass; // TODO: subclass with return Mass+fuel;
    }

    //Sets if forces should resolve on this unit or not
    void SetResolveForces(bool);

    double GetMaxWarpFieldStrength(float rampmult = 1.f) const;
    void DecreaseWarpEnergy(bool insystem, float time = 1.0f);
    void IncreaseWarpEnergy(bool insystem, float time = 1.0f);
    //Rotates about the axis
    void Rotate(const Vector &axis);

    virtual QVector realPosition() = 0;
    virtual void UpdatePhysics3(const Transformation &trans,
            const Matrix &transmat,
            bool lastframe,
            UnitCollection *uc,
            Unit *superunit) = 0;

    /**
     * Fire engine takes a unit vector for direction
     * and how fast the fuel speed and mass coming out are
     */
    /*unit vector... might default to "r"*/
    void FireEngines(const Vector &Direction, float FuelSpeed, float FMass);
    //applies a force for the whole gameturn upon the center of mass
    void ApplyForce(const Vector &Vforce);
    //applies a force for the whole gameturn upon the center of mass, using local coordinates
    void ApplyLocalForce(const Vector &Vforce);
    //applies a force that is multipled by the mass of the ship
    void Accelerate(const Vector &Vforce);
    //Apply a torque in world level coords
    void ApplyTorque(const Vector &Vforce, const QVector &Location);
    //Applies a torque in local level coordinates
    void ApplyLocalTorque(const Vector &Vforce, const Vector &Location);
    //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
    void ApplyBalancedLocalTorque(const Vector &Vforce, const Vector &Location);

    //convenient shortcut to applying torques with vector and position
    void ApplyLocalTorque(const Vector &torque);


    //Clamps thrust to the limits struct
    Vector ClampThrust(const Vector &thrust, bool afterburn);
    //Takes a unit vector for direction of thrust and scales to limits
    Vector MaxThrust(const Vector &thrust);
    //Thrusts by ammt and clamps accordingly (afterburn or not)
    virtual void Thrust(const Vector &amt, bool afterburn = false);
    //Applies lateral thrust
    void LateralThrust(float amt);
    //Applies vertical thrust
    void VerticalThrust(float amt);
    //Applies forward thrust
    void LongitudinalThrust(float amt);
    //Clamps desired velocity to computer set limits
    Vector ClampVelocity(const Vector &velocity, const bool afterburn);
    //Clamps desired angular velocity to computer set limits
    Vector ClampAngVel(const Vector &vel);
    //Clamps desired torque to computer set limits of thrusters
    Vector ClampTorque(const Vector &torque);
    //scales unit size torque to limits in that direction
    Vector MaxTorque(const Vector &torque);
    //Applies a yaw of amt
    void YawTorque(float amt);
    //Applies a pitch of amt
    void PitchTorque(float amt);
    //Applies a roll of amt
    void RollTorque(float amt);

    double MaxSpeed() const;
    double MaxAfterburnerSpeed() const;
};

#endif //VEGA_STRIKE_ENGINE_CMD_MOVABLE_H
