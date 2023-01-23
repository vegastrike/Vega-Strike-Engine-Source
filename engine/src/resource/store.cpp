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


#include "store.h"


Store::Store(std::vector<Product> stock, double cash):
    stock(stock),
    cash(Resource<double>(cash, 0.0)),
    unlimited_funds(cash == -1.0) {}


void Store::Add(Product product, const int quantity) {
    Product new_product = product;
    new_product.quantity.Set(quantity);
    stock.push_back(new_product);
}

void Store::Add(int index, int quantity) {
    stock[index].quantity += quantity;
}

void Store::Subtract(int index, int quantity) {
    stock[index].quantity -= quantity;
}


bool Store::Buy(Customer& seller, std::string product_name, double quantity)
{
    // Find product at customer/seller
    int index = seller.ProductIndex(product_name);
    if(index == -1) {   // Seller doesn't have product
        return false;
    }

    // Check if seller has enough to sell
    Product product = seller.stock[index];
    if(product.quantity < quantity) {
        return false;
    }

    // Calculate transaction cost
    double total_cost = quantity * product.price;

    // Check if buyer has enough funds
    if(cash < total_cost && !unlimited_funds) {
        return false;
    }

    // Complete the transaction
    int buyer_index = ProductIndex(product_name);
    if(buyer_index == -1) {
        Add(product, quantity);
    } else {
        Add(index, quantity);
    }

    seller.Subtract(index, quantity);
    if(!seller.unlimited_funds) {
        seller.cash += total_cost;
    }

    if(!unlimited_funds) {
        cash -= total_cost;
    }

    return true;
}


bool Store::InStock(std::string product_name) {
    for(Product &in_store_product : stock) {
        if(in_store_product == product_name) {
            return (in_store_product.quantity > 0.0);
        }
    }

    return false;
}

double Store::GetStock(std::string product_name) {
    for(Product &in_store_product : stock) {
        if(in_store_product == product_name ) {
            return true;
        }
    }

    return -1;
}

bool Store::InStock(const int index) {
    return (stock[index].quantity > 0.0);
}

double Store::GetStock(const int index) {
    return stock[index].quantity;
}

int Store::ProductIndex(std::string product_name) {
    int index = 0;

    for(Product &in_store_product : stock) {
        if(in_store_product == product_name) {
            return index;
        }

        index++;
    }

    return -1;
}



bool Store::Sell(Customer& buyer, std::string product_name, double quantity)
{
    return buyer.Buy(*this, product_name, quantity);
}

void Store::SetFunds(double cash)
{
    this->cash.Set(cash);
    unlimited_funds = false;
}

void Store::Stock(std::vector<Product> stock)
{
    stock.insert(stock.end(), stock.begin(), stock.end());
}
