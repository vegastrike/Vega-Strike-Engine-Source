/*
 * component_pricing.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "cmd/component_pricing.h"

#include <cmath>  //For std::fabs.
#include <string>

#include <boost/format.hpp>

#include "cmd/unit_csv_factory.h"
#include "cmd/unit_generic.h"
#include "components/cloak.h"
#include "components/component_utils.h"
#include "components/energy_container.h"
#include "components/player_ship.h"
#include "components/reactor.h"
#include "components/repair_bot.h"
#include "components/shield.h"
#include "components/ship_functions.h"
#include "configuration/configuration.h"
#include "resource/manifest.h"
#include "src/vega_cast_utils.h"

// One family of parts the shops sell in a ranked ladder: the units.json key prefix and the
// column the ladder is ranked by. The tiers are found by asking the manifest, so a ladder the
// assets extend needs no change here.
struct ComponentLadder {
    const char *prefix;
    const char *figure;
};

// The families a component type is sold in. The sensors are sold in five, which is why this is a
// list: a component is priced at the nearest tier of any family that applies to it.
struct ComponentLadders {
    ComponentType          type;
    const ComponentLadder *families;
    int                    count;
};

static const ComponentLadder kReactorLadders[] = {
        {"reactor", "Reactor_Recharge"},
};
static const ComponentLadder kShieldLadders[] = {
        {"quadshield", "Shield_Recharge"},
        {"dualshield", "Shield_Recharge"},
};
static const ComponentLadder kArmorLadders[] = {
        {"armor", "armor"},
};
static const ComponentLadder kCapacitorLadders[] = {
        {"capacitor", "Primary_Capacitor"},
};
static const ComponentLadder kRadarLadders[] = {
        {"skyscope", "Radar_Range"}, {"starscanner", "Radar_Range"},  {"hawkeye", "Radar_Range"},
        {"surveyor", "Radar_Range"}, {"rlaantracker", "Radar_Range"},
};
static const ComponentLadder kEcmLadders[] = {
        {"ecm_package", "ecm"},
};
static const ComponentLadder kRepairBotLadders[] = {
        {"repair_droid", "repair"},
};
static const ComponentLadder kJumpDriveLadders[] = {
        {"jump_drive", "Jump_Drive_Present"},
};

static const ComponentLadders kComponentLadders[] = {
        {ComponentType::Reactor, kReactorLadders, 1},     {ComponentType::Shield, kShieldLadders, 2},
        {ComponentType::Armor, kArmorLadders, 1},         {ComponentType::Capacitor, kCapacitorLadders, 1},
        {ComponentType::Radar, kRadarLadders, 5},         {ComponentType::ECM, kEcmLadders, 1},
        {ComponentType::RepairBot, kRepairBotLadders, 1}, {ComponentType::JumpDrive, kJumpDriveLadders, 1},
};

// The figure a component's ladder is ranked by - the rated recharge of a reactor or a shield, the
// rating of an armour facet, a capacitor's capacity, a radar's range, the strength of an ECM or a
// repair system. Zero when the component has no figure to be ranked by, and so no tier to be
// priced at.
static double ComponentFigure(const Component *component) {
    switch (component->type) {
        case ComponentType::Reactor:
            return vega_dynamic_const_cast_ptr<const Reactor>(component)->MaxCapacity();
        case ComponentType::Shield:
            return vega_dynamic_const_cast_ptr<const Shield>(component)->MaxRegeneration();
        case ComponentType::Armor:
            return vega_dynamic_const_cast_ptr<const DamageableLayer>(component)->AverageMaxLayerValue();
        case ComponentType::Capacitor:
            return vega_dynamic_const_cast_ptr<const EnergyContainer>(component)->MaxLevel();
        case ComponentType::Radar:
            return vega_dynamic_const_cast_ptr<const CRadar>(component)->GetMaxRange();
        case ComponentType::ECM:
            return vega_dynamic_const_cast_ptr<const ECM>(component)->Get();
        case ComponentType::RepairBot:
            return vega_dynamic_const_cast_ptr<const RepairBot>(component)->Get();
        case ComponentType::JumpDrive:
            // Presence is the figure: the shops sell one jump drive.
            return 1.0;
        default:
            return 0.0;
    }
}

// The families this component's type is sold in, or nullptr when its type is not sold in one.
static const ComponentLadders *ComponentLaddersFor(const Component *component) {
    for (const ComponentLadders &candidate : kComponentLadders) {
        if (candidate.type == component->type) {
            return &candidate;
        }
    }
    return nullptr;
}

// A shield is sold as a two-facet or a four-facet part, ranked the same way either way, so it is
// priced against the ladder matching its facets. Every other type has one ladder.
static bool LadderApplies(const Component *component, const ComponentLadder &ladder) {
    if (component->type != ComponentType::Shield) {
        return true;
    }

    const int         facets = vega_dynamic_const_cast_ptr<const DamageableLayer>(component)->NumberOfFacets();
    const std::string prefix = ladder.prefix;
    return facets <= 2 ? prefix == "dualshield" : prefix == "quadshield";
}

// Whether the shops list a part at all. A name they do not know is not a price.
// Manifest::HasCargo() accepts a units.json key with or without its __upgrades suffix.
static bool PartIsListed(const std::string &key) {
    return Manifest::MPL().HasCargo(key);
}

// What a new part costs, which is what putting a part right is priced against. A name the shops
// do not list has no new price of its own.
static double NewPartPrice(const std::string &name) {
    if (!PartIsListed(name)) {
        return 0.0;
    }
    return Manifest::MPL().GetCargoByName(name).GetPrice();
}

// The shops write a ladder's tier in one of three ways: <prefix>01__upgrades for most families,
// <prefix>1__upgrades for the sensors, and bare <prefix>__upgrades for the one-tier jump drive.
// Ask the manifest rather than format a guess into it: two of the three spellings were wrong when
// this was built from a format string.
static std::string LadderTierKey(const std::string &prefix, int tier) {
    const std::string padded = (boost::format("%1%%2$02d__upgrades") % prefix % tier).str();
    if (Manifest::MPL().HasCargo(padded)) {
        return padded;
    }

    const std::string plain = (boost::format("%1%%2$d__upgrades") % prefix % tier).str();
    if (Manifest::MPL().HasCargo(plain)) {
        return plain;
    }

    // A one-tier family carries no number at all, and has nothing above it to find.
    const std::string bare = prefix + "__upgrades";
    return tier == 1 && Manifest::MPL().HasCargo(bare) ? bare : std::string();
}

// The price of the tier nearest this component's figure, or zero when its type is not sold in a
// ladder, it has no figure to rank, or its figure is beyond the ladder - which is the capital
// ship case, and is priced against the ship instead.
static double NearestTierPrice(Unit *base_unit, const Component *component) {
    const ComponentLadders *ladders = ComponentLaddersFor(component);
    const double            figure  = ComponentFigure(component);
    if (ladders == nullptr || figure <= 0.0) {
        return 0.0;
    }

    double nearest_tier_price = 0.0;
    double nearest_distance   = -1.0;
    double lowest_figure      = -1.0;
    double highest_figure     = -1.0;
    for (int family = 0; family < ladders->count; ++family) {
        const ComponentLadder &ladder = ladders->families[family];
        if (!LadderApplies(component, ladder)) {
            continue;
        }

        // Tiers are numbered from one with no gaps, so the first one the shops do not list is the
        // end of the ladder.
        for (int tier = 1;; ++tier) {
            const std::string tier_key = LadderTierKey(ladder.prefix, tier);
            if (tier_key.empty()) {
                break;
            }

            const double tier_figure = UnitCSVFactory::GetVariable(tier_key, ladder.figure, 0.0);
            if (tier_figure <= 0.0) {
                continue;
            }
            if (lowest_figure < 0.0 || tier_figure < lowest_figure) {
                lowest_figure = tier_figure;
            }
            if (tier_figure > highest_figure) {
                highest_figure = tier_figure;
            }
            const double distance = std::fabs(tier_figure - figure);
            if (nearest_distance < 0.0 || distance < nearest_distance) {
                nearest_distance   = distance;
                nearest_tier_price = base_unit->PriceCargo(tier_key);
            }
        }
    }

    if (lowest_figure < 0.0 || figure < lowest_figure || figure > highest_figure) {
        return 0.0;
    }
    return nearest_tier_price;
}

// What a component is worth. A component the ship bought is worth its own part. One with no part
// of its own - the hull, the armour, the shields, and the rest of what the ship came with - is
// worth the part the ship carries for it, else the nearest tier the shops would sell, and where
// there is neither, a share of the ship's own price: capital ships mount equipment the shops
// never stock.
double ComponentPrice(Unit *player_unit, Unit *base_unit, const Component *component, double ship_price) {
    const std::string part_key = component->GetUpgradeKey();
    if (!part_key.empty() && PartIsListed(part_key)) {
        return base_unit->PriceCargo(part_key);
    }

    // The part the ship carries for this component is the part that would replace it, so it is
    // what the component is worth - and it is exact, where a tier or a share is a guess.
    if (player_unit != nullptr) {
        const std::string carried_key = player_unit->CarriedPartKey(component);
        if (!carried_key.empty()) {
            const double carried_price = NewPartPrice(carried_key);
            if (carried_price > 0.0) {
                return carried_price;
            }
        }
    }

    const double tier_price = NearestTierPrice(base_unit, component);
    if (tier_price > 0.0) {
        return tier_price;
    }

    return configuration().economics.component_share_dbl * ship_price;
}

// What putting a component right takes: repairing it, or replacing it when Repair() cannot - a
// destroyed one. The decision and the price are made in one place, or the price on the row and
// the base's charge come out different.
// A base charges more for its work on a harder difficulty, as it always has. This is the charge
// for the work, not for the hardware: a replacement part costs what it costs whatever the
// difficulty, as it would if the player bought it from the shop.
static double ServiceCharge(double value) {
    return value * g_game.difficulty;
}

// What putting a component right costs. A damaged one is repaired, costing its value in
// proportion to its damage; a destroyed one is replaced whole, which costs its value in full.
// Nothing caps the bill: a wreck can cost more to put right than the ship is worth, as a car can.
ServiceQuote ServiceQuoteFor(Unit *player_unit, Unit *base_unit, const Component *component, double ship_price) {
    const double value = ComponentPrice(player_unit, base_unit, component, ship_price);
    // Nothing left of it: Resource::RepairFully() refuses a destroyed component, so a base
    // replaces one rather than repairing it.
    if (component->PercentOperational() <= 0.0) {
        return ServiceQuote{value, true};
    }
    return ServiceQuote{ServiceCharge(value * ComponentDamage(component)), false};
}

// The price of the ship the player is flying, which is what a component with no price of its own
// is valued against.
double PlayerShipPrice(void) {
    try {
        return PlayerShip::GetActiveShip().ShipPrice();
    } catch (const NoActiveShipNotFoundException &) {
        return 0.0;
    }
}

// What one round of Basic Repair costs: every damaged component put right, at the price the base
// will charge for it.
double BasicRepairPrice(Unit *player_unit, Unit *base_unit) {
    const double ship_price = PlayerShipPrice();
    double       price      = 0.0;
    for (const HudText &component : player_unit->DamagedComponents()) {
        price += ServiceQuoteFor(player_unit, base_unit, component.component, ship_price).price;
    }
    return price;
}

double GetOperational(Unit *playerUnit, const Cargo *item) {
    ComponentType type      = GetComponentTypeFromName(item->GetName());
    Component    *component = playerUnit->GetComponentByType(type);

    if (component) {
        return component->PercentOperational();
    }

    return 0.0;
}

// What a new part costs, which is what putting a part right is priced against. A name the shops
// do not list has no new price of its own.
// What the base charges to put a part right: what a new one costs, in proportion to how much of
// this one is damaged. What the player paid for this part does not come into it - a cheap
// second-hand part costs as much to mend as a new one - and the difficulty scales the work, as
// it does for a component.
double RepairPrice(Unit *playerUnit, const Cargo *item) {
    Component   *component = playerUnit->GetComponentByType(GetComponentTypeFromName(item->GetName()));
    const double damage    = component != nullptr ? ComponentDamage(component) : 0.0;
    return ServiceCharge(NewPartPrice(item->GetName()) * damage);
}
