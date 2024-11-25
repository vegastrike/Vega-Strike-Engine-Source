/*
 * drive_upgrade.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_DRIVE_UPGRADE_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_DRIVE_UPGRADE_H

#include "component.h"

class Drive;

/** A DriveUpgrade applies a modifier to Drive class.
 *  This is a unique use case from other components, as allowing a slow
 *  ship (e.g. cargo) to upgrade to a fighter engine would break both
 *  game balance and believability. 
 *  The game previously supported both additive and multiplicative upgrades.
 *  I've removed the additive one for simplicity's sake. 
 */
class DriveUpgrade : public Component {
public:
    Drive *drive;

// Limits
    //max ypr--both pos/neg are symmetrical
    double yaw;
    double pitch;
    double roll;

    //side-side engine thrust max
    double lateral;

    //vertical engine thrust max
    double vertical;

    //forward engine thrust max
    double forward;

    //reverse engine thrust max
    double retro;

    //after burner acceleration max
    double afterburn;

// Computer
    //Computers limitation of speed
    double speed;
    
    //Computer's restrictions of YPR to limit space combat maneuvers
    double max_yaw_left;
    double max_yaw_right;
    double max_pitch_down;
    double max_pitch_up;
    double max_roll_left;
    double max_roll_right;

    double fuel_consumption;

// Constructors
    DriveUpgrade(Drive *drive = nullptr);

    // Component Methods
    virtual void Load(std::string upgrade_key, 
                      std::string unit_key = "");      
    
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;

    virtual bool CanDowngrade() const;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_name) const;

    virtual bool Upgrade(const std::string upgrade_name);
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_DRIVE_UPGRADE_H
