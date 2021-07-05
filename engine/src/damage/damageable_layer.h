#ifndef DAMAGEABLELAYER_H
#define DAMAGEABLELAYER_H

#include <vector>

#include "damageable_facet.h"

/**
 * @brief The DamageableLayer class represents an object to be damaged.
 * This can be shields, armor, hull or subsystem.
 * A layer can be regenerative (e.g. shields) or not (everything else).
 */
class DamageableLayer
{
public:
    const bool core_layer;                  // Damage to the core layer has a chance of also
                                            // damaging internal components such as propulsion.

private:
    const unsigned int number_of_facets;    // How many facets. e.g. dual shield (front and rear).
    std::vector<DamageableFacet> facets;    // The facets container

    const float damage_component_chance = 0.03;
public:
    DamageableLayer();

    void DealDamage( const Vector &attack_vector, Damage &damage );
    void Disable();
    void Enable();
    const DamageableFacet& GetFacet(const Vector& attack_vector);
};

#endif // DAMAGEABLELAYER_H
