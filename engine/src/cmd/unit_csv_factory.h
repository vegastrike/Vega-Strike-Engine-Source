/*
 * unit_csv_factory.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_CSV_FACTORY_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_CSV_FACTORY_H

#include <map>
#include <string>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include "vsfilesystem.h"

const std::string keys[] = {"Key", "Directory",	"Name",	"STATUS",	"Object_Type",
                            "Combat_Role",	"Textual_Description",	"Hud_image",	"Unit_Scale",	"Cockpit",
                            "CockpitX", "CockpitY",	"CockpitZ",	"Mesh",	"Shield_Mesh",	"Rapid_Mesh",	"BSP_Mesh",
                            "Use_BSP", "Use_Rapid",	"NoDamageParticles", "Mass",	"Moment_Of_Inertia",
                            "Fuel_Capacity",	"Hull", "Armor_Front_Top_Right",	"Armor_Front_Top_Left",
                            "Armor_Front_Bottom_Right", "Armor_Front_Bottom_Left",	"Armor_Back_Top_Right",
                            "Armor_Back_Top_Left", "Armor_Back_Bottom_Right",	"Armor_Back_Bottom_Left",	"Shield_Front_Top_Right",
                            "Shield_Back_Top_Left",	"Shield_Front_Bottom_Right",	"Shield_Front_Bottom_Left",
                            "Shield_Back_Top_Right",	"Shield_Front_Top_Left",	"Shield_Back_Bottom_Right",
                            "Shield_Back_Bottom_Left",	"Shield_Recharge",	"Shield_Leak",	"Warp_Capacitor",
                            "Primary_Capacitor",	"Reactor_Recharge",	"Jump_Drive_Present",	"Jump_Drive_Delay",
                            "Wormhole",	"Outsystem_Jump_Cost",	"Warp_Usage_Cost",	"Afterburner_Type",
                            "Afterburner_Usage_Cost",	"Maneuver_Yaw",	"Maneuver_Pitch",	"Maneuver_Roll",
                            "Yaw_Governor",	"Pitch_Governor",	"Roll_Governor",	"Afterburner_Accel",
                            "Forward_Accel",	"Retro_Accel",	"Left_Accel",	"Right_Accel",	"Top_Accel",
                            "Bottom_Accel",	"Afterburner_Speed_Governor",	"Default_Speed_Governor",	"ITTS",
                            "Radar_Color",	"Radar_Range",	"Tracking_Cone",	"Max_Cone", "Lock_Cone",	"Hold_Volume",
                            "Can_Cloak",	"Cloak_Min",	"Cloak_Rate",	"Cloak_Energy",	"Cloak_Glass",	"Repair_Droid",
                            "ECM_Rating",	"ECM_Resist",	"Ecm_Drain",	"Hud_Functionality",	"Max_Hud_Functionality",
                            "Lifesupport_Functionality",	"Max_Lifesupport_Functionality",	"Comm_Functionality",
                            "Max_Comm_Functionality",	"FireControl_Functionality",	"Max_FireControl_Functionality",
                            "SPECDrive_Functionality",	"Max_SPECDrive_Functionality",	"Slide_Start",	"Slide_End",
                            "Activation_Accel",	"Activation_Speed",	"Upgrades",	"Prohibited_Upgrades",
                            "Sub_Units",	"Sound",	"Light",	"Mounts",	"Net_Comm",	"Dock",	"Cargo_Import",	"Cargo",
                            "Explosion",	"Num_Animation_Stages",	"Upgrade_Storage_Volume",	"Heat_Sink_Rating",
                            "Shield_Efficiency",	"Num_Chunks",	"Chunk_0",	"Collide_Subunits",	"Spec_Interdiction",
                            "Tractorability",
                            // These values are not in units.csv! There are probably more but I stopped mapping.
                            // TODO: map all missing values using the commented out code below!
                            "FaceCamera", "Unit_Role", "Attack_Preference", "Hidden_Hold_Volume", "Equipment_Space"};


class UnitCSVFactory {
    static std::string DEFAULT_ERROR_VALUE;
    static std::map<std::string, std::map<std::string, std::string>> units;

    static inline std::string _GetVariable(std::string unit_key, std::string const &attribute_key) {
        if (units.count(unit_key) == 0) {
            return DEFAULT_ERROR_VALUE;
        }

        std::map<std::string, std::string> unit_attributes = UnitCSVFactory::units[unit_key];

        // TODO: Use this code to find more missing key as shown above.
        // Note: The following code can probably be cleaner with find...
        /*bool attribute_found = false;

        for(const std::string& key : keys) {
            if(key == attribute_key) {
                attribute_found = true;
                break;
            }
        }

        if (!attribute_found)
        {
            std::cout << attribute_key << " attribute not found.\n";
            assert(0);
        }*/

        if (unit_attributes.count(attribute_key) == 0) {
            return DEFAULT_ERROR_VALUE;
        }

        return unit_attributes[attribute_key];
    }

    friend class UnitJSONFactory;
    friend class UnitOptimizeFactory;
public:
    static void ParseCSV(VSFileSystem::VSFile &file, bool saved_game);

    template<class T>
    static inline T GetVariable(std::string unit_key, std::string const &attribute_key, T default_value) = delete;
    static bool HasVariable(std::string unit_key, std::string const &attribute_key) {
        if (units.count(unit_key) == 0) {
            return false;
        }

        std::map<std::string, std::string> unit_attributes = UnitCSVFactory::units[unit_key];

        return (unit_attributes.count(attribute_key) > 0);
    }

    static bool HasUnit(std::string unit_key) {
        return (units.count(unit_key) > 0);
    }


    static std::map<std::string, std::string> GetUnit(std::string key) {
        return UnitCSVFactory::units[key];
    }
};

// Template Specialization
template<>
inline std::string UnitCSVFactory::GetVariable(std::string unit_key,
        std::string const &attribute_key,
        std::string default_value) {
    std::string result = _GetVariable(unit_key, attribute_key);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }

    return result;
}

// Need this in because "abcd" is const char* and not std::string
/*template <>
inline const char* UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, const char* default_value)
{
    std::string result = _GetVariable(unit_key, attribute_key);
    if(result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }

    return result.c_str();
}*/

template<>
inline bool UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, bool default_value) {
    std::string result = _GetVariable(unit_key, attribute_key);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    } else if (result.empty()) {
        return default_value;
    }
    boost::algorithm::to_lower(result);
    return (result == "true" || result == "1");
}

template<>
inline float UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, float default_value) {
    std::string result = _GetVariable(unit_key, attribute_key);

    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    } else if (result.empty()) {
        return default_value;
    }

    try {
        return std::stof(result);
    } catch (std::invalid_argument &) {
        return default_value;
    }
}

template<>
inline double UnitCSVFactory::GetVariable(std::string unit_key,
        std::string const &attribute_key,
        double default_value) {
    std::string result = _GetVariable(unit_key, attribute_key);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stod(result);
    } catch (std::invalid_argument &) {
        return default_value;
    }
}

template<>
inline int UnitCSVFactory::GetVariable(std::string unit_key, std::string const &attribute_key, int default_value) {
    std::string result = _GetVariable(unit_key, attribute_key);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    } else if (result.empty()) {
        return default_value;
    }
    try {
        return std::stoi(result);
    } catch (std::invalid_argument &) {
        return default_value;
    }
}

std::string GetUnitKeyFromNameAndFaction(const std::string unit_name, const std::string unit_faction);

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_CSV_FACTORY_H
