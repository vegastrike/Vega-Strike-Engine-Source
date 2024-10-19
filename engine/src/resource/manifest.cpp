/*
 * manifest.cpp
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

#include "manifest.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

//#include "xml_support.h"  // TODO: replace this later
#include "json.h"
#include "random_utils.h"

// TODO: get rid of this function when we move to C++20

static bool ends_with(const std::string& str, const std::string& suffix){
    return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}



// TODO: get rid of this helper function and others like it.
static std::string getJSONValue(const json::jobject& object, const std::string &key, const std::string &default_value) {
    if(object.has_key(key)) {
        std::string value = object.get(key);
        value = value.substr(1, value.size() - 2);
        return value;
    }

    return default_value;
}

static float getJSONValue(const json::jobject& object, const std::string &key, const float &default_value) {
    if(object.has_key(key)) {
        try {
            return std::stof(object.get(key));
        } catch(...) {}
        try {
            return std::stoi(object.get(key));
        } catch(...) {}
    }

    return default_value;
}


Manifest::Manifest() {
    _items = std::vector<Cargo>();
}

Manifest::Manifest(std::string category) {
    Manifest& mpl = Manifest::MPL();

    auto predicate = [&category](Cargo c) {return c.GetCategory() == category;};
    std::copy_if(mpl._items.begin(), mpl._items.end(), 
             std::back_inserter(_items), predicate);
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

        std::vector<std::string> parts = json::parsing::parse_array(json_text.c_str());
        for (const std::string &part_text : parts) {
            json::jobject part = json::jobject::parse(part_text);

            std::string name = getJSONValue(part, "file", "");
            std::string category = getJSONValue(part, "categoryname", "");

            Cargo cargo = Cargo(name,
                                category,
                                std::stoi(getJSONValue(part, "price", "")),     // Price
                                1,                                              // Quantity
                                std::stof(getJSONValue(part, "mass", "")),      // Mass
                                std::stof(getJSONValue(part, "volume", "")));   // Volume
            cargo.SetDescription(getJSONValue(part, "description", ""));        // Description
            _items.push_back(cargo);
        }
    }
}

Manifest& Manifest::MPL() {
    static Manifest mpl = Manifest(1);
    
    return mpl;
}

Cargo Manifest::GetCargoByName(const std::string name) {
    const std::string upgrades_suffix = "__upgrades";
    std::string filename;

    // Check if we need to remove __upgrades suffix
    if(ends_with(name, upgrades_suffix)) {
        filename = name.substr(0, name.length() - upgrades_suffix.length());
    } else {
        filename = name;
    }


    for(const Cargo& c : getItems()) {
        if(c.name == filename) {
            return c;
        }
    }

    return Cargo();
}

Cargo Manifest::GetRandomCargo(int quantity) {
    // TODO: Need to figure a better solution here
    if(_items.empty()) {
        return Cargo();
    }

    int index = randomInt(_items.size()-1); 
    Cargo c = _items[index];
    c.SetQuantity(quantity);
    return c;
}



Cargo Manifest::GetRandomCargoFromCategory(std::string category, int quantity) {
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

Manifest Manifest::GetCategoryManifest(std::string category) {
    Manifest manifest;

    std::copy_if(_items.begin(), _items.end(), back_inserter(manifest._items), 
            [category](Cargo c) {
        return c.GetCategory() == category;
    });

    return manifest;
}

Manifest Manifest::GetMissionManifest() {
    Manifest manifest;

    std::copy_if(_items.begin(), _items.end(), back_inserter(manifest._items), 
            [](Cargo c) {
        return c.name.find("mission") != std::string::npos;
    });

    return manifest;
}
