// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
 * cockpit_events.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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

#ifndef COCKPITEVENTS_H
#define COCKPITEVENTS_H

// TODO: convert to enum class
enum CockpitEvent {
    EVENTID_FIRST,

    /// Warp is possible at this time
    WARP_READY = EVENTID_FIRST,

    /// Warp ceased to be possible at this time
    WARP_UNREADY,

    /// Warp started
    WARP_ENGAGED,

    /// Warp stopped
    WARP_DISENGAGED,

    /// Asap autopilot engaged
    ASAP_ENGAGED,

    /// Asap autopilot disengaged
    ASAP_DISENGAGED,

    /// Asap docking available
    ASAP_DOCKING_AVAILABLE,

    /// Asap docking engaged
    ASAP_DOCKING_ENGAGED,

    /// Asap docking disengaged
    ASAP_DOCKING_DISENGAGED,

    /// Docking ready
    DOCK_AVAILABLE,

    /// Docking no longer ready
    DOCK_UNAVAILABLE,

    /// Docking triggered yet not ready
    DOCK_FAILED,

    /// Jump ready
    JUMP_AVAILABLE,

    /// Jump no longer ready
    JUMP_UNAVAILABLE,

    /// Jump triggered yet not ready
    JUMP_FAILED,

    /// Weapons lock warning active
    LOCK_WARNING,

    /// Missile lock warning acgive
    MISSILELOCK_WARNING,

    /// Eject light
    EJECT_WARNING,

    /// Governor enabled
    FLIGHT_COMPUTER_ENABLED,

    /// Governor disabled
    FLIGHT_COMPUTER_DISABLED,

    /// Warp loop, warp speed 0 (+1 = warp 1, +2 = warp 2, etc...)
    WARP_LOOP0,
    WARP_LOOP9 = WARP_LOOP0 + 9,
    /// Last warp level
    WRAP_LOOPLAST = WARP_LOOP9,

    /// Warp threshold, warp speed 0 (+1 = warp 1, +2 = warp 2, etc...)
    WARP_SKIP0,
    WARP_SKIP9 = WARP_SKIP0 + 9,
    /// Last warp level
    WRAP_SKIPLAST = WARP_SKIP9,

    /// Just after all valid values
    NUM_EVENTS
};

#endif // COCKPITEVENTS_H
