/*
 * components_manager.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "components_manager.h"
#include "component_utils.h"
#include "resource/random_utils.h"
#include "configuration/configuration.h"
#include "cmd/unit_csv_factory.h"
#include "vs_logging.h"
#include "resource/manifest.h"
#include "src/vega_cast_utils.h"
#include <boost/format.hpp>
#include <iostream>

Resource<double> ComponentsManager::credits = Resource<double>(0.0, 0.0);

void ComponentsManager::Load(std::string unit_key) {
    mass = base_mass = UnitCSVFactory::GetVariable(unit_key, "Mass", 0.0);

    // Consumer
    std::string prohibited_upgrades_string = UnitCSVFactory::GetVariable(unit_key, "Prohibited_Upgrades", std::string());

    if(prohibited_upgrades_string.empty()) {
        return;
    }

    std::vector<std::string> upgrades;

    boost::split(upgrades, prohibited_upgrades_string, boost::is_any_of(";"));
    for (const std::string& upgrade : upgrades) {
        std::vector<std::string> parts;
        boost::split(parts, upgrade, boost::is_any_of(":"));
        if (parts.size() == 1) {
            const std::string& category = parts[0];
            prohibited_upgrades.emplace_back(category, 0);
        } else if (parts.size() == 2) {
            const std::string& category = parts[0];
            const int limit = locale_aware_stoi(parts[1]);
            //const std::pair<const std::string, const int> pair(category, limit);
            prohibited_upgrades.emplace_back(category, limit);
        } else {
            VS_LOG(error, (boost::format("%1%: Invalid format in prohibited upgrades string: %2%") % __FUNCTION__ % upgrade));
        }
    }
}

void ComponentsManager::Serialize(std::map<std::string, std::string>& unit) const {
    unit["Mass"] = std::to_string(base_mass);

    // Serialize prohibited upgrades
    if (!prohibited_upgrades.empty()) {
        std::string prohibited_upgrades_string;
        for (const auto& upgrade : prohibited_upgrades) {
            if (!prohibited_upgrades_string.empty()) {
                prohibited_upgrades_string += ";";
            }
            prohibited_upgrades_string += boost::str(boost::format("%s:%d") % upgrade.first % upgrade.second);
        }
        unit["Prohibited_Upgrades"] = prohibited_upgrades_string;
    }
}


double ComponentsManager::GetMass() const {
    return mass;
}

void ComponentsManager::SetMass(double mass) {
    this->mass = mass;
}

void ComponentsManager::SetPlayerShip() {
    player_ship = true;
}

bool ComponentsManager::IsPlayerShip() const {
    return player_ship;
}

void ComponentsManager::DamageRandomSystem() {
    double percent = 1 - hull.Percent();

    Component* components[] = {&fuel, &energy, &ftl_energy, &reactor,
                               &afterburner, &cloak, &drive, &ftl_drive,
                               &jump_drive, &computer, &radar, &shield,
                               &ecm, &repair_bot, &ship_functions};

    for(Component* component : components) {
        double chance_to_damage = randomDouble();
        if (chance_to_damage < percent) {
            component->DamageByPercent(chance_to_damage/10.0);
        }
    }

    // TODO: GenerateHudText();

    /*
        Things not handled at the moment:
        1. Mounts
        2. Upgrade volume
        3. Cargo volume
        4. Cargo

        TODO: mounts

        To think about:
        1. Should we damage the cargo?
            - To be handled by carrier.
            - More applicable for really large ships
        2. Should we damage the upgrade volume?
            - Probably not. How would that work in real life?!
        3. Should we damage the cargo volume?
            - Probably not. How would that work in real life?!
            - More applicable for really large ships, where you shoot at one of the cargo containers
    */



    // TODO: take actual damage into account when damaging components
    /*
    if (degrees >= 20 && degrees < 35) {
        //DAMAGE MOUNT
        if (randnum >= .65 && randnum < .9) {
            ecm.Damage();
        } else if (getNumMounts()) {
            unsigned int whichmount = rand() % getNumMounts();
            if (randnum >= .9) {
                DestroyMount(&mounts[whichmount]);
            } else if (mounts[whichmount].ammo > 0 && randnum >= .75) {
                mounts[whichmount].ammo *= float_to_int(dam);
            } else if (randnum >= .7) {
                mounts[whichmount].time_to_lock += (100 - (100 * dam));
            } else if (randnum >= .2) {
                mounts[whichmount].functionality *= dam;
            } else {
                mounts[whichmount].maxfunctionality *= dam;
            }
        }
        return;
    }

            //Do something NASTY to the cargo
            if (cargo.size() > 0) {
                unsigned int i = 0;
                unsigned int cargorand_o = rand();
                unsigned int cargorand;
                do {
                    cargorand = (cargorand_o + i) % cargo.size();
                } while ((cargo[cargorand].GetQuantity() == 0
                        || cargo[cargorand].IsMissionFlag()) && (++i) < cargo.size());
                cargo[cargorand].SetQuantity(cargo[cargorand].GetQuantity() * float_to_int(dam));
            }
    }*/
}

bool ComponentsManager::ShipDamaged() const {
    const Component* components[] = {&fuel, &energy, &ftl_energy, &reactor,
                               &afterburner, &cloak, &drive, &ftl_drive,
                               &jump_drive, &computer, &radar, &shield,
                               &ecm, &repair_bot, &ship_functions};
    
    for(const Component* component : components) {
        if(component->Damaged()) {
            return true;
        }
    }

    return false;
}

/** This function works by category only.
 *  The previous implementation also worked by specific upgrades.
 */
bool ComponentsManager::AllowedUpgrade(const Cargo& upgrade) const {
    for(const std::pair<const std::string, const int>& prohibited_upgrade : prohibited_upgrades) {
        if(prohibited_upgrade.first != upgrade.GetCategory()) {
            continue;
        }

        Manifest category_upgrades = upgrade_space.GetCategoryManifest(prohibited_upgrade.first);

        if(category_upgrades.Size() > prohibited_upgrade.second) {
            VS_LOG_FLUSH_EXIT(fatal,(boost::format("ComponentsManager::AllowedUpgrade: %1% is greater than %2%\n") % category_upgrades.Size() 
                      % prohibited_upgrade.second), EXIT_FAILURE);
        } else if(category_upgrades.Size() == prohibited_upgrade.second) {
            return false;
        }
    }

    return true;
}

bool ComponentsManager::UpgradeAlreadyInstalled(const Cargo& upgrade) const {
    const ComponentType component_type = GetComponentTypeFromName(upgrade.GetName());
    const Component *component = GetComponentByType(component_type);
    if(component) {
        return component->Installed();
    }

    // Note that this method returns true for error, so we err on the side of caution
    // and not install an upgrade.
    return true;
}

/** A convenience struct to hold the data used below */
struct HudText {
    const Component *component;
    const std::string name;
    const bool damageable;

    HudText(Component *component, std::string name, bool damageable):
        component(component), name(name), damageable(damageable) {}
};

/* This function is run when:
    1. A player ship is created
    2. A player ship is loaded from a saved game
    3. An upgrade/downgrade has occured
    4. DamageRandomSystem above is called
*/

void ComponentsManager::GenerateHudText(std::string getDamageColor(double)) {
    std::string report;

    report += configuration().graphics.hud.damage_report_heading + "\n\n";

    // TODO: this should be taken from assets so "FTL Drive" would be "SPEC Drive"
    const HudText hud_texts[] = {
        HudText(&hull, "Hull", true),
        HudText(&armor, "Armor", true),
        HudText(&shield, "Shield", true),
        HudText(&reactor, "Reactor", true),
        HudText(&fuel, "Fuel", true),
        HudText(&energy, "Capacitor", true),
        HudText(&ftl_energy, "FTL capacitor", true),
        HudText(&drive, "Drive", true),
        HudText(&ftl_drive, "FTL Drive", false),
        HudText(&jump_drive, "Jump Drive", false),
        HudText(&afterburner, "Afterburner", true),
        HudText(&computer, "Computer", true),
        HudText(&radar, "Radar", true),
        HudText(&ecm, "ECM", true),
        HudText(&cloak, "Cloak", true),
        HudText(&repair_bot, "Repair System", false)
    };


    for(const HudText& text : hud_texts) {
        if(text.component->Installed()) {
            std::string new_hud_text = PrintFormattedComponentInHud(
                text.component->PercentOperational(),
                text.name, text.damageable, getDamageColor);
            report += new_hud_text;
        }
    }

    // Ship Functions
    report += ship_functions.GetHudText(getDamageColor);

    hud_text = report;
}

std::string ComponentsManager::GetHudText() {
    return hud_text;
}

std::string ComponentsManager::GetTitle(bool show_cargo, bool show_star_date, std::string date) {
    const double empty_volume = show_cargo ? cargo_hold.MaxCapacity()
                                : upgrade_space.MaxCapacity();
    const double available_volume = show_cargo ? cargo_hold.AvailableCapacity()
                                : upgrade_space.AvailableCapacity();
    
    // Cargo mass renders your ship harder to manoeuver. Display it.
    double mass_percent = mass / base_mass * 100;
    const std::string mass_string = (boost::format("base %1%/ current %2% (%3$.0f%%)") % base_mass % mass % mass_percent).str();
    
    if (show_star_date) {
        return (boost::format("Stardate: %1$s      Credits: %2$.2f      "
                              "Space left: %3$.6g of %4$.6g cubic meters   Mass: %5%")
                              % date
                              % credits.Value()
                              % available_volume
                              % empty_volume
                              % mass_string).str();
    } else {
        return (boost::format("Credits: %1$.2f      "
                              "Space left: %2$.6g of %3$.6g cubic meters   Mass: %4%")
                              % credits.Value()
                              % available_volume
                              % empty_volume
                              % mass_string).str();
    }
}

bool ComponentsManager::BuyCargo(ComponentsManager *seller, Cargo *item, int quantity) {
    return _Buy(&cargo_hold, seller, item, quantity);
}

bool ComponentsManager::SellCargo(ComponentsManager *buyer, Cargo *item, int quantity) {
    return _Sell(&cargo_hold, buyer, item, quantity);
}

bool ComponentsManager::BuyUpgrade(ComponentsManager *seller, Cargo *item, int quantity) {
    return _Buy(&upgrade_space, seller, item, quantity);
}

bool ComponentsManager::SellUpgrade(ComponentsManager *buyer, Cargo *item, int quantity) {
    return _Sell(&upgrade_space, buyer, item, quantity);
}

bool ComponentsManager::_Buy(CargoHold *hold, ComponentsManager *seller, Cargo *item, int quantity) {
    // Some sanity checks
    int index = seller->cargo_hold.GetIndex(*item);
    if(index == -1) {
        return false;
    }

    // Check quantity is available in seller's hold
    quantity = std::min(item->GetQuantity(), quantity);

    // Check the maximum you can afford
    const double price = seller->PriceCargo(item->GetName());
    VS_LOG(trace, (boost::format("Buy price: %1% seller price: %2% percent: %3%%%") 
        % item->GetPrice() % price % (price/item->GetPrice())).str());

    int max_affordable_quantity = static_cast<int>(std::floor(credits / price));
    quantity = std::min(max_affordable_quantity, quantity);

    // Check the maximum you can fit in your hold, but only if volume != 0
    if(item->GetVolume() > 0) {
        int max_stackable_quantity = static_cast<int>(std::floor(hold->AvailableCapacity() / item->GetVolume()));
        quantity = std::min(max_stackable_quantity, quantity);
    }
    
    // Sanity check of the quantity - isn't 0 or negative
    if(quantity <= 0) {
        return false;
    }

    // Actual transaction
    Cargo sold_cargo = seller->cargo_hold.RemoveCargo(seller, index, quantity);
    ComponentsManager::credits -= price * quantity;
    hold->AddCargo(this, sold_cargo);
    return true;
}

/** This function is called by SellCargo to add variability to the sale price */
double ComponentsManager::PriceCargo(const std::string &cargo_name) {
    if(cargo_hold.HasCargo(cargo_name)) {
        Cargo cargo = cargo_hold.GetCargoByName(cargo_name);
        return cargo.GetPrice();
    }

    if(Manifest::MPL().HasCargo(cargo_name)) {
        Cargo cargo = Manifest::MPL().GetCargoByName(cargo_name);
        return cargo.GetPrice();
    }

    return configuration().cargo.space_junk_price_dbl;
}

bool ComponentsManager::_Sell(CargoHold *hold, ComponentsManager *buyer, Cargo *item, int quantity) {
    CargoHold *buyer_hold = &buyer->cargo_hold;

    int index = hold->GetIndex(*item);
    if(index == -1) {
        return false;
    }

    // Note: For now, NPCs can always afford to buy our stuff
    // Check the maximum you can fit in your hold
    // Disabled. Check returns a negative number for small items
    //int max_stackable_quantity = static_cast<int>(std::floor(buyer->cargo_hold.AvailableCapacity() / item->GetVolume()));
    //quantity = std::min(max_stackable_quantity, quantity);

    // Quantity sanity check - must be positive
    if (quantity <= 0) {
        return false;
    }

    Cargo cargo = hold->RemoveCargo(this, index, quantity);

    quantity = std::min(quantity, cargo.GetQuantity());

    const double price = buyer->PriceCargo(item->GetName());
    VS_LOG(trace, (boost::format("Sell price: %1% buyer price: %2% percent: %3%%%") 
        % item->GetPrice() % price % (price/item->GetPrice())).str());


    // Only get paid if not selling "mission" cargo.
    // i.e. other peoples' money
    if(!cargo.IsMissionFlag()) {
        credits += price * quantity;
    }

    buyer_hold->AddCargo(buyer, cargo);
    return true;
}

Component* ComponentsManager::GetComponentByType(const ComponentType type) {
    return const_cast<Component*>(
        static_cast<const ComponentsManager&>(*this).GetComponentByType(type)
    );
}

const Component* ComponentsManager::GetComponentByType(const ComponentType type) const {
    switch(type) {
        case ComponentType::Hull: return &hull;
        case ComponentType::Armor: return &armor;
        case ComponentType::Shield: return &shield;
        case ComponentType::Afterburner: return &afterburner;
        case ComponentType::AfterburnerUpgrade: return &afterburner_upgrade;
        case ComponentType::Drive: return &drive;
        case ComponentType::DriveUpgrade: return &drive_upgrade;
        case ComponentType::FtlDrive: return &ftl_drive;
        case ComponentType::JumpDrive: return &jump_drive;
        case ComponentType::Reactor: return &reactor;
        case ComponentType::Fuel: return &fuel;
        case ComponentType::Capacitor: return &energy;
        case ComponentType::FtlCapacitor: return &ftl_energy;
        
        case ComponentType::Cloak: return &cloak;
        case ComponentType::Radar: return &radar;
        case ComponentType::ECM: return &ecm;
        case ComponentType::RepairBot: return &repair_bot;
        default: return nullptr;
    }
}
