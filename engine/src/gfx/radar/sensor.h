// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * sensor.h
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


#ifndef VEGA_STRIKE_ENGINE_GFX_RADAR_SENSOR_H
#define VEGA_STRIKE_ENGINE_GFX_RADAR_SENSOR_H

#include <vector>
#include "track.h"

class Unit;
struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

// TODO: this is basically a wrapper over CRadar. Remove

namespace Radar {

// Sensor is a proxy for two types of information:
//   1. The operational parameters of the radar (e.g. range)
//   2. Information detected by the radar (e.g. tracks)
//
// Blink rate for SPEC repulsor blips. Slower than the threat rates (7.5 medium /
// 20 fast) so a repulsor does not read as an incoming threat.
constexpr float kRepulsorBlinkRate = 5.0f;

// Radar blip size (pixels) for a repulsor: scaled linearly by its effect on SPEC,
// from a single pixel at the bubble edge up to five at the object.
constexpr float kRepulsorBlipSizeMin = 1.0f;
constexpr float kRepulsorBlipSizeMax = 5.0f;

inline float RepulsorBlipSize(double effect) {
    return kRepulsorBlipSizeMin
            + static_cast<float>(effect) * (kRepulsorBlipSizeMax - kRepulsorBlipSizeMin);
}

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
    // Is the track an object currently compressing our SPEC field -- within the
    // clear-space bubble the autopilot steers around? Always false when SPEC is
    // not active, or for our own ship and our target.
    bool IsRepulsor(const Track &) const;
    // How strongly the track compresses the SPEC field: the autopilot's proximity
    // weight (1 - sig / bubble), 0 when it is not a repulsor at all.
    double GetRepulsorEffect(const Track &) const;
    // Is the SPEC (in-system ftl) drive currently active on the player ship?
    bool IsSpecActive() const;
    // The colour a tracked target's radar cross is drawn in while SPEC is active.
    GFXColor GetSpecTargetColor() const;
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
