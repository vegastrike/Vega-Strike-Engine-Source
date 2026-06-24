/*
 * cargo.cpp
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


#include "resource/cargo.h"

#include <cfenv>

#include "resource/manifest.h"
#include "resource/random_utils.h"
#include "json_utils.h"
#include "configuration/configuration.h"
#include "src/vega_cast_utils.h"

#include <iostream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "root_generic/macosx_math.h"
#include "root_generic/vega_random.h"
#include "src/vega_cast_utils.h"

static const std::string default_product_name("DEFAULT_PRODUCT_NAME");

// A simple utility function to parse a boolean value
static bool _parse_bool(const std::string& value) {
    return (value == "1" || value == "true");
}

static const double minimum_mass_and_volume = 0.01;

// Constructors

Cargo::Cargo(): name(default_product_name) {}


Cargo::Cargo(std::string name, std::string description, int quantity, double price, 
             std::string category, double mass, double volume, bool mission, 
             bool component, bool installed, bool integral, bool weapon, bool passenger, 
             bool slave, double functionality) :
             name(name), description(description), quantity(quantity), price(price), 
             category(category), mass(std::max(minimum_mass_and_volume,mass)), 
             volume(std::max(minimum_mass_and_volume,volume)), mission(mission),
             component(component), installed(installed), integral(integral) {
    this->functionality = functionality;
}

Cargo::Cargo(std::string name, std::string category, double price, int quantity, 
          double mass, double volume): name(name), quantity(quantity),
          price(price), category(category), mass(std::max(minimum_mass_and_volume,mass)),
          volume(std::max(minimum_mass_and_volume,volume)) {}
             

/*  0 name
    1 category
    2 price
    3 quantity
    4 mass
    5 volume
    6 func
    7 max_func
    8 description
    9 mission
    10 installed
    11 integral
    12 component
    13 weapon
    14 passenger
    15 slave
*/
Cargo::Cargo(std::string& cargo_text) {
    std::vector<std::string> cargo_parts;
    boost::split(cargo_parts,cargo_text,boost::is_any_of(";"));

    for(long unsigned int i=0;i<cargo_parts.size();i++) {
        switch (i) {
        case 0:
            name = cargo_parts[0];
            break;
        case 1:
            category = cargo_parts[1];
            break;
        case 2:
            price = locale_aware_stod(cargo_parts[2]);
            break;
        case 3:
            quantity = locale_aware_stoi(cargo_parts[3]);
            break;
        case 4:
            mass = std::max(locale_aware_stod(cargo_parts[4]), minimum_mass_and_volume);
            break;
        case 5:
            volume = std::max(locale_aware_stod(cargo_parts[5]), minimum_mass_and_volume);
            break;
        case 6:
            functionality = Resource<double>(locale_aware_stod(cargo_parts[6]), 0.0, 1.0);
            break;
        case 7:
            break; // max_functionality is always 1.0. cargo_parts[6] not used.
        case 8:
            description = cargo_parts[8];
            break;
        case 9:
            mission = _parse_bool(cargo_parts[9]);
            break;
        case 10:
            installed = component = _parse_bool(cargo_parts[10]);
            break;
        case 11:
            integral = _parse_bool(cargo_parts[11]);
            break;
        case 12:
            component = _parse_bool(cargo_parts[12]);
            break;
        case 13:
            weapon = _parse_bool(cargo_parts[13]);
            break;
        case 14:
            passenger = _parse_bool(cargo_parts[14]);
            break;
        case 15:
            slave = _parse_bool(cargo_parts[15]);
            break;
        default:
            break;
        }
    }
}

Cargo::Cargo(boost::json::object json): 
    name(JsonGetStringWithDefault(json, "file", "")),
    description(JsonGetStringWithDefault(json, "description", "")),
    quantity(1), 
    price(locale_aware_stoi(JsonGetStringWithDefault(json, "price", "0"))),
    category(JsonGetStringWithDefault(json, "categoryname", "")),
    mass(std::max(locale_aware_stod(JsonGetStringWithDefault(json, "mass", "0.0")), minimum_mass_and_volume)),
    volume(std::max(locale_aware_stod(JsonGetStringWithDefault(json, "volume", "0.1")), minimum_mass_and_volume)),
    mission(false), 
    component(GetBool(json, "upgrade", false)),
    installed(false),
    integral(false),
    weapon(GetBool(json, "weapon", false)),
    functionality(Resource<double>(1.0, 0.0, 1.0)) {
    if(volume <=0) {
        volume = 0.1;
    }
}

// Getters
std::string Cargo::GetName() const { 
    return name; 
}

std::string Cargo::GetDescription() const { 
    return description; 
}

int Cargo::GetQuantity() const { 
    return quantity.Value(); 
}

double Cargo::GetPrice() const { 
    return price; 
}

double Cargo::GetTotalValue() const {
    return price * quantity.Value();
}

std::string Cargo::GetCategory() const {
    return category;
}

double Cargo::GetVolume() const {
    return volume;
}

double Cargo::GetMass() const {
    return mass;
}

bool Cargo::IsMissionFlag() const {
    return mission;
}

bool Cargo::IsComponent() const { 
    return component; 
}

bool Cargo::IsInstalled() const {
    return installed;
}

bool Cargo::IsIntegral() const {
    return integral;
}

bool Cargo::IsWeapon() const { 
    return weapon; 
}

bool Cargo::IsPassenger() const { 
    return passenger; 
}

bool Cargo::IsSlave() const { 
    return slave; 
}

double Cargo::GetFunctionality() const {
    return functionality.Value();
}

std::string Cargo::GetCategoryPython() const {
    return category;
}

std::string Cargo::GetContentPython() const {
    return name;
}

std::string Cargo::GetDescriptionPython() const {
    return description;
}

// For script_call_unit_generic
std::string* Cargo::GetNameAddress() { 
    return &name; 
}

std::string* Cargo::GetCategoryAddress() { 
    return &category; 
}

// Setters

void Cargo::SetName(const std::string& name) { 
    this->name = name; 
}

void Cargo::SetDescription(const std::string& description) { 
    this->description = description; 
}

void Cargo::SetQuantity(const int quantity) { 
    this->quantity = quantity; 
}

void Cargo::SetPrice(const double price) { 
    this->price = price; 
}

void Cargo::SetCategory(const std::string& category) {
    this->category = category;
}

void Cargo::SetMass(double mass) {
    this->mass = mass;
}

void Cargo::SetVolume(double volume) {
    this->volume = std::max(minimum_mass_and_volume,volume);
}

void Cargo::SetMissionFlag(bool flag) {
    this->mission = flag;
}

void Cargo::SetComponent(bool component) {
    this->component = component;
}

void Cargo::SetInstalled(bool installed) {
    this->installed = installed;
}

void Cargo::SetIntegral(bool integral) {
    this->integral = integral;
}

void Cargo::SetSlave(bool slave) {
    this->slave = slave;
}

void Cargo::SetFunctionality(double func) {
    functionality = func;
}

// Misc.

void Cargo::Add(int quantity) {
    this->quantity += quantity;
}

bool Cargo::CanSell(bool ship_damaged) const {
    if(!component) {
        return true;
    }

    if(integral) {
        return false;
    }

    if(weapon) {
        return true;
    }

    const bool must_fix_first = configuration().physics.must_repair_to_sell;
    return (!must_fix_first || !ship_damaged);
}

bool Cargo::Damaged() const {
    // Can't call functionality.Damaged() because we're using value to record damage and not adjusted value.
    return functionality.Value() < functionality.MaxValue();
}

void Cargo::RandomDamage() {
    // Can't call functionality.RandomDamage() because we're using value to record damage and not adjusted value.
    functionality -= randomDouble();
}

double Cargo::RepairPrice() const {
    return .5 * price * (1 - functionality.Value()); // TODO: * configuration()->general.difficulty;
}

std::string Cargo::Serialize() const {
    return (boost::format("{%s;%s;%f;%d;%f;%f;%f;%f;%s;%d;%d;%d;%d;%d;%d;%d}")
        % this->name % this->category % this->price % this->quantity.Value() % this->mass
        % this->volume % this->functionality.Value() % 1.0 % this->description % this->mission
        % this->installed % this->integral % this->component % this->weapon % 
        this->passenger % this->slave
    ).str();
}

// Operators
bool Cargo::operator==(const Cargo &other) const {
    return name == other.name;
}

bool Cargo::operator<(const Cargo &other) const {
    return (category == other.category) ? (name < other.name) : (category < other.category);
}

Cargo Cargo::GetCargoQtyAndPriceOldWay(double price, double price_deviation, double quantity, double quantity_deviation,
        double min_cargo_price, double max_cargo_price, const Cargo &cargo) {
    Cargo return_value = cargo; // Copy the cargo item
    const double average_weight = abs(configuration().cargo.price_recenter_factor_dbl);
    // VS_LOG(trace, (boost::format("average_weight (abs(configuration().cargo.price_recenter_factor_dbl)) = %1%") % average_weight));
    return_value.SetQuantity(double_to_int(quantity - quantity_deviation));
    const double base_price = return_value.GetPrice();
    return_value.SetPrice(return_value.GetPrice() * (price - price_deviation));

    // The comment in the code describing this as the "stupid way" has been there for a long time. It did not originate with me.
    // I do not blame the original developers for going with a simpler solution when a true normal-distribution, standard-deviation
    // algorithm may not have been as readily available. But times have changed. C++11 provides this functionality built in.
    // -- Stephen G. Tuggy 2026-06-24

    //stupid way
    return_value.SetQuantity(return_value.GetQuantity() + double_to_int((quantity_deviation * 2 + 1) * VegaRandom::Instance().GenRandReal2()));
    return_value.SetPrice(return_value.GetPrice() + price_deviation * 2.0 * VegaRandom::Instance().RandomDouble());
    return_value.SetPrice(abs(return_value.GetPrice()));
    return_value.SetPrice((return_value.GetPrice() + (base_price * average_weight)) / (average_weight + 1));
    if (return_value.GetQuantity() <= 0) {
        return_value.SetQuantity(0);
    }
    //quantity more than zero
    else if (max_cargo_price > min_cargo_price + .01) {
        double renorm_price = (base_price - min_cargo_price) / (max_cargo_price - min_cargo_price);
        const double max_price_quant_adj = configuration().cargo.max_price_quant_adj_dbl;
        const double min_price_quant_adj = configuration().cargo.min_price_quant_adj_dbl;
        const double powah = configuration().cargo.price_quant_adj_power_dbl;
        // VS_LOG(trace, (boost::format("max_price_quant_adj = %1%") % max_price_quant_adj));
        // VS_LOG(trace, (boost::format("min_price_quant_adj = %1%") % min_price_quant_adj));
        // VS_LOG(trace, (boost::format("powah = %1%") % powah));
        renorm_price = std::pow(renorm_price, powah);
        renorm_price *= (max_price_quant_adj - min_price_quant_adj);
        renorm_price += 1;
        if (renorm_price > .001) {
            return_value.SetQuantity(return_value.GetQuantity() / float_to_int(renorm_price));
            if (return_value.GetQuantity() < 1) {
                return_value.SetQuantity(1);
            }
        }
    }
    const double min_price = configuration().cargo.min_cargo_price_dbl;
    // VS_LOG(trace, (boost::format("min_price (configuration().cargo.min_cargo_price_dbl) = %1%") % min_price));
    if (return_value.GetPrice() < min_price) {
        return_value.SetPrice(min_price);
    }
    return return_value;
}

Cargo Cargo::GetCargoQtyAndPriceCpp11StdDev(double price, double price_deviation, double quantity,
        double quantity_deviation, double min_cargo_price, double max_cargo_price, const Cargo &cargo) {
    constexpr double kMinQuantity = 0.0;
    constexpr double kMaxQuantity = std::numeric_limits<int32_t>::max();
    constexpr double kMinDeviation = 0.01;
    constexpr double kMaxDeviation = 5.0;

    Cargo return_value = cargo;
    std::fesetround(FE_TONEAREST);

    const double true_minimum_price = std::max(min_cargo_price, configuration().cargo.min_cargo_price_dbl);
    double true_maximum_price = max_cargo_price;
    if (true_minimum_price > true_maximum_price) {
        true_maximum_price = true_minimum_price;
    }
    double quantity_constrained = quantity;
    if (quantity_constrained > kMaxQuantity) {
        quantity_constrained = kMaxQuantity;
    } else if (quantity_constrained < kMinQuantity) {
        quantity_constrained = kMinQuantity;
    }
    double price_constrained = price;
    if (price_constrained > max_cargo_price) {
        price_constrained = max_cargo_price;
    } else if (price_constrained < true_minimum_price) {
        price_constrained = true_minimum_price;
    }
    double quantity_deviation_constrained = quantity_deviation;
    if (quantity_deviation_constrained > kMaxDeviation) {
        quantity_deviation_constrained = kMaxDeviation;
    } else if (quantity_deviation_constrained < kMinDeviation) {
        quantity_deviation_constrained = kMinDeviation;
    }
    double price_deviation_constrained = price_deviation;
    if (price_deviation_constrained > kMaxDeviation) {
        price_deviation_constrained = kMaxDeviation;
    } else if (price_deviation_constrained < kMinDeviation) {
        price_deviation_constrained = kMinDeviation;
    }

    double qty_dbl = VegaRandom::Instance().NormalDistribution(quantity_constrained, quantity_deviation_constrained, 0.0, std::numeric_limits<int>::max());
    // Round to nearest whole number
    int qty_int = std::rint(qty_dbl);
    return_value.SetQuantity(qty_int);

    double price1 = VegaRandom::Instance().NormalDistribution(price_constrained, price_deviation_constrained, true_minimum_price, true_maximum_price);
    // Round to two decimal places
    double price_rounded = std::rint(price1 * 100.0) / 100.0;
    return_value.SetPrice(price_rounded);

    return return_value;
}
