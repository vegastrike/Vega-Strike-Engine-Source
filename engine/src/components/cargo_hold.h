/*
 * cargo_hold.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_CARGO_HOLD_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_CARGO_HOLD_H

#include "component.h"
#include "resource/cargo.h"
#include "resource/resource.h"
#include "resource/manifest.h"
#include <vector>

class ComponentsManager;

enum class HoldType {
    cargo, hidden, upgrade
};

/** A cargo hold. Can hold cargo. */
class CargoHold : public Component, public Manifest {
    Resource<double> capacity;
public:
    CargoHold();

    // Component Methods
    void Load(std::string unit_key) override;      
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    // Cargo Hold Methods
    bool CanAddCargo(const Cargo &carg) const;
    void AddCargo(ComponentsManager *manager, const Cargo &carg, bool sort = true);
    Cargo RemoveCargo(ComponentsManager *manager, const std::string& name, int quantity);
    Cargo RemoveCargo(ComponentsManager *manager, unsigned int index, int quantity);
    Cargo& GetCargo(unsigned int i);
    const Cargo& GetCargo(unsigned int i) const;

    double CurrentCapacity() const;
    double MaxCapacity() const;
    double AvailableCapacity() const;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_CARGO_HOLD_H
