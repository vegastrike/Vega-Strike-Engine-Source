/*
 * drive_upgrade.cpp
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

#include "drive_upgrade.h"

#include "drive.h"
#include "component_utils.h"
#include "unit_csv_factory.h"


DriveUpgrade::DriveUpgrade(Drive *drive) : 
    Component(),
    drive(drive),
    yaw(1.0), pitch(1.0), roll(1.0), lateral(1.0),
    vertical(1.0), forward(1.0), retro(1.0), afterburn(1.0),
    speed(1.0), max_yaw_left(1.0),
    max_yaw_right(1.0), max_pitch_down(1.0), max_pitch_up(1.0),
    max_roll_left(1.0), max_roll_right(1.0), fuel_consumption(1.0) {
    type = ComponentType::DriveUpgrade;    
}



// Component Methods
void DriveUpgrade::Load(std::string upgrade_key, 
                    std::string unit_key) {
    Component::Load(upgrade_key, unit_key);

    // Consumer
    fuel_consumption = UnitCSVFactory::GetVariable(upgrade_key, "Fuel_Consumption", 1.0);

    // DriveUpgrade
    yaw = UnitCSVFactory::GetVariable(upgrade_key, "Maneuver_Yaw", 1.0);
    pitch = UnitCSVFactory::GetVariable(upgrade_key, "Maneuver_Pitch", 1.0);
    roll = UnitCSVFactory::GetVariable(upgrade_key, "Maneuver_Roll", 1.0);

    DoubleYawPitchRollParser(upgrade_key, YPR::Yaw, max_yaw_right, max_yaw_left);
    DoubleYawPitchRollParser(upgrade_key, YPR::Pitch, max_pitch_up, max_pitch_down);
    DoubleYawPitchRollParser(upgrade_key, YPR::Roll, max_roll_right, max_roll_left);

    forward = UnitCSVFactory::GetVariable(upgrade_key, "Forward_Accel", 1.0);
    retro = UnitCSVFactory::GetVariable(upgrade_key, "Retro_Accel", 1.0);
    lateral = 0.5 * (UnitCSVFactory::GetVariable(upgrade_key, "Left_Accel", 1.0) +
            UnitCSVFactory::GetVariable(upgrade_key, "Right_Accel", 1.0));

    vertical = 0.5 * (UnitCSVFactory::GetVariable(upgrade_key, "Top_Accel", 1.0) +
            UnitCSVFactory::GetVariable(upgrade_key, "Bottom_Accel", 1.0));

    speed = UnitCSVFactory::GetVariable(upgrade_key, "Default_Speed_Governor", 1.0);
}      


void DriveUpgrade::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Yaw_Governor_Right"] = max_yaw_right;
    unit["Yaw_Governor_Left"] = max_yaw_left;
    unit["Pitch_Governor_Up"] = max_pitch_up;
    unit["Pitch_Governor_Down"] = max_pitch_down;
    unit["Roll_Governor_Right"] = max_roll_right;
    unit["Roll_Governor_Left"] = max_roll_left;

    unit["Forward_Accel"] = forward;
    unit["Retro_Accel"] = retro;
    unit["Left_Accel"] = unit["Right_Accel"] = lateral;
    unit["Bottom_Accel"] = unit["Top_Accel"] = vertical;

    unit["Default_Speed_Governor"] = speed;
}


// Can only upgrade/downgrade if Drive is undamaged.
// Otherwise, there are a lot of edge cases.
bool DriveUpgrade::CanDowngrade() const {
    return !drive->Damaged();
}

bool DriveUpgrade::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    // Component
    Component::Downgrade();

    // Add effects of upgrade on drive
    drive->yaw.SetMaxValue(drive->yaw.MaxValue() / yaw);
    drive->pitch.SetMaxValue(drive->pitch.MaxValue() / pitch);
    drive->roll.SetMaxValue(drive->roll.MaxValue() / roll);
    
    drive->lateral.SetMaxValue(drive->lateral.MaxValue() / lateral);
    drive->vertical.SetMaxValue(drive->vertical.MaxValue() / vertical);
    drive->forward.SetMaxValue(drive->forward.MaxValue() / forward);
    drive->retro.SetMaxValue(drive->retro.MaxValue() / retro);

    drive->speed.SetMaxValue(drive->speed.MaxValue() / speed);

    drive->max_yaw_left.SetMaxValue(drive->max_yaw_left.MaxValue() / max_yaw_left);
    drive->max_yaw_right.SetMaxValue(drive->max_yaw_right.MaxValue() / max_yaw_right);
    drive->max_pitch_up.SetMaxValue(drive->max_pitch_up.MaxValue() / max_pitch_up);
    drive->max_pitch_down.SetMaxValue(drive->max_pitch_down.MaxValue() / max_pitch_down);
    drive->max_roll_left.SetMaxValue(drive->max_roll_left.MaxValue() / max_roll_left);
    drive->max_roll_right.SetMaxValue(drive->max_roll_right.MaxValue() / max_roll_right);

    drive->SetConsumption(drive->GetConsumption() / fuel_consumption);

    yaw = pitch = roll = lateral = vertical = forward = retro = speed = fuel_consumption = 1.0;
    max_yaw_left = max_yaw_right = max_pitch_up = max_pitch_down = max_roll_left = max_roll_right = 1.0; 

    return true;
}

bool DriveUpgrade::CanUpgrade(const std::string upgrade_name) const {
    return !drive->Damaged();
}

bool DriveUpgrade::Upgrade(const std::string upgrade_name) {
    if(!CanUpgrade(upgrade_name)) {
        return false;
    }

    // Component
    Component::Upgrade(upgrade_name + "__upgrades");

    // Load modifiers
    Load(upgrade_name + "__upgrades");

    // Add effects of upgrade on drive
    drive->yaw.SetMaxValue(drive->yaw.MaxValue() * yaw);
    drive->pitch.SetMaxValue(drive->pitch.MaxValue() * pitch);
    drive->roll.SetMaxValue(drive->roll.MaxValue() * roll);
    
    drive->lateral.SetMaxValue(drive->lateral.MaxValue() * lateral);
    drive->vertical.SetMaxValue(drive->vertical.MaxValue() * vertical);
    drive->forward.SetMaxValue(drive->forward.MaxValue() * forward);
    drive->retro.SetMaxValue(drive->retro.MaxValue() * retro);

    drive->speed.SetMaxValue(drive->speed.MaxValue() * speed);

    drive->max_yaw_left.SetMaxValue(drive->max_yaw_left.MaxValue() * max_yaw_left);
    drive->max_yaw_right.SetMaxValue(drive->max_yaw_right.MaxValue() * max_yaw_right);
    drive->max_pitch_up.SetMaxValue(drive->max_pitch_up.MaxValue() * max_pitch_up);
    drive->max_pitch_down.SetMaxValue(drive->max_pitch_down.MaxValue() * max_pitch_down);
    drive->max_roll_left.SetMaxValue(drive->max_roll_left.MaxValue() * max_roll_left);
    drive->max_roll_right.SetMaxValue(drive->max_roll_right.MaxValue() * max_roll_right);

    drive->SetConsumption(drive->GetConsumption() * fuel_consumption);

    return true;
}


