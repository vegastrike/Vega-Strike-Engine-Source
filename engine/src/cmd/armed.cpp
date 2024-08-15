/*
 * armed.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy, and
 * other Vega Strike contributors
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


#include "armed.h"

#include "configuration/configuration.h"
#include "mount.h"
#include "mount_size.h"
#include "weapon_info.h"
#include "vs_globals.h"
#include "movable.h"
#include "universe.h"
#include "role_bitmask.h"
#include "beam.h"
#include "unit_util.h"
#include "vs_logging.h"
#include "resource/resource.h"
#include "vega_cast_utils.h"

#include <vector>

using std::vector;

/**********************************************************************
 * Armed
 * ********************************************************************/

typedef std::set<int> WeaponGroup;

template<bool FORWARD>
class WeaponComparator {
public:
    bool operator()(const WeaponGroup &a, const WeaponGroup &b) const {
        if (a.size() == b.size()) {
            for (WeaponGroup::const_iterator iterA = a.begin(), iterB = b.begin();
                    iterA != a.end() && iterB != b.end();
                    ++iterA, ++iterB) {
                if ((*iterA) < (*iterB)) {
                    return FORWARD;
                } else if ((*iterB) < (*iterA)) {
                    return !FORWARD;
                }
            }
            return false;
        } else if (a.size() < b.size()) {
            return FORWARD;
        } else {
            return !FORWARD;
        }
    }

    typedef std::set<WeaponGroup, WeaponComparator<FORWARD> > WeaponGroupSet;

    static bool checkmount(Unit *un, int i, bool missile) {
        return un->mounts[i].status < Mount::DESTROYED && (un->mounts[i].type->isMissile() == missile)
                && un->mounts[i].ammo != 0;
    }

    static bool isSpecial(const Mount &mount) {
        return mount.type->size == MOUNT_SIZE::SPECIAL || mount.type->size == MOUNT_SIZE::SPECIALMISSILE;
    }

    static bool notSpecial(const Mount &mount) {
        return !isSpecial(mount);
    }

    static void ToggleWeaponSet(Unit *un, bool missile) {
        if (un->mounts.size() == 0) {
            Unit *tur = NULL;
            for (un_iter i = un->getSubUnits(); (tur = *i) != NULL; ++i) {
                ToggleWeaponSet(tur, missile);
            }
            return;
        }
        WeaponGroup lightMissiles;
        WeaponGroup heavyMissiles;
        WeaponGroup allWeapons;
        WeaponGroup allWeaponsNoSpecial;
        WeaponGroupSet myset;
        unsigned int i;
        typename WeaponGroupSet::const_iterator iter;

        VS_LOG(info, (boost::format("ToggleWeaponSet: %s") % (FORWARD ? "true" : "false")));
        for (i = 0; i < un->mounts.size(); ++i) {
            if (checkmount(un, i, missile)) {
                WeaponGroup mygroup;
                for (unsigned int j = 0; j < un->mounts.size(); ++j) {
                    if (un->mounts[j].type == un->mounts[i].type) {
                        if (checkmount(un, j, missile)) {
                            mygroup.insert(j);
                        }
                    }
                }
                //WIll ignore if already there.
                myset.insert(mygroup);
                allWeapons.insert(i);
                if (notSpecial(un->mounts[i])) {
                    allWeaponsNoSpecial.insert(i);
                }
            }
        }
        const WeaponGroupSet mypairset(myset);
        for (iter = mypairset.begin(); iter != mypairset.end(); ++iter) {
            if ((*iter).size() && notSpecial(un->mounts[(*((*iter).begin()))])) {
                typename WeaponGroupSet::const_iterator iter2;
                for (iter2 = mypairset.begin(); iter2 != mypairset.end(); ++iter2) {
                    if ((*iter2).size() && notSpecial(un->mounts[(*((*iter2).begin()))])) {
                        WeaponGroup myGroup;
                        set_union((*iter).begin(), (*iter).end(), (*iter2).begin(), (*iter2).end(),
                                inserter(myGroup, myGroup.begin()));
                        myset.insert(myGroup);
                    }
                }
            }
        }
        static bool allow_special_with_weapons =
                XMLSupport::parse_bool(vs_config->getVariable("physics", "special_and_normal_gun_combo", "true"));
        if (allow_special_with_weapons) {
            myset.insert(allWeapons);
        }
        myset.insert(allWeaponsNoSpecial);
        for (iter = myset.begin(); iter != myset.end(); ++iter) {
            for (WeaponGroup::const_iterator iter2 = (*iter).begin(); iter2 != (*iter).end(); ++iter2) {
                VS_LOG(info, (boost::format("%d:%s ") % *iter2 % un->mounts[*iter2].type->name.c_str()));
            }
        }
        WeaponGroup activeWeapons;
        VS_LOG(info, "CURRENT: ");
        for (i = 0; i < un->mounts.size(); ++i) {
            if (un->mounts[i].status == Mount::ACTIVE && checkmount(un, i, missile)) {
                activeWeapons.insert(i);
                VS_LOG(info, (boost::format("%d:%s ") % i % un->mounts[i].type->name.c_str()));
            }
        }
        iter = myset.upper_bound(activeWeapons);
        if (iter == myset.end()) {
            iter = myset.begin();
        }
        if (iter == myset.end()) {
            return;
        }
        for (i = 0; i < un->mounts.size(); ++i) {
            un->mounts[i].DeActive(missile);
        }
        VS_LOG(info, "ACTIVE: ");
        for (WeaponGroup::const_iterator iter2 = (*iter).begin(); iter2 != (*iter).end(); ++iter2) {
            VS_LOG(info, (boost::format("%d:%s ") % *iter2 % un->mounts[*iter2].type->name.c_str()));
            un->mounts[*iter2].Activate(missile);
        }

        VS_LOG(info, "ToggleWeapon end...\n");
    }
};

/**********************************************************************
 * Armed
 * ********************************************************************/
Armed::Armed() {
    turretstatus = 0;
}

///cycles through the loop twice turning on all matching to ms weapons of size or after size
void Armed::ActivateGuns(const WeaponInfo *sz, bool ms) {
    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < getNumMounts(); ++i) {
            if (mounts[i].type == sz) {
                if ((mounts[i].status < Mount::DESTROYED) && (mounts[i].ammo != 0)
                        && (mounts[i].type->isMissile() == ms)) {
                    mounts[i].Activate(ms);
                } else {
                    sz = mounts[(i + 1) % getNumMounts()].type;
                }
            }
        }
    }
}

void Armed::Fire(unsigned int weapon_type_bitmask, bool listen_to_owner) {
    Unit *unit = static_cast<Unit *>(this);

    if ((unit->cloak.Active() && !configuration()->weapons.can_fire_in_cloak) ||
            (unit->graphicOptions.InWarp && !configuration()->weapons.can_fire_in_spec)) {
        UnFire();
        return;
    }
    unsigned int mountssize = mounts.size();
    _Universe->whichPlayerStarship(unit);
    vector<int> gunFireRequests;
    vector<int> missileFireRequests;
    vector<int> serverUnfireRequests;
    for (unsigned int counter = 0; counter < mountssize; ++counter) {
        unsigned int index = counter;
        Mount *i = &mounts[index];
        if (i->status != Mount::ACTIVE) {
            continue;
        }
        if (i->bank == true) {
            unsigned int best = index;
            unsigned int j;
            for (j = index + 1; j < mountssize; ++j) {
                if (i->NextMountCloser(&mounts[j], unit)) {
                    best = j;

                    i->UnFire();
                    i = &mounts[j];
                } else {
                    mounts[j].UnFire();
                }
                if (mounts[j].bank == false) {
                    ++j;
                    break;
                }
            }
            counter = j - 1;                       //will increment to the next one
            index = best;
        }
        const bool mis = i->type->isMissile();
        const bool locked_on = i->time_to_lock <= 0;
        const bool lockable_weapon = i->type->lock_time > 0;
        const bool autotracking_gun = (!mis) && isAutoTrackingMount(i->size) && locked_on;
        const bool fire_non_autotrackers = (0 == (weapon_type_bitmask & ROLES::FIRE_ONLY_AUTOTRACKERS));
        const bool locked_missile = (mis && locked_on && lockable_weapon);
        const bool missile_and_want_to_fire_missiles = (mis && (weapon_type_bitmask & ROLES::FIRE_MISSILES));
        const bool gun_and_want_to_fire_guns = ((!mis) && (weapon_type_bitmask & ROLES::FIRE_GUNS));
        if (configuration()->logging.verbose_debug && missile_and_want_to_fire_missiles && locked_missile) {
            VSFileSystem::vs_fprintf(stderr, "\n about to fire locked missile \n");
        }
        bool want_to_fire = (fire_non_autotrackers || autotracking_gun || locked_missile) &&
                //&& ( (ROLES::EVERYTHING_ELSE&weapon_type_bitmask&i->type->role_bits) || i->type->role_bits == 0 )
                ((locked_on && missile_and_want_to_fire_missiles) || gun_and_want_to_fire_guns);
        if ((*i).type->type == WEAPON_TYPE::BEAM) {
            if ((*i).type->energy_rate * static_cast<double>(simulation_atom_var) > unit->energy.Level()) {
                //NOT ONLY IN non-networking mode : anyway, the server will tell everyone including us to stop if not already done
                (*i).UnFire();
                continue;
            }
        } else
            //Only in non-networking mode
        if (i->type->energy_rate > unit->energy.Level()) {
            if (!want_to_fire) {
                i->UnFire();
            }

            continue;
        }
        if (want_to_fire) {
            //If in non-networking mode and mount fire has been accepted or if on server side


            //If we are on server or if the weapon has been accepted for fire we fire
            if (i->Fire(unit, unit->owner == NULL ? this : unit->owner, mis, listen_to_owner)) {
                //We could only refresh energy on server side or in non-networking mode, on client side it is done with
                //info the server sends with ack for fire
                //FOR NOW WE TRUST THE CLIENT SINCE THE SERVER CAN REFUSE A FIRE
                //if( Network==NULL || SERVER)
                if (i->type->type == WEAPON_TYPE::BEAM) {
                    if (i->ref.gun) {
                        if ((!i->ref.gun->Dissolved()) || i->ref.gun->Ready()) {
                            unit->energy.Deplete(true, i->type->energy_rate * static_cast<double>(simulation_atom_var));
                        }
                    }
                } else if (i->type->isMissile()) {    // FIXME  other than beams, only missiles are processed here?
                    unit->energy.Deplete(true, i->type->energy_rate);
                }
                //IF WE REFRESH ENERGY FROM SERVER : Think to send the energy update to the firing client with ACK TO fireRequest
                //fire only 1 missile at a time
                if (mis) {
                    weapon_type_bitmask &= (~ROLES::FIRE_MISSILES);
                }
            }

        }
        if (want_to_fire == false
                && (i->processed == Mount::FIRED || i->processed == Mount::REQUESTED
                        || i->processed == Mount::PROCESSED)) {
            i->UnFire();
        }
    }
}

void Armed::getAverageGunSpeed(float &speed, float &range, float &mmrange) const {
    speed = gunspeed;
    range = gunrange;
    mmrange = missilerange;
}

int Armed::getNumMounts() const {
    return mounts.size();
}

int Armed::LockMissile() const {
    bool missilelock = false;
    bool dumblock = false;
    for (int i = 0; i < getNumMounts(); ++i) {
        if (mounts[i].status == Mount::ACTIVE && mounts[i].type->lock_time > 0 && mounts[i].time_to_lock <= 0
                && mounts[i].type->isMissile()) {
            missilelock = true;
        } else if (mounts[i].status == Mount::ACTIVE && mounts[i].type->lock_time == 0 && mounts[i].type->isMissile()
                && mounts[i].time_to_lock <= 0) {
            dumblock = true;
        }
    }
    return missilelock ? 1 : (dumblock ? -1 : 0);
}

void Armed::LockTarget(bool myboo) {
    Unit *unit = static_cast<Unit *>(this);
    unit->computer.radar.locked = myboo;
    if (myboo && unit->computer.radar.canlock == false && false == UnitUtil::isSignificant(unit->Target())) {
        unit->computer.radar.locked = false;
    }
}

QVector Armed::PositionITTS(const QVector &absposit, Vector velocity, float speed, bool steady_itts) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    if (speed == FLT_MAX) {
        return unit->Position();
    }
    float difficultyscale = 1;
    if (g_game.difficulty < .99) {
        unit->GetVelocityDifficultyMult(difficultyscale);
    }
    velocity = (unit->cumulative_velocity.Scale(difficultyscale) - velocity);
    QVector posit(unit->Position() - absposit);
    QVector curguess(posit);
    for (unsigned int i = 0; i < 3; ++i) {
        float time = 0;
        if (speed > 0.001) {
            time = curguess.Magnitude() / speed;
        }
        if (steady_itts) {
            //** jay
            curguess = posit + unit->GetVelocity().Cast().Scale(time);
        } else {
            curguess = posit + velocity.Scale(time).Cast();
        }
    }
    return curguess + absposit;
}

void Armed::SelectAllWeapon(bool Missile) {
    for (int i = 0; i < getNumMounts(); ++i) {
        if (mounts[i].status < Mount::DESTROYED) {
            if (!isSpecialGunMount(as_integer(mounts[i].type->size))) {
                mounts[i].Activate(Missile);
            }
        }
    }
}

void Armed::setAverageGunSpeed() {
    float mrange = -1;
    float grange = -1;
    float speed = -1;
    bool beam = true;
    if (getNumMounts()) {
        grange = 0;
        speed = 0;
        mrange = 0;
        int nummt = 0;
        //this breaks the name, but... it _is_ more useful.
        for (int i = 0; i < getNumMounts(); ++i) {
            if (mounts[i].status == Mount::ACTIVE || mounts[i].status == Mount::INACTIVE) {
                if (mounts[i].type->isMissile()) {
                    if (mounts[i].type->range > mrange) {
                        mrange = mounts[i].type->range;
                    }
                } else {
                    if (mounts[i].type->range > grange) {
                        grange = mounts[i].type->range;
                    }
                    if (mounts[i].status == Mount::ACTIVE) {
                        speed += mounts[i].type->speed;
                        ++nummt;
                        beam &= (mounts[i].type->type == WEAPON_TYPE::BEAM);
                    }
                }
            }
        }
        if (nummt) {
            if (beam) {
                speed = FLT_MAX;
            } else {
                speed = speed / nummt;
            }
        }
    }
    this->missilerange = mrange;
    this->gunrange = grange;
    this->gunspeed = speed;
}

bool Armed::TargetLocked(const Unit *checktarget) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    if (!unit->computer.radar.locked) {
        return false;
    }
    return (checktarget == NULL) || (unit->computer.target == checktarget);
}

bool Armed::TargetTracked(const Unit *checktarget) {
    Unit *unit = static_cast<Unit *>(this);
    static bool must_lock_to_autotrack = XMLSupport::parse_bool(
            vs_config->getVariable("physics", "must_lock_to_autotrack", "true"));
    bool we_do_track = unit->computer.radar.trackingactive
            && (!_Universe->isPlayerStarship(unit) || TargetLocked() || !must_lock_to_autotrack);
    if (!we_do_track) {
        return false;
    }
    if (checktarget == NULL) {
        return true;
    }
    if (unit->computer.target != checktarget) {
        return false;
    }
    float mycone = unit->computer.radar.trackingcone;
    we_do_track = CloseEnoughToAutotrack(unit, unit->computer.target.GetUnit(), mycone);
    return we_do_track;
}

void Armed::ToggleWeapon(bool missile, bool forward) {
    Unit *unit = static_cast<Unit *>(this);
    if (forward) {
        WeaponComparator<true>::ToggleWeaponSet(unit, missile);
    } else {
        WeaponComparator<false>::ToggleWeaponSet(unit, missile);
    }
}

float Armed::TrackingGuns(bool &missilelock) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    float trackingcone = 0;
    missilelock = false;
    for (int i = 0; i < getNumMounts(); ++i) {
        if (mounts[i].status == Mount::ACTIVE && isAutoTrackingMount(mounts[i].size)) {
            trackingcone = unit->computer.radar.trackingcone;
        }
        if (mounts[i].status == Mount::ACTIVE && mounts[i].type->lock_time > 0 && mounts[i].time_to_lock <= 0) {
            missilelock = true;
        }
    }
    return trackingcone;
}

void Armed::UnFire() {
    Unit *unit = static_cast<Unit *>(this);

    if (this->getNumMounts() == 0) {
        Unit *tur = NULL;
        for (un_iter i = unit->getSubUnits(); (tur = *i) != NULL; ++i) {
            tur->UnFire();
        }
    } else {
        _Universe->whichPlayerStarship(unit);
        vector<int> unFireRequests;
        for (int i = 0; i < getNumMounts(); ++i) {
            if (mounts[i].status != Mount::ACTIVE) {
                continue;
            }
            mounts[i].UnFire();                  //turns off beams;
        }
        if (!unFireRequests.empty()) {
        }
    }
}
