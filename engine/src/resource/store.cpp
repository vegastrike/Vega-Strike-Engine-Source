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
    stock_(stock),
    cash_(Resource<double>(-1.0, 0.0)),
    unlimited_funds_(cash == -1.0) {}


void Store::Add(Product product, double quantity) {
    Product new_product = product;
    new_product.quantity_ = quantity;
    stock_.push_back(new_product);
}

void Store::Add(int index, double quantity) {
    stock_[index].quantity_ += quantity;
}

void Store::Subtract(int index, double quantity) {
    stock_[index].quantity_ += quantity;
}


bool Store::Buy(Customer& seller, std::string product_name, double quantity)
{
    // Find product at customer/seller
    int index = seller.ProductIndex(product_name);
    if(index == -1) {   // Seller doesn't have product
        return false;
    }

    // Check if seller has enough to sell
    Product product = seller.stock_[index];
    if(product.quantity_ < quantity) {
        return false;
    }

    // Calculate transaction cost
    double total_cost = quantity * product.unit_price_;

    // Check if buyer has enough funds
    if(cash_ < total_cost && !unlimited_funds_) {
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
    if(!seller.unlimited_funds_) {
        seller.cash_ += total_cost;
    }

    if(!unlimited_funds_) {
        cash_ -= total_cost;
    }

    return true;
}


bool Store::InStock(std::string product_name) {
    for(Product &in_store_product : stock_) {
        if(in_store_product == product_name && in_store_product.quantity_ > 0.0) {
            return true;
        }
    }

    return false;
}

int Store::ProductIndex(std::string product_name) {
    int index = 0;

    for(Product &in_store_product : stock_) {
        if(in_store_product == product_name) {
            return index;
        }

        index++;
    }

    return -1;
}



bool Store::Sell(Customer& buyer, std::string product_name, double quantity)
{
    buyer.Buy(*this, product_name, quantity);
}

void Store::SetFunds(double cash)
{
    cash_ = 1000.0; //cash;
    unlimited_funds_ = false;
}

void Store::Stock(std::vector<Product> stock)
{
    stock_.insert(stock_.end(), stock.begin(), stock.end());
}
