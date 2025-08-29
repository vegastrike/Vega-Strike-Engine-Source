/*
 * radar.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "components/radar.h"
#include "configuration/configuration.h"
#include "cmd/unit_csv_factory.h"
#include "components/component.h"

#define _USE_MATH_DEFINES
#include <math.h>


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
        tracking_active(true) {
    long default_max_range = static_cast<long>(configuration().components.computer.default_max_range_dbl);
    max_range = Resource<long>(default_max_range,0,default_max_range);
    tracking_cone = configuration().components.computer.default_tracking_cone_dbl;
    lock_cone = configuration().components.computer.default_lock_cone_dbl;
}

CRadar::~CRadar()
= default;

// Component Methods
void CRadar::Load(std::string unit_key) {
    Component::Load(unit_key);

    // Consumer
    // TODO: energy
    SetConsumption(0);

    // Radar
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
        std::cout << "Try stoi " << unit_key << std::endl;
        unsigned int value = stoi(iffval, 0);
        std::cout << "Success stoi " << unit_key << std::endl;
        if (value == 0) {
            // Unknown value
            capabilities = RadarCapabilities::NONE;
        } else {
            capabilities = value;
        }
    }

    tracking_active = true;
    const std::string max_range_string = UnitCSVFactory::GetVariable(unit_key, "Radar_Range", std::string("300000000"));
    max_range = Resource<long>(max_range_string,1,0);
    max_cone = cos(UnitCSVFactory::GetVariable(unit_key, "Max_Cone", 180.0) * M_PI / 180);
    tracking_cone = cos(UnitCSVFactory::GetVariable(unit_key, "Tracking_Cone", 180.0f) * M_PI / 180);
    lock_cone = cos(UnitCSVFactory::GetVariable(unit_key, "Lock_Cone", 180.0) * M_PI / 180);
    operational = max_range.Percent();
    installed = max_range > 0;
}

void CRadar::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Can_Lock"] = std::to_string(can_lock);
    unit["Radar_Color"] = std::to_string(capabilities);
    unit["Radar_Range"] = max_range.Serialize();

    // TODO: can't serialize if also doing acos on it
    // Also, can't use Resource because games stores in radian
    // and these can be minus.
    unit["Tracking_Cone"] = std::to_string(acos(tracking_cone) * 180. / M_PI);
    unit["Max_Cone"] = std::to_string(acos(max_cone) * 180. / M_PI);
    unit["Lock_Cone"] = std::to_string(acos(lock_cone) * 180. / M_PI);
}

bool CRadar::CanDowngrade() const {
    return installed && !integral;
}

bool CRadar::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    max_range = Resource<double>(0,0,0);
    max_cone = cos(0);
    tracking_cone = cos(0);
    lock_cone = cos(0);
    type = RadarType::SPHERE;
    capabilities = RadarCapabilities::NONE;
    locked = false;
    can_lock = false;

    installed = false;
    return true;
}

bool CRadar::CanUpgrade(const std::string upgrade_key) const {
    return !integral;
}

bool CRadar::Upgrade(const std::string upgrade_key) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    Load(upgrade_key);
    installed = true;
    operational = 1.0;
    return true;
}

void CRadar::Damage() {
    max_range.RandomDamage();
    // max_cone.RandomDamage();
    // lock_cone.RandomDamage();
    // tracking_cone.RandomDamage();

    // We calculate percent operational as a simple average
    operational = max_range.Percent();

    // Leaving this code here when we move to Resource<double> for max, lock and tracking cones.
    // operational = (max_range.Percent() + max_cone.Percent() + lock_cone.Percent() +
    //               tracking_cone.Percent()) / 4 * 100;
}

void CRadar::DamageByPercent(double percent) {
    max_range.DamageByPercent(percent);
    // max_cone.DamageByPercent(percent);
    // lock_cone.DamageByPercent(percent);
    // tracking_cone.DamageByPercent(percent);

    // We calculate percent operational as a simple average
    operational = max_range.Percent();
}

void CRadar::Repair() {
    max_range.RepairFully();

    operational = 1.0;
}

void CRadar::Destroy() {
    max_range.Destroy();
    operational = 0;
}


// Radar Code

// This code replaces and fixes the old code in Armed::LockTarget(bool)
void CRadar::Lock(bool always_lock) {
    if(!can_lock && !always_lock) {
        this->locked = false;
        return;
    }

    this->locked = true;
}

void CRadar::Unlock() {
    locked = false;
}

void CRadar::ToggleLock(bool always_lock) {
    if(locked) {
        locked = false;
        return;
    }

    Lock(always_lock);
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

float CRadar::GetMaxRange() const {
    return max_range.Value();
}

float CRadar::GetMaxCone() const {
    return max_cone;
}

float CRadar::GetLockCone() const {
    return lock_cone;
}

float CRadar::GetTrackingCone() const {
    return tracking_cone;
}

float CRadar::GetMinTargetSize() const {
    return min_target_size;
}

bool CRadar::Locked() const {
    return locked;
}

bool CRadar::CanLock() const {
    return can_lock;
}

bool CRadar::Tracking() const {
    return tracking_active;
}

double CRadar::Consume()
{
    return EnergyConsumer::Consume();
}
