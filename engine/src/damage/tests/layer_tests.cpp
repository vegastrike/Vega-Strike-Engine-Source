#include <gtest/gtest.h>

#include "damageable_layer.h"
#include "core_vector.h"

// Demonstrate some basic assertions.
TEST(Layer, Sanity) {
    Health health(10,10,0);
    DamageableLayer layer = DamageableLayer(0, FacetConfiguration::four, health, true);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(0,0,0)), 0);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1,0,1)), 0);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1,0,2)), 2);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(2,0,1)), 0);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1,0,1)), 2);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1,0,2)), 2);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-2,0,1)), 1);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1,0,-1)), 0);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1,0,-2)), 3);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(2,0,-1)), 0);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1,0,-1)), 3);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1,0,-2)), 3);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-2,0,-1)), 1);

}
