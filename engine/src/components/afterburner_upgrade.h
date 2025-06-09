/*
 * afterburner_upgrade.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_AFTERBURNER_UPGRADE_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_AFTERBURNER_UPGRADE_H

#include "component.h"

class Afterburner;

/** An AfterburnerUpgrade applies a modifier to Afterburner class.
 *  This is the same use case as DriveUpgrade. 
 *  The game previously supported both additive and multiplicative upgrades.
 *  I've removed the additive one for simplicity's sake. 
 *  The default value is 1.0 (no change).
 *  The upgrade can't be damaged. Instead, the afterburner itself can be damaged.
 */
class AfterburnerUpgrade : public Component {
    Afterburner *afterburner;
public:
    //after burner acceleration 
    double thrust;
    double speed;
    double consumption;

    AfterburnerUpgrade(Afterburner *afterburner = nullptr);
    
    double MaxAfterburnerSpeed() const;

    // Component Methods
    void Load(std::string unit_key) override;
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_AFTERBURNER_UPGRADE_H
