/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

#include "resource.h"

class Product
{
protected:
    std::string name_;
    Resource<double> quantity_;  // Can be a fraction for things such as fuel, water, etc.
    double unit_price_;          // Price per one of quantity

    friend bool operator==(const Product &lhs, const std::string &rhs);
    friend bool operator==(const std::string &lhs, const Product &rhs);
    friend bool operator!=(const Product &lhs, const std::string &rhs);
    friend bool operator!=(const std::string &lhs, const Product &rhs);
    friend class Store;

    // TODO: I'm not a fan of this. Clean this up (much) later
    friend class Carrier;
public:
    Product();
    Product(const std::string &name, const double quantity, const double unit_price);
    double add(double quantity);       // Return value - actual quantity
    double subtract(double quantity);
};

bool operator==(const Product &lhs, const std::string &rhs);
bool operator==(const std::string &lhs, const Product &rhs);
bool operator!=(const Product &lhs, const std::string &rhs);
bool operator!=(const std::string &lhs, const Product &rhs);

#endif // PRODUCT_H
