/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
// NO HEADER GUARD

#ifdef WIN32
#include <time.h>
#else //WIN32

#include <unistd.h>
#include <sys/time.h>

#define RESETTIME() startTime()
#define REPORTTIME(comment) endTime( comment, __FILE__, __LINE__ )

static timeval start;

static inline void startTime() {
    gettimeofday(&start, NULL);
}

static inline void endTime(const char *comment, const char *file, int lineno) {
    timeval end;
    gettimeofday(&end, NULL);
    double time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
    std::clog << file << "(" << comment << "):" << lineno << ": " << time << std::endl;
}

#endif //WIN32
