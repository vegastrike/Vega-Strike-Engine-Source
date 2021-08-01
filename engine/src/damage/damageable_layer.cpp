#include "damageable_layer.h"

#include <iostream>
#include <random>

void DamageableLayer::AdjustPower(const float& percent) {
    for(DamageableFacet& facet : facets) {
        facet.health.AdjustPower(percent);
    }
}

void DamageableLayer::DealDamage( const CoreVector &attack_vector, Damage &damage ) {
    int impacted_facet_index = GetFacetIndex(attack_vector);
    facets[impacted_facet_index].DealDamage(damage);
}


void DamageableLayer::Destroy() {
    for(DamageableFacet& facet : facets) {
        facet.health.health = 0;
        facet.health.destroyed = true;
    }
}


void DamageableLayer::Disable() {
    for(DamageableFacet& facet : facets) {
        facet.health.health = 0;
        facet.health.enabled = false;
    }
}


void DamageableLayer::Enable() {
    for(DamageableFacet& facet : facets) {
        facet.health.enabled = true;
    }
}

int DamageableLayer::GetFacetIndex(const CoreVector& attack_vector) {
    // Convenience Variables
    float i = attack_vector.i;
    float j = attack_vector.j;
    float k = attack_vector.k;

    if(configuration == FacetConfiguration::one) {
        return 0;
    } else if(configuration == FacetConfiguration::two) {
        if(k >= 0) {
            return 0;
        } else {
            return 1;
        }
    } else if(configuration == FacetConfiguration::four) {

    } else if(configuration == FacetConfiguration::six) {

    } else if(configuration == FacetConfiguration::eight) {
        if(i >= 0 && j >= 0 && k >= 0) { return 0; }
        if(i < 0 && j >= 0 && k >= 0) { return 1; }
        if(i >= 0 && j < 0 && k >= 0) { return 2; }
        if(i < 0 && j < 0 && k >= 0) { return 3; }
        if(i >= 0 && j >= 0 && k < 0) { return 4; }
        if(i < 0 && j >= 0 && k < 0) { return 5; }
        if(i >= 0 && j < 0 && k < 0) { return 6; }
        if(i < 0 && j < 0 && k < 0) { return 7; }
    }

    // The default
    return 0;
}

int DamageableLayer::GetFacetIndexByName(FacetName facet_name) {
    // We ignore the parameter and just return the single facet
    if(configuration == FacetConfiguration::one) {
        return 0;
    } else if(configuration == FacetConfiguration::two) {
        return Find(facet_name, two_configuration, 2);
    } else if(configuration == FacetConfiguration::four) {
        return Find(facet_name, four_configuration, 4);
    } else if(configuration == FacetConfiguration::six) {
        return Find(facet_name, six_configuration, 6);
    } else if(configuration == FacetConfiguration::eight) {
        return Find(facet_name, eight_configuration, 8);
    }

    return -1;
}

void DamageableLayer::InitFacetByName(FacetName facet_name, float facet_health) {
    int facet_index = GetFacetIndexByName(facet_name);

    facets[facet_index].health.health = facet_health;
    facets[facet_index].health.max_health = facet_health;
}

void DamageableLayer::InitFacetByName(FacetName facet_name, Health facet_health) {
    int facet_index = GetFacetIndexByName(facet_name);

    facets[facet_index].health = facet_health;
}


/** This is one of the few functions in libdamage to implement a non-generic
 * model. There are too many models for damaging components and we have to settle
 * for one. */
void DamageableLayer::ReduceLayerCapability(const float& percent,
                                            const float& chance_to_reduce_regeneration) {
    static std::random_device randome_device;
    static std::mt19937 gen(randome_device());

    // TODO: this feels a bit sloppy, as we're dealing in integers here.
    static std::uniform_int_distribution<> impact_distribution(1, 100);
    static std::uniform_int_distribution<> facet_distribution(0, facets.size()-1);

    bool affect_regeneration = impact_distribution(gen) <= chance_to_reduce_regeneration;
    int facet_index = facet_distribution(gen);

    if(affect_regeneration) {
        // Reduce regeneration
        facets[facet_index].health.ReduceRegeneration(percent);
    } else {
        // Reduce max health
        facets[facet_index].health.ReduceLayerMaximum(percent);
    }
}


float DamageableLayer::TotalLayerValue() {
    float total_value = 0.0f;
    for(const DamageableFacet& facet: facets) {
        total_value += facet.health.health;
    }
    return total_value;
}

float DamageableLayer::TotalMaxLayerValue() {
    float total_value = 0.0f;
    for(const DamageableFacet& facet: facets) {
        total_value += facet.health.max_health;
    }
    return total_value;
}

float DamageableLayer::AverageLayerValue() {
    float total_value = 0.0f;
    for(const DamageableFacet& facet: facets) {
        total_value += facet.health.health;
    }
    return total_value / facets.size();
}

float DamageableLayer::AverageMaxLayerValue() {
    float total_value = 0.0f;
    for(const DamageableFacet& facet: facets) {
        total_value += facet.health.max_health;
    }
    return total_value / facets.size();
}


float DamageableLayer::GetPercent(FacetName facet_name) {
    if(configuration == FacetConfiguration::eight) {
        if(facet_name == FacetName::front) {
            return facets[0].health.health/facets[0].health.max_health +
                    facets[1].health.health/facets[1].health.max_health +
                    facets[4].health.health/facets[4].health.max_health +
                    facets[5].health.health/facets[5].health.max_health;
        }

        if(facet_name == FacetName::rear) {
            return facets[2].health.health/facets[2].health.max_health +
                    facets[3].health.health/facets[3].health.max_health +
                    facets[6].health.health/facets[6].health.max_health +
                    facets[7].health.health/facets[7].health.max_health;
        }

        if(facet_name == FacetName::left) {
            return facets[0].health.health/facets[0].health.max_health +
                    facets[2].health.health/facets[2].health.max_health +
                    facets[4].health.health/facets[4].health.max_health +
                    facets[6].health.health/facets[6].health.max_health;
        }

        if(facet_name == FacetName::right) {
            return facets[1].health.health/facets[1].health.max_health +
                    facets[3].health.health/facets[3].health.max_health +
                    facets[5].health.health/facets[5].health.max_health +
                    facets[7].health.health/facets[7].health.max_health;
        }
    } else {
        if(facet_name == FacetName::front) {
            return facets[0].health.health/facets[0].health.max_health;
        }

        if(facet_name == FacetName::rear) {
            return facets[1].health.health/facets[1].health.max_health;
        }

        if(facet_name == FacetName::left) {
            return facets[2].health.health/facets[2].health.max_health;
        }

        if(facet_name == FacetName::right) {
            return facets[3].health.health/facets[3].health.max_health;
        }
    }

    return 0;
}

void DamageableLayer::Regenerate() {
    for(DamageableFacet& facet : facets) {
        facet.health.Regenerate();
    }
}

void DamageableLayer::UpdateRegeneration(const float& new_regeneration_value) {
    for(DamageableFacet& facet : facets) {
        facet.health.regeneration = new_regeneration_value;
    }
}
