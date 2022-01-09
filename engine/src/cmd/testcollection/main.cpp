/**
 * main.cpp
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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "unit.h"
#include <time.h>

#include "collection.h"

#define SIZE 60000

/* to build:
 *       g++  -pipe -O2 -DLIST_TESTING=1 -I. -I..  -o testcol ../collection.cpp ./main.cpp
 */
Unit *createUnit()
{
    return new Unit(false);
}

void Iteration(UnitCollection *c, int *levels2)
{
    Unit *unit = NULL;
    ++(*levels2);
    for (un_iter iter = c->createIterator(); unit = *iter;) {
        int temp = rand();
        if (temp < RAND_MAX / 400) {
            unit->Kill();
        } else if (temp < RAND_MAX / 102) {
            iter.remove();
            continue;
        } else if (temp < RAND_MAX / 90) {
            Iteration(c, levels2);
        }
        ++iter;
    }
}

int main()
{
    Unit *unit;
    srand(time(NULL));
    UnitCollection *c = new UnitCollection;
    Unit *u[SIZE];
    time_t seconds;
    for (int i = 0; i < SIZE; ++i) {
        u[i] = createUnit();
    }
    seconds = time(NULL);
    for (int i = 0; i < (SIZE / 2); ++i) {
        c->prepend(u[i]);
    }
    seconds = time(NULL) - seconds;
    printf("constructed list of size : %d  in %d seconds using prepend\n", SIZE / 2, seconds);
    printf("Randomnly inserting %d \n", SIZE / 2);
    int ii = SIZE / 2;
    seconds = time(NULL);
    while (ii < SIZE) {
        for (un_iter iter = c->createIterator(); !iter.isDone() && ii < SIZE; ++iter) {
            int rnd = rand();
            if (rnd < RAND_MAX / 200) {
                iter.postinsert(u[ii]);
                ++ii;
            } else if (rnd < RAND_MAX / 100) {
                iter.preinsert(u[ii]);
                ++ii;
            }
        }
    }
    seconds = time(NULL) - seconds;
    printf(".... took %d seconds \n", seconds);
    for (int i = 0; i < SIZE / 32; ++i) {
        if (rand() < RAND_MAX / 20) {
            u[i]->Kill();
        }
    }
    printf("randomly killed SIZE/32 to start off with \n");

    printf("beginning unitCollection removal/advance operations\n");
    seconds = time(NULL);
    int passes = 0;
    int levels = 0;
    while (!c->empty()) {
        ++passes;
        Iteration(c, &levels);
#if !oldtest
    }
#else
    UnitCollection::FreeUnusedNodes();
}
UnitCollection::FreeUnusedNodes();
#endif
    seconds = time(NULL) - seconds;
    printf("operations took %d seconds \n", seconds);
    levels = levels / passes;
    printf("Average number of concurrent iterators %d\n", levels);
    printf("verifying destruction of units\n");
    for (unsigned int i = 0; i < SIZE; i++) {
        if (u[i]->zapped != u[i]->killed) {
            printf("Unit at %d is zapped %d  killed %d\n", i, u[i]->zapped, u[i]->killed);
        } else {
            delete u[i];
        }
    }
    int size = 0;
    for (un_iter counter = c->createIterator(); !counter.isDone(); ++counter) {
        ++size;
    }
    printf("size of list is verified at %d  : %d killed \n", size, SIZE - size);
    delete c;
    printf("completed\n");
    return 0;
}

