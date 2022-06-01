/*
 * unit_csv_factory.cpp
 *
 * Copyright (C) 2021 Roy Falk
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

#include "unit_json_factory.h"
#include "unit_csv_factory.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#include "json.h"


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
                            "Tractorability"};

void UnitJSONFactory::ParseJSON(const std::string &filename) {
    const std::string json_filename = filename.substr(0, filename.size()-3) + "json";

    std::ifstream ifs(json_filename, std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    const std::string json_text = buffer.str();

    std::vector<std::string> units = json::parsing::parse_array(json_text.c_str());
    // Iterate over root
    for (const std::string &unit_text : units) {
        json::jobject unit = json::jobject::parse(unit_text);
        std::map<std::string, std::string> unit_attributes;

        for (const std::string &key : keys) {
            // For some reason, parser adds quotes
            if(unit.has_key(key)) {
                const std::string attribute = unit.get(key);
                const std::string stripped_attribute = attribute.substr(1, attribute.size() - 2);
                unit_attributes[key] = stripped_attribute;
            } else {
                unit_attributes[key] = "";
            }
        }

        std::string unit_key = unit.get("Key");
        std::string stripped_unit_key = unit_key.substr(1, unit_key.size() - 2);

        UnitCSVFactory::units[stripped_unit_key] = unit_attributes;
    }
}
