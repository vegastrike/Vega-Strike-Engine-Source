/*
 * health.cpp
 *
 * Copyright (C) 2021-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#include "health.h"

#include <algorithm>
#include <iostream>
#include <string>


Health::Health(int layer, double health, double regeneration) :
            layer(layer),
            health(health, 0, health) {
        destroyed = false;
        
        vulnerabilities.normal_damage = 1;
        vulnerabilities.phase_damage = 1;
    };

void Health::DealDamage(Damage &damage, InflictedDamage &inflicted_damage) {
    // If this layer is destroyed, it can no longer sustain damage
    if (destroyed) {
        return;
    }

    DealDamageComponent(0, damage.normal_damage, vulnerabilities.normal_damage, inflicted_damage);
    DealDamageComponent(1, damage.phase_damage, vulnerabilities.phase_damage, inflicted_damage);

    // TODO: implement other types of damage
}


/**
 * @brief Health::DealDamageComponent - deal a component of damage (normal, phased) and not damage
 * a component.
 * @param health - to subtract from
 * @param damage - to inflict
 * @param vulnerability - adjust for
 */
// TODO: type is ugly hack
void Health::DealDamageComponent(int type, double &damage, double vulnerability, InflictedDamage &inflicted_damage) {
    // Here we adjust for specialized weapons such as shield bypassing and shield leeching
    // which only damage the shield.
    // We also cap the actual damage at the current health
    const float adjusted_damage = std::min(damage * vulnerability, health.Value());

    // We check if there's any damage left to pass on to the next layer
    damage -= adjusted_damage;

    // Damage the current health
    health -= adjusted_damage;

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

    // A bit hardcoded
    if (layer < 2) {
        destroyed = true;
    }
}

void Health::Destroy() {
    health = 0;
    destroyed = true;
}


void Health::SetHealth(double new_health) {
    this->health.Set(new_health);
}