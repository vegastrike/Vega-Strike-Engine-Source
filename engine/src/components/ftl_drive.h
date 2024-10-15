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

#ifndef FTL_DRIVE_H
#define FTL_DRIVE_H

#include "component.h"
#include "energy_consumer.h"

class EnergyContainer;

class FtlDrive : public Component, public EnergyConsumer {
    // TODO: implement damage so something will actually happen
    // Right now, damage is recorded in component superclass but game doesn't
    // take it into account.
public:
    FtlDrive();
    FtlDrive(EnergyContainer *source);

    bool Enabled() const;

    // Component Methods
    virtual void Load(std::string upgrade_key, 
                      std::string unit_key, 
                      double ftl_factor = 1.0);      
    
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;

    virtual std::string Describe() const; // Describe component in base_computer 

    virtual bool CanDowngrade() const;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_name) const;

    virtual bool Upgrade(const std::string upgrade_name);
};

#endif // FTL_DRIVE_H
