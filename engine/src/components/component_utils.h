/*
 * src/components/component_printer.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_COMPONENT_UTILS_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_COMPONENT_UTILS_H

#include "component.h"
#include "energy_consumer.h"
#include "energy_container.h"
#include "resource/resource.h"

#include <string>

/** A collection of utility functions for lib component. */
const ComponentType GetComponentTypeFromName(const std::string name);

EnergyContainer* GetSource(ComponentType component_type, EnergyContainer* fuel,
                           EnergyContainer* energy, EnergyContainer* ftl_energy);

EnergyContainer* GetSourceFromConfiguration(const std::string source, EnergyContainer* fuel,
                                            EnergyContainer* energy, EnergyContainer* ftl_energy);

enum class YPR {
    Yaw, Pitch, Roll
};


void DoubleYawPitchRollParser(std::string unit_key, const YPR ypr,
                        double &right_value, double &left_value);

 void ResourceYawPitchRollParser(std::string unit_key, const YPR ypr,
                        Resource<double> &right_value, Resource<double> &left_value,
                        const double minimum_functionality = 0.0);
                                               
#endif // VEGA_STRIKE_ENGINE_COMPONENTS_COMPONENT_UTILS_H
