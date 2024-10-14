/**
 * energetic.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ENERGETIC_H
#define VEGA_STRIKE_ENGINE_CMD_ENERGETIC_H

#include "resource/resource.h"

class Energetic {
public:
    Energetic();
    virtual ~Energetic() = default;

    void DecreaseWarpEnergyInWarp();


    static float getFuelUsage(bool afterburner);
    void WCWarpIsFuelHack(bool transfer_warp_to_fuel);
    float ExpendMomentaryFuelUsage(float magnitude);
    float ExpendFuel(double quantity);
    void ExpendEnergy(const bool player_ship);
    void ExpendEnergy(float usage);
    void ExpendEnergyToRechargeShields();
    void ExpendFuel();
    float getWarpEnergy() const;


    float maxEnergyData() const;

    void MaintainECM();
    void MaintainShields();

    void rechargeEnergy();
    void RechargeWarpCapacitors(const bool player_ship);

    void setAfterburnerEnergy(float aft);
    void setEnergyRecharge(float enrech);

    float totalShieldEnergyCapacitance() const;

    static float VSDPercent();

    float warpCapData() const;

    float WarpEnergyMultiplier(const bool player_ship);


    float constrained_charge_to_shields;
    bool sufficient_energy_to_recharge_shields;

    float afterburnenergy;              //short fix
    int afterburntype;   //0--energy, 1--fuel
    //-1 means it is off. -2 means it doesn't exist. otherwise it's engaged to destination (positive number)
};

#endif //VEGA_STRIKE_ENGINE_CMD_ENERGETIC_H
