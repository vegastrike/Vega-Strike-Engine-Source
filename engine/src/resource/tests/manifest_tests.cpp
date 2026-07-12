/*
 * manifest_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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


#include <cassert>
#include <gtest/gtest.h>
#include "resource/manifest.h"
#include "cmd/unit_generic.h"

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

TEST(Cargo, GetCargoQtyAndPriceImplementations) {
    constexpr double kPriceDeviation = 0.1;
    constexpr double kQuantityDeviation = 0.5;
    constexpr double kMinPrice = 0.01;
    constexpr float  kMaxPriceMult = 5.00F;
    constexpr int    kMinQuantity = 0;
    const std::string kCargoCategory = "upgrades/Armor";

    const Manifest manifest = createManifest();
    const Manifest category_manifest = manifest.GetCategoryManifest(kCargoCategory);

    const std::vector<Cargo> cargo_list = category_manifest.GetItems();

    // Find the minimum and maximum prices in the cargo list
    // We start with extreme values but at the end, min < max
    float min_cargo_price = std::numeric_limits<float>::max();
    float max_cargo_price = 0.0F;
    for (const Cargo& cargo : cargo_list) {
        const float price1 = cargo.GetPrice();
        if (price1 < min_cargo_price) {
            min_cargo_price = price1;
        }
        if (price1 > max_cargo_price) {
            max_cargo_price = price1;
        }
    }

    max_cargo_price *= kMaxPriceMult;

    for (const Cargo& cargo : cargo_list) {
        Cargo cargo_old_way = Cargo::GetCargoQtyAndPriceOldWay(cargo.GetPrice(), kPriceDeviation, 1.0, kQuantityDeviation, kMinPrice, max_cargo_price, cargo);
        Cargo cargo_new_way = Cargo::GetCargoQtyAndPriceCpp11StdDev(cargo.GetPrice(), kPriceDeviation, 1.0, kQuantityDeviation, kMinPrice, max_cargo_price, cargo);

        ASSERT_EQ(cargo_old_way.GetName(), cargo.GetName());
        ASSERT_EQ(cargo_new_way.GetName(), cargo.GetName());
        ASSERT_EQ(cargo_old_way.GetCategory(), cargo.GetCategory());
        ASSERT_EQ(cargo_new_way.GetCategory(), cargo.GetCategory());
        ASSERT_EQ(cargo_old_way.GetCategory(), kCargoCategory);
        ASSERT_EQ(cargo_new_way.GetCategory(), kCargoCategory);
        ASSERT_EQ(cargo_old_way.GetDescription(), cargo.GetDescription());
        ASSERT_EQ(cargo_new_way.GetDescription(), cargo.GetDescription());
        ASSERT_EQ(cargo_old_way.GetMass(), cargo.GetMass());
        ASSERT_EQ(cargo_new_way.GetMass(), cargo.GetMass());
        ASSERT_EQ(cargo_old_way.GetVolume(), cargo.GetVolume());
        ASSERT_EQ(cargo_new_way.GetVolume(), cargo.GetVolume());

        ASSERT_GE(cargo_old_way.GetQuantity(), kMinQuantity);
        ASSERT_GE(cargo_new_way.GetQuantity(), kMinQuantity);
        ASSERT_GE(cargo_old_way.GetPrice(), kMinPrice);
        ASSERT_GE(cargo_new_way.GetPrice(), kMinPrice);
        // ASSERT_LE(cargo_old_way.GetPrice(), cargo.GetPrice() + kMaxPriceAdd);
        ASSERT_LE(cargo_new_way.GetPrice(), cargo.GetPrice() * kMaxPriceMult);
    }
}

TEST(Cargo, GetCargoQtyAndPriceHistograms) {
    constexpr double kPriceDeviation = 0.1;
    constexpr double kQuantityDeviation = 0.5;
    constexpr float kMaxPriceMult = 5.00F;
    constexpr auto kIterations = 10000;
    constexpr auto kHistogramDisplayAdjust = 200;
    const std::string kCargoCategory = "upgrades/Capacitors/Standard";

    const Manifest manifest = createManifest();
    const Manifest category_manifest = manifest.GetCategoryManifest(kCargoCategory);

    const std::vector<Cargo> cargo_list = category_manifest.GetItems();

    // Find the minimum and maximum prices in the cargo list
    // We start with extreme values but at the end, min < max
    float min_cargo_price = std::numeric_limits<float>::max();
    float max_cargo_price = 0.0F;
    for (const Cargo& cargo : cargo_list) {
        const float price1 = cargo.GetPrice();
        if (price1 < min_cargo_price) {
            min_cargo_price = price1;
        }
        if (price1 > max_cargo_price) {
            max_cargo_price = price1;
        }
    }
    max_cargo_price *= kMaxPriceMult;

    for (const Cargo& cargo : cargo_list) {
        // Borrows heavily from sample code at https://en.cppreference.com/cpp/numeric/random/normal_distribution
        // Retrieved 2026-05-23
        VS_LOG(trace, (boost::format("  Histograms for cargo %1%:") % cargo.GetName()));
        std::map<uint64_t, uint64_t> old_way_qty_histogram{};
        std::map<uint64_t, uint64_t> new_way_qty_histogram{};
        std::map<uint64_t, uint64_t> old_way_price_histogram{};
        std::map<uint64_t, uint64_t> new_way_price_histogram{};
        for (auto n{kIterations}; n; --n) {
            Cargo cargo_old_way = Cargo::GetCargoQtyAndPriceOldWay(cargo.GetPrice(), kPriceDeviation, 1.0, kQuantityDeviation, min_cargo_price,
                                  max_cargo_price, cargo);
            Cargo cargo_new_way = Cargo::GetCargoQtyAndPriceCpp11StdDev(cargo.GetPrice(), kPriceDeviation, 1.0, kQuantityDeviation, min_cargo_price,
                                  max_cargo_price, cargo);
            auto old_way_qty = [&cargo_old_way]{ return cargo_old_way.GetQuantity(); };
            auto new_way_qty = [&cargo_new_way]{ return cargo_new_way.GetQuantity(); };
            auto old_way_price = [&cargo_old_way] { return cargo_old_way.GetPrice() * 100; };
            auto new_way_price = [&cargo_new_way] { return cargo_new_way.GetPrice() * 100; };
            ++old_way_qty_histogram[old_way_qty()];
            ++new_way_qty_histogram[new_way_qty()];
            ++old_way_price_histogram[old_way_price()];
            ++new_way_price_histogram[new_way_price()];
        }
        VS_LOG(trace, "    Old way quantities:");
        for (const auto [k, v] : old_way_qty_histogram) {
            VS_LOG(trace, (boost::format("      %1$10d %2$s") % k % std::string(v / kHistogramDisplayAdjust, '*')));
        }
        VS_LOG(trace, "    New way quantities:");
        for (const auto [k, v] : new_way_qty_histogram) {
            VS_LOG(trace, (boost::format("      %1$10d %2$s") % k % std::string(v / kHistogramDisplayAdjust, '*')));
        }
        VS_LOG(trace, "    Old way prices:");
        for (const auto [k, v] : old_way_price_histogram) {
            const double actual_price = k / 100.0;
            VS_LOG(trace, (boost::format("      %1$8.2f %2$s") % actual_price % std::string(v / kHistogramDisplayAdjust, '*')));
        }
        VS_LOG(trace, "    New way prices:");
        for (const auto [k, v] : new_way_price_histogram) {
            const double actual_price = k / 100.0;
            VS_LOG(trace, (boost::format("      %1$8.2f %2$s") % actual_price % std::string(v / kHistogramDisplayAdjust, '*')));
        }
    }
}
