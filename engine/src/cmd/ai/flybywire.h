/**
 * flybywire.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef _CMD_FLYBYWIRE_H_
#define _CMD_FLYBYWIRE_H_
#include "order.h"
#include "vs_globals.h"
namespace Orders {
/**
 * This class attempts to match a given
 * Linear velocity given a desired velocity
 * That velocity can be local ("all ahead full")
 * or in world space ("lets fire thrusters towards planet")
 */
class MatchLinearVelocity : public Order {
protected:
///werld space... generally r*speed or local space
    Vector desired_velocity;
///specified in Local or World coordinates
    bool LocalVelocity;
    bool willfinish;
    bool afterburn;
public:
    MatchLinearVelocity(const Vector &desired, bool Local, bool afterburner, bool fini = true) : Order(MOVEMENT,
                                                                                                       SLOCATION),
                                                                                                 desired_velocity(
                                                                                                         desired),
                                                                                                 LocalVelocity(Local),
                                                                                                 willfinish(fini),
                                                                                                 afterburn(afterburner)
    {
        done = false;
    }
    void Execute();
    void SetDesiredVelocity(const Vector &desired, bool Local)
    {
        desired_velocity = desired;
        LocalVelocity = Local;
    }
    void SetAfterburn(bool use_afterburn)
    {
        afterburn = use_afterburn;
    }
    virtual ~MatchLinearVelocity();
    virtual std::string getOrderDescription()
    {
        return "mlv";
    }
};

/**
 *  This class attempts to match given turning velocity through firing minimum ammt of thrusters
 */
class MatchAngularVelocity : public Order {
protected:
///werld space or local space (pitch = 1 on the x axis)
    Vector desired_ang_velocity;
///specified in Local or World coordinates
    bool LocalAng;
///Whether this script should terminate upon reaching desired angular velocity
    bool willfinish;
public:
    MatchAngularVelocity(const Vector &desired, bool Local, bool fini = true) : Order(FACING, SLOCATION),
                                                                                desired_ang_velocity(desired),
                                                                                LocalAng(Local),
                                                                                willfinish(fini)
    {
        done = false;
    }
    void Execute();
    void SetDesiredAngularVelocity(const Vector &desired, bool Local)
    {
        desired_ang_velocity = desired;
        LocalAng = Local;
    }
    virtual ~MatchAngularVelocity();
    virtual std::string getOrderDescription()
    {
        return "mav";
    }
};
class MatchRoll : public Order {
    float desired_roll;
    bool willfinish;

public:
    MatchRoll(float desired, bool finish) : Order(Order::CLOAKING, SLOCATION)
    {
        this->desired_roll = desired;
        this->willfinish = finish;
    }
    virtual void Execute();
    void SetRoll(float roll)
    {
        desired_roll = roll;
    }
    float GetRoll()
    {
        return desired_roll;
    }
    virtual std::string getOrderDescription()
    {
        return "roll";
    }
};
/**
 * This class matches both angular and linear velocity.
 * It cannot have multiple inheritance because
 * of the colliding virtual "Execute" functions, I believe
 * Use a #define to "share code" between them
 */
class MatchVelocity : public MatchAngularVelocity {
protected:
///werld space... generally r*speed or local space
    Vector desired_velocity;
///Is the above in world space?
    bool LocalVelocity;
    bool afterburn;
public:
    MatchVelocity(const Vector &desired,
                  const Vector &desired_ang,
                  const bool Local,
                  const bool afterburner,
                  const bool fini = true) : MatchAngularVelocity(desired_ang, Local, fini),
                                            desired_velocity(desired),
                                            LocalVelocity(Local),
                                            afterburn(afterburner)
    {
        type = FACING | MOVEMENT;
        subtype = SLOCATION;
    }
    void Execute();
    void SetDesiredVelocity(const Vector &desired, const bool Local)
    {
        desired_velocity = desired;
        LocalVelocity = Local;
    }
    void SetAfterburn(bool use_afterburn)
    {
        afterburn = use_afterburn;
    }
    virtual ~MatchVelocity();
    virtual std::string getOrderDescription()
    {
        return "mv";
    }
};
}
/**
 * This class uses a parent's computer struct
 * to set the appropriate desired linear
 * and angular velocity based on
 * what the user input may be
 */
class FlyByWire : public Orders::MatchVelocity {
protected:
    Vector DesiredShiftVelocity;
    Vector DirectThrust;
///If shelton slide, do not have the computer match linear
    bool sheltonslide;
    bool controltype; //false for car
    bool inertial_flight_model; //true doesn't match speed vector, but allows manual thrust.
    bool inertial_flight_enable; //overrides inertial_flight_model if false - useful for AI, you know.
    bool stolen_setspeed;
    float stolen_setspeed_value;
public:
    FlyByWire();
    ~FlyByWire();
    void SwitchFlightMode()
    {
        controltype = !controltype;
    }
///Turns on or off velocity resolution
    void ThrustRight(float percent);
    void ThrustUp(float percent);
    void ThrustFront(float percent);
    void DirectThrustRight(float percent); //Not disabled by shelton slide
    void DirectThrustUp(float percent); //Not disabled by shelton slide
    void DirectThrustFront(float percent); //Not disabled by shelton slide
    void SheltonSlide(bool onoff);
    void InertialFlight(bool onoff);
    bool InertialFlight() const;
    bool InertialFlightEnable() const;
///Stops... sets desired velocity to 0
    void Stop(float percentage);
///pass in the percentage of the turn they were turnin right.  -%age indicates left
    void Right(float percentage);
///pass in the percentage of the turn they were turning up
    void Up(float percentage);
    void RollRight(float percentage);
///Specifies match speed to use afterbuner and gives in higher velocity
    void Afterburn(float percentage);
///matches set_speed to this velocity's magnitude
    void MatchSpeed(const Vector &velocity);
///negative is decel... 0 = nothing
    void Accel(float percentage);
    void Execute();
    virtual std::string getOrderDescription()
    {
        return "wire";
    }
private:
    FlyByWire(const FlyByWire &);
    FlyByWire &operator=(const FlyByWire &);
};
#endif

