#include "health.h"

#include <algorithm>
#include <iostream>

void Health::DealDamage( Damage &damage ) {
    // If this layer is destroyed, it can no longer sustain damage
    if(destroyed) {
        return;
    }

    DealDamageComponent(health, damage.normal_damage, vulnerabilities.normal_damage);

    // TODO: implement other types of damage


}


/**
 * @brief Health::DealDamageComponent - deal a component of damage (normal, phased) and not damage
 * a component.
 * @param health - to subtract from
 * @param damage - to inflict
 * @param vulnerability - adjust for
 */
void Health::DealDamageComponent( float &health, float &damage, float vulnerability ) {
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


void Health::Regenerate() {
    if(!enabled || destroyed || !regenerative) {
        return;
    }

    health = std::min(max_health, health + regeneration);
}
