/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_RESOURCE_PRODUCT_H
#define VEGA_STRIKE_ENGINE_RESOURCE_PRODUCT_H

#include <string>

#include "resource.h"

class Product
{
protected:
    std::string name;
    // TODO: Can be a fraction for things such as fuel, water, etc. But not for now.
    Resource<int> quantity;

    // TODO: move to int and not deal with cents.
    double price;          // Price per one of quantity

    friend bool operator==(const Product &lhs, const std::string &rhs);
    friend bool operator==(const std::string &lhs, const Product &rhs);
    friend bool operator!=(const Product &lhs, const std::string &rhs);
    friend bool operator!=(const std::string &lhs, const Product &rhs);
    friend class Store;

    // TODO: I'm not a fan of this. Clean this up (much) later
    friend class Carrier;
public:
    Product();
    Product(const std::string &name, const double quantity, const double price);
    double add(double quantity);       // Return value - actual quantity
    double subtract(double quantity);

    const std::string GetName() const { return name; }
    const unsigned int GetQuantity() const { return quantity.Value(); }
    const double GetPrice() const { return price; }

    void SetName(const std::string& name) { this->name = name; }
    void SetQuantity(const int& quantity) { this->quantity = quantity; }
    void SetPrice(const double& price) { this->price = price; }

    // For script_call_unit_generic
    std::string* GetNameAddress() { return &name;}
};

bool operator==(const Product &lhs, const std::string &rhs);
bool operator==(const std::string &lhs, const Product &rhs);
bool operator!=(const Product &lhs, const std::string &rhs);
bool operator!=(const std::string &lhs, const Product &rhs);

#endif //VEGA_STRIKE_ENGINE_RESOURCE_PRODUCT_H
