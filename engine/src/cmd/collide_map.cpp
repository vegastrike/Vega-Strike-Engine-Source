/*
 * collide_map.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include <algorithm>
#include <assert.h>
#include "collide_map.h"
#include "unit_generic.h"
#include "bolt.h"
#include "star_system.h"
#include "universe.h"
#include "vs_logging.h"

volatile bool apart_return = true;

void CollideArray::erase(iterator target) {
    count -= 1;
    if (target >= this->begin() && target < this->end()) {
        target->radius = 0;
        target->ref.unit = NULL;
        size_t diff = (target - this->begin());
        if (this->unsorted.size() > diff) {
            //for secondary collide arrays that have no unsorted array
            iterator tmp = &*(this->unsorted.begin() + diff);
            tmp->radius = 0;
            tmp->ref.unit = NULL;
        }
        return;
    } else if (target == NULL) {
        return;
    } else {
        CollidableBackref *targ = static_cast< CollidableBackref * > (&*target);
        std::list<CollidableBackref> *targlist = &toflattenhints[targ->toflattenhints_offset];
        std::list<CollidableBackref>::iterator endlist = targlist->end();
        for (std::list<CollidableBackref>::iterator i = targlist->begin(); i != endlist; ++i) {
            if (&*i == target) {
                targlist->erase(i);
                return;
            }
        }
    }
}

void CollideArray::UpdateBoltInfo(CollideArray::iterator iter, Collidable::CollideRef ref) {
    if (iter >= this->begin() && iter < this->end()) {
        (*(unsorted.begin() + (iter - this->begin()))).ref = ref;         //update both unsorted and sorted
        (*iter).ref = ref;
    } else if (!is_null(iter)) {
        (*iter).ref = ref;
    }
}

CollideArray::iterator CollideArray::changeKey(CollideArray::iterator iter, const Collidable &newKey) {
    if (iter >= this->begin() && iter < this->end()) {
        iterator tmp = &*(this->unsorted.begin() + (iter - this->begin()));
        *tmp = newKey;
    } else {
        *iter = newKey;
    }
    return iter;
}

float CollideArray::max_bolt_radius = 0;

CollideArray::iterator CollideArray::changeKey(CollideArray::iterator iter,
        const Collidable &newKey,
        CollideArray::iterator tless,
        CollideArray::iterator tmore) {
    return this->changeKey(iter, newKey);
}

template<int direction, bool always_replace>
class RadiusUpdate {
    float last_radius;
    double last_radius_key;

    float last_big_radius;
    double last_big_radius_key;
    CollideArray *cm;
public:
    RadiusUpdate(CollideArray *cm) {
        last_radius = 0;
        last_big_radius = 0;
        last_radius_key = 0;
        last_big_radius_key = 0;
        this->cm = cm;
    }

    void operator()(const Collidable &collidable, size_t index) {
        double key = collidable.getKey();
        float rad = collidable.radius;
        if (rad > 0) {
            if (rad > last_big_radius) {
                last_radius = last_big_radius = rad;
                last_radius_key = last_big_radius_key = key;
            } else if (rad > last_radius) {
                last_radius = rad;
                last_radius_key = key;
            } else {
                last_radius_key = key;
            }
        }
        if (last_big_radius && fabs(key - last_big_radius_key) > 2 * cm->max_bolt_radius * simulation_atom_var) {
            last_big_radius = last_radius;
            last_big_radius_key = last_radius_key;
            last_radius = 0;
            last_radius_key = key;
        }
        if (always_replace || cm->max_radius[index] < last_big_radius) {
            cm->max_radius[index] = last_big_radius;
        }
    }
};

template<int location_index>
class UpdateBackpointers {
public:
    void updateBackpointer(Collidable &collidable) {
        assert(collidable.radius != 0.0f);
        if (location_index != Unit::UNIT_ONLY && collidable.radius < 0) {
            Bolt *p_bolt = Bolt::BoltFromIndex(collidable.ref);
            if (p_bolt != nullptr) {
                p_bolt->location = &collidable;
            }
        } else {
            collidable.ref.unit->location[location_index] = &collidable;
        }
    }

    void operator()(Collidable &collidable) {
        updateBackpointer(collidable);
    }
};

void CollideArray::flatten() {
    sorted.resize(count);
    max_radius.resize(count);
    size_t len = unsorted.size();
    size_t index = count;
    RadiusUpdate<-1, true> collideUpdate(this);
    for (ptrdiff_t i = len; i >= 0; i--) {
        Collidable *tmp;
        if (i < static_cast<ptrdiff_t>(len) && (tmp = &unsorted[i])->radius != 0.0f) {
            sorted[--index] = *tmp;
            collideUpdate(*tmp, index);
        }

        std::list<CollidableBackref>::iterator listend = toflattenhints[i].end();
        for (std::list<CollidableBackref>::iterator j = toflattenhints[i].begin();
                j != listend;
                ++j) {
            if (j->radius != 0) {
                sorted[--index] = *j;
                collideUpdate(*j, index);
            }
        }
        toflattenhints[i].resize(0);
    }

    std::sort(sorted.begin(), sorted.end());
    unsorted = sorted;

    toflattenhints.resize(count + 1);
    if (location_index == Unit::UNIT_BOLT) {
        size_t i = 0;
        size_t size = sorted.size();
        ResizableArray::iterator iter = sorted.begin();
        UpdateBackpointers<Unit::UNIT_BOLT> update;
        RadiusUpdate<1, false> radUpdate(this);
        for (i = 0; i != size; ++i, ++iter) {
            update(*iter);
            radUpdate(*iter, i);
        }
    } else if (location_index == Unit::UNIT_ONLY) {
        for_each(sorted.begin(), sorted.end(), UpdateBackpointers<Unit::UNIT_ONLY>());
    } else {
        assert(0
                && "Only Support arrays of units_only and mixed units bolts");         //right now only support 2 array types;
    }
}

class CopyExample : public UpdateBackpointers<Unit::UNIT_ONLY> {
public:
    CollideArray::ResizableArray::iterator examplebegin;
    CollideArray::ResizableArray::iterator exampleend;
public:
    CopyExample(CollideArray::ResizableArray::iterator beg, CollideArray::ResizableArray::iterator end) {
        examplebegin = beg;
        exampleend = end;
    }

    void operator()(Collidable &collidable) {
        assert(examplebegin != exampleend);
        while (!(examplebegin->radius > 0)) {
            ++examplebegin;
            assert(examplebegin != exampleend);
        }
        collidable = *examplebegin++;
        updateBackpointer(collidable);
    }
};

class resizezero {
public:
    template<class T>
    void operator()(T &toclear) {
        toclear.resize(0);
    }
};

void CollideArray::flatten(CollideArray &hint) {
    if (location_index == Unit::UNIT_ONLY) {
        sorted.resize(count);
        for_each(toflattenhints.begin(), toflattenhints.end(), resizezero());
        toflattenhints.resize(count + 1);

        for_each(sorted.begin(), sorted.end(), CopyExample(hint.sorted.begin(), hint.sorted.end()));
    } else {
        VS_LOG(info, "Trying to use flatten hint on a array with both bolts and units");
        flatten();
    }
}

CollideArray::iterator CollideArray::insert(const Collidable &newKey, iterator hint) {
    if (newKey.radius < -max_bolt_radius * simulation_atom_var) {
        max_bolt_radius = -newKey.radius / simulation_atom_var;
    }
    if (this->begin() == this->end()) {
        count += 1;
        this->unsorted.push_back(newKey);
        this->toflattenhints.resize(2);
        this->sorted.push_back(newKey);
        return &sorted.back();
    } else if (hint >= this->begin() && hint <= this->end()) {
        count += 1;
        size_t len = hint - this->begin();
        std::list<CollidableBackref> *hintlist = &toflattenhints[len];
        return &*hintlist->insert(hintlist->end(), CollidableBackref(newKey, len));
    } else {
        return this->insert(newKey);         //don't use hint;
    }
}

CollideArray::iterator CollideArray::lower_bound(const Collidable &newKey) {
    return ::std::lower_bound(this->begin(), this->end(), newKey);
}

CollideArray::iterator CollideArray::insert(const Collidable &newKey) {
    return this->insert(newKey, this->lower_bound(newKey));
}

void CollideArray::checkSet() {
    if (this->begin() != this->end()) {
        for (iterator newiter = this->begin(), iter = newiter++; newiter != this->end(); iter = newiter++)
            assert(*iter < *newiter);
    }
}

Collidable::Collidable(Unit *un) {
    radius = un->rSize();
    if (radius <= FLT_MIN || !FINITE(radius)) {
        radius = 2 * FLT_MIN;
    }
    assert(!un->isSubUnit());
    this->SetPosition(un->LocalPosition());
    ref.unit = un;
}

bool CollideArray::Iterable(CollideArray::iterator a) {
    return a >= begin() && a < end();
}

template<class T>
class CheckBackref {
public:
    CollideMap::iterator operator()(T *input, unsigned int location_index) {
        return input->location[location_index];
    }
};
template<>
class CheckBackref<Bolt> {
public:
    CollideMap::iterator operator()(Bolt *input, unsigned int location_index) {
        return input->location;
    }
};
extern size_t nondecal_index(Collidable::CollideRef b);
template<class T, bool canbebolt>
class CollideChecker {
public:
    static void FixMinLookMaxLook(CollideMap *tmpcm, CollideMap::iterator tmptmore, double &minlook, double &maxlook) {
        double mid = (minlook + maxlook) * .5;
        minlook = (minlook + mid) * .5 - tmptmore->radius;
        maxlook = (maxlook + mid) * .5 + tmptmore->radius;
    }

    static bool CheckCollisionsInner(CollideMap::iterator cmbegin,
            CollideMap::iterator cmend,
            T *un,
            const Collidable &collider,
            unsigned int location_index,
            CollideMap::iterator tless,
            CollideMap::iterator tmore,
            double minlook,
            double maxlook) {
        CheckBackref<T> backref_obtain;
        if (backref_obtain(un, location_index) != cmbegin) {
            //if will happen in case of !Iterable
            while ((*tless)->getKey() >= minlook) {
                float rad = (*tless)->radius;
                bool boltSpecimen = canbebolt && (rad < 0);

                Collidable::CollideRef ref = (*tless)->ref;
                if (tless == cmbegin) {
                    if (canbebolt && boltSpecimen) {
                        if (CheckCollision(un, collider, ref, **tless)) {
                            if (endAfterCollide(un, location_index)) {
                                return true;
                            } else {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else if (rad != 0) {
                        if (canbebolt == true && BoltType(un)) {
                            CollideMap::iterator tmptmore = ref.unit->location[Unit::UNIT_ONLY];
                            CollideMap::iterator tmptless = tmptmore;
                            ++tmptmore;
                            CollideMap *tmpcm = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];
                            return CollideChecker<T, false>::CheckCollisionsInner(tmpcm->begin(), tmpcm->end(),
                                    un, collider, Unit::UNIT_ONLY,
                                    tmptless, tmptmore,
                                    minlook, maxlook);
                        }
                        if (CheckCollision(un, collider, ref.unit, **tless)) {
                            if (endAfterCollide(un, location_index)) {
                                return true;
                            } else {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                } else {
                    if (canbebolt && boltSpecimen) {
                        if (CheckCollision(un, collider, ref, **tless--)) {
                            if (endAfterCollide(un, location_index)) {
                                return true;
                            }
                        }
                    } else if (rad != 0) {
                        if (canbebolt == true && BoltType(un)) {
                            CollideMap::iterator tmptmore = ref.unit->location[Unit::UNIT_ONLY];
                            CollideMap::iterator tmptless = tmptmore;
                            ++tmptmore;
                            CollideMap *tmpcm = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];
                            return CollideChecker<T, false>::CheckCollisionsInner(tmpcm->begin(), tmpcm->end(),
                                    un, collider, Unit::UNIT_ONLY,
                                    tmptless, tmptmore,
                                    minlook, maxlook);
                        }
                        if (CheckCollision(un, collider, ref.unit, **tless--)) {
                            if (endAfterCollide(un, location_index)) {
                                return true;
                            }
                        }
                    } else {
                        --tless;
                    }
                }
            }
        }
        while (tmore != cmend && (*tmore)->getKey() <= maxlook) {
            float rad = (*tmore)->radius;
            bool boltSpecimen = canbebolt && (rad < 0);
            Collidable::CollideRef ref = (*tmore)->ref;
            if (canbebolt && boltSpecimen) {
                if (CheckCollision(un, collider, ref, **tmore++)) {
                    if (endAfterCollide(un, location_index)) {
                        return true;
                    }
                }
            } else if (rad != 0) {
                //not null unit
                if (canbebolt == true && BoltType(un)) {
                    CollideMap::iterator tmptmore = ref.unit->location[Unit::UNIT_ONLY];
                    CollideMap::iterator tmptless = tmptmore;
                    ++tmptmore;
                    CollideMap *tmpcm = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];
                    return CollideChecker<T, false>::CheckCollisionsInner(tmpcm->begin(), tmpcm->end(),
                            un, collider, Unit::UNIT_ONLY,
                            tmptless, tmptmore,
                            minlook, maxlook);
                }
                if (CheckCollision(un, collider, ref.unit, **tmore++)) {
                    if (endAfterCollide(un, location_index)) {
                        return true;
                    }
                }
            } else {
                ++tmore;
            }
        }
        return false;
    }

    static bool ComputeMaxLookMinLook(Unit *un,
            CollideMap *cm,
            CollideMap::iterator collider,
            CollideMap::iterator begin,
            CollideMap::iterator end,
            double sortedloc,
            float radius,
            double &minlook,
            double &maxlook) {
        maxlook = sortedloc + 2.0625 * radius;
        minlook = sortedloc - 2.0625 * radius;
        return false;
    }

    static bool ComputeMaxLookMinLook(Bolt *un,
            CollideMap *cm,
            CollideMap::iterator collider,
            CollideMap::iterator cmbegin,
            CollideMap::iterator cmend,
            double sortedloc,
            float rad,
            double &minlook,
            double &maxlook) {
        float dboltdist = -2.0625 * rad;
        float boltdist = -1.0625 * rad;
        if (collider >= cmbegin && collider < cmend) {
            float maxrad = cm->max_radius[collider - cmbegin];
            if (maxrad == 0) {
                return true;
            }
            boltdist += maxrad;
            if (dboltdist < boltdist) {
                boltdist = dboltdist;
            }
        } else {
            boltdist += fabs(rad);
        }
        maxlook = sortedloc + boltdist;
        minlook = sortedloc - boltdist;
        return false;
    }

    static bool CheckCollisions(CollideMap *cm, T *un, const Collidable &collider, unsigned int location_index) {
        CollideMap::iterator tless, tmore;
        double sortedloc = collider.getKey();
        float rad = collider.radius;
        CollideMap::iterator cmbegin = cm->begin();
        CollideMap::iterator cmend = cm->end();
        if (cmbegin == cmend) {
            return false;
        }
        double minlook, maxlook;
        CollideMap::iterator startIter = CheckBackref<T>()(un, location_index);
        if (ComputeMaxLookMinLook(un, cm, startIter, cmbegin, cmend, sortedloc, rad, minlook, maxlook)) {
            return false;
        }  //no units in area
        if (!cm->Iterable(startIter)) {
            CollideArray::CollidableBackref *br = static_cast< CollideArray::CollidableBackref * > (startIter);
            CollideMap::iterator tmploc = cmbegin + br->toflattenhints_offset;
            if (tmploc == cmend) {
                tmploc--;
            }
            tless = tmore = tmploc;     //don't decrease tless
        } else {
            tless = tmore = startIter;
            if (tless != cmbegin) {
                --tless;
            }
        }
        ++tmore;
        return CheckCollisionsInner(cmbegin, cmend,
                un, collider, location_index,
                tless, tmore,
                minlook, maxlook);
    }

    static bool doUpdateKey(Bolt *b) {
        return true;
    }

    static bool doUpdateKey(Unit *un) {
        return false;
    }

    static bool endAfterCollide(Bolt *b, unsigned int location_index /*meaningless, just for templtae goodness*/ ) {
        return true;
    }

    static bool endAfterCollide(Unit *un, unsigned int location_index) {
        return is_null(un->location[location_index]);
    }

    static bool ApartPositive(const Collidable &a, const Collidable &b) {
        float aradius = a.radius;
        float bradius = b.radius;
        return (a.GetPosition() - b.GetPosition()).MagnitudeSquared()
                > aradius * aradius + aradius * bradius * 2 + bradius * bradius;
    }

    static bool ApartNeg(const Collidable &a, const Collidable &b) {
        //return apart_return;
        double tempy = a.position.j - b.position.j;
        double tempz = a.position.k - b.position.k;
        float radiussum = b.radius - a.radius; //a is negative
        if (fabs(tempy) > radiussum || fabs(tempz) > radiussum) {
            return true;
        }
        double tempx = (a.position.i - b.position.i);
        tempx *= tempx;
        tempy *= tempy;
        tempz *= tempz;
        return (tempx + tempy + tempz) > radiussum * radiussum;
    }

    static bool CheckCollision(Unit *a, const Collidable &aiter, Unit *b, const Collidable &biter) {
        if (!ApartPositive(aiter, biter)) {
            return a->Collide(b);
        }
        return false;
    }

    static bool CheckCollision(Bolt *a, const Collidable &aiter, Unit *b, const Collidable &biter) {
        if (!ApartNeg(aiter, biter)) {
            if (a->Collide(b)) {
                a->Destroy(nondecal_index(aiter.ref));
                return true;
            }
        }
        return false;
    }

    static bool BoltType(Bolt *a) {
        return true;
    }

    static bool BoltType(Unit *a) {
        return false;
    }

    static bool CheckCollision(Bolt *a, const Collidable &aiter, Collidable::CollideRef b, const Collidable &biter) {
        return false;
    }

    static bool CheckCollision(Unit *un, const Collidable &aiter, Collidable::CollideRef b, const Collidable &biter) {
        if (!ApartNeg(biter, aiter)) {
            return Bolt::CollideAnon(b, un);
        }
        return false;
    }
};

bool CollideMap::CheckCollisions(Bolt *bol, const Collidable &updated) {
    return CollideChecker<Bolt, true>::CheckCollisions(this, bol, updated, Unit::UNIT_BOLT);
}

bool CollideMap::CheckUnitCollisions(Bolt *bol, const Collidable &updated) {
    return CollideChecker<Bolt, false>::CheckCollisions(this, bol, updated, Unit::UNIT_ONLY);
}

bool CollideMap::CheckCollisions(Unit *un, const Collidable &updated) {
    //need to check beams
    if (un->activeStarSystem == NULL) {
        un->activeStarSystem = _Universe->activeStarSystem();
    } else
        assert(un->activeStarSystem == _Universe->activeStarSystem());
    return CollideChecker<Unit, true>::CheckCollisions(this, un, updated, Unit::UNIT_BOLT);
}

bool CollideMap::CheckUnitCollisions(Unit *un, const Collidable &updated) {
    //need to check beams
    if (un->activeStarSystem == NULL) {
        un->activeStarSystem = _Universe->activeStarSystem();
    } else
        assert(un->activeStarSystem == _Universe->activeStarSystem());
    return CollideChecker<Unit, false>::CheckCollisions(this, un, updated, Unit::UNIT_ONLY);
}

