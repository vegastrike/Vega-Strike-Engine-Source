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
#ifndef VEGA_STRIKE_ENGINE_RESOURCE_COUT_UTIL_H
#define VEGA_STRIKE_ENGINE_RESOURCE_COUT_UTIL_H

#include <iostream>

void printPlayerMessage(std::string key,
                        std::string message,
                        std::string value) {
    if(key == "player_ship") {
        std::cout << message << " : " << value << std::endl;
    } 
}

void printOnceInAHundred(int &i,
                         std::string key,
                         std::string message,
                         std::string value) {
    if(i==0) {
        printPlayerMessage(key,message,value);
    }
    i++;
    if(i==100) i=0;
}


#endif //VEGA_STRIKE_ENGINE_RESOURCE_COUT_UTIL_H
