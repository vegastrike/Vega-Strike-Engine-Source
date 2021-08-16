#ifndef DAMAGEABLEFACET_H
#define DAMAGEABLEFACET_H

#include "health.h"
#include "core_vector.h"
#include "facet_configuration.h"

#include <float.h>

struct DamageableLayer;

/**
 * @brief The DamageableFacet class represents a rectangular plane that can be damaged.
 * In such a way, we can model a DamageableLayer, such as a front and rear shields.
 * When hit, we can then identify which facet of the layer is actually hit and apply the damage to it.
 */
struct DamageableFacet
{
    // TODO: do we need this?
    //DamageableLayer *parent_layer;
    FacetConfiguration configuration;
    FacetName facet_name;

    Health health;

    DamageableFacet(FacetConfiguration configuration,
                    FacetName facet_name,
                    const Health& health_template) :
    configuration(configuration),
    facet_name(facet_name),
    health(health_template) {}


    void DealDamage( Damage &damage, InflictedDamage& inflicted_damage );
};

#endif // DAMAGEABLEFACET_H
