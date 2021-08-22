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
    bool core_layer;    // Damage to the core layer has a chance of also
                        // damaging internal components such as propulsion.

    unsigned int number_of_facets;    // How many facets. e.g. dual shield (front and rear).
    std::vector<Health> facets;    // The facets container

    //static float damage_component_chance = 0.03;
    DamageableLayer(int layer_index, int number_of_facets, std::vector<Health>& facets, bool core_layer):
        layer_index(layer_index),
        number_of_facets(number_of_facets),
        facets(facets),
        core_layer(core_layer) {}

    DamageableLayer():
        layer_index(0),
        core_layer(false),
        number_of_facets(1) {
        facets.push_back(Health(layer_index));
    }

    void AdjustPower(const float& percent);
    void DealDamage( const CoreVector &attack_vector, Damage &damage, InflictedDamage& inflicted_damage );
    void Destroy();
    void Disable();
    void Enable();
    void Enhance();

    int GetFacetIndex(const CoreVector& attack_vector);
    int GetFacetIndexByName(FacetName facet_name);
    void InitFacetByName(FacetName facet_name, float facet_health);
    void InitFacetByName(FacetName facet_name, Health facet_health);

    void ReduceLayerCapability(const float& percent,
                               const float& chance_to_reduce_regeneration);

    float TotalLayerValue();
    float TotalMaxLayerValue();
    float AverageLayerValue();
    float AverageMaxLayerValue();

    float GetPercent(FacetName facet_name);
    void Regenerate(float alternative_regeneration = -1);
    void UpdateRegeneration(const float& new_regeneration_value);
};

#endif // DAMAGEABLELAYER_H
