/*
 * track.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GFX_RADAR_TRACK_H
#define VEGA_STRIKE_ENGINE_GFX_RADAR_TRACK_H

#include "gfx/vec.h"

class Unit;
struct GFXColor;  // Edit from class to struct as defined in gfxlib_struct.

namespace Radar {

class Sensor;

// Track is a wrapper for Unit that restricts the available functionality
// to that which can be detected through the radar.
class Track {
public:
    struct Type {
        enum Value {
            Unknown,
            Nebula,
            Star,
            Planet,
            DeadPlanet,
            JumpPoint,
            Asteroid,
            Base,
            CapitalShip,
            Ship,
            Cargo,
            Missile
        };
    };

    struct Relation {
        enum Value {
            Neutral,
            Friend,
            Enemy
        };
    };

public:
    // Get the track position relative to myself
    const Vector &GetPosition() const;
    // Get the relative distance between the track and me
    float GetDistance() const;
    // Get the absolute size of the track
    float GetSize() const;
    Type::Value GetType() const;

    // Track is exploding
    bool IsExploding() const;
    // The percentage of how far the explosion has come [0; 1]
    float ExplodingProgress() const;

    // Track is equipped with weapons
    bool HasWeapons() const;
    // Track is equipped with turrets
    bool HasTurrets() const;
    // Track has activated ECM
    bool HasActiveECM() const;

    // Track is tracking me
    bool HasLock() const;
    // Track has a weapon lock on me
    bool HasWeaponLock() const;

    // Determine if track is friend or foe
    Relation::Value GetRelation() const;

protected:
    // Produced by Sensor::CreateTrack
    friend class Sensor;
    Track(Unit *, const Unit *);
    Track(Unit *, const Unit *, const Vector &);
    Track(Unit *, const Unit *, const Vector &, float);

    Type::Value IdentifyType() const;

protected:
    Unit *player;
    const Unit *target;
    Vector position;
    float distance;
    Type::Value type;
};

} // namespace Radar

#endif //VEGA_STRIKE_ENGINE_GFX_RADAR_TRACK_H
