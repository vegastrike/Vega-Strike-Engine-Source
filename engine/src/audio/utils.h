/**
 * utils.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_UTILS_H
#define VEGA_STRIKE_ENGINE_AUDIO_UTILS_H

#include "Types.h"
#include "Exceptions.h"
#include <map>

namespace Audio {

// Forward declarations
class Listener;
class Source;

/** Get the game time stamp - ie, the time as it elapses in the game's universe */
Timestamp getGameTime();

/** Get the current real time stamp */
Timestamp getRealTime();

/** Estimate a distant source's gain
 * @remarks Computes source attenuation relative to a listener.
 *      The computation is an estimated (a rather accurate estimate though).
 *      Advanced factors like atmospheric absorption and environment filtering
 *      may make the final attenuation differ significantly, so this should only
 *      be used for culling purposes.
 */
Scalar estimateGain(const Source &src, const Listener &listener);

/** Make the thread sleep for at least 'ms' milliseconds.
 * @remarks sleep(0) is a very common way to implement a waiting loop:
 *      @code while (condition) sleep(0);
 *      @par Interval precision is highly dependant on the platform. Usually,
 *      it lays close to 15ms.
 */
void sleep(unsigned int ms);

/** Parse an enumerated string into its actual value
 * @param s The string to be parsed
 * @param enumMap A map associating strings to values
 * @remarks Returns enumMap[s] raising an exception on error.
 */
template<typename T>
T parseEnum(const std::string &s, const std::map<std::string, T> &enumMap) {
    typename std::map<std::string, T>::const_iterator it = enumMap.find(s);
    if (it != enumMap.end()) {
        return it->second;
    } else {
        throw InvalidParametersException("Enumerated value \"" + s + "\" not recognized");
    }
}

/** Parse an enumerated string into its actual value
 * @param s The string to be parsed
 * @param enumMap A map associating strings to values
 * @param deflt A default value to return when s is found in the enumeration map.
 * @remarks Returns enumMap[s] or the default, no exceptions raised.
 */
template<typename T>
T parseEnum(const std::string &s, const std::map<std::string, T> &enumMap, T deflt) {
    typename std::map<std::string, T>::const_iterator it = enumMap.find(s);
    if (it != enumMap.end()) {
        return it->second;
    } else {
        return deflt;
    }
}

}

#endif //VEGA_STRIKE_ENGINE_AUDIO_UTILS_H
