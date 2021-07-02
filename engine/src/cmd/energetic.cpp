/**
 * energetic.cpp
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

#include "energetic.h"

#include "configuration/configuration.h"
#include "damageable.h"
#include "vegastrike.h"

/* This class provides all energy generation methods to unit types -
 * ships, space installations, missiles, drones, etc. */


Energetic::Energetic(): energy(0),
    recharge(0),
    maxenergy(0),
    maxwarpenergy(0),
    warpenergy(0),
    fuel(0),
    afterburnenergy(0),
    afterburntype(0)
{
    jump.warpDriveRating = 0;
    jump.energy = 100;
    jump.insysenergy = configuration.warp.insystem_jump_cost * jump.energy;
    jump.drive = -2;
    jump.delay = 5;
    jump.damage = 0;
}

void Energetic::decreaseWarpEnergy( bool insys, float time )
{
    static float bleed_factor = configuration.physics.bleed_factor;
    static bool  wc_fuel_hack  = configuration.fuel.fuel_equals_warp;
    if (wc_fuel_hack)
        this->warpenergy = this->fuel;
    this->warpenergy -= (insys ? jump.insysenergy/bleed_factor : jump.energy)*time;
    if (this->warpenergy < 0)
        this->warpenergy = 0;
    if (wc_fuel_hack)
        this->fuel = this->warpenergy;
}

float Energetic::energyData() const
{
    const Damageable *damageable = reinterpret_cast<const Damageable*>(this);
    const Shield shield = damageable->shield;

    if (configuration.physics.max_shield_lowers_capacitance) {
        if ( maxenergy <= damageable->totalShieldEnergyCapacitance( shield ) )
            return 0;
        return ( (float) energy )/( maxenergy-damageable->totalShieldEnergyCapacitance( shield ) );
    } else {
        return ( (float) energy )/maxenergy;
    }
}

//Returns the current ammt of energy left
float Energetic::energyRechargeData() const
{
    return recharge;
}

float Energetic::fuelData() const
{
    return fuel;
}

float Energetic::getFuelUsage( bool afterburner )
{
    if (afterburner) {
        return configuration.fuel.afterburner_fuel_usage;
    }
    return configuration.fuel.normal_fuel_usage;
}

/**
 * @brief Energetic::WCWarpIsFuelHack - in Wing Commander, warp and fuel are the same variable.
 * Therefore, we need to transfer from one to the other to maintain equality
 * @param transfer_warp_to_fuel - true means fuel = warpenergy
 */
// TODO: this is still an ugly hack
void Energetic::WCWarpIsFuelHack(bool transfer_warp_to_fuel) {
    if (!configuration.fuel.fuel_equals_warp) {
        return;
    }

    if (transfer_warp_to_fuel) {
        fuel = warpenergy;
    } else {
        warpenergy = fuel;
    }
}

float Energetic::ExpendMomentaryFuelUsage( float magnitude )
{
    // TODO: have this make some kind of sense to someone other than the person who wrote the comment below.
    //HACK this forces the reaction to be Li-6+D fusion with efficiency governed by the getFuelUsage function
    float quantity = Energetic::getFuelUsage( false ) * simulation_atom_var * magnitude *
            configuration.fuel.fmec_exit_velocity_inverse / configuration.fuel.fuel_efficiency;

    return ExpendFuel(quantity);
}

/**
 * @brief expendFuel - reduce fuel by burning it
 * @param quantity - requested quantity to use
 * @return - actual quantity used
 */
float Energetic::ExpendFuel(float quantity) {
    fuel -= configuration.fuel.normal_fuel_usage * quantity;

    if (fuel < 0) {
        quantity += fuel;
        fuel   = 0;
    }

    return quantity;
}

float Energetic::getWarpEnergy() const
{
    return warpenergy;
}

void Energetic::increaseWarpEnergy( bool insys, float time )
{
    static bool WCfuelhack = configuration.fuel.fuel_equals_warp;
    if (WCfuelhack)
        this->warpenergy = this->fuel;
    this->warpenergy += (insys ? jump.insysenergy : jump.energy)*time;
    if (this->warpenergy > this->maxwarpenergy)
        this->warpenergy = this->maxwarpenergy;
    if (WCfuelhack)
        this->fuel = this->warpenergy;
}

float Energetic::maxEnergyData() const
{
    return maxenergy;
}

void Energetic::rechargeEnergy()
{
    if ( (!configuration.fuel.reactor_uses_fuel) || (fuel > 0) )
        energy += recharge * simulation_atom_var;
}

bool Energetic::refillWarpEnergy()
{
    static bool WCfuelhack = configuration.fuel.fuel_equals_warp;
    if (WCfuelhack)
        this->warpenergy = this->fuel;
    float tmp = this->maxwarpenergy;
    if (tmp < this->jump.energy)
        tmp = this->jump.energy;
    if (tmp > this->warpenergy) {
        this->warpenergy = tmp;
        if (WCfuelhack)
            this->fuel = this->warpenergy;
        return true;
    }
    return false;
}

void Energetic::setAfterburnerEnergy( float aft )
{
    afterburnenergy = aft;
}

void Energetic::setEnergyRecharge( float enrech )
{
    recharge = enrech;
}

void Energetic::setFuel( float f )
{
    fuel = f;
}



void Energetic::setMaxEnergy( float maxen )
{
    maxenergy = maxen;
}

float Energetic::shieldRechargeData() const
{
    const Damageable *damageable = reinterpret_cast<const Damageable*>(this);
    const Shield shield = damageable->shield;
    return shield.recharge;
}



float Energetic::warpCapData() const
{
    return maxwarpenergy;
}


float Energetic::warpEnergyData() const
{
    if (maxwarpenergy > 0)
        return ( (float) warpenergy )/( (float) maxwarpenergy );
    if (jump.energy > 0)
        return ( (float) warpenergy )/( (float) jump.energy );
    return 0.0f;
}
