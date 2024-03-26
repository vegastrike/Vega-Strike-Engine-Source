#include <gtest/gtest.h>

#include "armor.h"
#include "damage/damageable_layer.h"
#include "damage/health.h"
#include "damage/facet_configuration.h"
#include "unit_csv_factory.h"

#include <string>
#include <map>

const std::string DUMMY_ARMOR_KEY = "dummy_armor";
const std::string DUMMY_UNIT_KEY = "dummy_unit";
const std::string NAME = "Name";
const std::string MASS = "Mass";


// Utilities

/*  Discussion: use of macros
*   Since we are using both DamageableLayer and Armor and one has 
    a pointer to the other, creating both in a function is somewhat 
    diffcult.
    One option is pointers.
    Another is macros.
    However, returning std::pair<Armor, DamageableLayer> is asking for 
    trouble, as there is potentially a copy operation and the original
    DL object will go out of scope.
*/
#define CREATE_ARMOR \
    Health health = Health(1,100.0,0.0); \
    DamageableLayer layer(1, FacetConfiguration::eight, health, false); \
    Armor armor = Armor(&layer); \

    
void loadArmor(Armor &armor, double health, double max_health) {
    // "Ship"
    std::map<std::string, std::string> dummy_unit_map;
    for(int i=0;i<8;i++) {
        dummy_unit_map[armor_facets[i]] = std::to_string(health);
    }
    UnitCSVFactory::LoadUnit(DUMMY_UNIT_KEY, dummy_unit_map);

    // "Armor"
    std::map<std::string, std::string> dummy_armor_map;
    dummy_armor_map[NAME] = DUMMY_ARMOR_KEY;
    dummy_armor_map[MASS] = 10.0;

    for(int i=0;i<8;i++) {
        dummy_armor_map[armor_facets[i]] = std::to_string(max_health);
    }
    UnitCSVFactory::LoadUnit(DUMMY_ARMOR_KEY, dummy_armor_map);

    armor.Load(DUMMY_ARMOR_KEY, DUMMY_UNIT_KEY);
}

void testArmorValues(DamageableLayer &layer, double health, double max_health) {
    for(int i=0;i<8;i++) {
        EXPECT_EQ(layer.facets[i].health.Value(), health);
        EXPECT_EQ(layer.facets[i].health.MaxValue(), max_health);
    }
}


// Tests
TEST(Armor, Load) {
    CREATE_ARMOR

    loadArmor(armor, 25, 50);

    testArmorValues(layer, 25, 50);
}

TEST(Armor, Save) {
    Health health = Health(1,100.0,0.0);
    DamageableLayer layer(1, FacetConfiguration::eight, health, false);
    Armor armor = Armor(&layer);

    std::map<std::string, std::string> csv;
    armor.SaveToCSV(csv);

    for(int i=0;i<8;i++) {
        EXPECT_EQ(std::stod(csv[armor_facets[i]]), 100);
    }
}

TEST(Armor, CanDowngrade) {
    CREATE_ARMOR
    loadArmor(armor, 25, 50);

    armor.SetIntegral(false);
    EXPECT_TRUE(armor.CanDowngrade());

    armor.SetIntegral(true);
    EXPECT_FALSE(armor.CanDowngrade());
}

TEST(Armor, Downgrade) {
    CREATE_ARMOR
    loadArmor(armor, 25, 50);

    EXPECT_TRUE(armor.Downgrade());
    
    testArmorValues(layer, 0, 0);
}

TEST(Armor, CanUpgrade) {
    CREATE_ARMOR
    loadArmor(armor, 25, 50);

    EXPECT_TRUE(armor.Downgrade());
    EXPECT_TRUE(armor.CanUpgrade(DUMMY_ARMOR_KEY));
}

TEST(Armor, Upgrade) {
    CREATE_ARMOR
    loadArmor(armor, 25, 50);

    EXPECT_TRUE(armor.Downgrade());
    EXPECT_TRUE(armor.Upgrade(DUMMY_ARMOR_KEY));
    testArmorValues(layer, 50, 50);
}

#undef CREATE_ARMOR