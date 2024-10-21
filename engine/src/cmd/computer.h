/**
 * computer.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COMPUTER_H
#define VEGA_STRIKE_ENGINE_CMD_COMPUTER_H

#include "container.h"
#include "config.h"
#include "vs_globals.h"
#include "configxml.h"

/**
 * The computer holds all data in the navigation computer of the current unit
 * It is outside modifyable with GetComputerData() and holds only volatile
 * Information inside containers so that destruction of containers will not
 * result in segfaults.
 * Maximum speeds and turning restrictions are merely facts of the computer
 * and have nothing to do with the limitations of the physical nature
 * of space combat
 */
class Computer {
public:
    class RADARLIM {
    public:
        struct Brand {
            enum Value {
                SPHERE = 0,
                BUBBLE = 1,
                PLANE = 2
            };
        };
        struct Capability {
            enum Value {
                // For internal use
                IFF_UPPER_SHIFT = 16,
                IFF_LOWER_MASK = (1 << IFF_UPPER_SHIFT) - 1,
                IFF_UPPER_MASK = ~IFF_LOWER_MASK,

                // The lower 16 bits
                IFF_NONE = 0,
                IFF_FRIEND_FOE = 1 << 0,
                IFF_OBJECT_RECOGNITION = 1 << 1,
                IFF_THREAT_ASSESSMENT = 1 << 2,

                // The upper 16 bits
                IFF_SPHERE = Brand::SPHERE << IFF_UPPER_SHIFT,
                IFF_BUBBLE = Brand::BUBBLE << IFF_UPPER_SHIFT,
                IFF_PLANE = Brand::PLANE << IFF_UPPER_SHIFT
            };
        };
        //the max range the radar can handle
        float maxrange;
        //the dot with (0,0,1) indicating the farthest to the side the radar can handle.
        float maxcone;
        float lockcone;
        float trackingcone;
        //The minimum radius of the target
        float mintargetsize;
        // What kind of type and capability the radar supports
        int capability;
        bool locked;
        bool canlock;
        bool trackingactive;

        Brand::Value GetBrand() const;
        bool UseFriendFoe() const;
        bool UseObjectRecognition() const;
        bool UseThreatAssessment() const;

        RADARLIM();
    };

    RADARLIM radar;
    bool ecmactive;
    //The nav point the unit may be heading for
    Vector NavPoint;
    //The target that the unit has in computer
    UnitContainer target;
    //Any target that may be attacking and has set this threat
    UnitContainer threat;
    //Unit that it should match velocity with (not speed) if null, matches velocity with universe frame (star)
    UnitContainer velocity_ref;
    bool force_velocity_ref;
    //The threat level that was calculated from attacking unit's threat
    float threatlevel;
    //The speed the flybywire system attempts to maintain
    float set_speed;
    
    //Whether or not an 'lead' indicator appears in front of target
    unsigned char slide_start;
    unsigned char slide_end;
    bool itts;
    
    // In hud - Maneuver (true) Travel (false)
    bool combat_mode;

    Computer();
};

#endif //VEGA_STRIKE_ENGINE_CMD_COMPUTER_H
