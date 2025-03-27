/*
 * reactor.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_REACTOR_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_REACTOR_H

#include "component.h"
#include "components/energy_container.h"
#include "components/energy_consumer.h"
#include "resource/resource.h"

class EnergyManager;

class Reactor: public Component, public EnergyConsumer
{
private:
    Resource<double> capacity;          // Capacity per second
    double atom_capacity;               // Capacity per atom
    const double conversion_ratio;            // Used to calculate fuel consumption

    EnergyContainer *energy;
    EnergyContainer *ftl_energy;

public:
    Reactor(EnergyContainer *source,
            EnergyContainer *energy,
            EnergyContainer *ftl_energy,
            double conversion_ratio = 0.0001); // < 0.01 or very short flight
    ~Reactor() override;


    void Load(std::string unit_key) override;

    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    void Damage() override;
    void DamageByPercent(double percent) override;
    void Repair() override;

    bool Damaged() const override;
    bool Installed() const override;

    void Generate();
    double Capacity() const;
    double MaxCapacity() const;
    void SetCapacity(double capacity);

    // EnergyConsumer method(s)
    double Consume() override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_REACTOR_H
