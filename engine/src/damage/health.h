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
    int layer; // The layer we're in, for recording damage
    float max_health;       // The absolute maximum, for a new, undamaged part
    float adjusted_health;  // The current max, for a potentially damaged part
                            // or max health for shields when there's not enough power
                            // for them
                            // or shields are declining in SPEC
    float health;
    float max_regeneration;  // The absolute maximum, for a new, undamaged part
    float regeneration; // The current capability of a potentially damaged part
    bool regenerative;
    bool destroyed;
    bool enabled;
    Damage vulnerabilities;
    // TODO: implement "shield leaks"


public:

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

    Health(int layer, float health = 1, float regeneration = 0) :
        Health(layer, health, health, regeneration) {}


    Health(int layer, float max_health, float health, float regeneration) :
        layer(layer),
        max_health(max_health),
        adjusted_health(max_health),
        health(health),
        max_regeneration(regeneration),
        regeneration(regeneration),
        regenerative(regeneration > 0 ) {
        destroyed = false;
        if(layer == 0) regenerative = false;
        enabled = regenerative; // Only relevant for regenerative objects (e.g. shields).
        vulnerabilities.normal_damage = 1;
        vulnerabilities.phase_damage = 1;
    };

    const float Percent() const {
        return max_health != 0 ? health/max_health : 0.0f;
    }

    void AdjustPower(const float& percent);
    void AdjustPercentage();
    void DealDamage( Damage &damage, InflictedDamage& inflicted_damage );
    void DealDamageComponent( int type, float &damage, float vulnerability, InflictedDamage& inflicted_damage );
    void Disable();
    void Destroy();
    void Enable();
    void Enhance(float percent = 1.5f);
    void ReduceLayerMaximum(const float& percent);
    void ReduceLayerMaximumByOne();
    void ReduceLayerMaximumByOnePercent();
    void ReduceRegeneration(const float& percent);
    void Regenerate();
    void Regenerate(float recharge_rate);
    void SetHealth(float health);
    void Update(float health);
};

#endif // HEALTH_H
