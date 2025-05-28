/*
 * src/components/component_utils.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <boost/format.hpp>

#include "component_utils.h"

#include "component.h"
#include "dummy_component.h"

#include "components/reactor.h"
#include "components/energy_container.h"

#include "components/afterburner.h"
#include "components/afterburner_upgrade.h"
#include "components/drive.h"
#include "components/drive_upgrade.h"
#include "components/jump_drive.h"
#include "ftl_drive.h"
#include "components/cloak.h"

#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"




const ComponentType GetComponentTypeFromName(const std::string name) {
    std::string upgrade_key = name + "__upgrades";

    const std::string upgrade_category = UnitCSVFactory::GetVariable(upgrade_key, "Upgrade_Type", std::string());
    if(upgrade_category == "None") {
        return ComponentType::NoComponent;
    } else if(upgrade_category == "Dummy") {
        return ComponentType::Dummy;
    } else if(upgrade_category == "Hull") {
        return ComponentType::Hull;
    } else if(upgrade_category == "Armor") {
        return ComponentType::Armor;
    } else if(upgrade_category == "Shield") {
        return ComponentType::Shield;
    } else if(upgrade_category == "Afterburner") {
        return ComponentType::Afterburner;
    } else if(upgrade_category == "Afterburner_Upgrade") {
        return ComponentType::AfterburnerUpgrade;
    } else if(upgrade_category == "Drive") {
        return ComponentType::Drive;
    } else if(upgrade_category == "Drive_Upgrade") {
        return ComponentType::DriveUpgrade;
    } else if(upgrade_category == "Ftl_Drive") {
        return ComponentType::FtlDrive;
    } else if(upgrade_category == "Jump_Drive") {
        return ComponentType::JumpDrive;
    } else if(upgrade_category == "Reactor") {
        return ComponentType::Reactor;
    } else if(upgrade_category == "Capacitor") {
        return ComponentType::Capacitor;
    } else if(upgrade_category == "FTL_Capacitor") {
        return ComponentType::FtlCapacitor;
    } else if(upgrade_category == "Fuel") {
        return ComponentType::Fuel;
    } else if(upgrade_category == "Cloak") {
        return ComponentType::Cloak;
    } else if(upgrade_category == "Radar") {
        return ComponentType::Radar;
    } else if(upgrade_category == "ECM") {
        return ComponentType::ECM;
    } else if(upgrade_category == "Repair_Bot") {
        return ComponentType::RepairBot;
    } else {
        return ComponentType::NoComponent;
    }
}


EnergyContainer* GetSource(ComponentType component_type, EnergyContainer* fuel,
                           EnergyContainer* energy, EnergyContainer* ftl_energy) {
    switch(component_type) {
        case ComponentType::Shield:
            return GetSourceFromConfiguration(configuration()->components.shield.energy_source, fuel, energy, ftl_energy);;

        case ComponentType::FtlDrive:
            return GetSourceFromConfiguration(configuration()->components.ftl_drive.energy_source, fuel, energy, ftl_energy);

        case ComponentType::Reactor:
            return GetSourceFromConfiguration(configuration()->components.reactor.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::Drive:
            return GetSourceFromConfiguration(configuration()->components.drive.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::Afterburner:
            return GetSourceFromConfiguration(configuration()->components.afterburner.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::JumpDrive:
            return GetSourceFromConfiguration(configuration()->components.jump_drive.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::Cloak:
            return GetSourceFromConfiguration(configuration()->components.cloak.energy_source,
                                              fuel, energy, ftl_energy);

        default:
            return nullptr;
    }
}

// 0 infinite, 1 fuel, 2 energy, 3 ftl_energy, 4 disabled
EnergyContainer* GetSourceFromConfiguration(const EnergyConsumerSource source, EnergyContainer* fuel,
                                      EnergyContainer* energy, EnergyContainer* ftl_energy) {
    switch(source) {
        case EnergyConsumerSource::Infinite: return nullptr;
        case EnergyConsumerSource::Fuel: return fuel;
        case EnergyConsumerSource::Energy: return energy;
        case EnergyConsumerSource::FTLEnergy: return ftl_energy;
        default: return nullptr;
    }
}

EnergyContainer* GetSourceFromConfiguration(const std::string& energy_source, EnergyContainer* fuel,
    EnergyContainer* energy, EnergyContainer* ftl_energy)
{
    if (energy_source == "Infinite" || energy_source == "infinite") {
        return nullptr;
    }
    if (energy_source == "Fuel" || energy_source == "fuel") {
        return fuel;
    }
    if (energy_source == "Energy" || energy_source == "energy") {
        return energy;
    }
    if (energy_source == "FTLEnergy" || energy_source == "ftl_energy") {
        return ftl_energy;
    }
    return nullptr;
}

// For Drive and DriveUpgrade
const std::string yaw_governor[] = {"Yaw_Governor", "Yaw_Governor_Right", "Yaw_Governor_Left"};
const std::string pitch_governor[] = {"Pitch_Governor", "Pitch_Governor_Up", "Pitch_Governor_Down"};
const std::string roll_governor[] = {"Roll_Governor", "Roll_Governor_Right", "Roll_Governor_Left"};

const std::string* GetGovernor(const YPR ypr) {
    switch(ypr) {
        case YPR::Yaw: return yaw_governor; break;
        case YPR::Pitch: return pitch_governor; break;
        case YPR::Roll: return roll_governor; break;
    }
}


void DoubleYawPitchRollParser(std::string unit_key, const YPR ypr,
                        double &right_value, double &left_value) {
    const std::string* governor = GetGovernor(ypr);

    const double main_value = UnitCSVFactory::GetVariable(unit_key, governor[0], 1.0);
    right_value = UnitCSVFactory::GetVariable(unit_key, governor[1], main_value);
    left_value = UnitCSVFactory::GetVariable(unit_key, governor[2], main_value);
}

void ResourceYawPitchRollParser(std::string unit_key, const YPR ypr,
                        Resource<double> &right_value, Resource<double> &left_value,
                        const double minimum_functionality) {
    double left, right;
    DoubleYawPitchRollParser(unit_key, ypr, right, left);
    left *= M_PI / 180.0;
    right *= M_PI / 180.0;
    right_value = Resource<double>(right,right * minimum_functionality,right);
    left_value = Resource<double>(left,right * minimum_functionality,left);
}


/* This function is used by vdu.cpp to print the component
    with the color showing damage */
std::string PrintFormattedComponentInHud(double percent, std::string component_name,
                                         bool damageable,
                                         std::string GetDamageColor(double)) {
    const bool print_percent_working = configuration()->graphics.hud.print_damage_percent;

    const std::string damage_color = GetDamageColor(percent);
    const int int_percent = percent * 100;
    static const std::string white_color = GetDamageColor(1.0);

    // Note we reset color to white/undamaged at the end of each line
    if(print_percent_working && damageable) {
        return (boost::format("%1%%2% (%3%%%)%4%\n") %
            damage_color % component_name % int_percent % 
            white_color).str();
    } else {
        return (boost::format("%1%%2%%3%\n") %
            damage_color % component_name % white_color).str();
    }
}
