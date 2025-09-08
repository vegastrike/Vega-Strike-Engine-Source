/*
 * resizable.h
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

#ifndef VEGA_STRIKE_ENGINE_SRC_RESIZABLE_H
#define VEGA_STRIKE_ENGINE_SRC_RESIZABLE_H

#include <cassert>
#include <cstring>

#include "vs_logging.h"

template<class ITEM>
class Resizable {
    unsigned int num;
    unsigned int alloc;
    ITEM *q;
public:
    Resizable() {
        num = 0;
        alloc = 16;
        q = new ITEM[16]{};
    }

    Resizable(const Resizable<ITEM> &c) {
        num = c.num;
        alloc = c.alloc;
        q = new ITEM[alloc]{};
        for (unsigned int i = 0; i < c.num; ++i) {
            q[i] = const_cast<Resizable<ITEM>&>(c)[i];
        }
    }

    ~Resizable() {
        delete[] q;
        q = nullptr;
        num = 0;
        alloc = 0;
    }

private:
    void grow_storage(unsigned int n) {
        while (n + num > alloc) {
            alloc *= 2;
            std::vector<ITEM> tmp;
            for (unsigned int i = 0; i < num; ++i) {
                tmp.push_back(q[i]);
            }
            delete [] q;
            q = new ITEM[alloc]{};
            unsigned int i = 0;
            for (auto item : tmp) {
                q[i++] = item;
            }
            for (; i < alloc; ++i) {
                ITEM blank_item{};
                q[i] = blank_item;
            }
        }
    }

public:
    void assert_free(unsigned int n) {
        grow_storage(n);
    }

    void push_back_nocheck(const ITEM &a) {
        q[num] = a;
        num++;
    }

    void inc_num(unsigned int n) {
        num += n;
    }

    void push_back(const ITEM &a) {
        grow_storage(1);
        q[num] = a;
        num++;
    }

    void push_back3(const ITEM aa[3]) {
        grow_storage(3);
        q[num] = aa[0];
        q[num + 1] = aa[1];
        q[num + 2] = aa[2];
        num += 3;
    }

    void push_back(const ITEM &aa, const ITEM &bb, const ITEM &cc) {
        grow_storage(3);
        q[num] = aa;
        q[num + 1] = bb;
        q[num + 2] = cc;
        num += 3;
    }

    void push_backN(const ITEM *aa, const unsigned int N) {
        grow_storage(N);
        for (unsigned int i = 0; i < N; i++) {
            q[num + i] = aa[i];
        }
        num += N;
    }

    ITEM *begin() {
        return q;
    }

    ITEM *end() {
        return q + num;
    }

    void clear() {
        num = 0;
    }

    ITEM &back() {
        return q[num - 1];
    }

    ITEM &pop_back() {
        num--;
        return q[num];
    }

    unsigned int size() {
        return num;
    }

    ITEM &operator[](unsigned int i) {
        if (i >= num) {
            VS_LOG(error, "out of bounds");
            static ITEM blank_item;
            return blank_item;
        }
        return q[i];
    }

    ITEM &at(unsigned int i) {
        if (i >= num) {
            VS_LOG(error, "out of bounds");
            static ITEM blank_item;
            return blank_item;
        }
        return q[i];
    }
};
#endif // VEGA_STRIKE_ENGINE_SRC_RESIZABLE_H
