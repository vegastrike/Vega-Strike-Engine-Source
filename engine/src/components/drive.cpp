/*
 * drive.cpp
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

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

#include "components/drive.h"

#include "component_utils.h"
#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"



Drive::Drive(EnergyContainer *source):
    Component(0.0, 0.0, true, true),
    EnergyConsumer(source, false),
    yaw(1,0,1), pitch(1,0,1), roll(1,0,1), lateral(1,0,1),
    vertical(1,0,1), forward(1,0,1), retro(1,0,1),
    speed(1,0,1), max_yaw_left(1,0,1),
    max_yaw_right(1,0,1), max_pitch_down(1,0,1), max_pitch_up(1,0,1),
    max_roll_left(1,0,1), max_roll_right(1,0,1) {
    type = ComponentType::Drive;
    upgrade_name = "Drive";
    installed = true;
}

Drive::~Drive()
= default;


// Component Methods
void Drive::Load(std::string unit_key) {
    const double game_speed = configuration()->physics.game_speed_dbl;
    const double game_accel = configuration()->physics.game_accel_dbl;
    const double game_accel_speed = game_speed * game_accel;

    // Minimum drive capability for limp home (in %)
    const double minimal_drive_functionality = configuration()->components.drive.minimum_drive_dbl;

    Component::Load(unit_key);

    // Consumer
    // We do not support all options here.
    if(configuration()->components.drive.energy_source == "fuel") {
        SetConsumption(1.0);
    } else {
        SetConsumption(1.0);
        infinite = true;
    }

    // Drive
    yaw = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Maneuver_Yaw", std::string("0.0")),
                                                       M_PI / 180.0, minimal_drive_functionality);
    pitch = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Maneuver_Pitch", std::string("0.0")),
                                                       M_PI / 180.0, minimal_drive_functionality);
    roll = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Maneuver_Roll", std::string("0.0")),
                                                       M_PI / 180.0, minimal_drive_functionality);

    ResourceYawPitchRollParser(unit_key, YPR::Yaw, max_yaw_right, max_yaw_left);
    ResourceYawPitchRollParser(unit_key, YPR::Pitch, max_pitch_up, max_pitch_down);
    ResourceYawPitchRollParser(unit_key, YPR::Roll, max_roll_right, max_roll_left);

    forward = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Forward_Accel", std::string("0.0")),
                                                           game_accel_speed, minimal_drive_functionality);
    retro = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Retro_Accel", std::string("0.0")),
                                                           game_accel_speed, minimal_drive_functionality);

    double accel = UnitCSVFactory::GetVariable(unit_key, "accel", -1.0f);
    if(accel != -1.0f) {
        const std::string accel_string = std::to_string(accel);
        lateral = Resource<double>(accel_string, game_accel_speed, minimal_drive_functionality);
        vertical = Resource<double>(accel_string, game_accel_speed, minimal_drive_functionality);
    } else {
        const double lateral_accel = 0.5 * UnitCSVFactory::GetVariable(unit_key, "Left_Accel", 0.0) +
                                 0.5 * UnitCSVFactory::GetVariable(unit_key, "Right_Accel", 0.0);
        const std::string lateral_accel_string = std::to_string(lateral_accel);
        lateral = Resource<double>(lateral_accel_string, game_accel_speed, minimal_drive_functionality);

        const double vertical_accel = 0.5 * UnitCSVFactory::GetVariable(unit_key, "Top_Accel", 0.0) +
                                    0.5 * UnitCSVFactory::GetVariable(unit_key, "Bottom_Accel", 0.0);
        const std::string vertical_accel_string = std::to_string(vertical_accel);
        vertical = Resource<double>(vertical_accel_string, game_accel_speed, minimal_drive_functionality);
    }

    speed = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Default_Speed_Governor", std::string("0.0")),
                                                         game_speed, minimal_drive_functionality);

    // We calculate percent operational as a simple average
    operational = (yaw.Percent() + pitch.Percent() + roll.Percent() +
                  lateral.Percent() + vertical.Percent() + forward.Percent() +
                  retro.Percent() + speed.Percent() +
                  max_yaw_left.Percent() + max_yaw_right.Percent() + max_pitch_down.Percent() +
                  max_pitch_up.Percent() + max_roll_left.Percent() + max_roll_right.Percent()) / 14;
}




void Drive::SaveToCSV(std::map<std::string, std::string>& unit) const {
    const double game_speed = configuration()->physics.game_speed_dbl;
    const double game_accel = configuration()->physics.game_accel_dbl;
    const double game_accel_speed = (game_speed * game_accel);
    const double to_degrees = M_PI / 180;
    unit["Maneuver_Yaw"] = yaw.Serialize(to_degrees);
    unit["Maneuver_Pitch"] = pitch.Serialize(to_degrees);
    unit["Maneuver_Roll"] = roll.Serialize(to_degrees);

    unit["Yaw_Governor_Right"] = max_yaw_right.Serialize(to_degrees);
    unit["Yaw_Governor_Left"] = max_yaw_left.Serialize(to_degrees);
    unit["Pitch_Governor_Up"] = max_pitch_up.Serialize(to_degrees);
    unit["Pitch_Governor_Down"] = max_pitch_down.Serialize(to_degrees);
    unit["Roll_Governor_Right"] = max_roll_right.Serialize(to_degrees);
    unit["Roll_Governor_Left"] = max_roll_left.Serialize(to_degrees);


    unit["Forward_Accel"] = forward.Serialize(game_accel_speed);
    unit["Retro_Accel"] = retro.Serialize(game_accel_speed);
    unit["Left_Accel"] = unit["Right_Accel"] = lateral.Serialize(game_accel_speed);
    unit["Bottom_Accel"] = unit["Top_Accel"] = vertical.Serialize(game_accel_speed);

    unit["Default_Speed_Governor"] = speed.Serialize(game_speed);
}

// Drive is integrated and so cannot be upgraded/downgraded
// Use DriveUpgrade to make changes
bool Drive::CanDowngrade() const {
    return false;
}

bool Drive::Downgrade() {
    return false;
}

bool Drive::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool Drive::Upgrade(const std::string upgrade_key) {
    return false;
}


void Drive::Damage() {
    yaw.RandomDamage();
    pitch.RandomDamage();
    roll.RandomDamage();

    lateral.RandomDamage();
    vertical.RandomDamage();
    forward.RandomDamage();
    retro.RandomDamage();

    speed.RandomDamage();

    max_yaw_left.RandomDamage();
    max_yaw_right.RandomDamage();
    max_pitch_down.RandomDamage();
    max_pitch_up.RandomDamage();
    max_roll_left.RandomDamage();
    max_roll_right.RandomDamage();

    // We calculate percent operational as a simple average
    operational = (yaw.Percent() + pitch.Percent() + roll.Percent() +
                  lateral.Percent() + vertical.Percent() + forward.Percent() +
                  retro.Percent() + speed.Percent() +
                  max_yaw_left.Percent() + max_yaw_right.Percent() + max_pitch_down.Percent() +
                  max_pitch_up.Percent() + max_roll_left.Percent() + max_roll_right.Percent()) / 14;
}

void Drive::DamageByPercent(double percent) {
    yaw.DamageByPercent(percent);
    pitch.DamageByPercent(percent);
    roll.DamageByPercent(percent);

    lateral.DamageByPercent(percent);
    vertical.DamageByPercent(percent);
    forward.DamageByPercent(percent);
    retro.DamageByPercent(percent);

    speed.DamageByPercent(percent);

    max_yaw_left.DamageByPercent(percent);
    max_yaw_right.DamageByPercent(percent);
    max_pitch_down.DamageByPercent(percent);
    max_pitch_up.DamageByPercent(percent);
    max_roll_left.DamageByPercent(percent);
    max_roll_right.DamageByPercent(percent);

    operational = (yaw.Percent() + pitch.Percent() + roll.Percent() +
                  lateral.Percent() + vertical.Percent() + forward.Percent() +
                  retro.Percent() + speed.Percent() +
                  max_yaw_left.Percent() + max_yaw_right.Percent() + max_pitch_down.Percent() +
                  max_pitch_up.Percent() + max_roll_left.Percent() + max_roll_right.Percent()) / 14;
}

void Drive::Repair() {
    yaw.RepairFully();
    pitch.RepairFully();
    roll.RepairFully();

    lateral.RepairFully();
    vertical.RepairFully();
    forward.RepairFully();
    retro.RepairFully();

    speed.RepairFully();

    max_yaw_left.RepairFully();
    max_yaw_right.RepairFully();
    max_pitch_down.RepairFully();
    max_pitch_up.RepairFully();
    max_roll_left.RepairFully();
    max_roll_right.RepairFully();

    operational.RepairFully();
}

double Drive::Consume()
{
    return EnergyConsumer::Consume();
}








