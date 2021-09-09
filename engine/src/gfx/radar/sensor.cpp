/**
* sensor.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "cmd/unit_generic.h"
#include "cmd/planet.h"
#include "cmd/unit_util.h"
#include "cmd/unit_find.h"
#include "sensor.h"
#include "universe.h"

extern Unit *getTopLevelOwner(); // WTF... located in star_system_generic.cpp

namespace Radar
{

Sensor::Sensor(Unit *player)
    : player(player),
      closeRange(30000.0),
      useThreatAssessment(false)
{
}

Unit *Sensor::GetPlayer() const
{
    return player;
}

bool Sensor::UseFriendFoe() const
{
    assert(player);

    return player->GetComputerData().radar.UseFriendFoe();
}

bool Sensor::UseObjectRecognition() const
{
    assert(player);

    return player->GetComputerData().radar.UseObjectRecognition();
}

bool Sensor::UseThreatAssessment() const
{
    assert(player);

    return player->GetComputerData().radar.UseThreatAssessment();
}

float Sensor::GetCloseRange() const
{
    return closeRange;
}

float Sensor::GetMaxRange() const
{
    assert(player);

    return player->GetComputerData().radar.maxrange;
}

float Sensor::GetMaxCone() const
{
    assert(player);

    return player->GetComputerData().radar.maxcone;
}

float Sensor::GetLockCone() const
{
    assert(player);

    return player->GetComputerData().radar.lockcone;
}

Track Sensor::CreateTrack(const Unit *target) const
{
    assert(player);

    return Track(player, target);
}

Track Sensor::CreateTrack(const Unit *target, const Vector& position) const
{
    assert(player);

    return Track(player, target, position);
}

bool Sensor::IsTracking(const Track& track) const
{
    assert(player);

    return (track.target == player->Target());
}

bool Sensor::InsideNebula() const
{
    assert(player);

    return (player->GetNebula() != NULL);
}

bool Sensor::InRange(const Track& track) const
{
    return (track.GetDistance() <= GetMaxRange());
}

// FIXME: This is a convoluted way to obtain the radar tracks

class CollectRadarTracks
{
public:
    CollectRadarTracks()
        : sensor(NULL),
          player(NULL),
          collection(NULL)
    {}

    void init(const Sensor *sensor, Sensor::TrackCollection *collection, Unit *player)
    {
        this->sensor = sensor;
        this->collection = collection;
        this->player = player;
    }

    bool acquire(const Unit *target, float distance)
    {
        assert(sensor);
        assert(collection);
        assert(player);
        assert(target);

        static bool draw_significant_blips =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_significant_blips", "true" ) );
        static bool untarget_out_cone  =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "untarget_beyond_cone", "false" ) );
        static float minblipsize =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "min_radarblip_size", "0" ) );

        if (target != player) {
            const bool isCurrentTarget = (player->Target() == target);
            double dummy;
            if (!player->InRange( target, dummy, isCurrentTarget && untarget_out_cone, true, true ))
            {
                if (isCurrentTarget)
                    player->Target(NULL);
                return true;
            }
            if (!isCurrentTarget &&
                !draw_significant_blips &&
                (getTopLevelOwner() == target->owner) &&
                (distance > player->GetComputerData().radar.maxrange))
                return true;

            // Blips will be sorted later as different radars need to sort them differently
            if (target->rSize() > minblipsize)
            {
                collection->push_back(sensor->CreateTrack(target));
            }
            if (target->isPlanet() == _UnitType::planet && target->radial_size > 0)
            {
                const Unit *sub = NULL;
                for (un_kiter i = target->viewSubUnits(); (sub = *i) != NULL; ++i)
                {
                    if (target->rSize() > minblipsize)
                    {
                        collection->push_back(sensor->CreateTrack(sub));
                    }
                }
            }
        }
        return true;
    }

private:
    const Sensor *sensor;
    Unit *player;
    Sensor::TrackCollection *collection;
};

// FIXME: Scale objects according to distance and ignore those below a given threshold (which improves with better sensors)
const Sensor::TrackCollection& Sensor::FindTracksInRange() const
{
    assert(player);

    collection.clear();

    // Find all units within range
    static float maxUnitRadius =
        XMLSupport::parse_float(vs_config->getVariable("graphics", "hud", "radar_search_extra_radius", "1000"));
    static bool allGravUnits =
        XMLSupport::parse_bool(vs_config->getVariable("graphics", "hud", "draw_gravitational_objects", "true"));

    UnitWithinRangeLocator<CollectRadarTracks> unitLocator(GetMaxRange(), maxUnitRadius);
    unitLocator.action.init(this, &collection, player);
    if (! is_null(player->location[Unit::UNIT_ONLY]))
    {
        findObjects(_Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY],
                    player->location[Unit::UNIT_ONLY],
                    &unitLocator);
    }
    if (allGravUnits)
    {
        Unit *target = player->Target();
        const Unit *gravUnit;
        bool foundtarget = false;
        for (un_kiter i = _Universe->activeStarSystem()->gravitationalUnits().constIterator();
             (gravUnit = *i) != NULL;
             ++i)
        {
            unitLocator.action.acquire(gravUnit, UnitUtil::getDistance(player, gravUnit));
            if (gravUnit == target)
                foundtarget = true;
        }
        if (target && !foundtarget)
            unitLocator.action.acquire(target, UnitUtil::getDistance(player, target));
    }
    return collection;
}

Sensor::ThreatLevel::Value Sensor::IdentifyThreat(const Track& track) const
{
    assert(player);

    if (!UseThreatAssessment())
        return ThreatLevel::None;

    if (track.IsExploding())
        return ThreatLevel::None;

    if (track.HasWeaponLock())
    {
        // I am being targetted by...
        switch (track.GetType())
        {
        case Track::Type::Missile:
            return ThreatLevel::High;

        case Track::Type::CapitalShip:
            if (track.GetRelation() == Track::Relation::Enemy)
                return ThreatLevel::Medium;
            break;

        case Track::Type::Ship:
            if (track.HasTurrets())
                return ThreatLevel::Medium;

            if (!track.HasWeapons())
            {
                // So what are you going to threaten me with? Exhaustion gas?
                return ThreatLevel::None;
            }
            break;

        default:
            break;
        }
        return ThreatLevel::Low;
    }
    return ThreatLevel::None;
}

GFXColor Sensor::GetColor(const Track& track) const
{
    assert(player);

    static GFXColor friendColor  = vs_config->getColor("friend", GFXColor(-1, -1, -1, -1));
    static GFXColor enemyColor   = vs_config->getColor("enemy", GFXColor(-1, -1, -1, -1));
    static GFXColor neutralColor = vs_config->getColor("neutral", GFXColor(-1, -1, -1, -1));
    static GFXColor baseColor    = vs_config->getColor("base", GFXColor( -1, -1, -1, -1 ));
    static GFXColor planetColor  = vs_config->getColor("planet", GFXColor( -1, -1, -1, -1 ));
    static GFXColor jumpColor    = vs_config->getColor("jump", GFXColor( 0, 1, 1, .8 ));
    static GFXColor starColor    = vs_config->getColor("star", GFXColor( 1, 1, 1, 1 ));
    static GFXColor missileColor = vs_config->getColor("missile", GFXColor( .25, 0, .5, 1 ));
    static GFXColor cargoColor   = vs_config->getColor("cargo", GFXColor( .6, .2, 0, 1 ));
    static GFXColor noColor      = vs_config->getColor("black_and_white", GFXColor( .5, .5, .5 ));

    Track::Type::Value trackType = track.GetType();
    ThreatLevel::Value threatLevel = IdentifyThreat(track);

    if (UseThreatAssessment())
    {
        switch (trackType)
        {
        case Track::Type::Missile:
            return missileColor;

        default:
            break;
        }
    }

    if (UseObjectRecognition())
    {
        switch (trackType)
        {
        case Track::Type::Nebula:
        case Track::Type::Star:
            return starColor;

        case Track::Type::Planet:
        case Track::Type::DeadPlanet:
            return planetColor;

        case Track::Type::Base:
            if (track.HasLock())
                return enemyColor;
            return baseColor;

        case Track::Type::JumpPoint:
            return jumpColor;

        default:
            break;
        }
    }

    if (UseFriendFoe())
    {
        switch (trackType)
        {
        case Track::Type::CapitalShip:
        case Track::Type::Ship:
            {
                if (track.HasLock())
                    return enemyColor;

                switch (track.GetRelation())
                {
                case Track::Relation::Friend:
                    return friendColor;

                case Track::Relation::Enemy:
                    if (UseThreatAssessment() && (threatLevel == ThreatLevel::None))
                        return neutralColor;

                    return enemyColor;

                default:
                    return neutralColor;
                }
            }

        case Track::Type::Cargo:
            return cargoColor;

        default:
            break;
        }
    }
    return noColor;
}

} // namespace Radar
