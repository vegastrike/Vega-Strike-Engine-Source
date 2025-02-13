/*
 * upgradeable_unit.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "upgradeable_unit.h"


#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "components/component.h"
#include "components/component_utils.h"
#include "unit_const_cache.h"
#include "faction_generic.h"
#include "unit_generic.h"
#include "weapon_info.h"
#include "vega_cast_utils.h"
#include "vs_logging.h"
#include "unit_csv_factory.h"

std::vector<std::string> ParseUnitUpgrades(const std::string &upgrades) {
    if(upgrades.size() == 0) {
        return std::vector<std::string>();
    }

    std::string upg_string = std::string(upgrades);

    // Expected upgrades string is in the format of {<upgrade>;<mount offset>;<subunit offset}{...}
    // Trim leading and trailing spaces
    boost::algorithm::trim(upg_string);

    // Remove all spaces and {
    boost::replace_all(upg_string, " ", "");
    boost::replace_all(upg_string, "{", "");

    // Remove last }
    boost::replace_last(upg_string, "}", "");

    const std::string delimiter = "}";

    std::vector<std::string> upgrades_vector;
    boost::split(upgrades_vector, upg_string, boost::is_any_of(delimiter));

    return upgrades_vector;
}

// A simple struct for holding all the data
// TODO: figure out what the numbers mean
// TODO: move this to Cargo or a subclass of cargo
// TODO: separate cargo from upgrades 
struct CargoUpgrade {
    std::string name;
    std::string category;

    CargoUpgrade(const std::string upgrade_string) {
        if(upgrade_string.empty()) {
            return;
        }

        const std::string delimiter = ";";

        std::vector<std::string> upgrade_vector;
        boost::split(upgrade_vector, upgrade_string, boost::is_any_of(delimiter));

        // TODO: figure out format and change 3 to the real size and implement the rest
        if(upgrade_vector.size() < 3) {
            return;
        }

        name = upgrade_vector.at(0);
        category = upgrade_vector.at(1);
    }
};

// TODO: why do we have to use such kludges?!
unsigned int convert_to_int(std::string s) {
    if(s.size() == 0) {
        return 0;
    }

    try {
        return std::stoi(s);
    } catch(...) {
        return 0;
    }
}

UpgradeableUnit::UpgradeableUnit()
{

}

extern int GetModeFromName(const char *input_buffer);



UpgradeOperationResult UpgradeableUnit::UpgradeUnit(const std::string upgrade_name,
                     bool upgrade, bool apply) {
    Unit* unit = vega_dynamic_cast_ptr<Unit>(this);
    const std::string upgrade_key = upgrade_name + UPGRADES_SUFFIX;
    const ComponentType component_type = GetComponentTypeFromName(upgrade_name);
    
    UpgradeOperationResult result;

    switch(component_type) {
        case ComponentType::Armor:
            result.upgradeable = true;
            result.success = unit->armor.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case ComponentType::Shield:
            result.upgradeable = true;
            result.success = unit->shield.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;

        case ComponentType::Capacitor:
            result.upgradeable = true;
            result.success = unit->energy.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case ComponentType::FtlCapacitor:
            result.upgradeable = true;
            result.success = unit->ftl_energy.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case ComponentType::Reactor:
            result.upgradeable = true;
            result.success = unit->reactor.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;

        case ComponentType::Afterburner: break; // Integrated
        case ComponentType::AfterburnerUpgrade:
            result.upgradeable = true;
            result.success = unit->afterburner_upgrade.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case ComponentType::Drive: break; // Integrated
        case ComponentType::DriveUpgrade:
            result.upgradeable = true;
            result.success = unit->drive_upgrade.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case ComponentType::FtlDrive:
            result.upgradeable = true;
            result.success = unit->ftl_drive.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;
        
        case ComponentType::JumpDrive:
            result.upgradeable = true;
            result.success = unit->jump_drive.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;

        case ComponentType::Cloak:
            result.upgradeable = true;
            result.success = unit->cloak.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;

        
        /*case UpgradeType::ECM:
            result.upgradeable = true;
            result.success = unit->ecm.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case UpgradeType::Radar:
            result.upgradeable = true;
            result.success = unit->radar.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;*/

        /*case UpgradeType::Repair_Droid:
            result.upgradeable = true;
            result.success = unit->repair_droid.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;*/

        default:
            //std::cout << "Unhandled type for " << upgrade_name << std::endl;
            break;
    }

    return result;
}

bool UpgradeableUnit::RepairUnit(const std::string upgrade_name) {
    Unit* unit = vega_dynamic_cast_ptr<Unit>(this);
    const std::string upgrade_key = upgrade_name + UPGRADES_SUFFIX;
    const ComponentType component_type = GetComponentTypeFromName(upgrade_name);

    switch(component_type) {
        case ComponentType::Hull:
            if(unit->hull.Damaged()) {
                unit->hull.Repair();
                return true;
            }
            break;
        case ComponentType::Armor:
            if(unit->armor.Damaged()) {
                unit->armor.Repair();
                return true;
            }
            break;
        case ComponentType::Shield:
            if(unit->shield.Damaged()) {
                unit->shield.Repair();
                return true;
            }
            break;
        case ComponentType::Capacitor:
            if(unit->energy.Damaged()) {
                unit->energy.Repair();
                return true;
            }
            break;
        case ComponentType::FtlCapacitor:
            if(unit->ftl_energy.Damaged()) {
                unit->ftl_energy.Repair();
                return true;
            }
            break;
        case ComponentType::Reactor:
            if(unit->reactor.Damaged()) {
                unit->reactor.Repair();
                return true;
            }
            break;
        case ComponentType::Afterburner: 
            if(unit->afterburner.Damaged()) {
                unit->afterburner.Repair();
                return true;
            }
            break;
        case ComponentType::AfterburnerUpgrade:
            if(unit->afterburner_upgrade.Damaged()) {
                unit->afterburner_upgrade.Repair();
                return true;
            }
            break;
        case ComponentType::Drive:
            if(unit->drive.Damaged()) {
                unit->drive.Repair();
                return true;
            }
            break;
        case ComponentType::DriveUpgrade:
            if(unit->drive_upgrade.Damaged()) {
                unit->drive_upgrade.Repair();
                return true;
            }
            break;
        case ComponentType::FtlDrive:
            if(unit->ftl_drive.Damaged()) {
                unit->ftl_drive.Repair();
                return true;
            }
            break;
        case ComponentType::JumpDrive:
            if(unit->jump_drive.Damaged()) {
                unit->jump_drive.Repair();
                return true;
            }
            break;
        case ComponentType::Cloak:
            if(unit->cloak.Damaged()) {
                unit->cloak.Repair();
                return true;
            }
            break;

        
        /*case UpgradeType::ECM:
            result.upgradeable = true;
            result.success = unit->ecm.CanWillUpDowngrade(upgrade_key, upgrade, apply);    
            break;
        case UpgradeType::Radar:
            result.upgradeable = true;
            result.success = unit->radar.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;*/

        /*case UpgradeType::Repair_Droid:
            result.upgradeable = true;
            result.success = unit->repair_droid.CanWillUpDowngrade(upgrade_key, upgrade, apply);
            break;*/

        default:
            //std::cout << "Unhandled type for " << upgrade_name << std::endl;
            break;
    }

    return false;
}

// TODO: remove unit parameter
void UpgradeableUnit::UpgradeUnit(const std::string &upgrades) {
    const std::string delimiter = ";";

    std::vector<std::string> upgrades_vector = ParseUnitUpgrades(upgrades);

    for(const std::string& upgrade : upgrades_vector) {
        std::vector<std::string> upgrade_vector;
        boost::split(upgrade_vector, upgrade, boost::is_any_of(delimiter));

        const unsigned int mount_offset = convert_to_int(upgrade_vector[1]);
        const unsigned int subunit_offset = convert_to_int(upgrade_vector[2]);

        const Unit *upgradee = UnitConstCache::getCachedConst(StringIntKey(upgrade, FactionUtil::GetUpgradeFaction()));
        if (!upgradee) {
            upgradee = UnitConstCache::setCachedConst(StringIntKey(upgrade, FactionUtil::GetUpgradeFaction()),
                    new Unit(upgrade.c_str(),
                            true,
                            FactionUtil::GetUpgradeFaction()));
        }

        double percent = 1.0;
        const int mode = GetModeFromName(upgrade.c_str());

        // TODO: change this when we make this a sub-class of unit
        Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
        unit->Upgrade(upgradee, mount_offset, subunit_offset, mode, true, percent, nullptr);

        // Code to handle DriveUpgrade and AfterburnerUpgrade
        // Should eventually replace all the code above
        CargoUpgrade cargo_upgrade(upgrade);
        ComponentType component_type = GetComponentTypeFromName(cargo_upgrade.name);
        if(component_type == ComponentType::AfterburnerUpgrade) {
            unit->afterburner_upgrade.Load(cargo_upgrade.name + "__upgrades");
        } else if(component_type == ComponentType::DriveUpgrade) {
            unit->drive_upgrade.Load(cargo_upgrade.name + "__upgrades");
        }
    }
}

const Unit *getUnitFromUpgradeName(const string &upgradeName, int myUnitFaction = 0);

static const std::string LOAD_FAILED = "LOAD_FAILED";


bool UpgradeableUnit::UpgradeMounts(const Unit *up,
                                    int mountoffset,
                                    bool touchme,
                                    bool downgrade,
                                    int &numave,        // number of used parts?
                                    double &percentage) // percentage of used parts
{
    Unit* unit = vega_dynamic_cast_ptr<Unit>(this);
    const int num_mounts = unit->getNumMounts();
    const int num_up_mounts = up->getNumMounts();

    // Some basic checks
    if(!up) {
        return false;
    }

    // All weapons come with one mount at least
    if(num_up_mounts == 0) {
        return true;
    }

    // there needs to be some mounts to be able to mount to
    if (num_mounts == 0) {
        // would be nice to make this more meaningful but that's a little harder given
        // the casting of `unit` from `this`.
        VS_LOG(debug, "No mounts to attach to.");
        return false;
    }

    int j = mountoffset;
    int i = 0;
    bool cancompletefully = true;

    int jmod = j % num_mounts; // Apparently, the game can specify a mount offset greater tan mount points

    Mount* upgraded_mount = &unit->mounts[jmod];
    const Mount* upgrading_mount = &up->mounts[i];

    //only continue if the upgrador has active mounts
    if(upgrading_mount->status == Mount::DESTROYED || upgrading_mount->status == Mount::UNCHOSEN) {
        return false;
    }

    //make sure since we're offsetting the starting we don't overrun the mounts
    bool is_ammo = (string::npos != string(up->name).find("_ammo"));             //is this ammo for a weapon rather than an actual weapon

    // Upgrade
    if (!downgrade) {

        // Standard upgrade - autotracking and tractor capability are marked with MOUNT_UPGRADE
        if (upgrading_mount->type->name.find("_UPGRADE") == string::npos) {
            //check for capability increase rather than actual weapon upgrade
            //only look at this mount if it can fit in the rack
            if (upgraded_mount->MountFits(*upgrading_mount)) {
                if (!upgrading_mount->SameWeapon(upgraded_mount) || upgraded_mount->status
                        == Mount::DESTROYED || upgraded_mount->status == Mount::UNCHOSEN) {

                    return upgraded_mount->CanUpgradeMount(upgrading_mount, unit, is_ammo, numave, percentage, touchme);
                } else {
                    if (is_ammo && upgrading_mount->type->name == upgraded_mount->type->name) {
                        //if is ammo and is same weapon type
                        if (upgraded_mount->ammo != -1 && upgrading_mount->ammo != -1) {
                            std::string ammoname = up->name.get();
                            std::size_t ammopos = ammoname.find("_ammo");
                            std::string weaponname = ammoname.substr(0, ammopos);

                            // Do NOT delete this Unit because it will be either fetched
                                    // from a cache or - if it has to be created - it will
                                    // be automatically put in a cache.
                                    // Deletion will corrupt the cache!

                            const Unit *weapon = getUnitFromUpgradeName(weaponname);

                            cancompletefully = upgraded_mount->AddAmmo(upgraded_mount, weapon, touchme);
                        }
                    } else {
                        cancompletefully = false;
                    }
                }
            } else {
                //since we cannot fit the mount in the slot we cannot complete fully
                cancompletefully = false;
            }
        } else { // Add capability to mount
            // Modify mount size (e.g. tractor_capability__upgrades but also autotracking. See mount_size.cpp)
            unsigned int siz = 0;
            siz = ~siz;

            if (((siz & upgrading_mount->size) | upgraded_mount->size) != upgraded_mount->size) {
                if (touchme) {
                    upgraded_mount->size |= upgrading_mount->size;
                }
                ++numave;
                ++percentage;
            } else {
                cancompletefully = false;
            }
            //we need to |= the mount type
        }
    }             //DOWNGRADE
    else {
        if (upgrading_mount->type->name != "MOUNT_UPGRADE") {

            bool found = false;                     //we haven't found a matching gun to remove
            ///go through all guns
            for (unsigned int k = 0; k < (unsigned int) num_mounts; ++k) {
                //we want to start with bias
                int jkmod = (jmod + k) % num_mounts;
                if (Mount::UNCHOSEN == unit->mounts[jkmod].status) {
                    //can't sell weapon that's already been sold/removed
                    continue;
                }
                ///search for right mount to remove starting from j. this is the right name
                if (boost::iequals(unit->mounts[jkmod].type->name.c_str(),
                               upgrading_mount->type->name.c_str())) {
                    //we got one, but check if we're trying to sell non-existent ammo
                    if (is_ammo && unit->mounts[jkmod].ammo <= 0) {
                        //whether it's gun ammo or a missile, you can't remove ammo from an infinite source, and you can't remove ammo if there isn't any
                        continue;
                    } else {
                        found = true;
                    }
                    ///calculate scrap value (if damaged)
                    percentage += unit->mounts[jkmod].Percentage(upgrading_mount);
                    //if we modify
                    if (touchme) {
                        //if downgrading ammo based upgrade, checks for infinite ammo
                        if (is_ammo && upgrading_mount->ammo && upgrading_mount->ammo != -1
                                && unit->mounts[jkmod].ammo != -1) {
                            //remove upgrade-worth, else remove remaining
                            unit->mounts[jkmod].ammo -=
                                    (unit->mounts[jkmod].ammo >= upgrading_mount->ammo) ? upgrading_mount->ammo
                                                                                     : unit->mounts[jkmod].ammo;
                            //if none left
                            if (!unit->mounts[jkmod].ammo) {
                                ///deactivate weapon
                                if (upgrading_mount->IsMissileMount()) {
                                    unit->mounts[jkmod].status = Mount::UNCHOSEN;
                                }
                            }
                        } else {
                            ///deactivate weapon
                            unit->mounts[jkmod].status = Mount::UNCHOSEN;
                            unit->mounts[jkmod].ammo = -1;                                     //remove all ammo
                        }
                    }
                    break;
                }
            }
            if (!found) {
                //we did not find a matching weapon to remove
                cancompletefully = false;
            }
        } else {
            cancompletefully = false;
        }
    }
    return cancompletefully;
}

