/*
 * radar.h
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

#ifndef RADAR_H
#define RADAR_H

#include <string>
#include <map>
#include <memory>
#include <iostream>

#include "computer.h"

class Unit;

enum class RadarType {
    SPHERE,
    BUBBLE,
    PLANE
};

enum RadarCapabilities {
    NONE = 0,
    FRIEND_FOE = 1,
    OBJECT_RECOGNITION = 2,
    THREAT_ASSESSMENT = 4
};

// Can't call it radar because of namespace collision
class CRadar
{
    // TODO: move floats to doubles
    //the max range the radar can handle
    float max_range;

    //the dot with (0,0,1) indicating the farthest to the side the radar can handle.
    float max_cone;
    float lock_cone;
    float tracking_cone;

    //The minimum radius of the target
    float min_target_size;

    // What kind of type and capability the radar supports
    RadarType type;
    unsigned int capabilities;
    bool locked;
    bool can_lock;
    bool tracking_active;

    std::unique_ptr<CRadar> original;

    Computer *computer;

    friend class Armed;
    friend class Unit;
public:
    CRadar();
    CRadar(std::string unit_key, Computer* computer);

    void WriteUnitString(std::map<std::string, std::string> &unit);

    void Damage();
    void Repair();

    void Lock();
    void Unlock() {
        locked = false;
    }
    void ToggleLock() {
        locked = !locked;
    }

    void Track(const bool track) { tracking_active = track; }
    void ToggleTracking() { tracking_active = !tracking_active; }

    RadarType GetType() const; // Formerly GetBrand
    bool UseFriendFoe() const;
    bool UseObjectRecognition() const;
    bool UseThreatAssessment() const;

    float GetMaxRange() const { return max_range; }
    float GetMaxCone() const { return max_cone; }
    float GetLockCone() const { return lock_cone; }
    float GetTrackingCone() const { return tracking_cone; }
    float GetMinTargetSize() const { return min_target_size; }

    bool Locked() const { return locked; }
    bool CanLock() const { return can_lock; }
    bool Tracking() const { return tracking_active; }
};

#endif // RADAR_H
