#ifndef DAMAGEABLEFACET_H
#define DAMAGEABLEFACET_H

#include "health.h"
#include "core_vector.h"

#include <float.h>

/**
 * @brief The DamageableFacet class represents a rectangular plane that can be damaged.
 * In such a way, we can model a DamageableLayer, such as a front and rear shields.
 * When hit, we can then identify which facet of the layer is actually hit and apply the damage to it.
 */
class DamageableFacet
{
    CoreVector minimum_vector;
    CoreVector maximum_vector;
public:
    Health health;

    DamageableFacet(const CoreVector& min_v, const CoreVector& max_v, const Health& health_template) :
    minimum_vector(min_v),
    maximum_vector(max_v),
    health(health_template) {}

    DamageableFacet(const CoreVector& min_v, const CoreVector& max_v, const DamageableFacet& facet_template) :
        DamageableFacet(min_v, max_v, facet_template.health) {}

    DamageableFacet():
        minimum_vector(-FLT_MAX, -FLT_MAX, -FLT_MAX),
        maximum_vector(FLT_MAX, FLT_MAX, FLT_MAX),
        health() {}


    void DealDamage( Damage &damage );

    bool InFacet(const CoreVector& vector) const;
};

#endif // DAMAGEABLEFACET_H
