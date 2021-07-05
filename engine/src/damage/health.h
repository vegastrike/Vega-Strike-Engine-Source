#ifndef HEALTH_H
#define HEALTH_H

#include "damage.h"

/**
 * @brief The Health struct represents the health of something.
 * It can be a shield, armor, hull or subsystem.
 * @details
 * More specifically, it represents the health of DamageableFacet.
 * If the health of a non-regenerative facet is zero, it is destroyed.
 * If this facet is the sole facet of a DamageableLayer, it is destroyed.
 * If this DamageableLayer is mortal, the whole DamageableObject is destroyed.
 * Therefore, destroying the hull of a ship destoys it.
 * But, destroying the reactor or life-support of a ship also disabled it,
 * killing everyone inside,
 * potentially without actually destroying the ship.
 */
struct Health
{
public:
    const float max_health;
    float health;
    float regeneration;
    bool regenerative;
    bool destroyed;
    bool enabled;
    Damage vulnerabilities;

    /**
     * @brief The Effect enum specifies what happens when the health of a specific object is zero.
     */
    enum class Effect {
        none,       // Nothing happens
        isolated,   // The facet/layer/subsystem is destroyed
        disabling,  // The whole DamageableObject is disabled but not destroyed.
                    // It can then be tractored to another ship and repaired or sold.
        destroying  // The DamageableObject is destroyed, potentially leaving debris behind
    } effect;

    Health(float max_health = 1, float health = 1, float regeneration = 0, bool regenerative = false) :
        max_health(max_health), health(health), regeneration(regeneration), regenerative(regenerative) {
        destroyed = false;
    };

    float percent() {
        return health/max_health;
    }

    void DealDamage( Damage &damage );
    void Regenerate();
};

#endif // HEALTH_H
