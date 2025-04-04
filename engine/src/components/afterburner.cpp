/*
 * afterburner.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
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

#include "components/afterburner.h"

#include "component_utils.h"
#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"

Afterburner::~Afterburner()
= default;

Afterburner::Afterburner(EnergyContainer *source) :
    Component(0.0, 0.0, true, true), EnergyConsumer(source, false), thrust(1,0,1), speed(1,0,1) {
    type = ComponentType::Afterburner;
}



// Component Methods
void Afterburner::Load(std::string unit_key) {
    static const double game_speed = vega_config::config->physics.game_speed;
    static const double game_accel = vega_config::config->physics.game_accel;
    static const double game_accel_speed = game_speed * game_accel;
    Component::Load(unit_key);

    thrust = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Afterburner_Accel", std::string("0.0")), game_accel_speed);
    speed = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Afterburner_Speed_Governor", std::string("0.0")), game_speed);
    double consumption = UnitCSVFactory::GetVariable(unit_key, "Afterburner_Usage_Cost", 1.0);
    SetConsumption(consumption);

    // We calculate percent operational as a simple average
    operational = (thrust.Percent() + speed.Percent()) / 2;
}

void Afterburner::SaveToCSV(std::map<std::string, std::string>& unit) const {
    static const double game_speed = vega_config::config->physics.game_speed;
    static const double game_accel = vega_config::config->physics.game_accel;
    static const double game_accel_speed = game_speed * game_accel;
    unit["Afterburner_Accel"] = thrust.Serialize(game_accel_speed);
    unit["Afterburner_Speed_Governor"] = speed.Serialize(game_speed);
}

// Afterburner is integrated and so cannot be upgraded/downgraded
// Use AfterburnerUpgrade to make changes
bool Afterburner::CanDowngrade() const {
    return false;
}

bool Afterburner::Downgrade() {
    return false;
}

bool Afterburner::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool Afterburner::Upgrade(const std::string upgrade_key) {
    return false;
}

void Afterburner::Damage() {
    thrust.RandomDamage();
    speed.RandomDamage();

    // We calculate percent operational as a simple average
    operational = (thrust.Percent() + speed.Percent()) / 2;
}

void Afterburner::DamageByPercent(double percent) {
    thrust.DamageByPercent(percent);
    speed.DamageByPercent(percent);

    operational = (thrust.Percent() + speed.Percent()) / 2;
}

void Afterburner::Repair() {
    thrust.RepairFully();
    speed.RepairFully();

    operational.RepairFully();
}

double Afterburner::Consume()
{
    return EnergyConsumer::Consume();
}
