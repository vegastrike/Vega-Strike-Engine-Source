/*
 * drive.cpp
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

#include "drive.h"
#include "energy_types.h"

// Note that usage_factor should be closely related to normal drive's usage factor.
// In most cases it should be 1.0 and the difference should be modelled using the afterburner_level.
Drive::Drive(EnergyType type, 
             double usage_factor, 
             double drive_level, 
             double mass, 
             double simulation_atom_var):
             EnergyConsumer(type, 
                            EnergyConsumerClassification::Afterburner, 
                            EnergyConsumerType::Constant, 
                            0.0),
             usage_factor(usage_factor) {
    consumption = usage_factor * drive_level * mass * simulation_atom_var;
}