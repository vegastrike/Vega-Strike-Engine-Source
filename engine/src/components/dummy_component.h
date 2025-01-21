/*
 * dummy_component.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_DUMMY_COMPONENT_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_DUMMY_COMPONENT_H

#include "component.h"

/** A dummy component. Does nothing. Comes in useful in some places. */
class DummyComponent : public Component {
public:
    DummyComponent();

    // Component Methods
    virtual void Load(std::string unit_key);      
    
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;

    virtual bool CanDowngrade() const;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_key) const;

    virtual bool Upgrade(const std::string upgrade_key);
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_DUMMY_COMPONENT_H
