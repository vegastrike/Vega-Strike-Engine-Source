#ifndef FACET_CONFIGURATION_H
#define FACET_CONFIGURATION_H

#include "core_vector.h"
#include <float.h>

/**
 * @brief The FacetConfiguration enum represents how many facets a layer has and it what configuration.
 * @details The use of enum class enforces the correctness of the parameter without checking.
 * You can't pass 17 for example.
 * I debated keeping the existing dual/quad but as it turns out, dual is Greek and Quad is latin.
 * It is also easier to understand five and six than penta and sexta (No! Hexa! Mixed again!).
 * This can also be extended to other facet configurations, such as pyramid
 * (left, right, top, bottom and rear).
 *
 * For consistency's sake, always start left to right, top to bottom and finally front to rear.
 * That is x,y,z. This is also how both core vector and Vector class are ordered.
 */
enum class FacetConfiguration {
    one,    // A single facet covering all directions
    two,    // Front and rear
    four,   // Front, rear, left and right
    six,    // A cube. Front, rear, left, right, top and bottom.
    eight   // Front and rear, left and right, top and bottom
};


enum class FacetName {
    single,             // 0

    left,               // 0
    right,              // 1
    top,                // 2
    bottom,             // 3
    front,              // 4
    rear,               // 5

    left_top_front,     // 0
    right_top_front,    // 1
    left_bottom_front,  // 2
    right_bottom_front, // 3
    left_top_rear,      // 4
    right_top_rear,     // 5
    left_bottom_rear,   // 6
    right_bottom_rear   // 7
};

const FacetName one_configuration[1] = { FacetName::single};

const FacetName two_configuration[2] = { FacetName::front,
                                         FacetName::rear};

const FacetName four_configuration[4] = { FacetName::left,
                                          FacetName::right,
                                          FacetName::front,
                                          FacetName::rear};

const FacetName six_configuration[6] = { FacetName::left,
                                         FacetName::right,
                                         FacetName::top,
                                         FacetName::bottom,
                                         FacetName::front,
                                         FacetName::rear};

const FacetName eight_configuration[8] = { FacetName::left_top_front,
                                           FacetName::right_top_front,
                                           FacetName::left_bottom_front,
                                           FacetName::right_bottom_front,
                                           FacetName::left_top_rear,
                                           FacetName::right_top_rear,
                                           FacetName::left_bottom_rear,
                                           FacetName::right_bottom_rear };

int Find(FacetName facet_name, const FacetName facet_array[], int size);

int FacetConfigurationSize(FacetConfiguration configuration);

const FacetName* FacetConfigurationByName(FacetConfiguration configuration);

#endif // FACET_CONFIGURATION_H
