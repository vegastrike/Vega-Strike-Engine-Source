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

Armor createArmor(double health) {
    Armor armor = Armor();

    std::vector<double> facets;
    for(int i=0;i<8;i++) {
        facets.push_back(health);
    }
    armor.UpdateFacets(facets);

    return armor;
}
    
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

void testArmorValues(Armor &armor, double health, double max_health) {
    for(int i=0;i<8;i++) {
        EXPECT_EQ(armor.facets[i].health.Value(), health);
        EXPECT_EQ(armor.facets[i].health.MaxValue(), max_health);
    }
}


// Tests
TEST(Armor, Load) {
    Armor armor = createArmor(1);

    loadArmor(armor, 25, 50);

    testArmorValues(armor, 25, 50);
}

TEST(Armor, Save) {
    Armor armor = createArmor(100);

    std::map<std::string, std::string> csv;
    armor.SaveToCSV(csv);

    for(int i=0;i<8;i++) {
        EXPECT_EQ(std::stod(csv[armor_facets[i]]), 100);
    }
}

TEST(Armor, CanDowngrade) {
    Armor armor = createArmor(1);
    loadArmor(armor, 25, 50);

    armor.SetIntegral(false);
    EXPECT_TRUE(armor.CanDowngrade());

    armor.SetIntegral(true);
    EXPECT_FALSE(armor.CanDowngrade());
}

TEST(Armor, Downgrade) {
    Armor armor = createArmor(1);
    loadArmor(armor, 25, 50);

    EXPECT_TRUE(armor.Downgrade());
    
    testArmorValues(armor, 0, 0);
}

TEST(Armor, CanUpgrade) {
    Armor armor = createArmor(1);
    loadArmor(armor, 25, 50);

    EXPECT_TRUE(armor.Downgrade());
    EXPECT_TRUE(armor.CanUpgrade(DUMMY_ARMOR_KEY));
}

TEST(Armor, Upgrade) {
    Armor armor = createArmor(1);
    loadArmor(armor, 25, 50);

    EXPECT_TRUE(armor.Downgrade());
    EXPECT_TRUE(armor.Upgrade(DUMMY_ARMOR_KEY));
    testArmorValues(armor, 50, 50);
}

