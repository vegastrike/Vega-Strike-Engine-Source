/*
 * cargo.h
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

#ifndef VEGA_STRIKE_ENGINE_RESOURCE_CARGO_H
#define VEGA_STRIKE_ENGINE_RESOURCE_CARGO_H

#include "resource/resource.h"

#include <string>
#include <vector>
#include <boost/json.hpp>


class Cargo {
protected:
    std::string name;
    std::string description = "";

    // TODO: Can be a fraction for things such as fuel, water, etc. But not for now.
    Resource<int> quantity = 0;

    // TODO: move to int and not deal with cents.
    double price = 0.0;          // Price per one of quantity
    std::string category = "";   // TODO: move to product

    double mass = 0.0;
    double volume = 0.0;
    bool mission = false;
    bool component = false;
    bool installed = false; 
    bool integral = false;
    bool weapon = false;
    bool passenger = false;
    bool slave = false;

    Resource<double> functionality = Resource<double>(1.0,0.0,1.0);

    friend class Manifest;
    friend class CargoHold;
public:
// Constructors
    Cargo();
    Cargo(std::string name, std::string description, int quantity, double price, std::string category, 
          double mass, double volume, bool mission = false, 
          bool component = false, bool installed = false, bool integral = false, bool weapon = false, 
          bool passenger = false, bool slave = false, double functionality = 1.0);
    Cargo(std::string name, std::string category, float price, int quantity, 
          float mass, float volume);

    // Parse string from save file
    Cargo(std::string& cargo_text);
    Cargo(boost::json::object json);

// Getters
    std::string GetName() const;
    std::string GetDescription() const;
    int GetQuantity() const;
    double GetPrice() const;
    double GetTotalValue() const;
    std::string GetCategory() const;
    double GetVolume() const;
    double GetMass() const; 
    
    bool IsMissionFlag() const;
    bool IsComponent() const;
    bool IsInstalled() const;
    bool IsIntegral() const;
    bool IsWeapon() const;
    bool IsPassenger() const;
    bool IsSlave() const;

    double GetFunctionality() const;

    std::string GetCategoryPython() const;
    std::string GetContentPython() const;
    std::string GetDescriptionPython() const;

    // For script_call_unit_generic
    std::string* GetNameAddress();
    std::string* GetCategoryAddress();

// Setters
    void SetName(const std::string& name);
    void SetDescription(const std::string &description);
    void SetQuantity(const int quantity);
    void SetPrice(const double price);
    void SetCategory(const std::string& category);
    void SetMass(double mass);
    void SetVolume(double volume);

    void SetMissionFlag(bool flag);
    void SetComponent(bool component);
    void SetInstalled(bool installed);
    void SetIntegral(bool integral);
    // Intentionally did not define SetWeapon and SetPassenger, 
    // as they are set by the Manifest and shouldn't be changed by the engine.
    void SetSlave(bool slave);
    void SetFunctionality(double func);

// Other methods
    void Add(int quantity);
    bool CanSell(bool ship_damaged = false) const;

    bool Damaged() const;
    void RandomDamage();
    double RepairPrice() const;

    std::string Serialize() const;
    
    bool operator==(const Cargo &other) const;
    bool operator<(const Cargo &other) const;
};

#endif //VEGA_STRIKE_ENGINE_RESOURCE_CARGO_H
