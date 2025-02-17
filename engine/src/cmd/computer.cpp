/*
 * computer.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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


#include "computer.h"
#include "configuration/configuration.h"

Computer::Computer() : NavPoint(0, 0, 0),
        velocity_ref(nullptr),
        force_velocity_ref(false),
        threatlevel(0),
        set_speed(0),
        slide_start(1),
        slide_end(1),
        itts(false),
        combat_mode(true) {
}

Computer::RADARLIM::RADARLIM() :
        maxrange(0),
        maxcone(-1),
        lockcone(0),
        trackingcone(0),
        mintargetsize(0),
        capability(Capability::IFF_NONE),
        locked(false),
        canlock(false),
        trackingactive(true) {

    maxrange = configuration()->computer_config.default_max_range;
    trackingcone = configuration()->computer_config.default_tracking_cone;
    lockcone = configuration()->computer_config.default_lock_cone;
}

Computer::RADARLIM::Brand::Value Computer::RADARLIM::GetBrand() const {
    switch (capability & Capability::IFF_UPPER_MASK) {
        case Capability::IFF_SPHERE:
            return Brand::SPHERE;
        case Capability::IFF_BUBBLE:
            return Brand::BUBBLE;
        case Capability::IFF_PLANE:
            return Brand::PLANE;
        default:
            assert(false);
            return Brand::SPHERE;
    }
}

bool Computer::RADARLIM::UseFriendFoe() const {
    // Backwardscompatibility
    if (capability == 0) {
        return false;
    } else if ((capability == 1) || (capability == 2)) {
        return true;
    }

    return (capability & Capability::IFF_FRIEND_FOE);
}

bool Computer::RADARLIM::UseObjectRecognition() const {
    // Backwardscompatibility
    if ((capability == 0) || (capability == 1)) {
        return false;
    } else if (capability == 2) {
        return true;
    }

    return (capability & Capability::IFF_OBJECT_RECOGNITION);
}

bool Computer::RADARLIM::UseThreatAssessment() const {
    return (capability & Capability::IFF_THREAT_ASSESSMENT);
}
