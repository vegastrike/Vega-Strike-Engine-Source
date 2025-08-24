/*
 * manifest.h
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

#ifndef MANIFEST_H
#define MANIFEST_H

#include <vector>
#include <string>

#include "resource/cargo.h"

/**
 * A manifest is a list of items in a cargo hold.
 * The master part list is a special, singleton instance holding all items
 * in the game. Its shorthand is MPL.
 * To prevent corruption of the MPL, Manifest is read-only.
 * All write functions are moved to the CargoHold class.
 **/
class Manifest {
protected:
    std::vector<Cargo> _items;

private:
    Manifest(int dummy); // Create the MPL singleton.
public:
    Manifest();
    
    void AddManifest(const std::vector<Cargo>& cargo_items); // For testing

    static Manifest& MPL(); // Get the master part list singleton
    Cargo GetCargoByName(const std::string name) const;
    Cargo GetRandomCargo(int quantity = 0) const;
    Cargo GetRandomCargoFromCategory(std::string category, int quantity = 0) const;
    Manifest GetCategoryManifest(std::string category) const;
    Manifest GetMissionManifest() const;
    
    std::vector<Cargo> GetItems() const;
    bool Empty() const;
    int Size() const;

    int GetIndex(const Cargo& cargo) const;
    int GetIndex(const std::string& name, const std::string& category = "") const;

    bool HasCargo(const std::string& name) const;
};

#endif //MANIFEST_H

