/*
 * hull.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2024 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_HULL_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_HULL_H

#include "component.h"
#include "damage/damageable_layer.h"

/**
 * This is a minimum implementation of hull class.
 * A hull cannot be upgraded, only repaired.
 * We keep these functions to make the hull a separate component.
 * It cannot be sold (downgraded) but can be repaired.
 */
class Hull : public Component, public DamageableLayer {
    friend class Unit;
public:
    Hull();

    virtual void Load(std::string upgrade_key, std::string unit_key, 
                      Unit *unit);      // Load from dictionary
    virtual std::string SaveToJSON() const;   // Save component as JSON

    virtual std::string Describe() const; // Describe component in base_computer 

    virtual bool CanDowngrade() const;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_name) const;

    virtual bool Upgrade(const std::string upgrade_name);

    virtual void Damage();
    virtual void Repair();

    virtual bool Damaged() const;
    virtual bool Installed() const;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_HULL_H