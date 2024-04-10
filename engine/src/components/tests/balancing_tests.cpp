#include <gtest/gtest.h>

#include "energy_container.h"
#include "energy_manager.h"
#include "reactor.h"
#include "fuel.h"


double simulation_atom_var = 0.1;

struct EnergySetup {
    double capacity;
    double fuel_capacity;
    double energy_capacity;
    double ftl_capacity;

    EnergySetup(double capacity, double fuel_capacity, 
                double energy_capacity,
                double ftl_capacity):
                capacity(capacity), fuel_capacity(fuel_capacity),
                energy_capacity(energy_capacity), ftl_capacity(ftl_capacity) {}
};



struct ConsumerSetup {
    EnergyType energy_type;
    EnergyConsumerClassification classification;
    EnergyConsumerType consumer_type;
    double consumption;
};

double reactor_capacity = 15;

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

struct EnergyManager {
    Fuel fuel;
    EnergyContainer energy;
    EnergyContainer ftl_energy;
    Reactor reactor;

    EnergyManager(EnergySetup setup, 
                  double simulation_atom_var):
                  fuel(), energy(), ftl_energy(),
                  reactor(setup.capacity, &energy, &ftl_energy,
                            simulation_atom_var) {
        fuel.SetCapacity(setup.fuel_capacity);
        energy.SetCapacity(setup.energy_capacity);
        ftl_energy.SetCapacity(setup.ftl_capacity);
    }
};


double fuelBurn(EnergySetup setup, 
                std::vector<EnergyConsumer> energy_consumers, int seconds,
                int print_every_n = 1000) {
    int run_time = seconds / simulation_atom_var;
    EnergyManager manager = EnergyManager(setup, simulation_atom_var);
    
    // Add fuel consumers


    // Add energy consumers
    for(EnergyConsumer consumer : energy_consumers) {
        manager.energy.AddConsumer(consumer);
    }

    for(int i = 0;i<run_time;i++) {
        if(i%print_every_n == 0) {
            std::cout << i << " R: " << manager.reactor.Capacity() << 
                              " F: " << manager.fuel.Level() << 
                              " E: " << manager.energy.Level() <<
                              " S: " << manager.ftl_energy.Level() << std::endl;
        }
        
        if(manager.fuel.Depleted()) { 
            std::cout << "Exhausted all fuel in " << i * simulation_atom_var / 60 << " minutes.\n";
            break;
        }

        manager.fuel.Act();
        manager.reactor.Generate();
        manager.energy.Act();
        manager.ftl_energy.Act();
    }

    return manager.fuel.Percent();
};

// This tests a fighter ship with level 1 equipment
TEST(NoFuelBurn, Robin) {
    EnergySetup setup = EnergySetup(15.0, 3.51, 100.0, 200.0);
    int seconds = 20 * 60; // 20 minutes gameplay

    double result = fuelBurn(setup, std::vector<EnergyConsumer>(),
                             seconds);

    std::cout << "NoFuelBurn percent left: " << result * 100 << std::endl;
}

// This tests a fighter ship with level 1 equipment and steady 50MJ energy consumption
TEST(FuelBurn, RobinNaive_1) {
    EnergySetup setup = {15.0, 3.51, 100.0, 200.0};
    std::vector<EnergyConsumer> consumers_setup = {
        EnergyConsumer(EnergyType::Fuel, EnergyConsumerClassification::Drive, 
        EnergyConsumerType::Constant, 0.0001),
        EnergyConsumer(EnergyType::Fuel, EnergyConsumerClassification::Afterburner, 
        EnergyConsumerType::Constant, 0.0001 * 3 * .05), // Drive consumption x 3 but 5% of flight time
        EnergyConsumer(EnergyType::Energy, EnergyConsumerClassification::LifeSupport, 
        EnergyConsumerType::Constant, 50.0 * simulation_atom_var) // General consumer at 50 per second
    };
    
    int seconds = 60 * 60; // 60 minutes gameplay
    
    double result = fuelBurn(setup, consumers_setup, seconds);

    std::cout << "NaiveFuelBurn_1 percent left: " << result * 100 << std::endl;
}

// This tests a fighter ship with level 1 equipment and steady 150MJ energy consumption
TEST(FuelBurn, RobinNaive_2) {
    EnergySetup setup = {44.0, 3.51, 300.0, 200.0};
    std::vector<EnergyConsumer> consumers_setup = {
        EnergyConsumer(EnergyType::Fuel, EnergyConsumerClassification::Drive, 
        EnergyConsumerType::Constant, 0.0001),
        EnergyConsumer(EnergyType::Fuel, EnergyConsumerClassification::Afterburner, 
        EnergyConsumerType::Constant, 0.0001 * 3 * .05), // Drive consumption x 3 but 5% of flight time
        EnergyConsumer(EnergyType::Energy, EnergyConsumerClassification::LifeSupport, 
        EnergyConsumerType::Constant, 150.0 * simulation_atom_var) // General consumer at 50 per second
    };
    
    int seconds = 60 * 60; // 60 minutes gameplay
    
    double result = fuelBurn(setup, consumers_setup, seconds);

    std::cout << "NaiveFuelBurn_2 percent left: " << result * 100 << std::endl;
}