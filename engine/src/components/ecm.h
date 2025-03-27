/*
 * ecm.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_ECM_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_ECM_H

#include "component.h"
#include "components/energy_consumer.h"

class EnergyContainer;

/** A dummy component. Does nothing. Comes in useful in some places. */
class ECM : public Component, public EnergyConsumer {
    // TODO: take a deeper look at this much later...
    //how likely to fool missiles
    Resource<int> ecm;
    bool active;
public:
    ECM();
    explicit ECM(EnergyContainer *source);
    ~ECM() override;

    // Component Methods
    void Load(std::string unit_key) override;

    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    void Damage() override;
    void Repair() override;

    // EnergyConsumer override
    double Consume() override;

    // ECM Methods
    bool Active() const;
    bool BreakLock(void* missile) const;
    int Get() const;
    void Set(int ecm);
    void Toggle();

    void _upgrade(const std::string key);
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_ECM_H
