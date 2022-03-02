/**
 * unit_find.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#ifndef _UNIT_FIND_H_
#define _UNIT_FIND_H_
#include "unit_util.h"

template<class Locator>
void findObjectsFromPosition(CollideMap *cm,
        CollideMap::iterator location,
        Locator *check,
        QVector thispos,
        float thisrad,
        bool acquire_on_location) {
    CollideMap::iterator cmend = cm->end();
    CollideMap::iterator cmbegin = cm->begin();
    if (cmend != cmbegin && !is_null(location)) {
        CollideMap::iterator tless = location;
        CollideMap::iterator tmore = location;
        bool workA = true;
        bool workB = true;
        if (location != cmend && !cm->Iterable(location)) {
            CollideArray::CollidableBackref *br = static_cast< CollideArray::CollidableBackref * > (location);
            location = cmbegin + br->toflattenhints_offset;
            if (location == cmend) {
                location--;
            }
            tmore = location;
            tless = location;
        } else if (location != cmend) {
            ++tmore;
        } else {
            //location == cmend
            --location;
            if (acquire_on_location) {
                --tless;
            }                  //allowed because cmend!=cmbegin
            workB = false;
        }
        check->init(cm, location);
        if (!acquire_on_location) {
            if (tless != cmbegin) {
                --tless;
            } else {
                workA = false;
            }
        }
        while (workA || workB) {
            if (workA
                    && !check->cullless(tless)) {
                float rad = (*tless)->radius;
                if (rad != 0.0f && (check->BoltsOrUnits() || (check->UnitsOnly() == (rad > 0)))) {
                    float trad =
                            check->NeedDistance() ? ((*tless)->GetPosition() - thispos).Magnitude() - fabs(rad)
                                    - thisrad : 0;
                    if (!check->acquire(trad, tless)) {
                        workA = false;
                    }
                }
                if (tless != cmbegin) {
                    tless--;
                } else {
                    workA = false;
                }
            } else {
                workA = false;
            }
            if (workB
                    && tmore != cmend
                    && !check->cullmore(tmore)) {
                float rad = (*tmore)->radius;
                if (rad != 0.0f && (check->BoltsOrUnits() || (check->UnitsOnly() == (rad > 0)))) {
                    float trad =
                            check->NeedDistance() ? ((*tmore)->GetPosition() - thispos).Magnitude() - fabs(rad)
                                    - thisrad : 0;
                    if (!check->acquire(trad, tmore)) {
                        workB = false;
                    }
                }
                tmore++;
            } else {
                workB = false;
            }
        }
    }
}

template<class Locator>
void findObjects(CollideMap *cm, CollideMap::iterator location, Locator *check) {
    if (is_null(location)) {
        return;
    }
    QVector thispos = (**location).GetPosition();
    float thisrad = fabs((*location)->radius);
    findObjectsFromPosition(cm, location, check, thispos, thisrad, false);
}

class NearestUnitLocator {
    CollideMap::iterator location;
    double startkey;
    float rad;
public:
    Collidable::CollideRef retval;

    bool BoltsOrUnits() {
        return false;
    }

    bool UnitsOnly() {
        return true;
    }

    bool NeedDistance() {
        return true;
    }

    NearestUnitLocator() {
        retval.unit = NULL;
    }

    void init(CollideMap *cm, CollideMap::iterator parent) {
        this->location = parent;
        startkey = (*location)->getKey();
        rad = FLT_MAX;
        retval.unit = NULL;
    }

    bool cullless(CollideMap::iterator tless) {
        return rad != FLT_MAX && (startkey - rad) > (*tless)->getKey();
    }

    bool cullmore(CollideMap::iterator tmore) {
        return rad != FLT_MAX && (startkey + rad) < (*tmore)->getKey();
    }

    bool acquire(float distance, CollideMap::iterator i) {
        if (distance < rad) {
            rad = distance;
            retval = (*i)->ref;
        }
        return true;
    }
};
class NearestBoltLocator : public NearestUnitLocator {
public:
    bool UnitsOnly() {
        return false;
    }
};
class NearestObjectLocator : public NearestUnitLocator {
public:
    bool isUnit;

    bool UnitsOnly() {
        return false;
    }

    bool BoltsOrUnits() {
        return true;
    }

    bool acquire(float distance, CollideMap::iterator i) {
        Collidable::CollideRef lastret = retval;
        bool retval = NearestUnitLocator::acquire(distance, i);
        if (memcmp((void *) &retval, (void *) &lastret, sizeof(Collidable::CollideRef))) {
            isUnit = (*i)->radius > 0;
        }
        return retval;
    }
};
class NearestNavLocator : public NearestUnitLocator {
public:
    bool BoltsOrUnits() {
        return false;
    }

    bool UnitsOnly() {
        return true;
    }

    bool acquire(float distance, CollideMap::iterator i) {
        if (UnitUtil::isSignificant((*i)->ref.unit)) {
            return NearestUnitLocator::acquire(distance, i);
        }
        return true;
    }
};
class NearestNavOrCapshipLocator : public NearestUnitLocator {
public:
    bool BoltsOrUnits() {
        return false;
    }

    bool UnitsOnly() {
        return true;
    }

    bool acquire(float distance, CollideMap::iterator i) {
        if (UnitUtil::isSignificant((*i)->ref.unit) || UnitUtil::isCapitalShip((*i)->ref.unit)) {
            return NearestUnitLocator::acquire(distance, i);
        }
        return true;
    }
};
template<class T>
class UnitWithinRangeLocator {
public:
    T action;
    double startkey;
    float radius;
    float maxUnitRadius;

    UnitWithinRangeLocator(float radius, float maxUnitRadius) :
            startkey(0), radius(radius), maxUnitRadius(maxUnitRadius) {
    }

    bool UnitsOnly() {
        return true;
    }

    bool BoltsOrUnits() {
        return false;
    }

    bool NeedDistance() {
        return true;
    }

    void init(CollideMap *cm, CollideMap::iterator parent) {
        startkey = (*parent)->getKey();
    }

    bool cullless(CollideMap::iterator tless) {
        double tmp = startkey - radius - maxUnitRadius;
        return tmp > (*tless)->getKey();
    }

    bool cullmore(CollideMap::iterator tmore) {
        return startkey + radius + maxUnitRadius < (*tmore)->getKey();
    }

    bool acquire(float dist, CollideMap::iterator i) {
        if (dist < radius) {
            //Inside radius...
            return action.acquire((*i)->ref.unit, dist);
        }
        return true;
    }
};
template<class T>
class UnitWithinRangeOfPosition : public UnitWithinRangeLocator<T> {
public:
    UnitWithinRangeOfPosition(float radius, float maxUnitRadius, const Collidable &key_iterator) :
            UnitWithinRangeLocator<T>(radius, maxUnitRadius) {
        this->startkey = key_iterator.getKey();
    }

    void init(CollideMap *cm, CollideMap::iterator parent) {
    }
};
class UnitPtrLocator {
    const void *unit;
public:
    bool retval;

    bool BoltsOrUnits() {
        return false;
    }

    bool UnitsOnly() {
        return true;
    }

    bool NeedDistance() {
        return false;
    }

    UnitPtrLocator(const void *unit) {
        retval = false;
        this->unit = unit;
    }

    bool cullless(CollideMap::iterator tless) {
        return retval;
    }

    bool cullmore(CollideMap::iterator tmore) {
        return retval;
    }

    bool acquire(float distance, CollideMap::iterator i) {
        return retval = (((const void *) ((*i)->ref.unit)) == unit);
    }

    void init(CollideMap *cm, CollideMap::iterator parent) {
    }
};

#endif

