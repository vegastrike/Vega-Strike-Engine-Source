#include <gtest/gtest.h>

#include "energy_container.h"
#include "reactor.h"

double simulation_atom_var = 0.1;

bool fairlyEqual(double a, double b);

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
    EnergyContainer fuel;
    EnergyContainer energy;
    EnergyContainer ftl_energy;
    Reactor reactor;

    EnergyManager(EnergySetup setup, 
                  double simulation_atom_var):
                  fuel(EnergyType::Fuel), energy(EnergyType::Energy), 
                  ftl_energy(EnergyType::FTL),
                  reactor(&fuel, &energy, &ftl_energy) {
        fuel.SetCapacity(setup.fuel_capacity);
        energy.SetCapacity(setup.energy_capacity);
        ftl_energy.SetCapacity(setup.ftl_capacity);
        reactor.SetCapacity(setup.capacity);
    }

    void Print(int counter) {
        std::cout << counter << " R: " << reactor.Capacity() << 
            " F: " << fuel.Level() << 
            " E: " << energy.Level() <<
            " S: " << ftl_energy.Level() << std::endl;
    }
};

struct FuelBurnResult {
    double residue;
    int iterations;
    int seconds;
};

FuelBurnResult fuelBurn(EnergyManager& manager, 
                std::vector<EnergyConsumer>& consumers,
                int seconds,
                int print_every_n = 1000) {
    int run_time = seconds / simulation_atom_var;
    
    manager.Print(-1);
    EXPECT_FALSE(manager.fuel.Depleted());
    EXPECT_FALSE(manager.energy.Depleted());
    EXPECT_FALSE(manager.ftl_energy.Depleted());

    int i = 0;
    for(;i<run_time;i++) {
        if(i%print_every_n == 0) {
            manager.Print(i);
        }
        
        if(manager.fuel.Depleted()) { 
            std::cout << i << " Exhausted all fuel in " << i * simulation_atom_var / 60 << " minutes.\n";
            break;
        }

        manager.reactor.Generate();
        for(EnergyConsumer& consumer : consumers) {
            consumer.Consume();
        }
    }

    FuelBurnResult result {
        .residue = manager.fuel.Percent(),
        .iterations = i,
        .seconds = i/60
    };

    return result;
};

// This tests a very trivial example
TEST(FuelBurn, Trivial) {
    EnergySetup setup = EnergySetup(1.0, 1.0, 100.0, 200.0);
    EnergyManager manager = EnergyManager(setup, simulation_atom_var);
    int seconds = 10000; 

    EXPECT_EQ(manager.reactor.GetConsumption(), 0.0001);

    std::vector<EnergyConsumer> consumers = {};

    FuelBurnResult result = fuelBurn(manager, consumers, seconds, 1000);
    std::cout << "Reactor consumption: " << manager.reactor.GetAtomConsumption() << std::endl;

    std::cout << "NoFuelBurn percent left: " << result.residue * 100 << std::endl;

    // 1 / (sim_atom_var (0.1) * conversion_ration (0.001))
    EXPECT_GT(result.iterations, 99900);
    EXPECT_LT(result.iterations, 100900);
}

// This tests a fighter ship with level 1 equipment and steady 15MJ energy consumption
// Ship flies for 22 minutes
TEST(FuelBurn, RobinNaive_1) {
    EnergySetup setup = {15.0, 3.51, 100.0, 200.0};
    EnergyManager manager = EnergyManager(setup, simulation_atom_var);
    std::vector<EnergyConsumer> consumers = {
        EnergyConsumer(&manager.fuel, false, 0.001),           // Drive
        EnergyConsumer(&manager.fuel, false, 0.001 * 3 * .05), // Afterburner, Drive consumption x 3 but 5% of flight time
        EnergyConsumer(&manager.energy, false, 15.0) // General consumer at 15 per second
    };
    
    int seconds = 60 * 60; // 60 minutes gameplay
    
    FuelBurnResult result = fuelBurn(manager, consumers, seconds, 1000);
    EXPECT_GT(result.iterations, 12000); // More than 10 minutes

    std::cout << "RobinNaive_1 NoFuelBurn percent left: " << result.residue * 100 << std::endl;
    //EXPECT_EQ(0,1); // use these to see detailed prints
}

// This tests a fighter ship with level 1 equipment and steady 40MJ energy consumption
// Ship flies for 10 minutes
TEST(FuelBurn, RobinNaive_2) {
    EnergySetup setup = {44.0, 3.51, 300.0, 200.0};
    EnergyManager manager = EnergyManager(setup, simulation_atom_var);
    std::vector<EnergyConsumer> consumers = {
        EnergyConsumer(&manager.fuel, false, 0.001),           // Drive
        EnergyConsumer(&manager.fuel, false, 0.001 * 3 * .05), // Afterburner, Drive consumption x 3 but 5% of flight time
        EnergyConsumer(&manager.energy, false, 40) // General consumer at 40 per second
    };
    
    int seconds = 60 * 60; // 60 minutes gameplay
    
    FuelBurnResult result = fuelBurn(manager, consumers, seconds, 1000);
    EXPECT_GT(result.iterations, 6000); // More than 10 minutes

    std::cout << "NaiveFuelBurn_2 percent left: " << result.residue * 100 << std::endl;
    //EXPECT_EQ(0,1); // use these to see detailed prints
}