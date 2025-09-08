/*
 * damageable_layer.h
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_LAYER_H
#define VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_LAYER_H

#include <vector>

#include "damage/facet_configuration.h"
#include "damage/core_vector.h"
#include "damage/damage.h"
#include "resource/resource.h"

/**
 * @brief The DamageableLayer class represents an object to be damaged.
 * This can be shields, armor, hull or subsystem.
 * A layer can be regenerative (e.g. shields) or not (armor/hull).
 */
class DamageableLayer {
protected:
    int layer; // The layer we're in, for recording damage

    Damage vulnerabilities;
    // TODO: implement "shield leaks"

    bool core_layer;    // Damage to the core layer has a chance of also
                        // damaging internal components such as propulsion.

    int number_of_facets;    // How many facets. e.g. dual shield (front and rear).
    std::vector<Resource<double>> facets;    // The facets container

    friend class Damageable;
    friend struct DamageableObject;

public:
    DamageableLayer(int layer,
            FacetConfiguration configuration,
            double health_template,
            Damage vulnerabilities = Damage(1.0,1.0),
            bool core_layer = false,
            double regeneration = 0.0);

    DamageableLayer() = delete;

    virtual ~DamageableLayer();

    void DealDamage(const CoreVector &attack_vector, Damage &damage, InflictedDamage &inflicted_damage);
    void DealDamageComponent(int impacted_facet_index, int type, double &damage, double vulnerability, InflictedDamage &inflicted_damage);

    int GetFacetIndex(const CoreVector &attack_vector) const;

    double TotalLayerValue() const;
    double TotalMaxLayerValue() const;
    double AverageLayerValue() const;
    double AverageMaxLayerValue() const;

    virtual double Percent() const;
    double Percent(int facet) const;

    int Layer() const;
};

#endif //VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_LAYER_H
