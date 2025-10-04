/*
 * damageable_object.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "damage/damageable_object.h"
#include "damage/core_vector.h"
#include <boost/range/adaptor/reversed.hpp>
#include <iostream>


DamageableObject::~DamageableObject() = default;

DamageableObject::DamageableObject():
    number_of_layers(0), invulnerable(false) {}


DamageableObject::DamageableObject(std::vector<DamageableLayer*> layers):
    number_of_layers(3), layers(layers), invulnerable(false) {}


void DamageableObject::AddLayer(DamageableLayer* layer) {
    number_of_layers++;
    layers.push_back(layer);
    assert(number_of_layers<=3);
}

InflictedDamage DamageableObject::DealDamage(const CoreVector &attack_vector, Damage &damage) {
    InflictedDamage inflicted_damage(3); // Currently hard-coded default is 3!

    if(invulnerable) {
        return inflicted_damage;
    }

    // Higher index layers are outer layers. We therefore need to reverse the order.
    for (DamageableLayer* layer : boost::adaptors::reverse(layers)) {
        if(!layer) {
            continue;
        }

        layer->DealDamage(attack_vector, damage, inflicted_damage);

        // TODO: handle damage to components here?
        // Assumed the core layer has only one facet
        // TODO: unassume this
        /*if(layer.core_layer && layer.facets[0].health.destroyed) {

        }*/

        if (damage.Spent()) {
            break;
        }
    }

    return inflicted_damage;
}

void DamageableObject::Destroy() {
    layers[0]->facets[0] = 0;
}

// We make a lot of assumptions here:
// 1. The last layer is THE layer
// 2. There's only one facet in the last layer
// 3. Destroying it destroys the unit
bool DamageableObject::Destroyed() {
    if (layers.empty()) {
        return true;
    }

    return layers[0]->facets[0] == 0;
}

void DamageableObject::SetInvulnerable(bool invulnerable) {
    this->invulnerable = invulnerable;
}
