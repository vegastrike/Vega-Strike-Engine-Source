#include <gtest/gtest.h>

#include "resource.h"


// It's surprising, but gtest doesn't provide a good way to test equal.
bool fairlyEqual(double a, double b) {
    return a-b < 0.001 && b-a < 0.001;
}


TEST(Resource, Sanity) {
    Resource<float> resource = Resource<float>(10.0f, 0.0f, 10.0f);
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 1.0));

    resource.Set(5.0f);
    EXPECT_EQ(resource.Value(), 5.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.5));

    resource += 5.0f;
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 1.0));

    resource -= 2.0f;
    EXPECT_EQ(resource.Value(), 8.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.8));

    resource -= 12.0f;
    EXPECT_EQ(resource.Value(), 0.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.0));
}

TEST(Resource, Limitless) {
    Resource<float> resource = Resource<float>(10.0f, 0.0f);
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource.Set(5.0f);
    EXPECT_EQ(resource.Value(), 5.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource += 5.0f;
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource -= 2.0f;
    EXPECT_EQ(resource.Value(), 8.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource -= 12.0f;
    EXPECT_EQ(resource.Value(), 0.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource += 35.0f;
    EXPECT_EQ(resource.Value(), 35.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));
}

TEST(Resource, Damage) {
    Resource<float> resource = Resource<float>(10.0f, 0.0f, 10.0);
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 1.0));

    resource.DamageByValue(3.0f);
    EXPECT_EQ(resource.Value(), 7.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 7.0f);
    EXPECT_TRUE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.7));

    resource += 12.0f;
    EXPECT_EQ(resource.Value(), 7.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 7.0f);
    EXPECT_TRUE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.7));

    resource -= 2.0f;
    EXPECT_EQ(resource.Value(), 5.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 7.0f);
    EXPECT_TRUE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.5));


    resource.RepairByValue(7.0f);
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 1.0));

    resource.DamageByValue(3.0f);
    EXPECT_EQ(resource.Value(), 7.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 7.0f);
    EXPECT_TRUE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 0.7));

    resource.RepairFully();
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), 10.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), 10.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), 1.0));
}

TEST(Resource, Limitless_Damage) {
    Resource<float> resource = Resource<float>(10.0f, 0.0f);
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource.DamageByValue(3.0f);
    EXPECT_EQ(resource.Value(), 10.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource += 12.0f;
    EXPECT_EQ(resource.Value(), 22.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));

    resource -= 2.0f;
    EXPECT_EQ(resource.Value(), 20.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));


    resource.RepairByValue(7.0f);
    EXPECT_EQ(resource.Value(), 20.0f);
    EXPECT_EQ(resource.MaxValue(), -1.0f);
    EXPECT_EQ(resource.MinValue(), 0.0f);
    EXPECT_EQ(resource.AdjustedValue(), -1.0f);
    EXPECT_FALSE(resource.Damaged());
    EXPECT_TRUE(fairlyEqual(resource.Percent(), -1.0));
}
