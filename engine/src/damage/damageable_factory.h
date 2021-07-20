#ifndef DAMAGEABLEFACTORY_H
#define DAMAGEABLEFACTORY_H

#include <vector>

#include "damageable_object.h"
#include "damageable_layer.h"
#include "damageable_facet.h"
#include "health.h"

/**
 * @brief The FacetConfiguration enum represents how many facets a layer has and it what configuration.
 * @details The use of enum class enforces the correctness of the parameter without checking.
 * You can't pass 17 for example.
 * I debated keeping the existing dual/quad but as it turns out, dual is Greek and Quad is latin.
 * It is also easier to understand five and six than penta and sexta (No! Hexa! Mixed again!).
 * This can also be extended to other facet configurations, such as pyramid
 * (left, right, top, bottom and rear).
 */
enum class FacetConfiguration {
    one,    // A single facet covering all directions
    two,    // Front and rear
    four,   // Front, rear, left and right
    six,    // A cube. Front, rear, left, right, top and bottom.
    eight   // Front and rear, left and right, top and bottom
};


class DamageableFactory
{
public:
    static DamageableObject CreateStandardObject(float shield,
                                                 float shield_regeneration,
                                                 FacetConfiguration shield_configuration,
                                                 float armor,
                                                 float hull);

    static DamageableLayer CreateLayer(const FacetConfiguration configuration,
                                Health health_template,
                                bool core_layer);
};

#endif // DAMAGEABLEFACTORY_H
