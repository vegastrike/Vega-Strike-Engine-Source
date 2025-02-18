/*
 * damage.h
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_DAMAGE_H
#define VEGA_STRIKE_ENGINE_DAMAGE_DAMAGE_H

#include <vector>

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

struct InflictedDamage {
    double total_damage;         // Total inflicted damage
    double normal_damage;        // Go through shield, armor and finally hull and subsystems
    double phase_damage;         // Bypass shields
    double propulsion_damage;    // Disables the ship's drive

    // This array stores the damage inflicted to each layer
    // By default, it is  hard-coded to three layers:
    // shield, armor and hull. But this makes this implementation inflexible.
    std::vector<double> inflicted_damage_by_layer;

    InflictedDamage(int number_of_layers = 3) {
        total_damage = 0.0;
        normal_damage = 0.0;
        phase_damage = 0.0;
        propulsion_damage = 0.0;

        for (int i = 0; i < number_of_layers; i++) {
            inflicted_damage_by_layer.push_back(0.0f);
        }
    }

    InflictedDamage() = delete;
};

#endif //VEGA_STRIKE_ENGINE_DAMAGE_DAMAGE_H
