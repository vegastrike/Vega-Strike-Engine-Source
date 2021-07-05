#ifndef DAMAGEABLEFACET_H
#define DAMAGEABLEFACET_H

#include "health.h"

#include "gfx/vec.h"

/**
 * @brief The DamageableFacet class represents a rectangular plane that can be damaged.
 * In such a way, we can model a DamageableLayer, such as a front and rear shields.
 * When hit, we can then identify which facet of the layer is actually hit and apply the damage to it.
 */
class DamageableFacet
{
    const Vector minimum_vector;
    const Vector maximum_vector;
public:
    Health health;

    DamageableFacet(const Vector& min_v, const Vector& max_v, const Health& health_template) :
    minimum_vector(min_v),
    maximum_vector(max_v),
    health(health_template) {}

    DamageableFacet(const Vector& min_v, const Vector& max_v, const DamageableFacet& facet_template) :
        DamageableFacet(min_v, max_v, facet_template.health) {}

    void DealDamage( Damage &damage );

    bool InFacet(const Vector& vector) const;
};

#endif // DAMAGEABLEFACET_H
