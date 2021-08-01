#include "health.h"

#include <algorithm>
#include <iostream>



void Health::AdjustPower(const float& percent) {
    if(!regenerative) {
        // Not applicable for armor and hull
        return;
    }

    if(percent > 1 || percent < 0) {
        // valid values are between 0 and 1
        return;
    }

    adjusted_health = max_health * percent;

    if(adjusted_health < health) {
        health = adjusted_health;
    }
}

void Health::DealDamage( Damage &damage ) {
    // If this layer is destroyed, it can no longer sustain damage
    if(destroyed) {
        return;
    }

    std::cout << "pre-deal normal_damage " << damage.normal_damage <<
              " health " << health << std::endl;

    DealDamageComponent(damage.normal_damage, vulnerabilities.normal_damage);
    DealDamageComponent(damage.phase_damage, vulnerabilities.phase_damage);

    // TODO: implement other types of damage

    std::cout << "post-deal normal_damage " << damage.normal_damage <<
                 " health " << health << std::endl;

}


/**
 * @brief Health::DealDamageComponent - deal a component of damage (normal, phased) and not damage
 * a component.
 * @param health - to subtract from
 * @param damage - to inflict
 * @param vulnerability - adjust for
 */
void Health::DealDamageComponent( float &damage, float vulnerability ) {
    // Here we adjust for specialized weapons such as shield bypassing and shield leeching
    // which only damage the shield.
    // We also cap the actual damage at the current health
    float adjusted_damage = std::min(damage * vulnerability, health);

    // We check if there's any damage left to pass on to the next layer
    damage -= adjusted_damage;

    // Damage the current health
    health -= adjusted_damage;

    if(health == 0 && !regenerative) {
        destroyed = true;
    }
}

void Health::Disable() {
    if(regenerative && enabled) {
        enabled = false;
    }
}

void Health::Enable() {
    if(regenerative && !enabled) {
        enabled = true;
    }
}

void Health::ReduceLayerMaximum(const float& percent) {
    max_health = std::max(0.0f, max_health - factory_max_health * percent);
    health = std::min(health, max_health);
}

void Health::ReduceRegeneration(const float& percent) {
    regeneration = std::max(0.0f, regeneration - factory_regeneration * percent);
}

void Health::Regenerate() {
    if(!enabled || destroyed || !regenerative) {
        return;
    }

    health = std::min(max_health, health + regeneration);
}
