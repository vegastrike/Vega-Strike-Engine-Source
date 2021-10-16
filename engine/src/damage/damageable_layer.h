#ifndef DAMAGEABLELAYER_H
#define DAMAGEABLELAYER_H

#include <vector>

#include "facet_configuration.h"
#include "core_vector.h"
#include "health.h"

/**
 * @brief The DamageableLayer class represents an object to be damaged.
 * This can be shields, armor, hull or subsystem.
 * A layer can be regenerative (e.g. shields) or not (pretty much everything else).
 */
struct DamageableLayer
{

    int layer_index;
    FacetConfiguration configuration;

    unsigned int number_of_facets;    // How many facets. e.g. dual shield (front and rear).
    std::vector<Health> facets;    // The facets container

    bool core_layer;    // Damage to the core layer has a chance of also
                        // damaging internal components such as propulsion.

    friend class Damageable;
    friend class DamageableObject;

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
                    std::vector<Health>& facets,
                    bool core_layer);
    DamageableLayer();

    void AdjustPower(const float& percent);
    void DealDamage( const CoreVector &attack_vector, Damage &damage, InflictedDamage& inflicted_damage );
    void Destroy();
    void Disable();
    void GradualDisable();
    void Discharge(float discharge_rate, float minimum_discharge);
    void Enable();
    bool Enabled();
    void Enhance();

    int GetFacetIndex(const CoreVector& attack_vector);

    void ReduceLayerCapability(const float& percent,
                               const float& chance_to_reduce_regeneration);

    float TotalLayerValue();
    float TotalMaxLayerValue();
    float AverageLayerValue();
    float AverageMaxLayerValue();

    float GetMaxHealth();
    float GetPercent(FacetName facet_name);

    void Regenerate(float recharge_rate);
    void RegenerateOrDischarge(float recharge, bool velocity_discharge, float discharge_rate);
    float GetRegeneration();
    void UpdateRegeneration(const float& new_regeneration_value);
};

#endif // DAMAGEABLELAYER_H
