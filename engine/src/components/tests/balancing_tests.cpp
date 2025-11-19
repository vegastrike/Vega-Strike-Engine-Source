/*
 * balancing_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "components/energy_container.h"
#include "components/reactor.h"
#include "configuration/game_config.h"

extern float simulation_atom_var;

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
                  fuel(ComponentType::Fuel), energy(ComponentType::Capacitor),
                  ftl_energy(ComponentType::FtlCapacitor),
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

    FuelBurnResult(double residue, int iterations, int seconds):
        residue(residue), iterations(iterations), seconds(seconds) {}
};

FuelBurnResult fuelBurn(EnergyManager& manager,
                std::vector<EnergyConsumer>& consumers,
                int seconds,
                int print_every_n = 1000) {
    simulation_atom_var = 0.1;
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

    FuelBurnResult result(manager.fuel.Percent(), i, i/60);

    return result;
};

// This tests a very trivial example
TEST(FuelBurn, Trivial) {
    simulation_atom_var = 0.1;
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
    simulation_atom_var = 0.1;
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
    simulation_atom_var = 0.1;
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

// Use this test to figure out why FTL drive is running out of energy
/*TEST(FTLDrive, Sanity) {
    EnergySetup setup = {99.0, 25, 1.0, 1.0};
    EnergyManager manager = EnergyManager(setup, simulation_atom_var);
    std::vector<EnergyConsumer> consumers = {
        EnergyConsumer(&manager.ftl_energy, false, 120) // General consumer at 40 per second
    };

    manager.Print(-1);
    EXPECT_FALSE(manager.fuel.Depleted());
    EXPECT_FALSE(manager.energy.Depleted());
    EXPECT_FALSE(manager.ftl_energy.Depleted());

    int i = 0;
    for(;i<10;i++) {
        manager.Print(i);

        manager.reactor.Generate();
        for(EnergyConsumer& consumer : consumers) {
            if(consumer.CanConsume()) {
                consumer.Consume();
            } else {
                std::cout << "Not enough FTL energy.\n";
                EXPECT_EQ(0,1);
            }

        }
    }

    FuelBurnResult result(manager.fuel.Percent(), i, i/60);

    EXPECT_EQ(0,1); // use these to see detailed prints
}*/
