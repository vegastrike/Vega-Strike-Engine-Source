/*
 * Copyright (C) 2021-2022 Roy Falk, Stephen G. Tuggy,
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


#include "damageable_layer.h"

#include <random>
#include <cassert>
#include <iostream>

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
    for (int i = 0; i < size; i++) {
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
        double health_array[],
        bool core_layer) {

    int size = as_integer(configuration);

    std::vector<Health> facets;
    for (int i = 0; i < size; i++) {
        Health health(layer_index, health_array[i]);
        facets.push_back(health);
    }

    this->layer_index = layer_index;
    this->configuration = configuration;
    this->number_of_facets = size;
    this->facets = facets;
    this->core_layer = core_layer;
}

DamageableLayer::DamageableLayer(int layer_index, int number_of_facets, std::vector<Health> &facets, bool core_layer) :
        layer_index(layer_index),
        number_of_facets(number_of_facets),
        facets(facets),
        core_layer(core_layer) {
}

DamageableLayer::DamageableLayer() :
        layer_index(0),
        number_of_facets(0),
        core_layer(false) {
}



void DamageableLayer::DealDamage(const CoreVector &attack_vector, Damage &damage, InflictedDamage &inflicted_damage) {
    if (number_of_facets == 0) {
        return;
    }

    int impacted_facet_index = GetFacetIndex(attack_vector);
    facets[impacted_facet_index].DealDamage(damage, inflicted_damage);
}

void DamageableLayer::Destroy() {
    for (Health &facet : facets) {
        facet.Destroy();
    }
}


int DamageableLayer::GetFacetIndex(const CoreVector &attack_vector) {
    if (number_of_facets == 0) {
        return -1;
    }

    // Convenience Variables
    double i = attack_vector.i;
    double j = attack_vector.j;
    double k = attack_vector.k;

    if (configuration == FacetConfiguration::one) {
        return 0;
    } else if (configuration == FacetConfiguration::two) {
        if (k >= 0) {
            return 0;
        } else {
            return 1;
        }
    } else if (configuration == FacetConfiguration::four) {
        double a = i + k;
        double b = i - k;
        if (a >= 0 && b >= 0) {
            return 0;
        }

        if (a >= 0 && b < 0) {
            return 2;
        }

        if (a < 0 && b >= 0) {
            return 3;
        }

        if (a < 0 && b < 0) {
            return 1;
        }

        return 0;

    } else if (configuration == FacetConfiguration::eight) {
        if (i >= 0 && j >= 0 && k >= 0) {
            return 0;
        }
        if (i < 0 && j >= 0 && k >= 0) {
            return 1;
        }
        if (i >= 0 && j < 0 && k >= 0) {
            return 2;
        }
        if (i < 0 && j < 0 && k >= 0) {
            return 3;
        }
        if (i >= 0 && j >= 0 && k < 0) {
            return 4;
        }
        if (i < 0 && j >= 0 && k < 0) {
            return 5;
        }
        if (i >= 0 && j < 0 && k < 0) {
            return 6;
        }
        if (i < 0 && j < 0 && k < 0) {
            return 7;
        }
    }

    // The default
    return 0;
}



double DamageableLayer::TotalLayerValue() const {
    double total_value = 0.0f;
    for (const Health &facet : facets) {
        total_value += facet.health.Value();
    }
    return total_value;
}

double DamageableLayer::TotalMaxLayerValue() const {
    double total_value = 0.0f;
    for (const Health &facet : facets) {
        total_value += facet.health.MaxValue();
    }
    return total_value;
}

double DamageableLayer::AverageLayerValue() const {
    double total_value = 0.0f;
    for (const Health &facet : facets) {
        total_value += facet.health.Value();
    }
    return total_value / facets.size();
}

double DamageableLayer::AverageMaxLayerValue() const {
    double total_value = 0.0f;
    for (const Health &facet : facets) {
        total_value += facet.health.MaxValue();
    }
    return total_value / facets.size();
}


double DamageableLayer::GetMaxHealth() const {
    if (number_of_facets == 0) {
        return 0.0f;
    }

    return facets[0].health.MaxValue();
}

double DamageableLayer::GetPercent(FacetName facet_name) const {
    if (number_of_facets == 0) {
        return 0.0f;
    }

    double numerator, denominator;
    // One, Two or Four shield configurations
    // Note the fallthrough
    switch (configuration) {
        case FacetConfiguration::one:
        case FacetConfiguration::two:
        case FacetConfiguration::four:
            numerator = facets[as_integer(facet_name)].health.Value();
            denominator = facets[as_integer(facet_name)].health.MaxValue();
            return numerator / denominator;

        default:
            break; // Noop
    }


    // We handle the eight configuration outside the switch
    // as it is longer and more complex
    double percent = 0.0f;

    // Indices of facets for shield configuration eight
    static const int indices_array[4][4] = {{0, 2, 4, 6},    // left
            {1, 3, 5, 7},    // right
            {0, 1, 4, 5},    // front
            {2, 3, 6, 7}};   // rear

    int indices_index; // An index to the top array dimension

    switch (facet_name) {
        case FacetName::four_left:
            indices_index = 0;
            break;
        case FacetName::four_right:
            indices_index = 1;
            break;
        case FacetName::four_front:
            indices_index = 2;
            break;
        case FacetName::four_rear:
            indices_index = 3;
            break;
        default:
            // We received a malformed facet name
            return 0;
    }

    double aggregate_health = 0;
    double aggregate_max_health = 0;

    for (int i = 0; i < 4; i++) {
        int facet_index = indices_array[indices_index][i];
        const Health &facet = facets[facet_index];
        aggregate_health += facet.health.Value();
        aggregate_max_health += facet.health.MaxValue();
    }

    percent = aggregate_health / aggregate_max_health;
    return percent;
}





void DamageableLayer::UpdateFacets(const std::vector<double> new_facets) {
    int new_size = new_facets.size();
    // TODO: assert(new_size == number_of_facets);

    if(new_size != number_of_facets) {
        number_of_facets = new_size;
        facets.clear();
        Health health = Health(layer_index, 0);
        for(int i=0;i<number_of_facets;i++) {
            facets.push_back(health);
        }
    }

    
    switch (number_of_facets) {
        case 0: 
            configuration = FacetConfiguration::zero;
            break;
        case 1:
            configuration = FacetConfiguration::one;
            facets[0].health.SetMaxValue(new_facets[0]);
            break;

        case 4:
            configuration = FacetConfiguration::four;
            facets[0].health.SetMaxValue(new_facets[3]);
            facets[1].health.SetMaxValue(new_facets[2]);
            facets[2].health.SetMaxValue(new_facets[0]);
            facets[3].health.SetMaxValue(new_facets[1]);
            break;
        case 2: 
            configuration = FacetConfiguration::two;
            for (unsigned int i = 0; i < number_of_facets; i++) {
                facets[i].health.SetMaxValue(new_facets[i]);
            }
            break;
        case 8:
            configuration = FacetConfiguration::eight;
            for (unsigned int i = 0; i < number_of_facets; i++) {
                facets[i].health.SetMaxValue(new_facets[i]);
            }
            break;
    }
}

