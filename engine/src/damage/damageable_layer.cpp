#include "damageable_layer.h"

#include <iostream>
#include <random>

void DamageableLayer::AdjustPower(const float& percent) {
    float adjusted_percent = std::max(std::min(percent, 1.0f), 0.0f);

    for(Health& facet : facets) {
        facet.AdjustPower(adjusted_percent);
    }
}

void DamageableLayer::DealDamage( const CoreVector &attack_vector, Damage &damage, InflictedDamage& inflicted_damage ) {
    int impacted_facet_index = GetFacetIndex(attack_vector);
    facets[impacted_facet_index].DealDamage(damage, inflicted_damage);
}


void DamageableLayer::Destroy() {
    for(Health& facet : facets) {
        facet.health = 0;
        facet.destroyed = true;
    }
}


void DamageableLayer::Disable() {
    for(Health& facet : facets) {
        facet.health = 0;
        facet.enabled = false;
    }
}

// Used for nicer graphics when entering SPEC
void DamageableLayer::GradualDisable(float percent) {
    const float current_percent = facets[0].adjusted_health / facets[0].max_health;
    AdjustPower(current_percent - percent);
}


void DamageableLayer::Enable() {
    for(Health& facet : facets) {
        facet.enabled = true;
    }
}

// TODO: test
// Boost shields to 150%
void DamageableLayer::Enhance() {
    for(Health& facet: facets) {
        // Don't enhance armor and hull
        if(!facet.regenerative) {
            continue;
        }

        facet.health = facet.max_health * 1.5;
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

    facets[facet_index].health = facet_health;
    facets[facet_index].max_health = facet_health;
}

void DamageableLayer::InitFacetByName(FacetName facet_name, Health facet_health) {
    int facet_index = GetFacetIndexByName(facet_name);

    facets[facet_index] = facet_health;
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
        facets[facet_index].ReduceRegeneration(percent);
    } else {
        // Reduce max health
        facets[facet_index].ReduceLayerMaximum(percent);
    }
}


float DamageableLayer::TotalLayerValue() {
    float total_value = 0.0f;
    for(const Health& facet: facets) {
        total_value += facet.health;
    }
    return total_value;
}

float DamageableLayer::TotalMaxLayerValue() {
    float total_value = 0.0f;
    for(const Health& facet: facets) {
        total_value += facet.max_health;
    }
    return total_value;
}

float DamageableLayer::AverageLayerValue() {
    float total_value = 0.0f;
    for(const Health& facet: facets) {
        total_value += facet.health;
    }
    return total_value / facets.size();
}

float DamageableLayer::AverageMaxLayerValue() {
    float total_value = 0.0f;
    for(const Health& facet: facets) {
        total_value += facet.factory_max_health;
    }
    return total_value / facets.size();
}




float DamageableLayer::GetPercent(FacetName facet_name) {
    // Two or Four shield configurations
    if(configuration != FacetConfiguration::eight) {
        int index = GetFacetIndexByName(facet_name);
        if(facets[index].factory_max_health == 0) {
            return 0;
        }

        return facets[index].health / facets[index].factory_max_health;
    }

    // Indices of facets for shield configuration eight
    static const int indices_array[4][4] = {{0,2,4,6},    // left
                                            {1,3,5,7},    // right
                                            {0,1,4,5},    // front
                                            {2,3,6,7}};   // rear

    int indices_index; // An index to the top array dimension
    if(facet_name == FacetName::left) {
        indices_index = 0;
    } else if(facet_name == FacetName::right) {
        indices_index = 1;
    } else if(facet_name == FacetName::front) {
        indices_index = 2;
    } else if(facet_name == FacetName::rear) {
        indices_index = 3;
    } else  {
        // We received a malformed facet name
        return 0;
    }

    float aggregate_health = 0;
    float aggregate_max_health = 0;

    for(int i=0;i<4;i++) {
        int facet_index = indices_array[indices_index][i];
        Health& facet = facets[facet_index];
        aggregate_health += facet.health;
        aggregate_max_health += facet.factory_max_health;
    }

    if(aggregate_max_health == 0) {
        return 0;
    }

    return aggregate_health / aggregate_max_health;
}

void DamageableLayer::Regenerate(float alternative_regeneration) {
    for(Health& facet : facets) {
        facet.Regenerate(alternative_regeneration);
    }
}

void DamageableLayer::UpdateRegeneration(const float& new_regeneration_value) {
    for(Health& facet : facets) {
        facet.regeneration = new_regeneration_value;
    }
}
