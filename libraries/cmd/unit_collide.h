/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLIDE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLIDE_H

#define SAFE_COLLIDE_DEBUG
#include "gfx_generic/vec.h"
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include "src/linecollide.h"
#include "cmd/collection.h"
#include "cmd/unit_generic.h"
#include "src/vs_logging.h"
#include <set>
#define COLLIDETABLESIZE sizeof (CTSIZ)
#define COLLIDETABLEACCURACY sizeof (CTACCURACY)
///objects that go over 16 sectors are considered huge and better to check against everything.
#define HUGEOBJECT sizeof (CTHUGE)

class StarSystem;
/**
 * Hashtable3d is a 3d datastructure that holds various starships that are
 * near enough to crash into each other (or also lights that are big enough
 * to shine on nearby units.
 */
template<class CTSIZ, class CTACCURACY, class CTHUGE>
class UnitHash3d {
///All objects that are too large to fit (fastly) in the collide table
    UnitCollection hugeobjects;
    UnitCollection ha;
    UnitCollection hb;
    UnitCollection *active_huge;
    UnitCollection *accum_huge;
    std::set<Unit *> act_huge;
    std::set<Unit *> acc_huge;
///The hash table itself. Holds most units to be collided with
    UnitCollection table[COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];
    StarSystem *activeStarSystem;
    Unit *debugUnit;

///hashes 3 values into the appropriate spot in the hash table

    static void hash_vec(double i, double j, double k, int &x, int &y, int &z) {
        x = hash_int(i);
        y = hash_int(j);
        z = hash_int(k);
    }

///hashes 3 vals into the appropriate place in hash table
    static void hash_vec(const QVector &t, int &x, int &y, int &z) {
        hash_vec(t.i, t.j, t.k, x, y, z);
    }

public:
    UnitHash3d(StarSystem *ss) {
        activeStarSystem = ss;
        active_huge = &ha;
        accum_huge = &hb;
    }

    void SwapHugeAccum() {
        if (active_huge == &ha) {
            active_huge = &hb;
            accum_huge = &ha;
        } else {
            active_huge = &ha;
            accum_huge = &hb;
        }
        accum_huge->clear();
        act_huge.swap(acc_huge);
        acc_huge.clear();
    }

    void AddHugeToActive(Unit *un) {
        if (acc_huge.find(un) == acc_huge.end()) {
            acc_huge.insert(un);
            accum_huge->prepend(un);
            if (act_huge.find(un) == act_huge.end()) {
                act_huge.insert(un);
                active_huge->prepend(un);
            }
        }
    }

///Hashes a single value to a value on the collide table truncated to all 3d constraints.  Consider using a swizzle
    static int hash_int(const double aye) {
        return ((int) (((aye < 0) ? (aye
                - COLLIDETABLEACCURACY) : aye)
                / COLLIDETABLEACCURACY)) % (COLLIDETABLESIZE / 2) + (COLLIDETABLESIZE / 2);
    }

///clears entire table
    void Clear() {
        if (!hugeobjects.empty()) {
            hugeobjects.clear();
        }
        if (this->active_huge->size()) {
            ha.clear();
        }
        if (this->accum_huge->size()) {
            hb.clear();
        }
        acc_huge.clear();
        act_huge.clear();
        for (int i = 0; i <= COLLIDETABLESIZE - 1; i++) {
            for (int j = 0; j <= COLLIDETABLESIZE - 1; j++) {
                for (int k = 0; k <= COLLIDETABLESIZE - 1; k++) {
                    if (!table[i][j][k].empty()) {
                        table[i][j][k].clear();
                    }
                }
            }
        }
    }

///returns any objects residing in the sector occupied by Exact
    int Get(const QVector &Exact, UnitCollection *retval[], bool GetHuge) {
        retval[1] = &table[hash_int(Exact.i)][hash_int(Exact.j)][hash_int(Exact.k)];
        retval[0] = active_huge;
        if (GetHuge) {
            retval[0] = &hugeobjects;
        }
        return 2;
    }

///Returns all objects too big to be conveniently fit in the array
    UnitCollection &GetHuge() {
        return hugeobjects;
    }

///Returns all objects within sector(s) occupied by target
    int Get(const LineCollide *target, UnitCollection *retval[], bool GetHuge) {
        unsigned int sizer = 1;
        double maxx = (ceil(target->Maxi.i / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double maxy = (ceil(target->Maxi.j / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double maxz = (ceil(target->Maxi.k / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        int x, y, z;
        if (target->Mini.i == maxx) {
            maxx += COLLIDETABLEACCURACY / 2;
        }
        if (target->Mini.j == maxy) {
            maxy += COLLIDETABLEACCURACY / 2;
        }
        if (target->Mini.k == maxz) {
            maxz += COLLIDETABLEACCURACY / 2;
        }
        retval[0] = &hugeobjects;
        if (!GetHuge) {
            retval[0] = active_huge;
        }
        if (target->hhuge) {
            return sizer;
        }      //we can't get _everything
        for (double i = target->Mini.i; i <= maxx; i += COLLIDETABLEACCURACY) {
            x = hash_int(i);
            for (double j = target->Mini.j; j <= maxy; j += COLLIDETABLEACCURACY) {
                y = hash_int(j);
                for (double k = target->Mini.k; k <= maxz; k += COLLIDETABLEACCURACY) {
                    z = hash_int(k);
                    if (!table[x][y][z].empty()) {
                        retval[sizer] = &table[x][y][z];
                        sizer++;
                        if (sizer >= HUGEOBJECT + 1) {
                            return sizer;
                        }
                    }
                }
            }
        }
        assert(sizer <= HUGEOBJECT + 1); //make sure we didn't overrun our array
        return sizer;
    }

///Adds objectToPut into collide table with limits specified by target.
    void Put(LineCollide *target, Unit *objectToPut) {
        int x, y, z;
        double maxx = (ceil(target->Maxi.i / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double maxy = (ceil(target->Maxi.j / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double maxz = (ceil(target->Maxi.k / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        //for huge calculation...not sure it's necessary
        double minx = (floor(target->Mini.i / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double miny = (floor(target->Mini.j / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double minz = (floor(target->Mini.k / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        if (target->Mini.i == maxx) {
            maxx += COLLIDETABLEACCURACY / 2;
        }
        if (target->Mini.j == maxy) {
            maxy += COLLIDETABLEACCURACY / 2;
        }
        if (target->Mini.k == maxz) {
            maxz += COLLIDETABLEACCURACY / 2;
        }
        if (fabs((maxx
                - minx)
                * (maxy
                        - miny)
                * (maxz
                        - minz)) > ((double) COLLIDETABLEACCURACY) * ((double) COLLIDETABLEACCURACY)
                * ((double) COLLIDETABLEACCURACY)
                * ((double) HUGEOBJECT)) {
            target->hhuge = true;
            hugeobjects.prepend(objectToPut);
            return;
        } else {
            target->hhuge = false;
        }
        for (double i = target->Mini.i; i < maxx; i += COLLIDETABLEACCURACY) {
            x = hash_int(i);
            for (double j = target->Mini.j; j < maxy; j += COLLIDETABLEACCURACY) {
                y = hash_int(j);
                for (double k = target->Mini.k; k < maxz; k += COLLIDETABLEACCURACY) {
                    z = hash_int(k);
                    table[x][y][z].prepend(objectToPut);
                }
            }
        }
    }

    static bool removeFromVector(UnitCollection &myvector, Unit *objectToKill) {
        return (myvector.remove(objectToKill));
    }

    bool Eradicate(Unit *objectToKill) {
        bool ret = removeFromVector(hugeobjects, objectToKill);
        for (unsigned int i = 0; i <= COLLIDETABLESIZE - 1; i++) {
            for (unsigned int j = 0; j <= COLLIDETABLESIZE - 1; j++) {
                for (unsigned int k = 0; k <= COLLIDETABLESIZE - 1; k++) {
                    ret |= removeFromVector(table[i][j][k], objectToKill);
                }
            }
        }
        return ret;
    }

///Removes objectToKill from collide table with span of Target
    bool Remove(const LineCollide *target, Unit *objectToKill) {
        bool ret = false;
        int x, y, z;
        double maxx = (ceil(target->Maxi.i / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double maxy = (ceil(target->Maxi.j / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        double maxz = (ceil(target->Maxi.k / COLLIDETABLEACCURACY)) * COLLIDETABLEACCURACY;
        if (target->Mini.i == maxx) {
            maxx += COLLIDETABLEACCURACY / 2;
        }
        if (target->Mini.j == maxy) {
            maxy += COLLIDETABLEACCURACY / 2;
        }
        if (target->Mini.k == maxz) {
            maxz += COLLIDETABLEACCURACY / 2;
        }
        if (!target->hhuge) {
            for (double i = target->Mini.i; i < maxx; i += COLLIDETABLEACCURACY) {
                x = hash_int(i);
                for (double j = target->Mini.j; j < maxy; j += COLLIDETABLEACCURACY) {
                    y = hash_int(j);
                    for (double k = target->Mini.k; k < maxz; k += COLLIDETABLEACCURACY) {
                        z = hash_int(k);
                        ret |= removeFromVector(table[x][y][z], objectToKill);
                    }
                }
            }
        }
        if (!ret && !target->hhuge) {
            VS_LOG(error, "Nonfatal Collide Error\n");
        }
        if (!ret || target->hhuge) {
            ret |= removeFromVector(hugeobjects, objectToKill);
        }
        return ret;
    }
};

const int tablehuge = 27;
const int coltableacc = 128;
const int coltablesize = 20;
class CollideTable {
    unsigned int blocupdate;
public:
    CollideTable(StarSystem *ss) : blocupdate(0), c(ss) {
    }

    void Update() {
        ++blocupdate;
    }

    UnitHash3d<char[coltablesize], char[coltableacc], char[tablehuge]> c;
};

void AddCollideQueue(LineCollide &, StarSystem *ss);
bool TableLocationChanged(const QVector &, const QVector &);
bool TableLocationChanged(const LineCollide &, const QVector &, const QVector &);
void KillCollideTable(LineCollide *lc, StarSystem *ss);
bool EradicateCollideTable(LineCollide *lc, StarSystem *ss);

class csOPCODECollider;
const unsigned int collideTreesMaxTrees = 16;
struct collideTrees {
    std::string hash_key;

    csOPCODECollider *rapidColliders[collideTreesMaxTrees];

    bool usingColTree() const {
        return rapidColliders[0] != NULL;
    }

    csOPCODECollider *colTree(Unit *un,
            const Vector &othervelocity);     //gets the appropriately scaled unit collide tree

    // Not sure at the moment where we decide to collide to the shield ...since all we ever compare to is colTree in Collide()
    // Yet, this is used somewhere.
    csOPCODECollider *colShield;

    int refcount;
    collideTrees(const std::string &hk, csOPCODECollider *cT, csOPCODECollider *cS);

    void Inc() {
        refcount++;
    }

    void Dec();
    static collideTrees *Get(const std::string &hash_key);
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLIDE_H
