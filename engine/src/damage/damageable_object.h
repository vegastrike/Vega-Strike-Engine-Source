/*
 * damageable_object.h
 *
 * Copyright (C) 2021 Roy Falk
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_OBJECT_H
#define VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_OBJECT_H

#include "damage.h"
#include "damageable_layer.h"
#include "core_vector.h"

/**
 * @brief The DamageableObject class represents an overall game object - ship, space station, missile, etc.
 */
struct DamageableObject {
    int number_of_layers;
    std::vector<DamageableLayer*> layers;        // Typically shield/armor/hull

    DamageableObject(std::vector<DamageableLayer*> layers);
    DamageableObject();

    InflictedDamage DealDamage(const CoreVector &attack_vector, Damage &damage);

    void Destroy();
    bool Destroyed();

    // Callbacks
    void (*explosion)(DamageableObject object);
    void *(*residual_debris)(DamageableObject object);
};

#endif //VEGA_STRIKE_ENGINE_DAMAGE_DAMAGEABLE_OBJECT_H
