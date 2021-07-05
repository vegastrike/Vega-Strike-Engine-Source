#ifndef FACETSFACTORY_H
#define FACETSFACTORY_H

#include <vector>
#include "damageable_facet.h"

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
    six     // A cube. Front, rear, left, right, top and bottom.
};

const std::vector<DamageableFacet> FacetsCreator(const FacetConfiguration& configuration,
                                           const DamageableFacet& template_facet);



#endif // FACETSFACTORY_H
