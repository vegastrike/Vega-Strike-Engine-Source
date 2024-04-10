/*
 * energy_manager.cpp
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

#include "energy_manager.h"
#include <iostream>

////////////////////////////////////////////////////////

// This section calculates the consumption of fuel and energy by different components. 
// TODO: It really should move to the python side and be  configurable.

// This is calculated to provide longer range ships with approximately 60 minutes of gameplay.
// TODO: This does not calculate for WC 
/*double getDriveConsumption(double factor, double mass) {
    return mass * factor * simulation_atom_var;
}

double getSPECDriveConsumption(double factor, double mass) {
    return mass * factor * simulation_atom_var;
}

double getJumpDriveConsumption(double factor, double mass) {
    return mass * factor;
}

// WC - 6 jumps
double getWCJumpDriveConsumption(double factor, double max_fuel) {
    return max_fuel / factor;
}

double getECMConsumption(double factor, double ecm_level) {
    return ecm_level * factor * simulation_atom_var;
}

double getShieldMaintenanceConsumption(double factor, int facets, double max_strength) {
    return facets * max_strength * factor * simulation_atom_var;
}

double getShieldRegenConsumption(double factor, int facets, double max_strength) {
    return facets * max_strength * factor * simulation_atom_var;
}

double getCloakConsumption(double factor, double mass) {
    return mass * factor * simulation_atom_var;
}*/

////////////////////////////////////////////////////////

/*EnergyManager::EnergyManager(EnergyContainer *fuel,
                             EnergyContainer *energy,
                             EnergyContainer *spec_energy,
                             Reactor *reactor):
                             fuel(fuel), energy(energy), 
                             ftl_energy(ftl_energy), reactor(reactor) {}

void EnergyManager::Act() {
    fuel->Act();
    double actual_reactor_usage = reactor->Generate();
    fuel->Charge(reactor->consumption - actual_reactor_usage); 
    energy->Act();
    ftl_energy->Act();
}

void EnergyManager::AddConsumer(EnergyType energy_type,
                                EnergyConsumerClassification classification, 
                                EnergyConsumerType consumer_type,
                                double consumption) {
    EnergyContainer* container = GetContainer(energy_type);
    EnergyConsumer consumer(energy_type, classification, consumer_type, consumption); 
    container->consumers.push_back(consumer);
}

double EnergyManager::Deplete(const EnergyType type, const double quantity) {
    EnergyContainer* container = GetContainer(type);
    return container->Deplete(quantity);
}

double EnergyManager::Deplete(EnergyConsumer consumer, const double quantity) {
    EnergyContainer *container = GetContainer(consumer.energy_type);
    if(!container) {
        return 1.0; // None energy type. Infinite energy.
    }
    
    return container->Deplete(quantity);
}

EnergyContainer* EnergyManager::GetContainer(const EnergyType type) {
    switch(type) {
        case EnergyType::Fuel:
            return fuel;
        case EnergyType::Energy:
            return energy;
        case EnergyType::FTL:
            return ftl_energy;
        default:
            return nullptr;
    }
}

const EnergyContainer* EnergyManager::GetConstContainer(const EnergyType type) const {
    switch(type) {
        case EnergyType::Fuel:
            return fuel;
        case EnergyType::Energy:
            return energy;
        case EnergyType::FTL:
            return ftl_energy;
    }
}

double EnergyManager::GetLevel(const EnergyType type) const {
    const EnergyContainer* container = GetConstContainer(type);
    return container->level.Value();
}
    
double EnergyManager::GetMaxLevel(const EnergyType type) const {
    const EnergyContainer* container = GetConstContainer(type);
    return container->level.MaxValue();
}

void EnergyManager::SetCapacity(const EnergyType type, const double capacity) {
    EnergyContainer* container = GetContainer(type);
    container->SetCapacity(capacity, true);
}

void EnergyManager::Refill(const EnergyType type) {
    EnergyContainer* container = GetContainer(type);
    container->level.SetToMax();
}

double EnergyManager::Percent(const EnergyType type) const {
    const EnergyContainer* container = GetConstContainer(type);
    if(!container) {
        return 0.0;
    }

    return container->level.Percent();
}

void EnergyManager::SetReactor(const double capacity, const double usage_factor, 
                               const double reactor_level, const double simulation_atom_var) { 
    this->reactor.capacity = capacity; 
    double c = usage_factor * reactor_level * simulation_atom_var;
    this->reactor.consumption = Resource<double>(c, 0.0, c);

    fuel.AddConsumer(reactor);
}*/
