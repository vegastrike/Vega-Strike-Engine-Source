/*
 * energetic.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ENERGETIC_H
#define VEGA_STRIKE_ENGINE_CMD_ENERGETIC_H

#include "resource/resource.h"

class Energetic {
public:
    Energetic();
    virtual ~Energetic() = default;

    void DecreaseWarpEnergyInWarp();


    static float getFuelUsage(bool afterburner);

    void RechargeWarpCapacitors(const bool player_ship);

    void setEnergyRecharge(float enrech);

    static float VSDPercent();



    float constrained_charge_to_shields;
    bool sufficient_energy_to_recharge_shields;

    // TODO: delete one and move the other to Afterburner class
    int afterburntype;   //0--energy, 1--fuel
    //-1 means it is off. -2 means it doesn't exist. otherwise it's engaged to destination (positive number)
};

#endif //VEGA_STRIKE_ENGINE_CMD_ENERGETIC_H
