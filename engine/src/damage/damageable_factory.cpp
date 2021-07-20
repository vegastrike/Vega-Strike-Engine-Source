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
    if(configuration == FacetConfiguration::one) {
        DamageableFacet facet(minimum_one_configuration, maximum_one_configuration, health_template);
        std::vector<DamageableFacet> facets = { facet};
        DamageableLayer layer(1, facets, core_layer);
        return layer;
    } else if(configuration == FacetConfiguration::eight) {
        DamageableFacet front_top_left_facet(CoreVector(0,0,0), CoreVector(FLT_MAX, FLT_MAX, FLT_MAX), health_template);
        DamageableFacet front_bottom_left_facet(CoreVector(0,-FLT_MAX,0), CoreVector(FLT_MAX, 0, FLT_MAX), health_template);
        DamageableFacet front_top_right_facet(CoreVector(-FLT_MAX,0,0), CoreVector(0, FLT_MAX, FLT_MAX), health_template);
        DamageableFacet front_bottom_right_facet(CoreVector(-FLT_MAX,-FLT_MAX,0), CoreVector(0, 0, FLT_MAX), health_template);
        DamageableFacet rear_top_left_facet(CoreVector(0,0,-FLT_MAX), CoreVector(FLT_MAX, FLT_MAX, 0), health_template);
        DamageableFacet rear_bottom_left_facet(CoreVector(0,-FLT_MAX,-FLT_MAX), CoreVector(FLT_MAX, 0, 0), health_template);
        DamageableFacet rear_top_right_facet(CoreVector(-FLT_MAX,0,-FLT_MAX), CoreVector(0, FLT_MAX, 0), health_template);
        DamageableFacet rear_bottom_right_facet(CoreVector(-FLT_MAX,-FLT_MAX,-FLT_MAX), CoreVector(0, 0, 0), health_template);
        std::vector<DamageableFacet> facets = { front_top_left_facet,
                                              front_bottom_left_facet,
                                              front_top_right_facet,
                                              front_bottom_right_facet,
                                              rear_top_left_facet,
                                              rear_bottom_left_facet,
                                              rear_top_right_facet,
                                              rear_bottom_right_facet};
        DamageableLayer layer(8, facets, core_layer);
        return layer;
    }


}



DamageableFacet CreateFacetFromTemplate(const DamageableFacet& template_facet,
                                        const CoreVector& min_v,
                                        const CoreVector& max_v) {
    return DamageableFacet(min_v, max_v, template_facet);
}

const std::vector<DamageableFacet> CreateLayerFromTemplate(const FacetConfiguration& configuration,
                                                           const DamageableFacet& template_facet,
                                                           bool core_layer) {
    if(configuration == FacetConfiguration::one) {
        const std::vector<DamageableFacet> facets = { template_facet };
        return facets;
    }
}
