/*
 * energy_consumer.h
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_ENERGYCONSUMER_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_ENERGYCONSUMER_H

#include "components/energy_container.h"

enum class EnergyConsumerSource {
    Infinite, Fuel, Energy, FTLEnergy, NoSource
};

EnergyConsumerSource GetSource(const int source);

class EnergyConsumer {
protected:
    double consumption;         // Directly converted to atomic. Mostly for book keeping.
    double atom_consumption;    // consumption per 0.1 seconds.

    static double simulation_atom_var;

private:
    EnergyContainer *source;
    bool partial; // Can power consumer with less energy than requested

protected:
    bool infinite; // Connected to infinite power source. Ignore source field. Always work.
public:
    virtual ~EnergyConsumer();
    EnergyConsumer(EnergyContainer *source = nullptr, bool partial = false, double consumption = 0.0, bool infinite = false);
    bool CanConsume() const;
    virtual double Consume();
    double GetConsumption() const;
    double GetAtomConsumption() const;
    void SetConsumption(double consumption);
    void SetSource(EnergyContainer* source);
    void ZeroSource();

};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_ENERGYCONSUMER_H
