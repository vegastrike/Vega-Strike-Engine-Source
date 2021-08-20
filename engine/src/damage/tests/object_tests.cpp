#include <gtest/gtest.h>

#include "damageable_factory.h"
#include "damageable_object.h"

// Demonstrate some basic assertions.
TEST(DamageableObject, Sanity) {
    CoreVector core_vector(1,1,1); // Left top front

    DamageableObject object;

    Health hull_health(0, 50, 0.0f);
    Health armor_health(1, 100, 0.0f);
    Health shield_health(2, 150, 10.0f);

    DamageableLayer hull = DamageableFactory::CreateLayer(0, FacetConfiguration::one, hull_health, true);
    DamageableLayer armor = DamageableFactory::CreateLayer(1, FacetConfiguration::eight, armor_health, false);
    DamageableLayer shield = DamageableFactory::CreateLayer(2, FacetConfiguration::four, shield_health, false);

    EXPECT_EQ(hull.layer_index, 0);
    EXPECT_EQ(armor.layer_index, 1);
    EXPECT_EQ(shield.layer_index, 2);

    object.layers[0] = hull;
    object.layers[1] = armor;
    object.layers[2] = shield;

    Damage damage;
    damage.normal_damage = 10;

    InflictedDamage inflicated_damage = object.DealDamage(core_vector, damage);
    EXPECT_EQ(object.layers[0].facets[0].health, 50);
    EXPECT_EQ(object.layers[1].facets[0].health, 100);
    EXPECT_EQ(object.layers[2].facets[0].health, 140);
    EXPECT_EQ(damage.normal_damage, 0);
    EXPECT_EQ(inflicated_damage.total_damage, 10);
    EXPECT_EQ(inflicated_damage.normal_damage, 10);
    EXPECT_EQ(inflicated_damage.phase_damage, 0);
    EXPECT_EQ(inflicated_damage.inflicted_damage_by_layer[0], 0);
    EXPECT_EQ(inflicated_damage.inflicted_damage_by_layer[1], 0);
    EXPECT_EQ(inflicated_damage.inflicted_damage_by_layer[2], 10);
}
