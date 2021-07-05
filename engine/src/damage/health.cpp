#include "health.h"

#include <algorithm>

void Health::DealDamage( Damage &damage ) {
    float modified_normal_damage = std::max(0.0f, damage.normal_damage * vulnerabilities.normal_damage);
    float modified_phase_damage = std::max(0.0f, damage.phase_damage * vulnerabilities.phase_damage);
    float modified_propulsion_damage = std::max(0.0f, damage.propulsion_damage * vulnerabilities.propulsion_damage);
    health -= modified_normal_damage;
    health -= modified_phase_damage;
    health -= modified_propulsion_damage;
    damage.normal_damage -= modified_normal_damage;
    damage.phase_damage -= modified_phase_damage;
    damage.propulsion_damage -= modified_propulsion_damage;
}


void Health::Regenerate() {
    if(!enabled || destroyed || !regenerative) {
        return;
    }

    health = std::min(max_health, health + regeneration);
}
