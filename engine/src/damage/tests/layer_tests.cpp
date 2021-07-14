#include <gtest/gtest.h>

#include "damageable_layer.h"
#include "damageable_factory.h"
#include "core_vector.h"

// Demonstrate some basic assertions.
TEST(Layer, Sanity) {
    CoreVector zero(0.0,0.0,0.0);
    CoreVector minus_one(-1,-1,-1);
    CoreVector one = CoreVector(1.0f,1.0f,1.0f);

    Health health(10,10,0);
    DamageableLayer layer = DamageableFactory::CreateLayer(FacetConfiguration::four, health, true);

//    EXPECT_TRUE(facet.InFacet(CoreVector()));
//    EXPECT_TRUE(facet.InFacet(CoreVector(1,1,1)));
//    EXPECT_FALSE(facet.InFacet(CoreVector(1,1,2)));
//    EXPECT_TRUE(facet.InFacet(CoreVector(-1,1,0)));
//    EXPECT_FALSE(facet.InFacet(CoreVector(1,2,0)));
}
