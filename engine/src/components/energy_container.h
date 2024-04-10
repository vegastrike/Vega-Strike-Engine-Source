/*
 * energy_container.h
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

#ifndef ENERGYCONTAINER_H
#define ENERGYCONTAINER_H

#include <vector>
#include <iostream>

#include "resource/resource.h"
#include "energy_types.h"
#include "energy_consumer.h"


/**
 * @brief The EnergyContainer class models the fuel cell, capacitor and SPEC capacitor
 */
class EnergyContainer
{
protected:
    EnergyType type;
    Resource<double> level;
    std::vector<EnergyConsumer> consumers;

    friend class EnergyManager;
public:
    EnergyContainer();
    EnergyContainer(EnergyType type, double capacity);

    void AddConsumer(EnergyType energy_type, 
                     EnergyConsumerClassification classification, 
                     EnergyConsumerType consumer_type,
                     double quantity);

    void AddConsumer(EnergyConsumer consumer) {
        consumers.push_back(consumer);
    }

    // Return value - any surplus charge
    double Charge(const double quantity);

    double Deplete(const double quantity);
    bool Depleted() const;

    void SetCapacity(const double capacity, bool refill = true);
    double Level() const;
    double MaxLevel() const;
    double Percent() const;
    void Refill();

    void Zero();

    void Act();

    void Use(EnergyConsumerClassification classification);

    bool InUse(EnergyConsumerClassification classification);
    double Powered(EnergyConsumerClassification classification);
};

#endif // ENERGYCONTAINER_H
