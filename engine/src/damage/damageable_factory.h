#ifndef DAMAGEABLEFACTORY_H
#define DAMAGEABLEFACTORY_H

#include <vector>

#include "damageable_object.h"
#include "damageable_layer.h"
#include "health.h"



class DamageableFactory
{
public:
    static DamageableObject CreateStandardObject(float shield,
                                                 float shield_regeneration,
                                                 FacetConfiguration shield_configuration,
                                                 float armor,
                                                 float hull);

    static DamageableLayer CreateLayer(int layer_index,
                                       FacetConfiguration configuration,
                                       Health health_template,
                                       bool core_layer);

    static DamageableLayer CreateLayer(int layer_index,
                                       FacetConfiguration configuration,
                                       float health_array[],
                                       float regeneration,
                                       bool core_layer);
};

#endif // DAMAGEABLEFACTORY_H
