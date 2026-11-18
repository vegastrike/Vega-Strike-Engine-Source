/*
 * component_pricing.h
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

#ifndef VEGA_STRIKE_ENGINE_CMD_COMPONENT_PRICING_H
#define VEGA_STRIKE_ENGINE_CMD_COMPONENT_PRICING_H

class Cargo;
class Component;
class Unit;

// What a base charges to put one of a ship's own components right, and whether it repairs the
// part or replaces it - a destroyed component is a replacement, which is what a base can do and a
// field repair cannot.
struct ServiceQuote {
    double price;
    bool   replacement;
};

// What a ship is worth to sell, the same figure its own price list shows.
double PlayerShipPrice();

// The quote for servicing one component.
ServiceQuote ServiceQuoteFor(Unit *player_unit, Unit *base_unit, const Component *component, double ship_price);

// What one component is worth: the part it came with, the part the ship carries for it, the
// nearest tier the shops sell, or a share of the ship's own price.
double ComponentPrice(Unit *player_unit, Unit *base_unit, const Component *component, double ship_price);

// What putting every damaged component right would cost, in total.
double BasicRepairPrice(Unit *player_unit, Unit *base_unit);

// What the base charges to put the part named by an upgrade or cargo item right.
double RepairPrice(Unit *player_unit, const Cargo *item);

// How much of a component still works, as a fraction.
double GetOperational(Unit *player_unit, const Cargo *item);

#endif  // VEGA_STRIKE_ENGINE_HEADER_COMPONENT_PRICING_H
