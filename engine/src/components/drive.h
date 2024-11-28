/*
 * drive.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_DRIVE_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_DRIVE_H

#include "component.h"
#include "energy_consumer.h"

#include "resource/resource.h"

class EnergyContainer;

class Drive : public Component, public EnergyConsumer {
    // TODO: implement damage so something will actually happen
    // Right now, damage is recorded in component superclass but game doesn't
    // take it into account.

    /* @discussion use of Resource class in Drive
        Unlike other use cases, where max, adjusted and value are all used, here
        we only use max and adjusted. This shouldn't be an issue as adjusted and 
        value should be identical unless someone sets value to less. */

    /* Afterburner was split from drive. Same comments apply there. */
public:
// Limits
    //max ypr--both pos/neg are symmetrical
    Resource<double> yaw;
    Resource<double> pitch;
    Resource<double> roll;

    //side-side engine thrust max
    Resource<double> lateral;

    //vertical engine thrust max
    Resource<double> vertical;

    //forward engine thrust max
    Resource<double> forward;

    //reverse engine thrust max
    Resource<double> retro;

// Computer
    //Computers limitation of speed during combat
    Resource<double> speed;

    //Computer's restrictions of YPR to limit space combat maneuvers
    Resource<double> max_yaw_left;
    Resource<double> max_yaw_right;
    Resource<double> max_pitch_down;
    Resource<double> max_pitch_up;
    Resource<double> max_roll_left;
    Resource<double> max_roll_right;

// Constructors
    Drive(EnergyContainer *source = nullptr);    

    // Component Methods
    virtual void Load(std::string upgrade_key, 
                      std::string unit_key = "");      
    
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;

    virtual bool CanDowngrade() const;
    virtual bool Downgrade();
    virtual bool CanUpgrade(const std::string upgrade_name) const;
    virtual bool Upgrade(const std::string upgrade_name);

    virtual void Damage();
    virtual void DamageByPercent(double percent);
    virtual void Repair();
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_DRIVE_H
