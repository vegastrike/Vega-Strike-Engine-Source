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

#include <cassert>

// TODO: this is a use of the code in a different library.
// I'm unhappy with this, so it needs to change.
#include "cmd/mount_size.h"

#include "random_utils.h"




DamageableLayer::DamageableLayer(int layer,
        FacetConfiguration configuration,
        double health_template,
        Damage vulnerabilities,
        bool core_layer, double regeneration):
        layer(layer),
        vulnerabilities(vulnerabilities),
        core_layer(core_layer),
        number_of_facets(as_integer(configuration)) {
    Resource<double> health = Resource<double>(health_template, 0.0, health_template);
    for (int i = 0; i < number_of_facets; i++) {
        facets.push_back(health);
    }
}



void DamageableLayer::DealDamage(const CoreVector &attack_vector, Damage &damage, InflictedDamage &inflicted_damage) {
    if (number_of_facets == 0) {
        return;
    }

    if (number_of_facets != facets.size()) {
        return;
    }

    int impacted_facet_index = GetFacetIndex(attack_vector);

    DealDamageComponent(impacted_facet_index, 0, damage.normal_damage, vulnerabilities.normal_damage, inflicted_damage);
    DealDamageComponent(impacted_facet_index, 1, damage.phase_damage, vulnerabilities.phase_damage, inflicted_damage);

    // TODO: implement other types of damage
}

/**
 * @brief DamageableLayer::DealDamageComponent - deal a component of damage (normal, phased) and not damage
 * a component.
 * @param health - to subtract from
 * @param damage - to inflict
 * @param vulnerability - adjust for
 */
// TODO: type is ugly hack
void DamageableLayer::DealDamageComponent(int impacted_facet_index, int type,
                                          double &damage, double vulnerability,
                                          InflictedDamage &inflicted_damage) {
    // Here we adjust for specialized weapons such as shield bypassing and shield leeching
    // which only damage the shield.
    // We also cap the actual damage at the current health
    const double adjusted_damage = std::min(damage * vulnerability, facets[impacted_facet_index].Value());

    // Damage the current health
    facets[impacted_facet_index].DamageByValue(adjusted_damage);

    // Adjust damage left to pass on to the next layer
    damage -= adjusted_damage;

    // Record damage
    switch (type) {
        case 0:
            inflicted_damage.normal_damage += adjusted_damage;
            break;
        case 1:
            inflicted_damage.phase_damage += adjusted_damage;
    }

    inflicted_damage.total_damage += adjusted_damage;
    inflicted_damage.inflicted_damage_by_layer.at(layer) += adjusted_damage;
}


int DamageableLayer::GetFacetIndex(const CoreVector &attack_vector) const {
    if (number_of_facets == 0) {
        return -1;
    }

    // Convenience Variables
    double i = attack_vector.i;
    // double j = attack_vector.j; not used
    double k = attack_vector.k;
    double a = i + k;
    double b = i - k;

    switch (number_of_facets) {
    case(1):
        return 0;
    case(2):
        if (k >= 0) {
            return 0;
        } else {
            return 1;
        }
    case(4):
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
    default:
        return 0;
    }
}


double DamageableLayer::TotalLayerValue() const {
    double total_value = 0.0f;
    for (const Resource<double> &facet : facets) {
        total_value += facet.Value();
    }
    return total_value;
}

double DamageableLayer::TotalMaxLayerValue() const {
    double total_value = 0.0f;
    for (const Resource<double> &facet : facets) {
        total_value += facet.MaxValue();
    }
    return total_value;
}

double DamageableLayer::AverageLayerValue() const {
    double total_value = 0.0f;
    for (const Resource<double> &facet : facets) {
        total_value += facet.Value();
    }
    return total_value / number_of_facets;
}

double DamageableLayer::AverageMaxLayerValue() const {
    double total_value = 0.0f;
    for (const Resource<double> &facet : facets) {
        total_value += facet.MaxValue();
    }
    return total_value / number_of_facets;
}

double DamageableLayer::Percent() const {
    if(number_of_facets == 0) {
        return 0;
    }

    double percents = 0;
    for(const Resource<double>& facet:facets){
        percents += facet.Percent();
    }

    return percents/number_of_facets;
}

double DamageableLayer::Percent(int facet) const {
    if(number_of_facets <= facet || facets.size() <= facet) {
        return 0;
    }

    if(facets[facet].MaxValue() == 0) {
        return 0;
    }

    return facets[facet].Percent();
}


int DamageableLayer::Layer() const {
    return layer;
}
