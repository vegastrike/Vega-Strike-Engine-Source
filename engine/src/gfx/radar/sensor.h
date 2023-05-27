// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * sensor.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef VEGA_STRIKE_ENGINE_GFX_RADAR_SENSOR_H
#define VEGA_STRIKE_ENGINE_GFX_RADAR_SENSOR_H

#include <vector>
#include "track.h"

class Unit;
struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

namespace Radar {

// Sensor is a proxy for two types of information:
//   1. The operational parameters of the radar (e.g. range)
//   2. Information detected by the radar (e.g. tracks)
class Sensor {
public:
    typedef std::vector<Track> TrackCollection;

    struct Capability {
        enum Value {
            None = 0,
            FriendFoe = 1 << 0,
            ObjectType = 1 << 1,
            ThreatAssessment = 1 << 2
        };
    };

    struct ThreatLevel {
        enum Value {
            None, // Not attacking
            Low, // Lock
            Medium, // Capship with lock
            High // Missile
        };
    };

public:
    Sensor(Unit *player);

    Unit *GetPlayer() const;
    float GetCloseRange() const;
    float GetMaxRange() const;
    float GetMaxCone() const;
    float GetLockCone() const;
    // Does the sensor support the Identify Friend or Foe capability?
    bool UseFriendFoe() const;
    // Can the sensor distinguish ships from planets?
    bool UseObjectRecognition() const;
    // Can the sensor detect harmful ships
    bool UseThreatAssessment() const;

    Track CreateTrack(const Unit *) const;
    Track CreateTrack(const Unit *, const Vector &) const;

    // I am tracking target
    bool IsTracking(const Track &) const;
    bool InsideNebula() const;
    bool InRange(const Track &) const;

    const TrackCollection &FindTracksInRange() const;

    ThreatLevel::Value IdentifyThreat(const Track &) const;

    GFXColor GetColor(const Track &) const;

protected:
    Unit *player;
    float closeRange;
    mutable TrackCollection collection;
    bool useThreatAssessment;
};

} // namespace Radar

#endif //VEGA_STRIKE_ENGINE_GFX_RADAR_SENSOR_H
