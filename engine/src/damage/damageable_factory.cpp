#include "damageable_factory.h"
#include "core_vector.h"

#include <float.h>

/// Facet Configurations
/// One
const CoreVector minimum_one_configuration(-FLT_MAX, -FLT_MAX, -FLT_MAX);
const CoreVector maximum_one_configuration(FLT_MAX, FLT_MAX, FLT_MAX);

/// Two

/// Eight


DamageableObject DamageableFactory::CreateStandardObject(float shield,
                                                         float shield_regeneration,
                                                         FacetConfiguration shield_configuration,
                                                         float armor,
                                                         float hull) {
    Health shield_health = Health(shield, shield, shield_regeneration);
    Health armor_health = Health(armor, armor, 0);
    Health hull_health = Health(hull, hull, 0);

    DamageableLayer shield_layer = CreateLayer(shield_configuration, shield_health, false);
    DamageableLayer armor_layer = CreateLayer(shield_configuration, armor_health, false);
    DamageableLayer hull_layer = CreateLayer(FacetConfiguration::one, hull_health, true);
    std::vector<DamageableLayer> layers = { shield_layer, armor_layer, hull_layer };

    DamageableObject object(layers, std::vector<DamageableObject>());
    return object;
}

DamageableLayer DamageableFactory::CreateLayer(FacetConfiguration configuration,
                                               Health health_template,
                                               bool core_layer) {
    int size = FacetConfigurationSize(configuration);
    const FacetName* facet_configuration_array = FacetConfigurationByName(configuration);

    std::vector<DamageableFacet> facets;
    for(int i=0;i<size;i++) {
        facets.push_back(DamageableFacet(configuration,
                                         facet_configuration_array[i],
                                         health_template));
    }

    DamageableLayer layer(size, facets, core_layer);
    return layer;
}

DamageableLayer DamageableFactory::CreateLayer(FacetConfiguration configuration,
                                               float health_array[],
                                               float regeneration,
                                               bool core_layer) {
    int size = FacetConfigurationSize(configuration);
    const FacetName* facet_configuration_array = FacetConfigurationByName(configuration);

    std::vector<DamageableFacet> facets;
    for(int i=0;i<size;i++) {
        Health health(health_array[i], health_array[i], regeneration);
        facets.push_back(DamageableFacet(configuration,
                                         facet_configuration_array[i],
                                         health));
    }

    DamageableLayer layer(size, facets, core_layer);
    return layer;
}


