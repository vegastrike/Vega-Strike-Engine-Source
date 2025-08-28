/*
 * cargo_hold.cpp
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

#include "cargo_hold.h"
#include "components_manager.h"

#include "vs_logging.h"
#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"

#include <numeric>

CargoHold::CargoHold(HoldType hold_type) : 
    Component(), hold_type(hold_type) {
    type = ComponentType::Dummy;
}


// Component Methods
void CargoHold::Load(std::string unit_key) {
    Component::Load(unit_key);

    switch(hold_type) {
        case HoldType::cargo:
            capacity = Resource<double>(0,0,UnitCSVFactory::GetVariable(unit_key, "Hold_Volume", 0.0f));
            break;
        case HoldType::hidden:
            capacity = Resource<double>(0,0,UnitCSVFactory::GetVariable(unit_key, "Hidden_Hold_Volume", 0.0f));
            break;
        case HoldType::upgrade:
            capacity = Resource<double>(0,0,UnitCSVFactory::GetVariable(unit_key, "Upgrade_Storage_Volume", 0.0f));
            break;
    }
}      

void CargoHold::SaveToCSV(std::map<std::string, std::string>& unit) const {}

bool CargoHold::CanDowngrade() const {
    return false;
}

bool CargoHold::Downgrade() {
    return false;
}

bool CargoHold::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool CargoHold::Upgrade(const std::string upgrade_key) {
    return false;
}

// Cargo Hold Methods
bool CargoHold::CanAddCargo(const Cargo &cargo) const {
    // Always can, in this case (this accounts for some odd precision issues)
    if ((cargo.quantity == 0) || (cargo.GetVolume() == 0)) {
        return true;
    }
    
    //Test volume availability
    double total_volume = cargo.quantity.Value() * cargo.GetVolume();
    return total_volume <= capacity.AdjustedValue() - capacity.Value();
}

Cargo CargoHold::RemoveCargo(ComponentsManager *manager, const std::string& name, 
                           int quantity) {
    int index = GetIndex(name);

    if(index == -1) {
        VS_LOG(error, "(previously) FATAL problem...removing cargo that is not in the hold.");
        Cargo cargo;
        cargo.quantity = 0;
        return cargo;
    }

    return RemoveCargo(manager, index, quantity);
}

Cargo CargoHold::RemoveCargo(ComponentsManager *manager, unsigned int index, 
                           int quantity) {
    if (index >= _items.size()) {
        VS_LOG(error, "(previously) FATAL problem...removing cargo that is past the end of array bounds.");
        return Cargo();
    }

    Cargo& cargo_in_hold = GetCargo(index);
    Cargo cargo_to_remove(cargo_in_hold);

    // Adjust quantity - can't exceed what's actually in hold
    quantity = std::min(quantity, cargo_in_hold.quantity.Value());
    cargo_to_remove.quantity = quantity;
    cargo_in_hold.quantity -= quantity;

    capacity -= quantity * cargo_to_remove.volume;

    if(configuration().physics.use_cargo_mass) {
        manager->mass -= quantity * cargo_to_remove.mass;
    }

    if (cargo_in_hold.quantity == 0) {
        _items.erase(_items.begin() + index);
    }
    return cargo_to_remove;
}

void CargoHold::AddCargo(ComponentsManager *manager, const Cargo &cargo, bool sort) {
    if(configuration().physics.use_cargo_mass) {
        manager->mass += cargo.quantity.Value() * cargo.mass;
    }

    bool found = false;

    for(Cargo& c: _items) {
        if(c.name == cargo.name && c.category == cargo.category) {
            found = true;
            c.quantity += cargo.quantity.Value();
        }
    }

    if(!found) {
        _items.push_back(cargo);
    }

    capacity += cargo.quantity.Value() * cargo.volume;
    
    if (sort) {
        std::sort(_items.begin(), _items.end());
    }
}

void CargoHold::Clear() {
    _items.clear();
}


Cargo& CargoHold::GetCargo(unsigned int i) {
    return _items[i];
}

const Cargo& CargoHold::GetCargo(unsigned int i) const {
    return _items[i];
}

double CargoHold::CurrentCapacity() const {
    return capacity.Value();
}

double CargoHold::MaxCapacity() const {
    return capacity.AdjustedValue();
}

double CargoHold::AvailableCapacity() const {
    return capacity.AdjustedValue() - capacity.Value();
}

std::string CargoHold::Serialize() const {
    std::string cargo_hold_text;

    for(const Cargo& c : _items) {
        cargo_hold_text += c.Serialize();
    }

    return cargo_hold_text;
}

/* There's a whole bunch of stuff that needs fleshing out:
   1. Hitchhikers disembark at next port automatically.
   2. Enslave doesn't affect paying customers on first click.
   3. Enslave starts with hitchhikers (potential enemy pilots)
   */
void CargoHold::Enslave() {
    // TODO: take HoldType into account
    // Get number of none-slave passengers and erase passengers
    int none_slave_passengers = std::accumulate(_items.begin(), _items.end(), 0,
        [](int current_sum, Cargo c) {
        if (c.IsPassenger() && !c.IsSlave()) {
            return current_sum + c.GetQuantity();
        } else {
            return current_sum;
        }
    });

    // If there are no passengers, exit
    if(none_slave_passengers == 0) {
        return;
    }

    // Delete all passengers
    _items.erase(std::remove_if(_items.begin(), _items.end(), 
            [](Cargo& c) {
        return (c.IsPassenger() && !c.IsSlave());
    }), _items.end());

    // Find the first slaves instance if exists
    auto it = std::find_if(_items.begin(), _items.end(), [](const Cargo& c) {
        return c.IsSlave();
    });

    // Look for existing slaves
    if (it != _items.end()) {
        // Found slaves
        // Get a pointer to the found element
        Cargo* existing_slaves = &(*it); 
        existing_slaves->Add(none_slave_passengers);
    } else {
        // Not found. Create a new instance
        // TODO: name should come from config.
        Cargo slaves = Manifest::MPL().GetCargoByName("Slaves");
        slaves.SetQuantity(none_slave_passengers);
        _items.push_back(slaves);
    }
}

// TODO: no need to be here. move to carrier
void CargoHold::Free() {
    auto slave_it = std::find_if(_items.begin(), _items.end(), [](const Cargo& c) {
        // TODO: name should come from config.
        return c.GetName() == "Slaves";
    });

    // No slaves to free found, exiting.
    if (slave_it == _items.end()) {
        return;
    }

    // Find the first hitchhikers instance if exists
    auto hitch_it = std::find_if(_items.begin(), _items.end(), [](const Cargo& c) {
        // TODO: name should come from config.
        return c.GetName() == "Hitchhiker";
    });

    // No hitchhikers found, modify slaves instance.
    if (hitch_it == _items.end()) {
        // TODO: name should come from config.
        Cargo* existing_slaves = &(*slave_it); 
        existing_slaves->SetName("Hitchhiker");
        existing_slaves->SetCategory("Passengers");
    } else {
        Cargo* existing_hitchhikers = &(*hitch_it);
        Cargo* existing_slaves = &(*slave_it);
        existing_hitchhikers->Add(existing_slaves->GetQuantity());
        _items.erase(slave_it);
    }
    
}

