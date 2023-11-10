/*
 * energy_container.cpp
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

#include "energy_container.h"

#include <iostream>

EnergyContainer::EnergyContainer(): type(EnergyType::Fuel), 
                                    level(Resource<double>(0.0,0.0,0.0)),
                                    consumers(std::vector<EnergyConsumer>()) {}

EnergyContainer::EnergyContainer(EnergyType type, double capacity): type(type), 
                                                   level(Resource<double>(capacity,0.0,capacity)),
                                                   consumers(std::vector<EnergyConsumer>()) {}

void EnergyContainer::AddConsumer(EnergyType energy_type,
                                  EnergyConsumerClassification classification, 
                                  EnergyConsumerType consumer_type,
                                  double quantity) {
    EnergyConsumer consumer(energy_type, classification, consumer_type, quantity); 
    consumers.push_back(consumer);
}

// Return value - any surplus charge
double EnergyContainer::Charge(const double quantity) {
    double old_level = level.Value();
    level += quantity;

    return quantity - level.Value() + old_level;
}

double EnergyContainer::Deplete(const double quantity) {
    double old_level = level.Value();
    level -= quantity;
    return quantity + old_level - level.Value();
}

void EnergyContainer::SetCapacity(const double capacity, bool refill) {
    if(refill) {
        level = Resource<double>(capacity,0,capacity);
    } else {
        level = Resource<double>(0,0,capacity);
    }
}

double EnergyContainer::Level() const { return level.Value(); }
double EnergyContainer::MaxLevel() const { return level.MaxValue(); }
double EnergyContainer::Percent() const { 
    if(level.MaxValue() == 0.0) {
        return 0.0;
    }
    
    return level.Value()/level.MaxValue(); 
}

void EnergyContainer::Zero() { level = 0; }

void EnergyContainer::Act() {
    for(EnergyConsumer& consumer : consumers) {
        if(consumer.consumer_type == EnergyConsumerType::Constant) {
            consumer.in_use = true;
        } else if(consumer.consumer_type == EnergyConsumerType::Variable) {
            consumer.in_use = false;
        }

        if(level < consumer.consumption) {
            consumer.powered = level / consumer.consumption;
            level = 0;   
            continue;
        }

        std::cout << TypeToSave(type) << " ol: " << level;
        level -= consumer.consumption;
        std::cout << " nl: " << level << std::endl;
        consumer.powered = 1.0;
    }
}

void EnergyContainer::Use(EnergyConsumerClassification classification) {
    for(EnergyConsumer& consumer : consumers) {
        consumer.in_use = true;
    }
}

bool EnergyContainer::InUse(EnergyConsumerClassification classification) {
    for(EnergyConsumer& consumer : consumers) {
        if(consumer.classification == classification) {
            return consumer.in_use;
        }
    }

    return false;
}

double EnergyContainer::Powered(EnergyConsumerClassification classification) {
    for(EnergyConsumer& consumer : consumers) {
        if(consumer.classification == classification) {
            return consumer.powered;
        }
    }
        
    return false;
}
