/*
 * radar.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "radar.h"
#include "configuration/configuration.h"
#include "unit_generic.h"
#include "unit_util.h"
#include "unit_csv_factory.h"
#include "components/component.h"

#include <random>

CRadar::CRadar():
        max_range(0),
        max_cone(-1),
        lock_cone(0),
        tracking_cone(0),
        min_target_size(0),
        type(RadarType::SPHERE),
        capabilities(RadarCapabilities::NONE),
        locked(false),
        can_lock(false),
        tracking_active(true),
        original(nullptr),
        computer(nullptr)
{

    max_range = configuration()->computer_config.default_max_range;
    tracking_cone = configuration()->computer_config.default_tracking_cone;
    lock_cone = configuration()->computer_config.default_lock_cone;
}

CRadar::CRadar(std::string unit_key, Computer* computer):
        max_range(0),
        max_cone(-1),
        lock_cone(0),
        tracking_cone(0),
        min_target_size(0),
        type(RadarType::SPHERE),
        capabilities(RadarCapabilities::NONE),
        locked(false),
        can_lock(false),
        tracking_active(true),
        original(nullptr),
        computer(nullptr) {
    can_lock = UnitCSVFactory::GetVariable(unit_key, "Can_Lock", true);

    // TODO: fix this
    // The Radar_Color column in the units.csv has been changed from a
    // boolean value to a string. The boolean values are supported for
    // backwardscompatibility.
    // When we save this setting, it is simply converted from an integer
    // number to a string, and we need to support this as well.
    std::string iffval = UnitCSVFactory::GetVariable(unit_key, "Radar_Color", std::string());

    if ((iffval.empty()) || (iffval == "FALSE") || (iffval == "0")) {
        capabilities = RadarCapabilities::NONE;
    } else if ((iffval == "TRUE") || (iffval == "1")) {
        type = RadarType::SPHERE;
        capabilities = RadarCapabilities::FRIEND_FOE;
    } else if (iffval == "THREAT") {
        type = RadarType::SPHERE;
        capabilities = RadarCapabilities::FRIEND_FOE |
                RadarCapabilities::THREAT_ASSESSMENT;
    } else if (iffval == "BUBBLE_THREAT") {
        type = RadarType::BUBBLE;
        capabilities = RadarCapabilities::FRIEND_FOE |
                RadarCapabilities::OBJECT_RECOGNITION |
                RadarCapabilities::THREAT_ASSESSMENT;
    } else if (iffval == "PLANE") {
        type = RadarType::PLANE;
        capabilities = RadarCapabilities::FRIEND_FOE;
    } else if (iffval == "PLANE_THREAT") {
        type = RadarType::PLANE;
        capabilities = RadarCapabilities::FRIEND_FOE |
                RadarCapabilities::OBJECT_RECOGNITION |
                RadarCapabilities::THREAT_ASSESSMENT;
    } else {
        unsigned int value = stoi(iffval, 0);
        if (value == 0) {
            // Unknown value
            capabilities = RadarCapabilities::NONE;
        } else {
            capabilities = value;
        }
    }

    tracking_active = true;
    max_range = UnitCSVFactory::GetVariable(unit_key, "Radar_Range", FLT_MAX);
    max_cone = cos(UnitCSVFactory::GetVariable(unit_key, "Max_Cone", 180.0) * VS_PI / 180);
    tracking_cone = cos(UnitCSVFactory::GetVariable(unit_key, "Tracking_Cone", 180.0f) * VS_PI / 180);
    lock_cone = cos(UnitCSVFactory::GetVariable(unit_key, "Lock_Cone", 180.0) * VS_PI / 180);
    original = nullptr;
    this->computer = computer;
}

void CRadar::WriteUnitString(std::map<std::string, std::string> &unit) {
    unit["Can_Lock"] = std::to_string(can_lock);
    unit["Radar_Color"] = std::to_string(capabilities);
    unit["Radar_Range"] = std::to_string(max_range);
    unit["Tracking_Cone"] = std::to_string(acos(tracking_cone) * 180. / VS_PI);
    unit["Max_Cone"] = std::to_string(acos(max_cone) * 180. / VS_PI);
    unit["Lock_Cone"] = std::to_string(acos(lock_cone) * 180. / VS_PI);
}

void CRadar::Damage()
{
    return;
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> damage_distribution(0, 6);
    std::uniform_int_distribution<> size_distribution(0, 6);

    // Damage IFF capabilities
    if(std::rand() < 0.2) {
        // TODO: make this smarter and maybe degrade capabilities
        capabilities = NONE;
    }

    /*max_range = max_range * random20();
    max_cone = max_cone * random20();
    lock_cone = lock_cone * random20();
    tracking_cone = tracking_cone * random20();
    min_target_size = min_target_size * random20();*/

    // Original cone damage
    /*const float maxdam = configuration()->physics_config.max_radar_cone_damage;
    radar.max_cone += (1 - dam);
    if (radar.max_cone > maxdam) {
        radar.max_cone = maxdam;
    }

    const float maxdam = configuration()->physics_config.max_radar_lock_cone_damage;
    radar.lock_cone += (1 - dam);
    if (radar.lock_cone > maxdam) {
        radar.lock_cone = maxdam;
    }

    const float maxdam = configuration()->physics_config.max_radar_track_cone_damage;
    radar.tracking_cone += (1 - dam);
    if (radar.tracking_cone > maxdam) {
        radar.tracking_cone = maxdam;
    }*/

}

void CRadar::Repair()
{

}

// This code replaces and fixes the old code in Armed::LockTarget(bool)
void CRadar::Lock() {
    if(!computer) {
        return;
    }

    const Unit *target = computer->target.GetConstUnit();

    if(!target) {
        //std::cerr << "Target is null\n";
        return;
    }

    if(!can_lock) {
        std::cerr << "Can't lock\n";
        this->locked = false;
        return;
    }

    /*if(!UnitUtil::isSignificant(target)) {
        std::cerr << "Target insignificant\n";
        this->locked = false;
        return;
    }*/

    std::cout << "Target locked\n";
    this->locked = true;

}

RadarType CRadar::GetType() const {
    return type;
}

bool CRadar::UseFriendFoe() const {
    return (capabilities & RadarCapabilities::FRIEND_FOE);
}

bool CRadar::UseObjectRecognition() const {
    return (capabilities & RadarCapabilities::OBJECT_RECOGNITION);
}

bool CRadar::UseThreatAssessment() const {
    return (capabilities & RadarCapabilities::THREAT_ASSESSMENT);
}
