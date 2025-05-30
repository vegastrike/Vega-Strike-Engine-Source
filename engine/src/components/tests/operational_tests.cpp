#include <gtest/gtest.h>
#include <string>

#include "components/hull.h" 
#include "cmd/unit_csv_factory.h"

static const std::string upgrades_suffix_string = "__upgrades";
const std::string reactor_string = "reactor";


std::map<std::string,std::string> reactor_map = {
    { "Hull", "69/100"}
};

TEST(OperationalTests, DamageSystems) {
    UnitCSVFactory::LoadUnit(reactor_string + upgrades_suffix_string, reactor_map);

    Hull hull;

    hull.Load(reactor_string + upgrades_suffix_string);

    EXPECT_EQ(hull.PercentOperational(), 0.69);
}