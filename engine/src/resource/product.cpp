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

#include "resource/product.h"

static const std::string default_product_name("DEFAULT_PRODUCT_NAME");

Product::Product(): name(default_product_name), quantity(1), price(0.0) {}


Product::Product(const std::string &name, const double quantity, const double price) :
    name(name), quantity(quantity), price(price) {}


double Product::add(double quantity)
{
    return 0.0;
}


double Product::subtract(double quantity)
{
    return 0.0;
}


bool operator==(const Product &lhs, const std::string &rhs) {
    return lhs.name == rhs;
}

bool operator==(const std::string &lhs, const Product &rhs) {
    return lhs == rhs.name;
}


bool operator!=(const Product &lhs, const std::string &rhs) {
    return lhs.name != rhs;
}

bool operator!=(const std::string &lhs, const Product &rhs) {
    return lhs != rhs.name;
}
