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

void Health::AdjustPower(const double &percent) {
    if (!regenerative) {
        // Not applicable for armor and hull
        return;
    }

    if (percent > 1 || percent < 0) {
        // valid values are between 0 and 1
        return;
    }

    power = percent;
}

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
void Health::DealDamageComponent(int type, double &damage, float vulnerability, InflictedDamage &inflicted_damage) {
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

    if (health == 0 && !regenerative) {
        destroyed = true;
    }
}

void Health::Destroy() {
    health = 0;
    destroyed = true;
}

/*  This is a bit kludgy. Set power via keyboard only works when not suppressed.
*   If ship is in SPEC, power will be continuously set to 0.
*   Therefore, if you set power to 1/3, go to SPEC and out again, power will be
*   set to full again.
*/
void Health::SetPower(const double power) {
    if (regenerative) {
        this->power = power;
    }
}


/** Enhance adds some oomph to shields. 
 * Originally, I thought to just make them 150% one time.
 * However, this isn't really significant and it's hard to implement
 * with the underlying Resource class, which checks for max values.
 * Instead, this will upgrade the Max value of shields and repair them.
 */
void Health::Enhance(double percent) {
    // Don't enhance armor and hull
    if (!regenerative) {
        return;
    }

    // Sanity checks. Don't want to use enhance to downgrade 
    // and more than x100 is excessive.
    if(percent < 1.0 || percent > 100.0) {
        return;
    }

    health.SetMaxValue(health.MaxValue() * percent);
    regeneration.SetMaxValue(regeneration.MaxValue() * percent);
}


void Health::Regenerate() {
    if (!regenerative) {
        return;
    }

    if(health.Percent() < power) {
        health++;
    } else if(health.Percent() > power) {
        health--;
    }
}

void Health::Regenerate(float recharge_rate) {
    /*if (!enabled || destroyed || !regenerative) {
        return;
    }

    health = std::min(adjusted_health, health + recharge_rate);*/
}

void Health::SetHealth(float health) {
   /* health = std::min(max_health, health);
    health = std::max(0.0f, health);
    this->health = health;*/
}

void Health::Update(float new_health) {
    this->health.SetMaxValue(new_health);
}
