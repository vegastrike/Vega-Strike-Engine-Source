/*
 * cargo_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "resource/cargo.h"
#include "resource/manifest.h"

TEST(Cargo, Sanity) {
    std::string cargo_text_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true";
    Cargo c(cargo_text_string);
}

std::vector<Cargo> init(const std::string& cargo_string) {
    std::vector<std::string> cargo_text_elements;

    std::vector<Cargo> ship_manifest;

    boost::split(cargo_text_elements,cargo_string,boost::is_any_of("|"));

    for(std::string& cargo_text_element : cargo_text_elements) {
        std::cout << cargo_text_element << std::endl;
        std::cout << cargo_text_element.size() << std::endl;
        try {
            Cargo c(cargo_text_element);
            ship_manifest.push_back(c);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing cargo: " << e.what() << std::endl;
        }
    }

    return ship_manifest;
}

static Cargo& get_cargo(const std::string& cargo_name, std::vector<Cargo>& ship_manifest) {
    for(Cargo& c : ship_manifest) {
        if(c.GetName() == cargo_name) {
            return c;
        }
    }

    throw std::runtime_error("Cargo not found: " + cargo_name);
}

void print(std::vector<Cargo> ship_manifest) {
    for (const auto& cargo : ship_manifest) {
        std::cout << "Name: " << cargo.GetName()
                  << ", Category: " << cargo.GetCategory()
                  << ", Quantity: " << cargo.GetQuantity()
                  << ", Passenger: " << cargo.IsPassenger()
                  << ", Slave: " << cargo.IsSlave()
                  << std::endl;
    }
}


TEST(Cargo, Enslave_existing_slaves) {
    std::string cargo_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true|capacitor08;upgrades/Capacitors/Standard;60342.941406;1;16.000000;16.000000;1.000000;1.000000;;false;true|jump_drive;upgrades/Jump_Drives;7542.944336;1;16.000000;2.000000;1.000000;1.000000;;false;true|mult_overdrive06;upgrades/Overdrive;120685.796875;1;18.000000;18.000000;1.000000;1.000000;;false;true|reactor08;upgrades/Reactors/Standard;60342.917969;1;8.000000;100.000000;1.000000;1.000000;;false;true|add_spec_capacitor02;upgrades/SPEC_Capacitors;16971.439453;1;10.000000;6.000000;1.000000;1.000000;;false;true|hawkeye4;upgrades/Sensors/Confed;150857.171875;1;0.010000;21.000000;1.000000;1.000000;;false;true|quadshield10;upgrades/Shield_Systems/Standard_Quad_Shields;241371.500000;1;10.000000;61.000000;1.000000;1.000000;;false;true|afterburner;upgrades/integral;2000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|drive;upgrades/integral;6000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|ftl_drive;upgrades/integral;4500.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|hull;upgrades/integral;12000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|Slaves;Contraband;100;17;32;23;1;1;slaves to sell;false;false;false;false;false;true;true|Hitchhiker;Passengers;50;27;12;17;1;1;some people;false;false;false;false;false;true;false|passengers;Passengers;500;7;10;27;1;1;some more people;false;false;false;false;false;true;false";
    std::vector<Cargo> ship_manifest = init(cargo_string);

    print(ship_manifest);
    Enslave(ship_manifest);
    print(ship_manifest);

    Cargo slaves = get_cargo("Slaves", ship_manifest);
    ASSERT_EQ(slaves.GetQuantity(), 51);

    try {
        Cargo& hitchhiker = get_cargo("Hitchhiker", ship_manifest);
        FAIL() << "Exception not thrown. Hitchhiker cargo found";
    } catch (const std::exception& e) {}

    try {
        Cargo& hitchhiker = get_cargo("passengers", ship_manifest);
        FAIL() << "Exception not thrown. passengers cargo found";
    } catch (const std::exception& e) {}
}

TEST(Cargo, Enslave_no_existing_slaves) {
    std::string cargo_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true|capacitor08;upgrades/Capacitors/Standard;60342.941406;1;16.000000;16.000000;1.000000;1.000000;;false;true|jump_drive;upgrades/Jump_Drives;7542.944336;1;16.000000;2.000000;1.000000;1.000000;;false;true|mult_overdrive06;upgrades/Overdrive;120685.796875;1;18.000000;18.000000;1.000000;1.000000;;false;true|reactor08;upgrades/Reactors/Standard;60342.917969;1;8.000000;100.000000;1.000000;1.000000;;false;true|add_spec_capacitor02;upgrades/SPEC_Capacitors;16971.439453;1;10.000000;6.000000;1.000000;1.000000;;false;true|hawkeye4;upgrades/Sensors/Confed;150857.171875;1;0.010000;21.000000;1.000000;1.000000;;false;true|quadshield10;upgrades/Shield_Systems/Standard_Quad_Shields;241371.500000;1;10.000000;61.000000;1.000000;1.000000;;false;true|afterburner;upgrades/integral;2000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|drive;upgrades/integral;6000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|ftl_drive;upgrades/integral;4500.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|hull;upgrades/integral;12000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|Hitchhiker;Passengers;50;27;12;17;1;1;some people;false;false;false;false;false;true;false|passengers;Passengers;500;7;10;27;1;1;some more people;false;false;false;false;false;true;false";
    std::vector<Cargo> ship_manifest = init(cargo_string);

    std::string mpl_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true|capacitor08;upgrades/Capacitors/Standard;60342.941406;1;16.000000;16.000000;1.000000;1.000000;;false;true|jump_drive;upgrades/Jump_Drives;7542.944336;1;16.000000;2.000000;1.000000;1.000000;;false;true|mult_overdrive06;upgrades/Overdrive;120685.796875;1;18.000000;18.000000;1.000000;1.000000;;false;true|reactor08;upgrades/Reactors/Standard;60342.917969;1;8.000000;100.000000;1.000000;1.000000;;false;true|add_spec_capacitor02;upgrades/SPEC_Capacitors;16971.439453;1;10.000000;6.000000;1.000000;1.000000;;false;true|hawkeye4;upgrades/Sensors/Confed;150857.171875;1;0.010000;21.000000;1.000000;1.000000;;false;true|quadshield10;upgrades/Shield_Systems/Standard_Quad_Shields;241371.500000;1;10.000000;61.000000;1.000000;1.000000;;false;true|afterburner;upgrades/integral;2000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|drive;upgrades/integral;6000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|ftl_drive;upgrades/integral;4500.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|hull;upgrades/integral;12000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|Slaves;Contraband;100;17;32;23;1;1;slaves to sell;false;false;false;false;false;true;true|Hitchhiker;Passengers;50;27;12;17;1;1;some people;false;false;false;false;false;true;false|passengers;Passengers;500;7;10;27;1;1;some more people;false;false;false;false;false;true;false";
    std::vector<Cargo> mpl_manifest = init(mpl_string);
    
    // We need this because we get the Slaves cargo item from MPL.
    Manifest::MPL().AddManifest(mpl_manifest);

    print(ship_manifest);
    Enslave(ship_manifest);
    print(ship_manifest);

    Cargo slaves = get_cargo("Slaves", ship_manifest);
    ASSERT_EQ(slaves.GetQuantity(), 34);

    try {
        Cargo& hitchhiker = get_cargo("Hitchhiker", ship_manifest);
        FAIL() << "Exception not thrown. Hitchhiker cargo found";
    } catch (const std::exception& e) {}

    try {
        Cargo& hitchhiker = get_cargo("passengers", ship_manifest);
        FAIL() << "Exception not thrown. passengers cargo found";
    } catch (const std::exception& e) {}
}

TEST(Cargo, Free_existing_hitchhikers) {
    std::string cargo_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true|capacitor08;upgrades/Capacitors/Standard;60342.941406;1;16.000000;16.000000;1.000000;1.000000;;false;true|jump_drive;upgrades/Jump_Drives;7542.944336;1;16.000000;2.000000;1.000000;1.000000;;false;true|mult_overdrive06;upgrades/Overdrive;120685.796875;1;18.000000;18.000000;1.000000;1.000000;;false;true|reactor08;upgrades/Reactors/Standard;60342.917969;1;8.000000;100.000000;1.000000;1.000000;;false;true|add_spec_capacitor02;upgrades/SPEC_Capacitors;16971.439453;1;10.000000;6.000000;1.000000;1.000000;;false;true|hawkeye4;upgrades/Sensors/Confed;150857.171875;1;0.010000;21.000000;1.000000;1.000000;;false;true|quadshield10;upgrades/Shield_Systems/Standard_Quad_Shields;241371.500000;1;10.000000;61.000000;1.000000;1.000000;;false;true|afterburner;upgrades/integral;2000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|drive;upgrades/integral;6000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|ftl_drive;upgrades/integral;4500.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|hull;upgrades/integral;12000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|Slaves;Contraband;100;17;32;23;1;1;slaves to sell;false;false;false;false;false;true;true|Hitchhiker;Passengers;50;27;12;17;1;1;some people;false;false;false;false;false;true;false|passengers;Passengers;500;7;10;27;1;1;some more people;false;false;false;false;false;true;false";
    std::vector<Cargo> ship_manifest = init(cargo_string);


    print(ship_manifest);
    Free(ship_manifest);
    print(ship_manifest);

    try {
        Cargo& hitchhiker = get_cargo("Slaves", ship_manifest);
        FAIL() << "Exception not thrown. Slaves cargo found";
    } catch (const std::exception& e) {}

    Cargo hitchhikers = get_cargo("Hitchhiker", ship_manifest);
    ASSERT_EQ(hitchhikers.GetQuantity(), 44);

    Cargo passengers = get_cargo("passengers", ship_manifest);
    ASSERT_EQ(passengers.GetQuantity(), 7);
}

TEST(Cargo, Free_no_existing_hitchhikers) {
    std::string cargo_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true|capacitor08;upgrades/Capacitors/Standard;60342.941406;1;16.000000;16.000000;1.000000;1.000000;;false;true|jump_drive;upgrades/Jump_Drives;7542.944336;1;16.000000;2.000000;1.000000;1.000000;;false;true|mult_overdrive06;upgrades/Overdrive;120685.796875;1;18.000000;18.000000;1.000000;1.000000;;false;true|reactor08;upgrades/Reactors/Standard;60342.917969;1;8.000000;100.000000;1.000000;1.000000;;false;true|add_spec_capacitor02;upgrades/SPEC_Capacitors;16971.439453;1;10.000000;6.000000;1.000000;1.000000;;false;true|hawkeye4;upgrades/Sensors/Confed;150857.171875;1;0.010000;21.000000;1.000000;1.000000;;false;true|quadshield10;upgrades/Shield_Systems/Standard_Quad_Shields;241371.500000;1;10.000000;61.000000;1.000000;1.000000;;false;true|afterburner;upgrades/integral;2000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|drive;upgrades/integral;6000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|ftl_drive;upgrades/integral;4500.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|hull;upgrades/integral;12000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|Slaves;Contraband;100;17;32;23;1;1;slaves to sell;false;false;false;false;false;true;true|passengers;Passengers;500;7;10;27;1;1;some more people;false;false;false;false;false;true;false";
    std::vector<Cargo> ship_manifest = init(cargo_string);


    print(ship_manifest);
    Free(ship_manifest);
    print(ship_manifest);

    try {
        Cargo& hitchhiker = get_cargo("Slaves", ship_manifest);
        FAIL() << "Exception not thrown. Slaves cargo found";
    } catch (const std::exception& e) {}

    Cargo hitchhikers = get_cargo("Hitchhiker", ship_manifest);
    ASSERT_EQ(hitchhikers.GetQuantity(), 17);

    Cargo passengers = get_cargo("passengers", ship_manifest);
    ASSERT_EQ(passengers.GetQuantity(), 7);
}

TEST(Cargo, ConstructorAndGetters) {
    std::string cargo_str = "test_item;category;100.0;5;10.0;2.0;1.0;1.0;desc;false;true";
    Cargo c(cargo_str);

    EXPECT_EQ(c.GetName(), "test_item");
    EXPECT_EQ(c.GetDescription(), "desc");
    EXPECT_EQ(c.GetQuantity(), 5);
    EXPECT_DOUBLE_EQ(c.GetPrice(), 100.0);
    EXPECT_EQ(c.GetCategory(), "category");
    EXPECT_DOUBLE_EQ(c.GetMass(), 10.0);
    EXPECT_DOUBLE_EQ(c.GetVolume(), 2.0);
    EXPECT_FALSE(c.IsMissionFlag());
    EXPECT_TRUE(c.IsComponent());
    EXPECT_TRUE(c.IsInstalled());
    EXPECT_FALSE(c.IsIntegral());
    EXPECT_FALSE(c.IsWeapon());
    EXPECT_FALSE(c.IsPassenger());
    EXPECT_FALSE(c.IsSlave());
}

TEST(Cargo, Setters) {
    std::string cargo_str = "item;cat;10.0;1;1.0;1.0;1.0;1.0;;false;false";
    Cargo c(cargo_str);

    c.SetQuantity(42);
    EXPECT_EQ(c.GetQuantity(), 42);

    c.SetPrice(123.45);
    EXPECT_DOUBLE_EQ(c.GetPrice(), 123.45);

    c.SetDescription("newdesc");
    EXPECT_EQ(c.GetDescription(), "newdesc");
}

TEST(Cargo, EqualityOperators) {
    std::string cargo_str1 = "item;cat;10.0;1;1.0;1.0;1.0;1.0;;false;false";
    std::string cargo_str2 = "item;cat;10.0;1;1.0;1.0;1.0;1.0;;false;false";
    std::string cargo_str3 = "other;cat;10.0;1;1.0;1.0;1.0;1.0;;false;false";
    Cargo c1(cargo_str1);
    Cargo c2(cargo_str2);
    Cargo c3(cargo_str3);

    EXPECT_TRUE(c1 == c2);
    EXPECT_FALSE(c1 == c3);
}

TEST(Cargo, Serialization) {
    std::string cargo_str = "item;cat;10.0;1;1.0;1.0;1.0;1.0;desc;false;false";
    Cargo c(cargo_str);

    std::string serialized = c.Serialize();
    std::cout << "Serialized Cargo: " << serialized << std::endl;
    serialized = serialized.substr(1, serialized.size() - 2);
    Cargo c2(serialized);

    EXPECT_EQ(c2.GetName(), c.GetName());
    EXPECT_EQ(c2.GetCategory(), c.GetCategory());
    EXPECT_DOUBLE_EQ(c2.GetPrice(), c.GetPrice());
    EXPECT_EQ(c2.GetQuantity(), c.GetQuantity());
    EXPECT_DOUBLE_EQ(c2.GetMass(), c.GetMass());
    EXPECT_DOUBLE_EQ(c2.GetVolume(), c.GetVolume());
    EXPECT_EQ(c2.GetDescription(), c.GetDescription());
}

TEST(Cargo, IsPassengerAndIsSlave) {
    std::string passenger_str = "passenger;Passengers;100;1;1;1;1;1;A passenger;0;0;0;0;0;1;0";
    std::string slave_str = "slave;Contraband;100;1;1;1;1;1;A slave;0;0;0;0;0;1;1";
    Cargo passenger(passenger_str);
    Cargo slave(slave_str);

    EXPECT_TRUE(passenger.IsPassenger());
    EXPECT_FALSE(passenger.IsSlave());
    EXPECT_TRUE(slave.IsPassenger());
    EXPECT_TRUE(slave.IsSlave());
}

TEST(Cargo, AddAndRemoveQuantity) {
    std::string cargo_str = "item;cat;10.0;5;1.0;1.0;1.0;1.0;;false;false";
    Cargo c(cargo_str);

    c.Add(3);
    EXPECT_EQ(c.GetQuantity(), 8);

    // c.RemoveQuantity(2);
    // EXPECT_EQ(c.GetQuantity(), 6);

    // c.RemoveQuantity(10); // Should not go negative
    // EXPECT_GE(c.GetQuantity(), 0);
}


TEST(Cargo, CopyAndAssignment) {
    std::string cargo_str = "item;cat;10.0;5;1.0;1.0;1.0;1.0;;false;false";
    Cargo c1(cargo_str);
    Cargo c2 = c1;
    Cargo c3(c1);
    c3 = c1;

    EXPECT_EQ(c2.GetName(), c1.GetName());
    EXPECT_EQ(c3.GetName(), c1.GetName());
}

TEST(Cargo, Functionality) {
    std::string cargo_str = "item;cat;10.0;5;1.0;1.0;1.0;1.0;;false;false";
    Cargo c(cargo_str);

    EXPECT_DOUBLE_EQ(c.GetFunctionality(), 1.0);
    EXPECT_DOUBLE_EQ(c.RepairPrice(), 0.0);
    double original_price = c.GetPrice();
    EXPECT_FALSE(c.Damaged());
    c.SetFunctionality(0.8);
    EXPECT_DOUBLE_EQ(c.GetFunctionality(), 0.8);
    EXPECT_TRUE(c.Damaged());
    EXPECT_GT(c.RepairPrice(), 0.0);

    double orig_functionality = c.GetFunctionality();
    c.RandomDamage();
    // After random damage, damage should be >= original and <= 1.0
    EXPECT_LE(c.GetFunctionality(), orig_functionality);
    EXPECT_LE(c.GetPrice(), original_price);
}