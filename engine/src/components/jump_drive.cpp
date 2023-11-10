/*
 * jump_drive.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjaman Meyer, Roy Falk, Stephen G. Tuggy,
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

#include "jump_drive.h"

JumpDrive::JumpDrive(): 
    EnergyConsumer(EnergyType::None, 
                   EnergyConsumerClassification::JumpDrive,
                   EnergyConsumerType::Variable, 
                   0.0),
    delay(0.0){}

JumpDrive::JumpDrive(double consumption, double delay) : 
    EnergyConsumer(EnergyType::SPEC,
                   EnergyConsumerClassification::JumpDrive,
                   EnergyConsumerType::Variable, consumption),
                   delay(delay) {}

bool JumpDrive::Ready() { 
    return installed && enabled; 
}

void JumpDrive::SetDestination(int destination) { 
    this->destination = destination; 
}

