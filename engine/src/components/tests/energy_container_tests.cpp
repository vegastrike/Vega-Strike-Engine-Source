/*
 * energy_container_tests.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <map>
// #include <math.h>
// #include <utility>

#include "energy_container.h"
#include "energy_consumer.h"
#include "unit_csv_factory.h"

void printContainer(EnergyContainer& container) {
    std::cout << "Max Level: " << container.MaxLevel();
    std::cout << " Level: " << container.Level();
    std::cout << " Percent: " << container.Percent() << std::endl;
}

TEST(EnergyContainer, Sanity) {
    EnergyContainer container = EnergyContainer(ComponentType::Capacitor);
    EXPECT_EQ(container.Level(), 0.0);
    container.SetCapacity(10.0, true);

    printContainer(container);

    EnergyConsumer consumer = EnergyConsumer(&container, 
                                             false,
                                             1.0);
    

    int i=0;
    while(!container.Depleted() && i < 100) {
        std::cout << " Can consume: " << consumer.CanConsume() << std::endl;
        consumer.Consume();
        printContainer(container);
        i++;
    }

    // Uncomment to see prints
    //EXPECT_TRUE(consumer.CanConsume());
}

const std::string ship_name = "DummyShip";
const std::string capacitor_name = "DummyCapacitor";

static const std::string upgrades_suffix_string = "__upgrades";
static const std::string capacitor_string = "capacitor";
static const std::string upgrade_string = "upgrade";

static const std::map<std::string,std::string> capacitor_map = {
    { "Name", capacitor_name},
    { "Mass", "5.0"},
    { "Primary_Capacitor", "10.0" },
};


TEST(EnergyContainer, Upgrade) {
    UnitCSVFactory::LoadUnit(ship_name, capacitor_map);
    UnitCSVFactory::LoadUnit(capacitor_string + upgrades_suffix_string, capacitor_map);

    EnergyContainer ship_capacitor = EnergyContainer(ComponentType::Capacitor);

    ship_capacitor.Load(capacitor_string + upgrades_suffix_string, ship_name);

    // Check Values
    EXPECT_EQ(ship_capacitor.GetUpgradeName(), capacitor_name);  
    EXPECT_EQ(ship_capacitor.GetMass(), 5.0);
    
    EXPECT_EQ(ship_capacitor.Level(), 10.0);

    // Downgrade
    ship_capacitor.Downgrade();

    EXPECT_EQ(ship_capacitor.GetUpgradeName(), "");  
    EXPECT_EQ(ship_capacitor.GetMass(), 0.0);
    
    EXPECT_EQ(ship_capacitor.Level(), 0.0);

    ship_capacitor.Upgrade(capacitor_string + upgrades_suffix_string);

    EXPECT_EQ(ship_capacitor.GetUpgradeName(), capacitor_name);  
    EXPECT_EQ(ship_capacitor.GetMass(), 5.0);
    
    EXPECT_EQ(ship_capacitor.Level(), 10.0);
}
