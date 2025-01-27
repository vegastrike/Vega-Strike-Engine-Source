/*
 * ftl_drive.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_FTL_DRIVE_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_FTL_DRIVE_H

#include "component.h"
#include "energy_consumer.h"

class EnergyContainer;

class FtlDrive : public Component, public EnergyConsumer {
    bool enabled;

    // TODO: implement damage so something will actually happen
    // Right now, damage is recorded in component superclass but game doesn't
    // take it into account.
public:
    FtlDrive();
    FtlDrive(EnergyContainer *source);

    void Enable();
    void Disable();
    void Toggle();
    bool Enabled() const;

    // Component Methods
    void Load(std::string unit_key) override;      
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_FTL_DRIVE_H
