/*
 * dummy_component.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
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

#include "dummy_component.h"


DummyComponent::DummyComponent() : 
    Component() {
    type = ComponentType::Dummy;
}


// Component Methods
void DummyComponent::Load(std::string unit_key) {
    Component::Load(unit_key);
}      

void DummyComponent::SaveToCSV(std::map<std::string, std::string>& unit) const {}

bool DummyComponent::CanDowngrade() const {
    return false;
}

bool DummyComponent::Downgrade() {
    return false;
}

bool DummyComponent::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool DummyComponent::Upgrade(const std::string upgrade_key) {
    return false;
}

