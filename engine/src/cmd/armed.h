/**
 * armed.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ARMED_H
#define VEGA_STRIKE_ENGINE_CMD_ARMED_H

#include "vec.h"
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
    virtual ~Armed() = default;

    //Fires all active guns that are or arent Missiles
    //if bitmask is (1<<31) then fire off autotracking of that type;
    void Fire(unsigned int bitmask, bool beams_target_owner = false);

    //Gets the average gun speed of the unit::caution SLOW
    void getAverageGunSpeed(float &speed, float &grange, float &mrange) const;
    int getNumMounts() const;

    //-1 is no lock necessary 1 is locked
    int LockMissile() const;

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
