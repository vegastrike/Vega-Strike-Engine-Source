/*
 * computer.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "computer.h"


Computer::Computer() : 
        nav_point(0, 0, 0),
        target(nullptr),
        threat(nullptr),
        velocity_reference(nullptr),
        force_velocity_ref(false),
        threatlevel(0),
        set_speed(0),
        slide_start(1),
        slide_end(1),
        original_itts(false),
        itts(false),
        combat_mode(true) {
}


// Component Methods
void Computer::Load(std::string unit_key) {
    Component::Load(unit_key);
}      

void Computer::SaveToCSV(std::map<std::string, std::string>& unit) const {}

bool Computer::CanDowngrade() const {
    return false;
}

bool Computer::Downgrade() {
    return false;
}

bool Computer::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool Computer::Upgrade(const std::string upgrade_key) {
    return false;
}

void Computer::Damage() {
    itts = false;

    // We calculate percent operational as a simple average
    operational = itts != original_itts ? 0 : 1;
}

void Computer::DamageByPercent(double percent) {
    itts = false;

    // We calculate percent operational as a simple average
    operational = itts != original_itts ? 0 : 1;
}

void Computer::Repair() {
    itts = original_itts;    

    operational = 1.0;
}