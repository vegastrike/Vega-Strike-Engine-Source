/*
 * damage.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_DAMAGE_H
#define VEGA_STRIKE_ENGINE_DAMAGE_DAMAGE_H

#include <map>

/**
 * @brief The Damage struct provides a cleaner way for a weapon to deliver multiple effects. e.g. both damage and a torque, or damage to specific sub-systems.
 */
struct Damage {
    // TODO: generalize this later
    // TODO: add shield leach
    double normal_damage;        // Go through shield, armor and finally hull and subsystems
    double phase_damage;         // Bypass shields
    double propulsion_damage;    // Disables the ship's drive
    //double blast_effect;         // Add torque to the ship

    bool Spent() {
        return (normal_damage == 0 && phase_damage == 0 && propulsion_damage == 0);// && blast_effect == 0);
    }

    Damage(double normal_damage = 0,
            double phase_damage = 0,
            double propulsion_damage = 0,
            double blast_effect = 0) :
            normal_damage(normal_damage),
            phase_damage(phase_damage),
            propulsion_damage(propulsion_damage) {
    }

    //blast_effect(blast_effect),
};

/**
 * @brief The DamageLayer enum names each layer of a DamageableObject,
 * for DamageableObject::GetLayer and InflictedDamage::inflicted_damage_by_layer.
 */
enum class DamageLayer {
    hull,
    armor,
    shield,
};

struct InflictedDamage {
    double total_damage;         // Total inflicted damage
    double normal_damage;        // Go through shield, armor and finally hull and subsystems
    double phase_damage;         // Bypass shields
    double propulsion_damage;    // Disables the ship's drive

    // This map stores the damage inflicted to each layer
    std::map<DamageLayer, double> inflicted_damage_by_layer;

    InflictedDamage() {
        total_damage = 0.0;
        normal_damage = 0.0;
        phase_damage = 0.0;
        propulsion_damage = 0.0;

        inflicted_damage_by_layer[DamageLayer::hull] = 0.0;
        inflicted_damage_by_layer[DamageLayer::armor] = 0.0;
        inflicted_damage_by_layer[DamageLayer::shield] = 0.0;
    }
};

#endif //VEGA_STRIKE_ENGINE_DAMAGE_DAMAGE_H
