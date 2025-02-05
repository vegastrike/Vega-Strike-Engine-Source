/*
 * Copyright (C) 2021-2023 Roy Falk, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_LAYER_H
#define VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_LAYER_H

#include <vector>

#include "facet_configuration.h"
#include "core_vector.h"
#include "health.h"

/**
 * @brief The DamageableLayer class represents an object to be damaged.
 * This can be shields, armor, hull or subsystem.
 * A layer can be regenerative (e.g. shields) or not (pretty much everything else).
 */
struct DamageableLayer {

    int layer_index;
    FacetConfiguration configuration;

    unsigned int number_of_facets;    // How many facets. e.g. dual shield (front and back).
    std::vector<Health> facets;    // The facets container

    bool core_layer;    // Damage to the core layer has a chance of also
    // damaging internal components such as propulsion.

    friend class Damageable;
    friend struct DamageableObject;

    //static float damage_component_chance = 0.03;
    DamageableLayer(int layer_index,
            FacetConfiguration configuration,
            Health health_template,
            bool core_layer);

    DamageableLayer(int layer_index,
            FacetConfiguration configuration,
            float health_array[],
            float regeneration,
            bool core_layer);

    DamageableLayer(int layer_index,
            int number_of_facets,
            std::vector<Health> &facets,
            bool core_layer);
    DamageableLayer();

    void AdjustPower(const float &percent);
    void DealDamage(const CoreVector &attack_vector, Damage &damage, InflictedDamage &inflicted_damage);
    void Destroy();
    void Disable();
    void GradualDisable();
    void Discharge(float discharge_rate, float minimum_discharge);
    void Enable();
    bool Enabled();
    void Enhance();

    int GetFacetIndex(const CoreVector &attack_vector);

    void ReduceLayerCapability(const float &percent,
            const float &chance_to_reduce_regeneration);

    float TotalLayerValue();
    float TotalMaxLayerValue();
    float AverageLayerValue();
    float AverageMaxLayerValue();

    float GetMaxHealth();
    float GetPercent(FacetName facet_name);

    void FullyCharge();
    void Regenerate(float recharge_rate);
    void RegenerateOrDischarge(float recharge, bool velocity_discharge, float discharge_rate);
    float GetRegeneration();
    void UpdateFacets(const float new_facet_strength);
    void UpdateFacets(const unsigned int size, const float new_facets[4]);
    void UpdateRegeneration(const float &new_regeneration_value);
};

#endif //VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_LAYER_H
