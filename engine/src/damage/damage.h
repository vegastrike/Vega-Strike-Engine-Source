#ifndef DAMAGE_H
#define DAMAGE_H

/**
 * @brief The Damage struct provides a cleaner way for a weapon to deliver multiple effects. e.g. both damage and a torque, or damage to specific sub-systems.
 */
struct Damage {
    float normal_damage;        // Go through shield, armor and finally hull and subsystems
    float phase_damage;         // Bypass shields
    float propulsion_damage;    // Disables the ship's drive
    float blast_effect;         // Add torque to the ship

    bool Spent() {
        return (normal_damage == 0 && phase_damage == 0 && propulsion_damage == 0 && blast_effect == 0);
    }
};

#endif // DAMAGE_H
