/*
 * manifest_tests.cpp
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
#include <iostream>

#include "resource/manifest.h"

#include <cassert>
#include <iostream>

extern std::vector<Cargo> init(const std::string& cargo_string);

Manifest createManifest() {
    std::string cargo_string = "armor06;upgrades/Armor;120685.710938;1;15.000000;0.000000;1.000000;1.000000;;false;true|capacitor08;upgrades/Capacitors/Standard;60342.941406;1;16.000000;16.000000;1.000000;1.000000;;false;true|jump_drive;upgrades/Jump_Drives;7542.944336;1;16.000000;2.000000;1.000000;1.000000;;false;true|mult_overdrive06;upgrades/Overdrive;120685.796875;1;18.000000;18.000000;1.000000;1.000000;;false;true|reactor08;upgrades/Reactors/Standard;60342.917969;1;8.000000;100.000000;1.000000;1.000000;;false;true|add_spec_capacitor02;upgrades/SPEC_Capacitors;16971.439453;1;10.000000;6.000000;1.000000;1.000000;;false;true|hawkeye4;upgrades/Sensors/Confed;150857.171875;1;0.010000;21.000000;1.000000;1.000000;;false;true|quadshield10;upgrades/Shield_Systems/Standard_Quad_Shields;241371.500000;1;10.000000;61.000000;1.000000;1.000000;;false;true|afterburner;upgrades/integral;2000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|drive;upgrades/integral;6000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|ftl_drive;upgrades/integral;4500.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|hull;upgrades/integral;12000.000000;1;0.100000;0.100000;1.000000;1.000000;;false;true|Slaves;Contraband;100;17;32;23;1;1;slaves to sell;false;false;false;false;false;true;true|Hitchhiker;Passengers;50;27;12;17;1;1;some people;false;false;false;false;false;true;false|passengers;Passengers;500;7;10;27;1;1;some more people;false;false;false;false;false;true;false";
    std::vector<Cargo> ship_manifest = init(cargo_string);

    Manifest m;
    m.AddManifest(ship_manifest);
    return m;
}

TEST(Manifest, GetItems) {
    Manifest manifest = createManifest();

    std::vector<Cargo> items = manifest.GetItems();
    ASSERT_EQ(items.size(), manifest.Size());
}

TEST(Manifest, GetCargoByName) {
    Manifest manifest = createManifest();

    Cargo c = manifest.GetCargoByName("armor06");
    ASSERT_EQ(c.GetName(), "armor06");
    ASSERT_EQ(c.GetCategory(), "upgrades/Armor");
}

TEST(Manifest, EmptyAndSize) {
    Manifest manifest;
    ASSERT_TRUE(manifest.Empty());
    ASSERT_EQ(manifest.Size(), 0);

    manifest = createManifest();
    ASSERT_FALSE(manifest.Empty());
    ASSERT_GT(manifest.Size(), 0);
}

TEST(Manifest, GetIndex) {
    Manifest manifest = createManifest();
    int armor = manifest.GetIndex("armor06");
    int capacitor = manifest.GetIndex("capacitor08");
    int typo = manifest.GetIndex("cpacitor08");
    ASSERT_EQ(armor, 0);
    ASSERT_EQ(capacitor, 1);
    ASSERT_EQ(typo, -1); // Not found should return -1 
}

TEST(Manifest, HasCargo) {
    Manifest manifest = createManifest();
    ASSERT_TRUE(manifest.HasCargo("armor06"));
    ASSERT_FALSE(manifest.HasCargo("Diamond"));
}

TEST(Manifest, GetRandomCargo) {
    Manifest manifest = createManifest();
    Cargo c = manifest.GetRandomCargo();
    // How to test?
}

TEST(Manifest, GetCategoryManifest) {
    Manifest manifest = createManifest();
    Manifest armorManifest = manifest.GetCategoryManifest("upgrades/Armor");
    
    ASSERT_EQ(armorManifest.Size(), 1);
    Cargo c = armorManifest.GetCargoByName("armor06");
    ASSERT_EQ(c.GetName(), "armor06");
    ASSERT_EQ(c.GetCategory(), "upgrades/Armor");
    ASSERT_TRUE(armorManifest.HasCargo("armor06"));
    ASSERT_FALSE(armorManifest.HasCargo("capacitor08"));
}

TEST(Manifest, Singleton) {
    // Manifest& mpl1 = Manifest::MPL();
    // Manifest& mpl2 = Manifest::MPL();
    // assert(&mpl1 == &mpl2);
}

