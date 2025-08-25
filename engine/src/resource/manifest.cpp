/*
 * manifest.cpp
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

#include "resource/manifest.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#include <boost/json.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "resource/random_utils.h"
#include "resource/json_utils.h"


Manifest::Manifest() {
    _items = std::vector<Cargo>();
}


// Called by MPL if it is empty
Manifest::Manifest(int dummy) {
    _items = std::vector<Cargo>();

    // TODO: get this from some configuration maybe?!?
    static std::string json_filenames[] = {
        "master_part_list.json",
        "master_ship_list.json",
        "master_component_list.json",
        "master_asteroid_list.json",
    };

    for(const std::string& json_filename : json_filenames) {
        std::ifstream ifs(json_filename, std::ifstream::in);

        if(ifs.fail()) {
            continue;
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();

        const std::string json_text = buffer.str();
        boost::json::value json_value = boost::json::parse(json_text);
        boost::json::array root_array = json_value.get_array();

        for(boost::json::value& item_value : root_array) {
            try {
                boost::json::object item = item_value.get_object();

                Cargo cargo = Cargo(item);
                _items.push_back(cargo);
            } catch (...) {
                std::cerr << "Failed to parse " << item_value << std::endl;
            }

        }
    }
}

// For testing
void Manifest::AddManifest(const std::vector<Cargo>& cargo_items) {
    _items = cargo_items;
}

Manifest& Manifest::MPL() {
    static Manifest mpl = Manifest(1);

    return mpl;
}



Cargo Manifest::GetCargoByName(const std::string name) const {
    const std::string upgrades_suffix = "__upgrades";
    std::string filename;

    // Check if we need to remove __upgrades suffix
    if(boost::algorithm::ends_with(name, upgrades_suffix)) {
        filename = name.substr(0, name.length() - upgrades_suffix.length());
    } else {
        filename = name;
    }


    for(const Cargo& c : _items) {
        if(c.name == filename) {
            return c;
        }
    }

    throw std::runtime_error("Cargo with name '" + name + "' not found in manifest.");
}

Cargo Manifest::GetRandomCargo(int quantity) const {
    // TODO: Need to figure a better solution here
    if(_items.empty()) {
        return Cargo();
    }

    int index = randomInt(_items.size()-1);
    Cargo c = _items[index];
    c.SetQuantity(quantity);
    return c;
}



Cargo Manifest::GetRandomCargoFromCategory(std::string category, int quantity) const {
    Manifest manifest = GetCategoryManifest(category);

    // If category is empty, return randomly from MPL itself.
    if(manifest._items.empty()) {
        manifest = GetMissionManifest();
        if(manifest._items.empty()) {
            return GetRandomCargo(quantity);
        }
    }

    return manifest.GetRandomCargo(quantity);
}

Manifest Manifest::GetCategoryManifest(std::string category) const {
    Manifest manifest;

    std::copy_if(_items.begin(), _items.end(), back_inserter(manifest._items),
            [category](Cargo c) {
        return c.GetCategory() == category;
    });

    return manifest;
}



Manifest Manifest::GetMissionManifest() const {
    Manifest manifest;

    std::copy_if(_items.begin(), _items.end(), back_inserter(manifest._items),
            [](Cargo c) {
        return c.IsMissionFlag();
    });

    return manifest;
}

std::vector<Cargo> Manifest::GetItems() const { 
    return _items; 
}

bool Manifest::Empty() const { 
    return _items.empty(); 
}

int Manifest::Size() const { 
    return _items.size(); 
}


// This is not as efficient as a hashtable
// TODO: think about this
int Manifest::GetIndex(const Cargo& cargo) const {
    int index = 0;
    for(Cargo c : _items) {
        if(cargo.name == c.name && cargo.category == c.category) {
            return index;
        }

        index++;
    }

    return -1;
}


int Manifest::GetIndex(const std::string& name, const std::string& category) const {
    int index = 0;
    for(Cargo c : _items) {
        if(name == c.name && category == c.category) {
            return index;
        } else if(name == c.name && category.empty()) {
            // If no category is specified, we match by name only
            return index;
        }

        index++;
    }

    return -1; // Not found
}


// Consider deleting this. It's almost exactly the same as GetIndex
bool Manifest::HasCargo(const std::string& name) const {
    for(const Cargo& c : _items) {
        if(c.name == name) {
            return true;
        }
    }

    return false;
}
