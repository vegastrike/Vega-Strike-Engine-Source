/*
 * energy_manager.h
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

#ifndef ENERGYMANAGER_H
#define ENERGYMANAGER_H

#include "energy_types.h"
#include "energy_container.h"
#include "reactor.h"

/*double getDriveConsumption(double factor, double mass);             // 1/36000
double getSPECDriveConsumption(double factor, double mass);
double getJumpDriveConsumption(double factor, double mass);
double getWCJumpDriveConsumption(double factor, double max_fuel);   // 1/6
double getECMConsumption(double factor, double ecm_level);          // ?
double getShieldMaintenanceConsumption(double factor, int facets, double max_strength); // 100
double getShieldRegenConsumption(double factor, int facets, double max_strength);       // 10
double getCloakConsumption(double factor, double mass);*/                                 // 25

class EnergyManager {
    EnergyContainer fuel;
    EnergyContainer energy;
    EnergyContainer spec_energy;
    Reactor reactor;

public:
    EnergyManager();

    void Act();

    void AddConsumer(EnergyType energy_type, 
                     EnergyConsumerClassification classification, 
                     EnergyConsumerType consumer_type,
                     double consumption);
    void AddConsumer(EnergyType energy_type, EnergyConsumer consumer) {
        EnergyContainer* container = GetContainer(energy_type);
        container->AddConsumer(consumer);
    }
    double Deplete(const EnergyType type, const double quantity);
    double Deplete(EnergyConsumer consumer, const double quantity);
    
    EnergyContainer* GetContainer(const EnergyType type);
    const EnergyContainer* GetConstContainer(const EnergyType type) const;
    
    double GetLevel(const EnergyType type) const;
    double GetMaxLevel(const EnergyType type) const;
    void SetCapacity(const EnergyType type, const double capacity);
    void Refill(const EnergyType type);
    double Percent(const EnergyType type) const;
    double GetReactorCapacity() {return reactor.capacity; }
    void SetReactorCapacity(const double capacity) { reactor.capacity = capacity; }
    void SetReactor(const double capacity, const double usage_factor, 
                    const double reactor_level, const double simulation_atom_var);
};


#endif // ENERGYMANAGER_H