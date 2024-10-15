#include <gtest/gtest.h>

#include "energy_container.h"
#include "energy_consumer.h"

void printContainer(EnergyContainer& container) {
    std::cout << "Max Level: " << container.MaxLevel();
    std::cout << " Level: " << container.Level();
    std::cout << " Percent: " << container.Percent() << std::endl;
}

TEST(EnergyContainer, Sanity) {
    EnergyContainer container = EnergyContainer(EnergyType::Energy);
    EXPECT_EQ(container.Level(), 0.0);
    container.SetCapacity(10.0, true);

    printContainer(container);

    EnergyConsumer consumer = EnergyConsumer(&container, 
                                             false,
                                             1.0);
    

    int i=0;
    while(!container.Depleted() && i < 100) {
        std::cout << " Can consume: " << consumer.CanConsume() << std::endl;
        consumer.Consume();
        printContainer(container);
        i++;
    }

    // Uncomment to see prints
    //EXPECT_TRUE(consumer.CanConsume());
}