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

    // These variables store actual inflicted damage as it traverses the layers
    float inflicted_normal_damage;
    float inflicted_phase_damage;
    float inflicted_propulsion_damage;
    float inflicted_blast_effect;

    // This array stores the damage inflicted to each layer
    // It is currently hard-coded to three layers for simplicity:
    // shield, armor and hull. But this makes this implementation inflexible.
    // TODO: make this flexible.
    float inflicted_damage_by_layer[3] = {0, 0, 0};

    bool Spent() {
        return (normal_damage == 0 && phase_damage == 0 && propulsion_damage == 0 && blast_effect == 0);
    }

    Damage(float normal_damage = 0,
           float phase_damage = 0,
           float propulsion_damage = 0,
           float blast_effect = 0) :
        normal_damage(normal_damage),
        phase_damage(phase_damage),
        propulsion_damage(propulsion_damage),
        blast_effect(blast_effect),
        inflicted_normal_damage(0),
        inflicted_phase_damage(0),
        inflicted_propulsion_damage(0),
        inflicted_blast_effect(0) {}
};

#endif // DAMAGE_H
