/*
 * energy_consumer.h
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

#ifndef ENERGYCONSUMER_H
#define ENERGYCONSUMER_H

#include "energy_types.h"
#include "energy_consumer.h"
#include "resource.h"

class EnergyConsumer {
protected:
    EnergyType energy_type;
    EnergyConsumerClassification classification;
    EnergyConsumerType consumer_type;
    bool in_use;
    double powered;
    Resource<double> consumption;  // This is when powered and for simulation_atom_var
    
    friend class EnergyContainer;
    friend class EnergyManager;
public:
    EnergyConsumer(EnergyType energy_type, EnergyConsumerClassification classification,
                    EnergyConsumerType consumer_type, double consumption):
                    energy_type(energy_type), classification(classification), consumer_type(consumer_type), in_use(false), 
                    powered(0.0), consumption(Resource<double>(consumption, 0.0, consumption)) {}

    double Powered() { return powered; }
    void Use() { in_use = true;}
    EnergyType GetEnergyType() { return energy_type; }
    double GetConsumption() const { return consumption.Value(); }
    void SetConsumption(const double consumption) { this->consumption = consumption; }
};

#endif // ENERGYCONSUMER_H
