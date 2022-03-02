// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * track.cpp
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


#include <algorithm>
#include "gfxlib.h"
#include "cmd/unit_generic.h"
#include "cmd/planet.h"
#include "cmd/unit_util.h"
#include "track.h"

namespace Radar {

Track::Track(Unit *player, const Unit *target)
        : player(player),
        target(target),
        distance(0.0) {
    position = player->LocalCoordinates(target);
    distance = UnitUtil::getDistance(player, target);
    type = IdentifyType();
}

Track::Track(Unit *player, const Unit *target, const Vector &position)
        : player(player),
        target(target),
        position(position) {
    distance = UnitUtil::getDistance(player, target);
    type = IdentifyType();
}

Track::Track(Unit *player, const Unit *target, const Vector &position, float distance)
        : player(player),
        target(target),
        position(position),
        distance(distance) {
    type = IdentifyType();
}

const Vector &Track::GetPosition() const {
    return position;
}

float Track::GetDistance() const {
    return distance;
}

Track::Type::Value Track::GetType() const {
    return type;
}

float Track::GetSize() const {
    assert(target);

    return target->rSize();
}

bool Track::IsExploding() const {
    assert(target);

    return target->IsExploding();
}

float Track::ExplodingProgress() const {
    assert(IsExploding());

    return target->ExplodingProgress();
}

bool Track::HasWeapons() const {
    assert(target);

    return (target->getNumMounts() > 0);
}

bool Track::HasTurrets() const {
    assert(target);

    return !(target->SubUnits.empty());
}

bool Track::HasActiveECM() const {
    assert(target);

    return (UnitUtil::getECM(target) > 0);
}

bool Track::HasLock() const {
    assert(player);
    assert(target);

    return (player == target->Target());
}

bool Track::HasWeaponLock() const {
    assert(player);
    assert(target);

    return (target->TargetLocked(player));
}

Track::Type::Value Track::IdentifyType() const {
    assert(target);

    switch (target->isUnit()) {
        case _UnitType::nebula:
            return Type::Nebula;

        case _UnitType::planet: {
            const Planet *planet = static_cast<const Planet *>(target);
            if (planet->isJumppoint()) {
                return Type::JumpPoint;
            }

            if (planet->hasLights()) {
                return Type::Star;
            }

            if (planet->isAtmospheric()) {
                return Type::Planet;
            }

            return Type::DeadPlanet;
        }
            break;

        case _UnitType::asteroid:
            return Type::Asteroid;

        case _UnitType::building:
            // FIXME: Can this ever happen?
            return Type::Unknown;

        case _UnitType::unit: {
            if (target->IsBase()) {
                return Type::Base;
            }

            if (UnitUtil::isCapitalShip(target)) {
                return Type::CapitalShip;
            }

            return Type::Ship;
        }

        case _UnitType::enhancement:
            return Type::Cargo;

        case _UnitType::missile:
            // FIXME: Is this correct?
            if (target->faction == FactionUtil::GetUpgradeFaction()) {
                return Type::Cargo;
            }

            return Type::Missile;

        default:
            assert(false);
            return Type::Unknown;
    }
}

Track::Relation::Value Track::GetRelation() const {
    assert(player);
    assert(target);

    const float relation = player->getRelation(target);
    if (relation > 0) {
        return Relation::Friend;
    }

    if (relation < 0) {
        return Relation::Enemy;
    }

    return Relation::Neutral;
}

} // namespace Radar
