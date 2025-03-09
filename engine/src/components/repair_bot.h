/*
 * repair_bot.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_REPAIR_BOT_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_REPAIR_BOT_H

#include "component.h"

/** A robot that can access ship systems and make some repairs. */
class RepairBot : public Component {
    // Repair bot type. 0 is no bot;
    unsigned char repair_bot;
    float next_repair_time;
    unsigned int next_repair_cargo;    // (~0 : select randomly)
public:
    RepairBot();

    // Component Methods
    void Load(std::string unit_key) override;      
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    // Bot Methods
    int Get() const;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_REPAIR_BOT_H
