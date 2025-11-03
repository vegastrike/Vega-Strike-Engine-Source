/*
 * cargo.cpp
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


#include "resource/cargo.h"
#include "resource/manifest.h"
#include "resource/random_utils.h"
#include "json_utils.h"
#include "configuration/configuration.h"

#include <iostream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

static const std::string default_product_name("DEFAULT_PRODUCT_NAME");

// A simple utility function to parse a boolean value
static bool _parse_bool(const std::string& value) {
    return (value == "1" || value == "true");
}

// Constructors

Cargo::Cargo(): name(default_product_name) {}


Cargo::Cargo(std::string name, std::string description, int quantity, double price, 
             std::string category, double mass, double volume, bool mission, 
             bool component, bool installed, bool integral, bool weapon, bool passenger, 
             bool slave, double functionality) :
             name(name), description(description), quantity(quantity), price(price), 
             category(category), mass(mass), volume(volume), mission(mission),
             component(component), installed(installed), integral(integral) {
    this->functionality = functionality;
}

Cargo::Cargo(std::string name, std::string category, float price, int quantity, 
          float mass, float volume): name(name), category(category),
          price(price), quantity(quantity), mass(mass), volume(volume) {}
             

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

    // pmx-20251103 str::stod() fails on the decimal point on french locale (decimal separator is ',')
    const char* loc = std::setlocale(LC_NUMERIC, "en_US.UTF-8");

    for(long unsigned int i=0;i<cargo_parts.size();i++) {
        switch (i) {
        case 0: name = cargo_parts[0]; break;
        case 1: category = cargo_parts[1]; break;
        case 2: price = std::stod(cargo_parts[2]); break;
        case 3: quantity = std::stoi(cargo_parts[3]); break;
        case 4: mass = std::stod(cargo_parts[4]); break;
        case 5: volume = std::stod(cargo_parts[5]); break;
        case 6: functionality = Resource<double>(std::stod(cargo_parts[6]), 0.0, 1.0); break;
        case 7: break; // max_functionality is always 1.0. cargo_parts[6] not used.
        case 8: description = cargo_parts[8]; break;
        case 9: mission = _parse_bool(cargo_parts[9]); break;
        case 10: installed = component = _parse_bool(cargo_parts[10]); break;
        case 11: integral = _parse_bool(cargo_parts[11]); break;
        case 12: component = _parse_bool(cargo_parts[12]); break;
        case 13: weapon = _parse_bool(cargo_parts[13]); break;
        case 14: passenger = _parse_bool(cargo_parts[14]); break;
        case 15: slave = _parse_bool(cargo_parts[15]); break;
        
        default:
            break;
        }
    }

    // Restore locale
    std::setlocale(LC_NUMERIC, loc);    
}

Cargo::Cargo(boost::json::object json): 
    name(JsonGetStringWithDefault(json, "file", "")),
    description(JsonGetStringWithDefault(json, "description", "")),
    quantity(1), 
    price(std::stoi(JsonGetStringWithDefault(json, "price", "0"))), 
    category(JsonGetStringWithDefault(json, "categoryname", "")),
    mass(std::stod(JsonGetStringWithDefault(json, "mass", "0.0"))), 
    volume(std::stod(JsonGetStringWithDefault(json, "volume", "0.0"))),
    mission(false), 
    component(GetBool(json, "upgrade", false)),
    weapon(GetBool(json, "weapon", false)),
    installed(false), 
    integral(false),
    functionality(Resource<double>(1.0, 0.0, 1.0)) {}

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
    this->volume = volume;
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



