
#include <gtest/gtest.h>

#include "store.h"
#include "product.h"

TEST(Store, Sanity) {
    Product used_car("Used Car", 1, 1000);
    Product bread("Bread", 1, 1.5);
    Product milk("Milk", 1, 2);
    Product gold_bar("Gold", 30, 250);
    Product cigarettes("Cigarettes", 20, 0.5);
    std::vector<Product> stock = { bread, milk, cigarettes, gold_bar };
    Store store(stock, 1000);

    Customer customer;
    std::vector<Product> inventory = {used_car};
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
