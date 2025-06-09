/*
 * ship_functions.cpp
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

#include "ship_functions.h"
#include "component_utils.h"
#include "cmd/unit_csv_factory.h"

ShipFunctions::ShipFunctions() :
    Component(),
    cockpit(Resource<double>(1,0,1)),
    communications(Resource<double>(1,0,1)),
    fire_control(Resource<double>(1,0,1)),
    life_support(Resource<double>(1,0,1)) {
    type = ComponentType::ShipFunctions;
}


// Component Methods
void ShipFunctions::Load(std::string unit_key) {
    Component::Load(unit_key);

    std::string values[] = {"cockpit", "communications", "fire_control", "life_support"};
    Resource<double> attribute_ptrs[] = {cockpit, communications, fire_control, life_support};

    for(int i=0;i<4;i++) {
        std::string value = UnitCSVFactory::GetVariable(unit_key, values[i], std::string());
        if(value.empty()) {
            attribute_ptrs[i] = Resource<double>(1.0, 0.0, 1.0);
        } else {
            attribute_ptrs[i] = Resource<double>(value, 1.0);
        }
    }

    ftl_interdiction = UnitCSVFactory::GetVariable(unit_key, "ftl_interdiction", 0.0);
}

void ShipFunctions::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["cockpit"] = cockpit.Serialize();
    unit["communications"] = cockpit.Serialize();
    unit["fire_control"] = cockpit.Serialize();
    unit["life_support"] = cockpit.Serialize();
    unit["ftl_interdiction"] = std::to_string(ftl_interdiction);
}

bool ShipFunctions::CanDowngrade() const {
    return false;
}

bool ShipFunctions::Downgrade() {
    return false;
}

bool ShipFunctions::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool ShipFunctions::Upgrade(const std::string upgrade_key) {
    return false;
}


void ShipFunctions::Repair() {
    cockpit.RepairFully();
    communications.RepairFully();
    fire_control.RepairFully();
    life_support.RepairFully();
}

// Ship Function Methods
double ShipFunctions::Value(Function function) const {
    switch (function) {
        case Function::cockpit: return cockpit.Value();
        case Function::communications: return communications.Value();
        case Function::fire_control: return fire_control.Value();
        case Function::life_support: return life_support.Value();
        case Function::ftl_interdiction: return ftl_interdiction;
        default: return 0;
    }
}

double ShipFunctions::Max(Function function) const {
    switch (function) {
        case Function::cockpit: return cockpit.MaxValue();
        case Function::communications: return communications.MaxValue();
        case Function::fire_control: return fire_control.MaxValue();
        case Function::life_support: return life_support.MaxValue();
        default: return 0;
    }
}

double ShipFunctions::Percent(Function function) const {
    switch (function) {
        case Function::cockpit: return cockpit.Percent();
        case Function::communications: return communications.Percent();
        case Function::fire_control: return fire_control.Percent();
        case Function::life_support: return life_support.Percent();
        default: return 0;
    }
}

void ShipFunctions::Damage(Function function) {
    switch (function) {
        case Function::cockpit: cockpit.RandomDamage(); break;
        case Function::communications: communications.RandomDamage(); break;
        case Function::fire_control: fire_control.RandomDamage(); break;
        case Function::life_support: life_support.RandomDamage(); break;
        case Function::ftl_interdiction: break; // Do nothing as doesn't support damage.
    }
}

std::string ShipFunctions::GetHudText(std::string getDamageColor(double)) {
    std::string report;
    report += PrintFormattedComponentInHud(cockpit.Percent(), "Cockpit", true, getDamageColor);
    report += PrintFormattedComponentInHud(communications.Percent(), "Communications", true, getDamageColor);
    report += PrintFormattedComponentInHud(fire_control.Percent(), "Fire Control", true, getDamageColor);
    report += PrintFormattedComponentInHud(life_support.Percent(), "Life Support", true, getDamageColor);
    return report;
}
