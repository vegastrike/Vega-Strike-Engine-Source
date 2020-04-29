// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <algorithm>
#include "gfxlib.h"
#include "cmd/unit_generic.h"
#include "cmd/planet_generic.h"
#include "cmd/unit_util.h"
#include "track.h"

namespace Radar
{

Track::Track(Unit *player, const Unit *target)
    : player(player),
      target(target),
      distance(0.0)
{
    position = player->LocalCoordinates(target);
    distance = UnitUtil::getDistance(player, target);
    type = IdentifyType();
}

Track::Track(Unit *player, const Unit *target, const Vector& position)
    : player(player),
      target(target),
      position(position)
{
    distance = UnitUtil::getDistance(player, target);
    type = IdentifyType();
}

Track::Track(Unit *player, const Unit *target, const Vector& position, float distance)
    : player(player),
      target(target),
      position(position),
      distance(distance)
{
    type = IdentifyType();
}

const Vector& Track::GetPosition() const
{
    return position;
}

float Track::GetDistance() const
{
    return distance;
}

Track::Type::Value Track::GetType() const
{
    return type;
}

float Track::GetSize() const
{
    assert(target);

    return target->rSize();
}

bool Track::IsExploding() const
{
    assert(target);

    return target->IsExploding();
}

float Track::ExplodingProgress() const
{
    assert(IsExploding());

    return target->ExplodingProgress();
}

bool Track::HasWeapons() const
{
    assert(target);

    return (target->GetNumMounts() > 0);
}

bool Track::HasTurrets() const
{
    assert(target);

    return !(target->SubUnits.empty());
}

bool Track::HasActiveECM() const
{
    assert(target);

    return (UnitUtil::getECM(target) > 0);
}

bool Track::HasLock() const
{
    assert(player);
    assert(target);

    return (player == target->Target());
}

bool Track::HasWeaponLock() const
{
    assert(player);
    assert(target);

    return (target->TargetLocked(player));
}

Track::Type::Value Track::IdentifyType() const
{
    assert(target);

    switch (target->isUnit())
    {
    case NEBULAPTR:
        return Type::Nebula;

    case PLANETPTR:
        {
            const Planet *planet = static_cast<const Planet *>(target);
            if (planet->isJumppoint())
                return Type::JumpPoint;

            if (planet->hasLights())
                return Type::Star;

            if (planet->isAtmospheric())
                return Type::Planet;

            return Type::DeadPlanet;
        }
        break;

    case ASTEROIDPTR:
        return Type::Asteroid;

    case BUILDINGPTR:
        // FIXME: Can this ever happen?
        return Type::Unknown;

    case UNITPTR:
        {
            if (target->IsBase())
                return Type::Base;

            if (UnitUtil::isCapitalShip(target))
                return Type::CapitalShip;

            return Type::Ship;
        }

    case ENHANCEMENTPTR:
        return Type::Cargo;

    case MISSILEPTR:
        // FIXME: Is this correct?
        if (target->faction == FactionUtil::GetUpgradeFaction())
            return Type::Cargo;

        return Type::Missile;

    default:
        assert(false);
        return Type::Unknown;
    }
}

Track::Relation::Value Track::GetRelation() const
{
    assert(player);
    assert(target);

    const float relation = player->getRelation(target);
    if (relation > 0)
        return Relation::Friend;

    if (relation < 0)
        return Relation::Enemy;

    return Relation::Neutral;
}

} // namespace Radar
