#include <gtest/gtest.h>

#include "resource.h"

TEST(Resource, Sanity) {
    Resource<float> resource = Resource<float>(10.0f, 0.0f);
    EXPECT_EQ(resource.Value(), 10.0f);

    resource.Set(5.0f);
    EXPECT_EQ(resource.Value(), 5.0f);

    resource += 5.0f;
    EXPECT_EQ(resource.Value(), 10.0f);

    resource -= 2.0f;
    EXPECT_EQ(resource.Value(), 8.0f);

    resource -= 12.0f;
    EXPECT_EQ(resource.Value(), 0.0f);
}
