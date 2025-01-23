/*
 * armed.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ARMED_H
#define VEGA_STRIKE_ENGINE_CMD_ARMED_H

#include "gfx/vec.h"
#include "movable.h"
#include "mount.h"
#include <vector>

struct WeaponInfo;
class Unit;

class Armed {
    // TODO: made this private
public:
    std::vector<Mount> mounts;
    float gunspeed;
    float gunrange;
    float missilerange;
    char turretstatus;

protected:
    //Activates all guns of that size
    void ActivateGuns(const WeaponInfo *, bool Missile);

public:
    Armed();

    //Fires all active guns that are or arent Missiles
    //if bitmask is (1<<31) then fire off autotracking of that type;
    void Fire(unsigned int bitmask, bool beams_target_owner = false);

    //Gets the average gun speed of the unit::caution SLOW
    void getAverageGunSpeed(float &speed, float &grange, float &mrange) const;
    int getNumMounts() const;

    //-1 is no lock necessary 1 is locked
    int LockMissile() const;

    void LockTarget(bool myboo);

    //Finds the position from the local position if guns are aimed at it with speed
    QVector PositionITTS(const QVector &firingposit, Vector firingvelocity, float gunspeed, bool smooth_itts) const;

    void SelectAllWeapon(bool Missile);

    //resets average gun speed (in event of weapon change
    void setAverageGunSpeed();

    bool TargetLocked(const Unit *checktarget = nullptr) const;
    bool TargetTracked(const Unit *checktarget = nullptr);

    void ToggleWeapon(bool Missile, bool forward = true);
    float TrackingGuns(bool &missileLock);

    //Stops all active guns from firing
    void UnFire();
};

#endif //VEGA_STRIKE_ENGINE_CMD_ARMED_H
