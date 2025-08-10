/*
 * cargo.cpp
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


#include "resource/cargo.h"
#include "resource/manifest.h"

#include <numeric>
#include <iostream>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>


// A simple utility function to parse a boolean value
static bool _parse_bool(const std::string& value) {
    return (value == "1" || value == "true");
}



Cargo::Cargo(): Product() {
    mass = 0;
    volume = 0;
    mission = false;
    installed = false;
    functionality = max_functionality = 1.0f;
}

Cargo::Cargo(std::string name, std::string category, float price, int quantity,
             float mass, float volume, float functionality, float max_functionality,
             bool mission, bool component, bool installed, bool integral,
             bool weapon, bool passenger, bool slave):
             Product(name, quantity, price), category(category), description(""),
             mass(mass), volume(volume), functionality(functionality),
             max_functionality(max_functionality), mission(mission),
             component(component), installed(installed), integral(integral),
             weapon(weapon), passenger(passenger), slave(slave) {}
/*  0 name
    1 category
    2 price
    3 quantity
    4 mass
    5 volume
    6 func
    7 max_func
    8 description
    9 mission
    10 installed
    11 integral
    12 component
    13 weapon
    14 passenger
    15 slave
*/
Cargo::Cargo(std::string& cargo_text) {
    std::vector<std::string> cargo_parts;
    boost::split(cargo_parts,cargo_text,boost::is_any_of(";"));

    for(long unsigned int i=0;i<cargo_parts.size();i++) {
        switch (i) {
        case 0: name = cargo_parts[0]; break;
        case 1: category = cargo_parts[1]; break;
        case 2: price = std::stod(cargo_parts[2]); break;
        case 3: quantity = std::stoi(cargo_parts[3]); break;
        case 4: mass = std::stod(cargo_parts[4]); break;
        case 5: volume = std::stod(cargo_parts[5]); break;
        case 6: functionality = Resource<double>(std::stod(cargo_parts[6]), 0.0, 1.0); break;
        case 7: break; // max_functionality is always 1.0. cargo_parts[6] not used.
        case 8: description = cargo_parts[8]; break;
        case 9: mission = _parse_bool(cargo_parts[9]); break;
        case 10: installed = _parse_bool(cargo_parts[10]); break;
        case 11: integral = _parse_bool(cargo_parts[11]); break;
        case 12: component = _parse_bool(cargo_parts[12]); break;
        case 13: weapon = _parse_bool(cargo_parts[13]); break;
        case 14: passenger = _parse_bool(cargo_parts[14]); break;
        case 15: slave = _parse_bool(cargo_parts[15]); break;

        default:
            break;
        }
    }
}

std::string Cargo::Serialize() const {
    return (boost::format("{%s;%s;%f;%d;%f;%f;%f;%f;%s;%d;%d;%d;%d;%d;%d;%d}")
        % this->name % this->category % this->price % this->quantity.Value() % this->mass
        % this->volume % this->functionality % 1.0 % this->description % this->mission
        % this->installed % this->integral % this->component % this->weapon %
        this->passenger % this->slave
    ).str();
}

float Cargo::GetFunctionality() {
    return functionality;
}

float Cargo::GetMaxFunctionality() {
    return max_functionality;
}

void Cargo::SetDescription(const std::string &description) {
    this->description = description;
}

void Cargo::SetFunctionality(float func) {
    functionality = func;
}

void Cargo::SetInstalled(bool installed) {
    this->installed = installed;
}

void Cargo::SetIntegral(bool installed) {
    this->integral = installed;
}

void Cargo::SetMaxFunctionality(float func) {
    max_functionality = func;
}

void Cargo::SetMissionFlag(bool flag) {
    this->mission = flag;
}

void Cargo::SetPrice(float price) {
    this->price = price;
}

void Cargo::SetMass(float mass) {
    this->mass = mass;
}

void Cargo::SetVolume(float vol) {
    this->volume = vol;
}

void Cargo::SetQuantity(int quantity) {
    this->quantity = quantity;
}

void Cargo::SetContent(const std::string &content) {
    this->name = content;
}

void Cargo::SetCategory(const std::string &category) {
    this->category = category;
}

void Cargo::SetSlave(bool slave) {
    this->slave = slave;
}

bool Cargo::IsMissionFlag() const {
    return mission;
}

bool Cargo::IsComponent() const {
    return component;
}

bool Cargo::IsInstalled() const {
    return installed;
}

bool Cargo::IsIntegral() const {
    return integral;
}

bool Cargo::IsWeapon() const {
    return weapon;
}

bool Cargo::IsPassenger() const {
    return passenger;
}

bool Cargo::IsSlave() const {
    return slave;
}

const std::string &Cargo::GetCategory() const {
    return category;
}

const std::string &Cargo::GetContent() const {
    return name;
}

const std::string &Cargo::GetDescription() const {
    return description;
}



std::string Cargo::GetCategoryPython() {
    return category;
}

std::string Cargo::GetContentPython() {
    return name;
}

std::string Cargo::GetDescriptionPython() {
    return description;
}

int Cargo::GetQuantity() const {
    return (quantity.Value());
}

float Cargo::GetVolume() const {
    return volume;
}

float Cargo::GetMass() const {
    return mass;
}

float Cargo::GetPrice() const {
    return price;
}




bool Cargo::operator==(const Cargo &other) const {
    return name == other.name;
}

bool Cargo::operator<(const Cargo &other) const {
    return (category == other.category) ? (name < other.name) : (category < other.category);
}


void Cargo::Add(int quantity) {
    this->quantity += quantity;
}

// TODO: no need to be here. move to carrier
/* There's a whole bunch of stuff that needs fleshing out:
   1. Hitchhikers disembark at next port automatically.
   2. Enslave doesn't affect paying customers on first click.
   3. Enslave starts with hitchhikers (potential enemy pilots)
   */
void Enslave(std::vector<Cargo>& ship_manifest) {
    // Get number of none-slave passengers and erase passengers
    int none_slave_passengers = std::accumulate(ship_manifest.begin(), ship_manifest.end(), 0,
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
    ship_manifest.erase(std::remove_if(ship_manifest.begin(), ship_manifest.end(),
            [](Cargo& c) {
        return (c.IsPassenger() && !c.IsSlave());
    }), ship_manifest.end());

    // Find the first slaves instance if exists
    auto it = std::find_if(ship_manifest.begin(), ship_manifest.end(), [](const Cargo& c) {
        return c.IsSlave();
    });

    // Look for existing slaves
    if (it != ship_manifest.end()) {
        // Found slaves
        // Get a pointer to the found element
        Cargo* existing_slaves = &(*it);
        existing_slaves->Add(none_slave_passengers);
    } else {
        // Not found. Create a new instance
        // TODO: name should come from config.
        Cargo slaves = Manifest::MPL().GetCargoByName("Slaves");
        slaves.SetQuantity(none_slave_passengers);
        ship_manifest.push_back(slaves);
    }
}

// TODO: no need to be here. move to carrier
void Free(std::vector<Cargo>& ship_manifest) {
    auto slave_it = std::find_if(ship_manifest.begin(), ship_manifest.end(), [](const Cargo& c) {
        // TODO: name should come from config.
        return c.GetName() == "Slaves";
    });

    // No slaves to free found, exiting.
    if (slave_it == ship_manifest.end()) {
        return;
    }

    // Find the first hitchhikers instance if exists
    auto hitch_it = std::find_if(ship_manifest.begin(), ship_manifest.end(), [](const Cargo& c) {
        // TODO: name should come from config.
        return c.GetName() == "Hitchhiker";
    });

    // No hitchhikers found, modify slaves instance.
    if (hitch_it == ship_manifest.end()) {
        // TODO: name should come from config.
        Cargo* existing_slaves = &(*slave_it);
        existing_slaves->SetName("Hitchhiker");
        existing_slaves->SetCategory("Passengers");
    } else {
        Cargo* existing_hitchhikers = &(*hitch_it);
        Cargo* existing_slaves = &(*slave_it);
        existing_hitchhikers->Add(existing_slaves->GetQuantity());
        ship_manifest.erase(slave_it);
    }

}

