#include "damageable_layer.h"

#include <random>
#include <cassert>

// TODO: this is a use of the code in a different library.
// I'm unhappy with this, so it needs to change.
#include "mount_size.h"


DamageableLayer::DamageableLayer(int layer_index,
                                 FacetConfiguration configuration,
                                 Health health_template,
                                 bool core_layer) {
    health_template.layer = layer_index;
    int size = as_integer(configuration);

    std::vector<Health> facets;
    for(int i=0;i<size;i++) {
        facets.push_back(health_template);
    }

    this->layer_index = layer_index;
    this->configuration = configuration;
    this->number_of_facets = size;
    this->facets = facets;
    this->core_layer = core_layer;
}

DamageableLayer::DamageableLayer(int layer_index,
                                 FacetConfiguration configuration,
                                 float health_array[],
                                 float regeneration,
                                 bool core_layer) {

    int size = as_integer(configuration);

    std::vector<Health> facets;
    for(int i=0;i<size;i++) {
        Health health(layer_index, health_array[i], regeneration);
        facets.push_back(health);
    }

    this->layer_index = layer_index;
    this->configuration = configuration;
    this->number_of_facets = size;
    this->facets = facets;
    this->core_layer = core_layer;
}

DamageableLayer::DamageableLayer(int layer_index, int number_of_facets, std::vector<Health>& facets, bool core_layer):
    layer_index(layer_index),
    number_of_facets(number_of_facets),
    facets(facets),
    core_layer(core_layer) {}

DamageableLayer::DamageableLayer():
    layer_index(0),
    number_of_facets(0),
    core_layer(false) {
}


void DamageableLayer::AdjustPower(const float& percent) {
    float adjusted_percent = std::max(std::min(percent, 1.0f), 0.0f);

    for(Health& facet : facets) {
        facet.AdjustPower(adjusted_percent);
    }
}

void DamageableLayer::DealDamage( const CoreVector &attack_vector, Damage &damage, InflictedDamage& inflicted_damage ) {
    if(number_of_facets == 0) {
        return;
    }

    int impacted_facet_index = GetFacetIndex(attack_vector);
    facets[impacted_facet_index].DealDamage(damage, inflicted_damage);
}


void DamageableLayer::Destroy() {
    for(Health& facet : facets) {
        facet.Destroy();
    }
}


void DamageableLayer::Disable() {
    for(Health& facet : facets) {
        facet.Disable();
    }
}


void DamageableLayer::Discharge(float discharge_rate, float minimum_discharge) {
    for(Health& facet : facets) {
        if(facet.health > minimum_discharge * facet.max_health) {
            facet.health *= discharge_rate;
        }
    }
}


// Used for nicer graphics when entering SPEC
void DamageableLayer::GradualDisable() {
    for(Health& facet : facets) {
        facet.ReduceLayerMaximumByOne();
    }
}


void DamageableLayer::Enable() {
    for(Health& facet : facets) {
        facet.Enable();
    }
}

bool DamageableLayer::Enabled() {
    if(number_of_facets == 0) {
        return false;
    }

    return facets[0].enabled;
}

// TODO: test
// Boost shields to 150%
void DamageableLayer::Enhance() {
    for(Health& facet: facets) {
        // Don't enhance armor and hull
        facet.Enhance();
    }
}

int DamageableLayer::GetFacetIndex(const CoreVector& attack_vector) {
    if(number_of_facets == 0) {
        return -1;
    }

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
        float a = i + k;
        float b = i - k;
        if(a >= 0 && b >= 0) {
            return 0;
        }

        if(a >= 0 && b < 0) {
            return 2;
        }

        if(a < 0 && b >= 0) {
            return 3;
        }

        if(a < 0 && b < 0) {
            return 1;
        }

        return 0;

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






/** This is one of the few functions in libdamage to implement a non-generic
 * model. There are too many models for damaging components and we have to settle
 * for one. */
void DamageableLayer::ReduceLayerCapability(const float& percent,
                                            const float& chance_to_reduce_regeneration) {
    if(number_of_facets == 0) {
        return;
    }

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
        total_value += facet.max_health;
    }
    return total_value / facets.size();
}


float CalculatePercentage(float numerator, float denominator) {
    return numerator / denominator;

    // All these checks potentially slow down the game
    // and cause the graphics to flicker
    /*if(denominator < numerator) {
        return 0.0; // This should really be an error
    }

    if(denominator <= 0.0f || numerator <0.0f) {
        return 0.0;
    }

    float percent = numerator / denominator;

    if(percent > 1.0f) {
        return 1.0;
    }

    if(percent <0.01) {
        return 0.0f;
    }

    // Possibly nicer alternative
    //return roundf(percent * 100) / 100.0;
    return percent;*/
}

float DamageableLayer::GetMaxHealth() {
    if(number_of_facets == 0) {
        return 0.0f;
    }

    return facets[0].max_health;
}


float DamageableLayer::GetPercent(FacetName facet_name) {
    if(number_of_facets == 0) {
        return 0.0f;
    }

    float numerator, denominator;
    // One, Two or Four shield configurations
    // Note the fallthrough
    switch(configuration) {
    case FacetConfiguration::one:
    case FacetConfiguration::two:
    case FacetConfiguration::four:
        numerator = facets[as_integer(facet_name)].health;
        denominator = facets[as_integer(facet_name)].max_health;
        return CalculatePercentage(numerator, denominator);

    default:
            break; // Noop
    }


    // We handle the eight configuration outside the switch
    // as it is longer and more complex
    float percent = 0.0f;

    // Indices of facets for shield configuration eight
    static const int indices_array[4][4] = {{0,2,4,6},    // left
                                            {1,3,5,7},    // right
                                            {0,1,4,5},    // front
                                            {2,3,6,7}};   // rear

    int indices_index; // An index to the top array dimension

    switch(facet_name) {
    case FacetName::four_left: indices_index = 0; break;
    case FacetName::four_right: indices_index = 1; break;
    case FacetName::four_front: indices_index = 2; break;
    case FacetName::four_rear: indices_index = 3; break;
    default:
        // We received a malformed facet name
        return 0;
    }

    float aggregate_health = 0;
    float aggregate_max_health = 0;

    for(int i=0;i<4;i++) {
        int facet_index = indices_array[indices_index][i];
        Health& facet = facets[facet_index];
        aggregate_health += facet.health;
        aggregate_max_health += facet.max_health;
    }

    percent = CalculatePercentage(aggregate_health, aggregate_max_health);
    return percent;
}

void DamageableLayer::Regenerate(float recharge_rate) {
    for(Health& facet : facets) {
        facet.Regenerate(recharge_rate);
    }
}


void DamageableLayer::RegenerateOrDischarge(float recharge_rate, bool velocity_discharge, float discharge_rate) {
    for(Health& facet : facets) {
        facet.Regenerate(recharge_rate);
    }
}


float DamageableLayer::GetRegeneration() {
    if(number_of_facets == 0) {
        return 0.0f;
    }

    return facets[0].regeneration;
}

void DamageableLayer::UpdateFacets(const unsigned int new_size, const float new_facets[4]) {
    assert(new_size == number_of_facets);

    switch (number_of_facets) {
    case 1:
        facets[0].Update(new_facets[0]);
        break;

    case 4:
        facets[0].Update(new_facets[3]);
        facets[1].Update(new_facets[2]);
        facets[2].Update(new_facets[0]);
        facets[3].Update(new_facets[1]);
        break;
    case 2: // Note the fallthrough
    case 8:
        for(unsigned int i=0;i<number_of_facets;i++) {
            facets[i].Update(new_facets[i]);
        }
        break;
    }
}


void DamageableLayer::UpdateRegeneration(const float& new_regeneration_value) {
    for(Health& facet : facets) {
        facet.regeneration = new_regeneration_value;
        facet.max_regeneration = new_regeneration_value;
        facet.regenerative = true;
        facet.enabled = true;
    }
}
