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

#ifndef STORE_H
#define STORE_H

#include <vector>

#include "product.h"
#include "resource.h"


// A bit of hackery to keep the names separate for now.
// In the future, there will probably be a significant difference
// between Store and Customer
class Store;
typedef Store Customer;

class Store
{
public: // TODO: remove
    std::vector<Product> stock_;
    Resource<double> cash_;
    bool unlimited_funds_;
public:
    Store(std::vector<Product> stock = std::vector<Product>(), double cash = -1.0);

    void Add(Product product, double quantity);
    void Add(int index, double quantity);
    void Subtract(int index, double quantity);

    bool InStock(std::string product_name);
    int ProductIndex(std::string product_name);

    // These are from the point of view of the store/called class and also affect the other party
    bool Buy(Customer& seller, std::string product_name, double quantity);
    bool Sell(Customer& buyer, std::string product_name, double quantity);
    void SetFunds(double cash);
    void Stock(std::vector<Product> stock);
};



#endif // STORE_H
