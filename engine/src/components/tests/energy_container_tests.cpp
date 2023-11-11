#include <gtest/gtest.h>

#include "energy_container.h"
#include "energy_consumer.h"

void printContainer(EnergyContainer& container) {
    std::cout << "Max Level: " << container.MaxLevel();
    std::cout << " Level: " << container.Level();
    std::cout << " Percent: " << container.Percent() << std::endl;
}

TEST(EnergyContainer, Sanity) {
    EnergyContainer container = EnergyContainer();
    EXPECT_EQ(container.Level(), 0.0);
    container.SetCapacity(10.0, true);

    printContainer(container);

    EnergyConsumer consumer = EnergyConsumer(EnergyType::Fuel, 
                                             EnergyConsumerClassification::Drive,
                                             EnergyConsumerType::Constant, 1.0);
    container.AddConsumer(consumer);

    int i=0;
    while(container.Level() > 0 && i < 100) {
        container.Act();
        printContainer(container);
        i++;
    }
}