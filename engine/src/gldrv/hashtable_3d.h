/*
 * hashtable_3d.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GLDRV_HASHTABLE_3D_H
#define VEGA_STRIKE_ENGINE_GLDRV_HASHTABLE_3D_H

#include "gfx/vec.h"
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include "linecollide.h"
#include "vs_logging.h"

//#define COLLIDETABLESIZE sizeof(CTSIZ)
//#define COLLIDETABLEACCURACY sizeof (CTACCURACY)
///objects that go over 16 sectors are considered huge and better to check against everything.
//#define HUGEOBJECT sizeof (CTHUGE)

/**
 * Hashtable3d is a 3d datastructure that holds various starships that are
 * near enough to crash into each other (or also lights that are big enough
 * to shine on nearby units.
 */
template<class T, int COLLIDETABLESIZE, int COLLIDETABLEACCURACY, int HUGEOBJECT>
class Hashtable3d {
///All objects that are too large to fit (fastly) in the collide table
    std::vector<T> hugeobjects;
///The hash table itself. Holds most units to be collided with
    std::vector<T> table[COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];

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
///Hashes a single value to a value on the collide table truncated to all 3d constraints.  Consider using a swizzle
    static int hash_int(const double aye) {
        return ((int) (((aye < 0) ? (aye
                - COLLIDETABLEACCURACY) : aye)
                / COLLIDETABLEACCURACY)) % (COLLIDETABLESIZE / 2) + (COLLIDETABLESIZE / 2);
    }

///clears entire table
    void Clear() {
        hugeobjects.clear();
        for (int i = 0; i <= COLLIDETABLESIZE - 1; i++) {
            for (int j = 0; j <= COLLIDETABLESIZE - 1; j++) {
                for (int k = 0; k <= COLLIDETABLESIZE - 1; k++) {
                    if (table[i][j][k].size()) {
                        table[i][j][k].clear();
                    }
                }
            }
        }
    }

///returns any objects residing in the sector occupied by Exact
    int Get(const QVector &Exact, std::vector<T> *retval[]) {
        retval[1] = &table[hash_int(Exact.i)][hash_int(Exact.j)][hash_int(Exact.k)];
        //retval+=hugeobjects;
        //blah = blooh;
        retval[0] = &hugeobjects;
        return 2;
    }

///Returns all objects too big to be conveniently fit in the array
    std::vector<T> &GetHuge() {
        return hugeobjects;
    }

///Returns all objects within sector(s) occupied by target
    int Get(const LineCollide *target, std::vector<T> *retval[]) {
        unsigned int sizer = 1;
        //int minx,miny,minz,maxx,maxy,maxz;
        //hash_vec(Min,minx,miny,minz);
        //hash_vec(Max,maxx,maxy,maxz);
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
        if (target->hhuge) {
            return sizer;      //we can't get _everything
        }
        for (double i = target->Mini.i; i < maxx; i += COLLIDETABLEACCURACY) {
            x = hash_int(i);
            for (double j = target->Mini.j; j < maxy; j += COLLIDETABLEACCURACY) {
                y = hash_int(j);
                for (double k = target->Mini.k; k < maxz; k += COLLIDETABLEACCURACY) {
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
    void Put(LineCollide *target, const T objectToPut) {
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
            hugeobjects.push_back(objectToPut);
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
                    table[x][y][z].push_back(objectToPut);
                }
            }
        }
    }

    static bool removeFromVector(std::vector<T> &myvector, T &objectToKill) {
        bool ret = false;
        typename std::vector<T>::iterator removal = myvector.begin();
        while (removal != myvector.end()) {
            removal = std::find(removal, myvector.end(), objectToKill);
            if (removal != myvector.end()) {
                ret = true;
                int offset = removal - myvector.begin();
                objectToKill = *removal;
                myvector.erase(removal);
                removal = myvector.begin() + offset;
            }
        }
        return ret;
    }

    bool Eradicate(T objectToKill) {
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
    bool Remove(const LineCollide *target, T &objectToKill) {
        //int minx,miny,minz,maxx,maxy,maxz;
        //hash_vec(target->Mini,minx,miny,minz);
        //hash_vec(target->Maxi,maxx,maxy,maxz);
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
            VS_LOG(error, "Nonfatal Collide Error");
        }
        if (!ret || target->hhuge) {
            ret |= removeFromVector(hugeobjects, objectToKill);
        }
        return ret;
    }
};

#endif //VEGA_STRIKE_ENGINE_GLDRV_HASHTABLE_3D_H
