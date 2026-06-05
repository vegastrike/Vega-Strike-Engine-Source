/*
 * hashtest.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#ifndef _WIN32
#include <ext/hash_map>
#define stdext __gnu_cxx
#else
#include <hash_map>
#endif
#ifndef _WIN32
#include <sys/time.h>
#include <sys/types.h>
#else
#include <windows.h>
static LONGLONG ttime;
static LONGLONG newtime = 0;
static LONGLONG freq;
static double   dblnewtime;
#endif
#include <map>
#include "root_generic/vega_random.h"

size_t globmax = 1024;
class size {
public:
    size_t num;

    size() {
        num = static_cast<size_t>(VegaRandom::Instance().RandomUInt32UpTo(globmax));
    }

    bool operator==(const size &a) const {
        return this->num == a.num;
    }

    explicit operator size_t() const {
        return num;
    }
};

bool operator<(const size &a, const size &b) {
    return a.num < b.num;
}

#ifndef _WIN32
namespace stdext {
template<>
struct hash<size> {
    hash<size_t> a;
public:
    size_t operator()(const size &s) const {
        return a(s.num);
    }
};
}
#endif
double firsttime = 0;

double queryTime() {
#ifdef _WIN32
    LONGLONG tmpnewtime;
    QueryPerformanceCounter( (LARGE_INTEGER*) &tmpnewtime );
    return ( (double) tmpnewtime )/(double) freq-firsttime;

#elif 1
    struct timeval tv;
    (void) gettimeofday(&tv, NULL);
    double tmpnewtime = (double) tv.tv_sec + (double) tv.tv_usec * 1.e-6;
    return tmpnewtime - firsttime;

#elif defined (HAVE_SDL)
    double tmpnewtime = SDL_GetTicks()*1.e-3;
    return tmpnewtime-firsttime;

#else
# error "We have no way to determine the time on this system."
    return 0.;
#endif
}

double getTime() {
    return queryTime();
}

std::map<size_t, size_t> pod;
stdext::hash_map<size, size> clas;

int main(int argc, char **argv) {
#ifdef _WIN32
    QueryPerformanceFrequency( (LARGE_INTEGER*) &freq );
    QueryPerformanceCounter( (LARGE_INTEGER*) &ttime );
#endif

    VegaRandom::Instance().InitGenRand(11376130011);

    const size_t lima = static_cast<size_t>(atof(argv[1]));
    const size_t limb = static_cast<size_t>(atof(argv[2]));
    const size_t limc = static_cast<size_t>(atof(argv[3]));
    const size_t limd = static_cast<size_t>(atof(argv[4]));
    const size_t lime = static_cast<size_t>(atof(argv[5]));
    globmax = lime;
    const std::vector<size> sdata(limb);
    const double first = getTime();
    for (size_t i = 0; i < lima; ++i) {
        size ins, dat;
        pod[ins.num] = dat.num;
        clas[ins] = dat;
    }
    const double second = getTime();
    int sum = 0;
    size tmp;
    for (size_t j = 0; j < limc; ++j) {
        for (size_t i = 0; i < limb; ++i) {
            size_t num = sdata[i].num;
            sum += (pod.find(num) != pod.end() ? pod.find(num)->second : 0);
        }
    }
    const double third = getTime();
    for (size_t j = 0; j < limc; ++j) {
        for (size_t i = 0; i < limb; ++i) {
            size_t num = sdata[i].num;
            tmp.num = num;
            sum += (clas.find(tmp) != clas.end() ? clas.find(tmp)->second.num : 0);
        }
    }
    const double fourth = getTime();
    for (size_t j = 0; j < limc; ++j) {
        for (size_t i = 0; i < limb; ++i) {
            size_t num = sdata[i].num;
            pod.insert(std::pair<size_t, size_t>(num, num));
        }
        if (j != limc - 1 && j < limd) {
            for (size_t i = 0; i < limb; ++i) {
                size_t num = sdata[i].num;
                if (pod.find(num) == pod.end()) {
                    //printf ("error deleting item not in class\n");
                } else {
                    pod.erase(pod.find(num));
                }
            }
        }
    }
    const double fifth = getTime();
    for (size_t j = 0; j < limc; ++j) {
        for (size_t i = 0; i < limb; ++i) {
            tmp.num = sdata[i].num;
            clas.insert(std::pair<size, size>(tmp, tmp));
        }
        if (j != limc - 1) {
            for (size_t i = 0; i < limb; ++i) {
                tmp.num = sdata[i].num;
                if (clas.find(tmp) == clas.end()) {
                    //printf ("error deleting item not in class\n");
                } else {
                    clas.erase(clas.find(tmp));
                }
            }
        }
    }
    const double sixth = getTime();
    for (size_t j = 0; j < limc; ++j) {
        for (size_t i = 0; i < limb; ++i) {
            size_t num = sdata[i].num;
            sum += pod.find(num)->second;
        }
    }
    const double seventh = getTime();
    for (size_t j = 0; j < limc; ++j) {
        for (size_t i = 0; i < limb; ++i) {
            size_t num = sdata[i].num;
            tmp.num = num;
            sum += clas.find(tmp)->second.num;
        }
    }
    const double eighth = getTime();
    printf("Outer Array Size %lu. Num Important Elem %lu. Repeats %lu. Num Erases %lu\nInitial Hit Percent %f\n",
            pod.size(), sdata.size(), limc, limd, sdata.size() / static_cast<double>(lime));
    printf("Find Percent Times %f %f\n", third - second, fourth - third);
    printf("Insert/Erase Times %f %f\n", fifth - fourth, sixth - fifth);
    printf("Find all itm Times %f %f\n", seventh - sixth, eighth - seventh);

    return sum;
}

