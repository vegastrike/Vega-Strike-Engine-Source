/*
 * reactor.cpp
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

#include "reactor.h"
#include <iostream>

Reactor::Reactor(): EnergyConsumer(EnergyType::None, 
                                  EnergyConsumerClassification::Reactor, 
                                  EnergyConsumerType::Constant, 
                                  0.0),
                    capacity(0.0),
                    energy_container(nullptr),
                    spec_energy_container(nullptr) {}

Reactor::Reactor(EnergyType energy_type, double usage_factor, double reactor_level,
                 double capacity, 
                 EnergyContainer *energy_container,
                 EnergyContainer *spec_energy_container,
                 double simulation_atom_var): 
                 EnergyConsumer(energy_type, 
                                EnergyConsumerClassification::Reactor, 
                                EnergyConsumerType::Constant, 
                                0.0),
                 capacity(capacity),
                 energy_container(energy_container),
                 spec_energy_container(spec_energy_container) {
    double c = usage_factor * reactor_level * simulation_atom_var;
    this->consumption = Resource<double>(c, 0.0, c);
}

double Reactor::Generate() {
    double actual_consumption = 0.0;
    double surplus = energy_container->Charge(capacity);
    surplus = spec_energy_container->Charge(surplus);
    surplus = capacity;

    if(capacity == 0.0) {
        // We generate nothing and so consume no fuel
        actual_consumption = 0.0;
    } else if(surplus == 0.0) {
        actual_consumption = consumption.MaxValue();
    } else {
        actual_consumption = (1 - (surplus / capacity)) * consumption.MaxValue();
    }

    return actual_consumption;
}