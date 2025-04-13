/*
 * carrier.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_CARRIER_H
#define VEGA_STRIKE_ENGINE_CMD_CARRIER_H

#include "resource/cargo.h"
#include "gfx/vec.h"

#include <string>
#include <vector>

class Unit;

// A unit (ship) that carries cargo
class Carrier {
public:
    std::vector<Cargo> cargo;

    Carrier();
    void SortCargo();
    static std::string cargoSerializer(const struct XMLType &input, void *mythis);

    bool CanAddCargo(const Cargo &carg) const;
    void AddCargo(const Cargo &carg, bool sort = true);
    int RemoveCargo(unsigned int i, int quantity, bool eraseZero = true);
    float PriceCargo(const std::string &s);
    Cargo &GetCargo(unsigned int i);
    const Cargo &GetCargo(unsigned int i) const;
    void GetSortedCargoCat(const std::string &category, size_t &catbegin, size_t &catend);
//below function returns NULL if not found
    Cargo *GetCargo(const std::string &s, unsigned int &i);
    const Cargo *GetCargo(const std::string &s, unsigned int &i) const;
    unsigned int numCargo() const;
    std::string GetManifest(unsigned int i, Unit *scanningUnit, const Vector &original_velocity) const;
    bool SellCargo(unsigned int i, int quantity, float &creds, Cargo &carg, Unit *buyer);
    bool SellCargo(const std::string &s, int quantity, float &creds, Cargo &carg, Unit *buyer);
    bool BuyCargo(const Cargo &carg, float &creds);
    bool BuyCargo(unsigned int i, unsigned int quantity, Unit *buyer, float &creds);
    bool BuyCargo(const std::string &cargo, unsigned int quantity, Unit *buyer, float &creds);
    void EjectCargo(unsigned int index);
    float getEmptyCargoVolume(void) const;
    float getCargoVolume(void) const;
    float getEmptyUpgradeVolume(void) const;
    float getUpgradeVolume(void) const;
    float getHiddenCargoVolume(void) const;
};

#endif //VEGA_STRIKE_ENGINE_CMD_CARRIER_H
