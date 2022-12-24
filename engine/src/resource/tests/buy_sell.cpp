/*
 * buy_sell.cpp
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


#include <gtest/gtest.h>

#include "preferred_types.h"
#include "store.h"
#include "product.h"

using namespace vega_types;

TEST(Store, Sanity) {
    Product used_car("Used Car", 1, 1000);
    Product bread("Bread", 1, 1.5);
    Product milk("Milk", 1, 2);
    Product gold_bar("Gold", 30, 250);
    Product cigarettes("Cigarettes", 20, 0.5);
    SequenceContainer<Product> stock = {bread, milk, cigarettes, gold_bar };
    Store store(stock, 1000);

    Customer customer;
    SequenceContainer<Product> inventory = {used_car};
    customer.Stock(inventory);
    customer.SetFunds(1000);
    EXPECT_EQ(customer.cash.Value(), 1000.0);
    EXPECT_FALSE(customer.unlimited_funds);

    // Successful buy
    bool result = customer.Buy(store, "Milk", 1);
    EXPECT_TRUE(result);

    // Unsuccessful sell - not enough quantity
    result = customer.Buy(store, "Milk", 7);
    EXPECT_FALSE(result);

    // Unsuccessful sell - not enough cash
    result = customer.Buy(store, "Gold", 7);
    EXPECT_FALSE(result);
}
