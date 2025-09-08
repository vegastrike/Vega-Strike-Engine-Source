/*
 * computer.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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


#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_COMPUTER_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_COMPUTER_H

#include "component.h"
#include "damage/core_vector.h"

class Unit;

// A stand-in for Unit
class ComponentsManager;

/**
 * The computer holds all data in the navigation computer of the current unit.
 * Maximum speeds and turning restrictions are merely facts of the computer
 * and have nothing to do with the limitations of the physical nature
 * of space combat
 */
class Computer : public Component {
    friend class Unit;

    // These are private as they require a getter that can cast them to
    // the "correct" "subclass". This is Unit for ComponentsManager and
    // Vector for CoreVector.

    //The nav point the unit may be heading for
    CoreVector nav_point;

    //The target that the unit has in computer
    ComponentsManager* target;
    //Any target that may be attacking and has set this threat

    ComponentsManager* threat;

    //Unit that it should match velocity with (not speed) if null, matches velocity with universe frame (star)
    ComponentsManager* velocity_reference;

    public:
    bool force_velocity_ref;

    //The threat level that was calculated from attacking unit's threat
    float threatlevel;

    //The speed the flybywire system attempts to maintain
    float set_speed;

    //Whether or not an 'lead' indicator appears in front of target
    unsigned char slide_start;
    unsigned char slide_end;

    // TODO: implement a better damage model, where stuff like target and NavPoint
    // can be disabled.

    bool original_itts;
    bool itts;

    // In hud - Maneuver (true) Travel (false)
    bool combat_mode;


public:
    Computer();

    // Component Methods
    void Load(std::string unit_key) override;

    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;
    bool Downgrade() override;
    bool CanUpgrade(const std::string upgrade_key) const override;
    bool Upgrade(const std::string upgrade_key) override;

    void Damage() override;
    void DamageByPercent(double percent) override;
    void Repair() override;

    ~Computer() override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_COMPUTER_H
