/*
 * manifest.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

#ifndef MANIFEST_H
#define MANIFEST_H

#include <vector>
#include <string>

#include "cargo.h"

/**
 * A manifest is a list of items in a cargo hold.
 * The master part list is a special, singleton instance holding all items
 * in the game. It is read only (const). Its short is MPL.
 **/
class Manifest {
    std::vector<Cargo> _items; 

    Manifest(int dummy); // Create the MPL singleton.
public:
    Manifest();
    Manifest(std::string category); // Create a subset of the MPL for a category

    static Manifest& MPL(); // Get the master part list singleton
    Cargo GetRandomCargo(int quantity = 0);
    Cargo GetRandomCargoFromCategory(std::string category, int quantity = 0);
    Manifest GetCategoryManifest(std::string category);
    Manifest GetMissionManifest();

    std::vector<Cargo> getItems() { return _items; }
    bool empty() { return _items.empty(); }
    int size() { return _items.size(); }
};




#endif //MANIFEST_H
