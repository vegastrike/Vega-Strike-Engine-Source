/*
 * Stream.cpp
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

//
// C++ Implementation: Audio::Stream
//


#include "Stream.h"

#include <utility>
#include <cstring>

//using namespace std;

namespace Audio {

using std::min;

Stream::Stream(const std::string &path) {
}

Stream::~Stream() {
}

double Stream::getLength() {
    return getLengthImpl();
}

double Stream::getPosition() const {
    return getPositionImpl();
}

void Stream::seek(double position) {
    seekImpl(position);
}

unsigned int Stream::read(void *buffer, unsigned int bufferSize) {
    void *rbuffer;
    void *rbufferEnd;
    unsigned int rbufferSize;
    unsigned int rode = 0;

    try {
        getBufferImpl(rbuffer, rbufferSize);
    } catch (const NoBufferException &) {
        nextBufferImpl();
        getBufferImpl(rbuffer, rbufferSize);
        curBufferPos = rbuffer;
    }
    rbufferEnd = ((char *) rbuffer) + rbufferSize;

    while (bufferSize > 0) {
        if (!((curBufferPos >= rbuffer) && (curBufferPos < rbufferEnd))) {
            nextBufferImpl();
            getBufferImpl(rbuffer, rbufferSize);
            curBufferPos = rbuffer;
            rbufferEnd = ((char *) rbuffer) + rbufferSize;
        }

        size_t remaining = min(bufferSize,
                (unsigned int) ((char *) rbufferEnd
                        - (char *) curBufferPos)); //is there no std::ptrdiff?
        memcpy(buffer, curBufferPos, remaining);
        buffer = (void *) ((char *) buffer + remaining);
        curBufferPos = (void *) ((char *) curBufferPos + remaining);
        bufferSize -= remaining;
        rode += remaining;
    }

    return rode;
}

};
