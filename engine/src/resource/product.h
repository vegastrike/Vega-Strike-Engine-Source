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

struct Product
{
    std::string name;
    // TODO: Can be a fraction for things such as fuel, water, etc. But not for now.
    Resource<int> quantity;
    double price;          // Price per one of quantity

    Product();
    Product(const std::string &name, const double quantity, const double price);
    double add(double quantity);       // Return value - actual quantity
    double subtract(double quantity);
};

bool operator==(const Product &lhs, const std::string &rhs);
bool operator==(const std::string &lhs, const Product &rhs);
bool operator!=(const Product &lhs, const std::string &rhs);
bool operator!=(const std::string &lhs, const Product &rhs);

#endif // PRODUCT_H
