/*
 * component_utils.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
    } else if(upgrade_category.empty()) {
        // Fallback: detect component type from fields present in the upgrade data.
        // This handles legacy upgrade data files that don't have the Upgrade_Type field.
        const std::string armor_val = UnitCSVFactory::GetVariable(upgrade_key, "armor", std::string());
        if(!armor_val.empty()) {
            return ComponentType::Armor;
        }

        const std::string shield_val = UnitCSVFactory::GetVariable(upgrade_key, "shield", std::string());
        if(!shield_val.empty()) {
            return ComponentType::Shield;
        }

        const std::string hull_val = UnitCSVFactory::GetVariable(upgrade_key, "Hull", std::string());
        if(!hull_val.empty()) {
            return ComponentType::Hull;
        }

        const std::string reactor_val = UnitCSVFactory::GetVariable(upgrade_key, "Reactor_Recharge", std::string());
        if(!reactor_val.empty()) {
            return ComponentType::Reactor;
        }

        const std::string fuel_val = UnitCSVFactory::GetVariable(upgrade_key, "Fuel_Capacity", std::string());
        if(!fuel_val.empty()) {
            return ComponentType::Fuel;
        }

        const std::string capacitor_val = UnitCSVFactory::GetVariable(upgrade_key, "Primary_Capacitor", std::string());
        if(!capacitor_val.empty()) {
            return ComponentType::Capacitor;
        }

        const std::string ftl_capacitor_val = UnitCSVFactory::GetVariable(upgrade_key, "Warp_Capacitor", std::string());
        if(!ftl_capacitor_val.empty()) {
            return ComponentType::FtlCapacitor;
        }

        const std::string warp_cost_val = UnitCSVFactory::GetVariable(upgrade_key, "Warp_Usage_Cost", std::string());
        if(!warp_cost_val.empty()) {
            return ComponentType::FtlDrive;
        }

        const std::string jump_present_val = UnitCSVFactory::GetVariable(upgrade_key, "Jump_Drive_Present", std::string());
        if(!jump_present_val.empty()) {
            return ComponentType::JumpDrive;
        }

        const std::string cloak_energy_val = UnitCSVFactory::GetVariable(upgrade_key, "Cloak_Energy", std::string());
        const std::string can_cloak_val = UnitCSVFactory::GetVariable(upgrade_key, "Can_Cloak", std::string());
        const std::string cloak_rate_val = UnitCSVFactory::GetVariable(upgrade_key, "Cloak_Rate", std::string());
        if(!cloak_energy_val.empty() || !can_cloak_val.empty() || !cloak_rate_val.empty()) {
            return ComponentType::Cloak;
        }

        const std::string radar_range_val = UnitCSVFactory::GetVariable(upgrade_key, "Radar_Range", std::string());
        if(!radar_range_val.empty()) {
            return ComponentType::Radar;
        }

        const std::string ecm_val = UnitCSVFactory::GetVariable(upgrade_key, "ecm", std::string());
        if(!ecm_val.empty()) {
            return ComponentType::ECM;
        }

        const std::string repair_val = UnitCSVFactory::GetVariable(upgrade_key, "repair", std::string());
        if(!repair_val.empty()) {
            return ComponentType::RepairBot;
        }

        // Drive vs. DriveUpgrade: distinguish using Object_Type.
        // Upgrade_Improvement indicates multiplier-based upgrades (DriveUpgrade),
        // while other Object_Type values (or empty) indicate base Drive components.
        const std::string drive_object_type = UnitCSVFactory::GetVariable(upgrade_key, "Object_Type", std::string());
        const double maneuver_yaw = UnitCSVFactory::GetVariable(upgrade_key, "Maneuver_Yaw", 0.0);
        const double forward_accel = UnitCSVFactory::GetVariable(upgrade_key, "Forward_Accel", 0.0);
        if(maneuver_yaw != 0.0 || forward_accel != 0.0) {
            if(drive_object_type == "Upgrade_Improvement") {
                return ComponentType::DriveUpgrade;
            }
            return ComponentType::Drive;
        }

        // Afterburner vs. AfterburnerUpgrade: similar logic using Object_Type.
        const std::string afterburner_object_type = UnitCSVFactory::GetVariable(upgrade_key, "Object_Type", std::string());
        const double afterburner_accel = UnitCSVFactory::GetVariable(upgrade_key, "Afterburner_Accel", 0.0);
        const double afterburner_speed = UnitCSVFactory::GetVariable(upgrade_key, "Afterburner_Speed_Governor", 0.0);
        if(afterburner_accel != 0.0 || afterburner_speed != 0.0) {
            if(afterburner_object_type == "Upgrade_Improvement") {
                return ComponentType::AfterburnerUpgrade;
            }
            return ComponentType::Afterburner;
        }

        return ComponentType::NoComponent;
    }
    else {
        // Unknown Upgrade_Type value — treat as NoComponent.
        return ComponentType::NoComponent;
    }
}


EnergyContainer* GetSource(ComponentType component_type, EnergyContainer* fuel,
                           EnergyContainer* energy, EnergyContainer* ftl_energy) {
    switch(component_type) {
        case ComponentType::Shield:
            return GetSourceFromConfiguration(configuration().components.shield.energy_source, fuel, energy, ftl_energy);;

        case ComponentType::FtlDrive:
            return GetSourceFromConfiguration(configuration().components.ftl_drive.energy_source, fuel, energy, ftl_energy);

        case ComponentType::Reactor:
            return GetSourceFromConfiguration(configuration().components.reactor.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::Drive:
            return GetSourceFromConfiguration(configuration().components.drive.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::Afterburner:
            return GetSourceFromConfiguration(configuration().components.afterburner.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::JumpDrive:
            return GetSourceFromConfiguration(configuration().components.jump_drive.energy_source,
                                              fuel, energy, ftl_energy);
        case ComponentType::Cloak:
            return GetSourceFromConfiguration(configuration().components.cloak.energy_source,
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
    const bool print_percent_working = configuration().graphics.hud.print_damage_percent;

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
