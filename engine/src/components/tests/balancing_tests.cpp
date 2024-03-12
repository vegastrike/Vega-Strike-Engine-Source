#include <gtest/gtest.h>

#include "energy_container.h"
#include "energy_manager.h"
#include "reactor.h"


double simulation_atom_var = 0.1;

struct ReactorSetup {
    double capacity;
    double level;
    double usage_factor;
};

struct ContainersSetup {
    double fuel_capacity;
    double energy_capacity;
    double spec_capacity;
};

struct ConsumerSetup {
    EnergyType energy_type;
    EnergyConsumerClassification classification;
    EnergyConsumerType consumer_type;
    double consumption;
};

double reactor_capacity = 15;
double reactor_level = 1.0;

double fuel_capacity = 3.51; // Robin
double energy_capacity = 100.0; // capacitor 1
double spec_capacity = 200.0;   // spec capacitor 1

double mass = 48;

// Consumers
// Fuel
double reactor_usage_factor = 0.001;
double drive = 1;
double afterburner = 3;

// Energy
double shield_recharge = 4;
double lifeSupport = 1;
double radar = 1;
double shieldRegen = 1;
double shieldMaintenance = 1;
double ECM = 10;
double Cloak = 10;

// SPEC
double SPECDrive = 1;
double JumpDrive = 1;

EnergyManager setup(ContainersSetup containers_setup, ReactorSetup reactor_setup, 
                    double simulation_atom_var) {
    EnergyManager manager = EnergyManager();
    
    manager.SetCapacity(EnergyType::Fuel, containers_setup.fuel_capacity);
    manager.SetCapacity(EnergyType::Energy, containers_setup.energy_capacity);
    manager.SetCapacity(EnergyType::FTL, containers_setup.spec_capacity);
    manager.SetReactor(reactor_setup.capacity, reactor_setup.usage_factor, 
                       reactor_setup.level, simulation_atom_var);

    return manager;
}

void setupConsumer(EnergyManager& manager, ConsumerSetup setup) {
    manager.AddConsumer(setup.energy_type, setup.classification, setup.consumer_type, setup.consumption);
}

double fuelBurn(ContainersSetup containers_setup, ReactorSetup reactor_setup,
                std::vector<ConsumerSetup> consumers_setup, int seconds,
                int print_every_n = 1000) {
    int run_time = seconds / simulation_atom_var;
    EnergyManager manager = setup(containers_setup, reactor_setup, simulation_atom_var);
    for(ConsumerSetup setup : consumers_setup) {
        setupConsumer(manager, setup);
    }

    for(int i = 0;i<run_time;i++) {
        if(i%print_every_n == 0) {
            std::cout << i << " R: " << manager.GetReactorCapacity() << 
                              " F: " << manager.GetLevel(EnergyType::Fuel) << 
                              " E: " << manager.GetLevel(EnergyType::Energy) <<
                              " S: " << manager.GetLevel(EnergyType::FTL) << std::endl;
        }
        
        if(manager.GetLevel(EnergyType::Fuel) < 0.001) { 
            std::cout << "Exhausted all fuel in " << i * simulation_atom_var / 60 << " minutes.\n";
            break;
        }

        manager.Act();
    }

    return manager.Percent(EnergyType::Fuel);
};

// This tests a fighter ship with level 1 equipment
TEST(NoFuelBurn, Robin) {
    ContainersSetup containers_setup = {3.51, 100.0, 200.0};
    ReactorSetup reactor_setup = {15.0, reactor_usage_factor, 1.0};
    int seconds = 20 * 60; // 20 minutes gameplay

    double result = fuelBurn(containers_setup, reactor_setup, std::vector<ConsumerSetup>(),
                             seconds);

    std::cout << "NoFuelBurn percent left: " << result * 100 << std::endl;
}

// This tests a fighter ship with level 1 equipment and steady 50MJ energy consumption
TEST(FuelBurn, RobinNaive_1) {
    ContainersSetup containers_setup = {3.51, 100.0, 200.0};
    ReactorSetup reactor_setup = {15.0, reactor_usage_factor, 1.0};
    std::vector<ConsumerSetup> consumers_setup = {
        {EnergyType::Fuel, EnergyConsumerClassification::Drive, 
        EnergyConsumerType::Constant, 0.0001},
        {EnergyType::Fuel, EnergyConsumerClassification::Afterburner, 
        EnergyConsumerType::Constant, 0.0001 * 3 * .05}, // Drive consumption x 3 but 5% of flight time
        {EnergyType::Energy, EnergyConsumerClassification::LifeSupport, 
        EnergyConsumerType::Constant, 50.0 * simulation_atom_var} // General consumer at 50 per second
    };
    
    int seconds = 60 * 60; // 60 minutes gameplay
    
    double result = fuelBurn(containers_setup, reactor_setup, consumers_setup,
                             seconds);

    std::cout << "NaiveFuelBurn_1 percent left: " << result * 100 << std::endl;
}

// This tests a fighter ship with level 1 equipment and steady 150MJ energy consumption
TEST(FuelBurn, RobinNaive_2) {
    ContainersSetup containers_setup = {3.51, 300.0, 200.0};
    ReactorSetup reactor_setup = {44.0, reactor_usage_factor, 3.0};
    std::vector<ConsumerSetup> consumers_setup = {
        {EnergyType::Fuel, EnergyConsumerClassification::Drive, 
        EnergyConsumerType::Constant, 0.0001},
        {EnergyType::Fuel, EnergyConsumerClassification::Afterburner, 
        EnergyConsumerType::Constant, 0.0001 * 3 * .05}, // Drive consumption x 3 but 5% of flight time
        {EnergyType::Energy, EnergyConsumerClassification::LifeSupport, 
        EnergyConsumerType::Constant, 150.0 * simulation_atom_var} // General consumer at 150 per second
    };
    
    int seconds = 60 * 60; // 60 minutes gameplay
    
    double result = fuelBurn(containers_setup, reactor_setup, consumers_setup,
                             seconds);

    std::cout << "NaiveFuelBurn_2 percent left: " << result * 100 << std::endl;
}