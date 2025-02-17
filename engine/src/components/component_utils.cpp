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

#include "component_utils.h"

#include "component.h"
#include "dummy_component.h"

#include "reactor.h"
#include "energy_container.h"

#include "afterburner.h"
#include "afterburner_upgrade.h"
#include "drive.h"
#include "drive_upgrade.h"
#include "jump_drive.h"
#include "ftl_drive.h"
#include "cloak.h"

#include "unit_csv_factory.h"
#include "configuration/configuration.h"




const ComponentType GetComponentTypeFromName(const std::string name) {
    std::string upgrade_key = name + "__upgrades";

    const std::string upgrade_category = UnitCSVFactory::GetVariable(upgrade_key, "Upgrade_Type", std::string());
    if(upgrade_category == "None") {
        return ComponentType::None;
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
    } else if(upgrade_category == "ECM") {
        return ComponentType::ECM;
    } else if(upgrade_category == "Repair_Bot") {
        return ComponentType::RepairBot;
    } else {
        return ComponentType::None;
    }
}


EnergyContainer* GetSource(ComponentType component_type, EnergyContainer* fuel,
                           EnergyContainer* energy, EnergyContainer* ftl_energy) {
    switch(component_type) {
        case ComponentType::Shield: return energy;
    
        case ComponentType::FtlDrive: return ftl_energy;
        
        case ComponentType::Reactor:
            return GetSourceFromConfiguration(configuration()->fuel.reactor_source, 
                                              fuel, energy, ftl_energy);
        case ComponentType::Drive:
            return GetSourceFromConfiguration(configuration()->fuel.drive_source, 
                                              fuel, energy, ftl_energy);
        case ComponentType::Afterburner:
            return GetSourceFromConfiguration(configuration()->fuel.afterburner_source, 
                                              fuel, energy, ftl_energy);
        case ComponentType::JumpDrive:
            return GetSourceFromConfiguration(configuration()->fuel.jump_drive_source, 
                                              fuel, energy, ftl_energy);
        case ComponentType::Cloak:
            return GetSourceFromConfiguration(configuration()->fuel.cloak_source, 
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
