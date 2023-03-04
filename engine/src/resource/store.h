/*
 * store.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Roy Falk, and other Vega Strike contributors.
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

#include "preferred_types.h"
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
    vega_types::SequenceContainer<Product> stock;
    Resource<double> cash;
    bool unlimited_funds;
public:
    explicit Store(vega_types::SequenceContainer<Product> stock = vega_types::SequenceContainer<Product>(), double cash = -1.0);

    void Add(Product product, const int quantity);
    void Add(int index, int quantity);
    void Subtract(int index, int quantity);

    bool InStock(std::string product_name);
    double GetStock(std::string product_name);
    bool InStock(const int index);
    double GetStock(const int index);

    int ProductIndex(std::string product_name);

    // These are from the point of view of the store/called class and also affect the other party
    bool Buy(Customer& seller, std::string product_name, double quantity);
    bool Sell(Customer& buyer, std::string product_name, double quantity);
    void SetFunds(double cash);
    void Stock(vega_types::SequenceContainer<Product> stock);
};



#endif // STORE_H
