/**
 * energetic.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
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

#ifndef ENERGETIC_H
#define ENERGETIC_H


class Energetic
{
public:
    Energetic();

    void decreaseWarpEnergy( bool insys, float time );

    float energyData() const;
    float energyRechargeData() const;

    float fuelData() const;

    static float getFuelUsage( bool afterburner );
    void WCWarpIsFuelHack(bool transfer_warp_to_fuel);
    float ExpendMomentaryFuelUsage( float magnitude );
    float ExpendFuel(float quantity);
    float getWarpEnergy() const;

    void increaseWarpEnergy( bool insys, float time );

    float maxEnergyData() const;

    void rechargeEnergy();
    bool refillWarpEnergy();

    void setAfterburnerEnergy( float aft );
    void setEnergyRecharge( float enrech );
    void setFuel( float f );
    void setMaxEnergy( float maxen );
    float shieldRechargeData() const;


    float warpCapData() const;
    float warpEnergyData() const;





    // TODO: move to StarFaring class when available
    struct UnitJump
    {
        float warpDriveRating = 0;
        float energy = 100;                            //short fix
        float insysenergy;  //short fix
        signed char   drive = -2; // disabled
        unsigned char delay = 5;
        unsigned char damage = 0;
        //negative means fuel
    }
    jump;

    //current energy
    float  energy = 0;

    //how much the energy recharges per second
    float recharge = 0;

    //maximum energy
    float maxenergy = 0;
    //maximum energy
    float maxwarpenergy = 0; //short fix
    //current energy
    float warpenergy = 0;    //short fix
protected:
    //fuel of this unit
    float  fuel = 0;
    float  afterburnenergy = 0;              //short fix
    int    afterburntype = 0;   //0--energy, 1--fuel
    //-1 means it is off. -2 means it doesn't exist. otherwise it's engaged to destination (positive number)
};

#endif // ENERGETIC_H
