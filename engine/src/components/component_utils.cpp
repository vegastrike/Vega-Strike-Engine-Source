/*
 * src/components/component_printer.cpp
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

#include "component_utils.h"

#include "component.h"

#include "reactor.h"
#include "energy_container.h"

#include "jump_drive.h"
#include "ftl_drive.h"
#include "cloak.h"

#include "unit_csv_factory.h"
#include "configuration/configuration.h"

#include <math.h>



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
    } else {
        return ComponentType::None;
    }
}


EnergyContainer* GetSource(ComponentType component_type, EnergyContainer* fuel,
                           EnergyContainer* energy, EnergyContainer* ftl_energy) {
    switch(component_type) {
        case ComponentType::None: 
        case ComponentType::Dummy:
        case ComponentType::Hull:
        case ComponentType::Armor:
        case ComponentType::AfterburnerUpgrade:
        case ComponentType::DriveUpgrade:
        case ComponentType::Capacitor:
        case ComponentType::FtlCapacitor:
        case ComponentType::Fuel:
            return nullptr;

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
