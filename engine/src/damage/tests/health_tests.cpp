#include <gtest/gtest.h>

#include "health.h"


// Demonstrate some basic assertions.
TEST(Shield, Sanity) {
    Damage damage;
    InflictedDamage inflicted_damage(3);
    damage.normal_damage = 10;
    EXPECT_EQ(damage.normal_damage, 10);

    Health health(100,100,25);
    health.vulnerabilities.normal_damage = 1;

    EXPECT_EQ(health.health, 100);
    EXPECT_EQ(health.factory_max_health, 100);
    EXPECT_EQ(health.regeneration, 25);
    EXPECT_TRUE(health.regenerative);
    EXPECT_FALSE(health.destroyed);
    EXPECT_TRUE(health.enabled);

    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 90);
    EXPECT_EQ(damage.normal_damage, 0);

    health.Disable();
    health.Regenerate();
    EXPECT_EQ(health.health, 90);

    health.Enable();
    health.Regenerate();
    EXPECT_EQ(health.health, 100);

    damage.normal_damage = 110;
    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 0);
    EXPECT_TRUE(health.regenerative);
    EXPECT_FALSE(health.destroyed);
    EXPECT_TRUE(health.enabled);
    EXPECT_EQ(damage.normal_damage, 10);
}


TEST(Armor, Sanity) {
    Damage damage;
    InflictedDamage inflicted_damage(3);
    damage.normal_damage = 10;
    EXPECT_EQ(damage.normal_damage, 10);

    Health health(100,100,0);
    health.vulnerabilities.normal_damage = 1;

    EXPECT_EQ(health.health, 100);
    EXPECT_EQ(health.max_health, 100);
    EXPECT_EQ(health.regeneration, 0);
    EXPECT_FALSE(health.regenerative);
    EXPECT_FALSE(health.destroyed);
    EXPECT_FALSE(health.enabled);

    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 90);
    EXPECT_EQ(damage.normal_damage, 0);

    health.Disable();
    health.Regenerate();
    EXPECT_EQ(health.health, 90);
    EXPECT_FALSE(health.enabled);

    health.Enable();
    health.Regenerate();
    EXPECT_EQ(health.health, 90);
    EXPECT_FALSE(health.enabled);

    damage.normal_damage = 110;
    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.   health, 0);
    EXPECT_FALSE(health.regenerative);
    EXPECT_TRUE(health.destroyed);
    EXPECT_FALSE(health.enabled);
    EXPECT_EQ(damage.normal_damage, 20);
}
