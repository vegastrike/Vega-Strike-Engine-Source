/*
 * ecm.cpp
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

#include "ecm.h"
#include "configuration/configuration.h"
#include "unit_csv_factory.h"
#include "vs_logging.h"

#include <boost/format.hpp>

ECM::ECM() : 
    Component(), EnergyConsumer(nullptr, false, 0), ecm(Resource<int>(0, 0, 0)), active(false) {
    type = ComponentType::ECM;
}

ECM::ECM(EnergyContainer *source): 
    Component(), EnergyConsumer(source, false, 0), ecm(Resource<int>(0, 0, 0)), active(false) {
    type = ComponentType::ECM;
}


// Component Methods
void ECM::Load(std::string unit_key) {
    Component::Load(unit_key);

    double consumption = configuration()->fuel.ecm_energy_cost * static_cast<double>(ecm);
    SetConsumption(consumption);

    _upgrade(upgrade_key);
}      

void ECM::SaveToCSV(std::map<std::string, std::string>& unit) const {
    // Can't use serialize. Only supports double.
    unit["ecm"] = (boost::format("%1%/%2%") % ecm.Value() % ecm.MaxValue()).str();
}

bool ECM::CanDowngrade() const {
    return (installed && !integral);
}

bool ECM::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    ecm = Resource<int>(0, 0, 0);
    active = false;
    installed = false;
    return true;
}

bool ECM::CanUpgrade(const std::string upgrade_key) const {
    return !integral;
}

bool ECM::Upgrade(const std::string upgrade_key) {
    _upgrade(upgrade_key);
    return true;
}

void ECM::Damage() {
    if(ecm) {
        ecm--;    
    }
}

void ECM::Repair() {
    ecm.RepairFully();
}

// Energy Consumer Methods
double ECM::Consume() {
    if(!active) {
        return 0.0;
    }

    return EnergyConsumer::Consume();
}

// ECM Methods
bool ECM::Active() const {
    return active;
}

bool ECM::BreakLock(void* missile) const {
    if(!active) {
        return false;
    }

    //TODO: replace this with something better.
    // Consider strength of tracking radar.

    // There are two separate checks for ECM breaking lock here
    // TODO: pick one
    float r = rand();
    float rand_max = static_cast<float>(RAND_MAX);
    float ecm_value = static_cast<float>(ecm.Value()) * simulation_atom_var / 32768;

    if (r / rand_max < ecm_value) {
        return true;
    }

    // Second check
    uintmax_t missile_hash = reinterpret_cast<uintmax_t>(missile) / 16383ULL;

    if (static_cast<int>(missile_hash % configuration()->physics_config.max_ecm) < ecm) {
        return true;
    }

    return false;
}

int ECM::Get() const {
    return active ? ecm.Value() : 0;
}

void ECM::Set(int ecm) {
    this->ecm = Resource<int>(ecm, 0, ecm);;
}

void ECM::Toggle() {
    active = !active;
}

void ECM::_upgrade(const std::string key) {
    // Can't use serialize. Only supports double.
    std::vector<std::string> result; 
    const std::string ecm_string = UnitCSVFactory::GetVariable(key, "ecm", std::string());

    if(ecm_string.empty()) {
        return;
    }

    boost::split(result, ecm_string, boost::is_any_of("/"));
    int max_ecm = 0;
    int current_ecm = 0;

    try {
        switch(result.size()) {
        case 1:
            max_ecm = current_ecm = std::stod(result[0]);
            installed = true;
            break;
        case 2:
            current_ecm = std::stod(result[0]);
            max_ecm = std::stod(result[1]);
            installed = true;
        }
    } catch (std::invalid_argument const& ex) {
        VS_LOG(error, (boost::format("%1%: %2% trying to convert ecm_string '%3%' to int") % __FUNCTION__ % ex.what() % ecm_string));
    } catch (std::out_of_range const& ex) {
        VS_LOG(error, (boost::format("%1%: %2% trying to convert ecm_string '%3%' to int") % __FUNCTION__ % ex.what() % ecm_string));
    }
    
    ecm = Resource<int>(current_ecm, 0, max_ecm);
}