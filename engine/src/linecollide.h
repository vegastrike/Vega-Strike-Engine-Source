/*
 * linecollide.h
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
#ifndef VEGA_STRIKE_ENGINE_LINECOLLIDE_H
#define VEGA_STRIKE_ENGINE_LINECOLLIDE_H

#include <stdlib.h>
#include "gfx_generic/vec.h"

/**
 * Line Collide provides a complete container for a unit that is put in a collide hashtable
 * For collisions. The mini, maxi vectors of the line collide are taken
 */
class Unit;
class Beam;
class Bolt;

struct LineCollide {
private:
    union OBJECT {
        Unit *u;
        Beam *b;
        Bolt *blt;
        int i;
    };

public:
    enum collidables { UNIT, BEAM, BALL, BOLT, PROJECTILE };

    ///The object that this LineCollide approximates
    OBJECT object;
    ///The minimum x,y,z that this object has
    QVector Mini;
    ///The maximum x,y,z that this object has
    QVector Maxi;
    /**
     * The last item that checked this for collisions
     * to prevent duplicate selection
     */
    void *lastchecked;

    ///Which type of unit it is. Used for subsequently calling object's Accurate collide func
    collidables type;

    ///If this object was saved as a huge object (hhuge for dos oddities)
    bool hhuge;

    LineCollide() :
            Mini(0, 0, 0), Maxi(0, 0, 0), lastchecked(NULL), type(UNIT), hhuge(false) {
        object.u = NULL;
    }

    LineCollide(void *objec, collidables typ, const QVector &st, const QVector &en) :
            Mini(st), Maxi(en), lastchecked(NULL), type(typ), hhuge(false) {
        this->object.u = (Unit *) objec;
    }

    LineCollide(const LineCollide &l) :
            Mini(l.Mini), Maxi(l.Maxi), lastchecked(NULL), type(l.type), hhuge(l.hhuge) {
        object = l.object;
    }

    LineCollide &operator=(const LineCollide &l) {
        object = l.object;
        type = l.type;
        Mini = l.Mini;
        Maxi = l.Maxi;
        hhuge = l.hhuge;
        lastchecked = NULL;
        return *this;
    }
};

#endif //VEGA_STRIKE_ENGINE_LINECOLLIDE_H

