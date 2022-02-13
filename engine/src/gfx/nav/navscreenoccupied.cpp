/**
 * navscreenoccupied.cpp
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


#include "navscreenoccupied.h"

#include "gfx/masks.h"

navscreenoccupied::navscreenoccupied() {
    sector0 = 0;
    sector1 = 0;
    sector2 = 0;
    sector3 = 0;
    sector4 = 0;
    sector5 = 0;
    sector6 = 0;
    sector7 = 0;
    sector8 = 0;
    sector9 = 0;

    screenskipby4[0] = -.5;
    screenskipby4[1] = 0.5;
    screenskipby4[2] = -.5;
    screenskipby4[3] = 0.5;

    markreturned = 0;
}

navscreenoccupied::navscreenoccupied(float x_small, float x_large, float y_small, float y_large, bool _markreturned) {
    sector0 = 0;
    sector1 = 0;
    sector2 = 0;
    sector3 = 0;
    sector4 = 0;
    sector5 = 0;
    sector6 = 0;
    sector7 = 0;
    sector8 = 0;
    sector9 = 0;

    screenskipby4[0] = x_small;
    screenskipby4[1] = x_large;
    screenskipby4[2] = y_small;
    screenskipby4[3] = y_large;

    markreturned = _markreturned;
}

void navscreenoccupied::reset() {
    sector0 = 0;
    sector1 = 0;
    sector2 = 0;
    sector3 = 0;
    sector4 = 0;
    sector5 = 0;
    sector6 = 0;
    sector7 = 0;
    sector8 = 0;
    sector9 = 0;
}

float navscreenoccupied::findfreesector(float x, float y) {
    float percent = ((x - screenskipby4[0]) / (screenskipby4[1] - screenskipby4[0]));
    float answer = 0.0;
    //strange structure, but it uses n tests for n cases, instead of 2 '(x<y)&&(x>z)'-per-case
    //though i could have done *10 and turned it onto an int... variety is fun
    if (percent >= 0.1) {
        //0.1 - n
        if (percent >= 0.2) {
            //0.2 - n
            if (percent >= 0.3) {
                //0.3 - n
                if (percent >= 0.4) {
                    //0.4 - n
                    if (percent >= 0.5) {
                        //0.5 - n
                        if (percent >= 0.6) {
                            //0.6 - n
                            if (percent >= 0.7) {
                                //0.7 - n
                                if (percent >= 0.8) {
                                    //0.8 - n
                                    if (percent >= 0.9) {                                           //0.9 - n
                                        answer = findfreefloat(sector9, y);
                                    } else {                                             //percent 0.8 - 0.9
                                        answer = findfreefloat(sector8, y);
                                    }
                                } else {
                                    //percent 0.7 - 0.8

                                    answer = findfreefloat(sector7, y);
                                }
                            } else {
                                //percent 0.6 - 0.7

                                answer = findfreefloat(sector6, y);
                            }
                        } else {
                            //percent 0.5 - 0.6

                            answer = findfreefloat(sector5, y);
                        }
                    } else {
                        //percent 0.4 - 0.5

                        answer = findfreefloat(sector4, y);
                    }
                } else {
                    //percent 0.3 - 0.4

                    answer = findfreefloat(sector3, y);
                }
            } else {
                //percent 0.2 - 0.3

                answer = findfreefloat(sector2, y);
            }
        } else {
            //percent 0.1 - 0.2

            answer = findfreefloat(sector1, y);
        }
    } else {
        //(percent < 0.1)

        answer = findfreefloat(sector0, y);
    }
    return answer;
}

float navscreenoccupied::findfreefloat(int &sector, float &y) {
    //convert the y into a height of 32
    //scan that 32 down untill a free power
    //set that power used and return its float

//( [.2] - [-.5] ) / ( [.5] - [-.5] )
//( [.7] ) / ( [1] ) = .7
    float percent = ((y - screenskipby4[2]) / (screenskipby4[3] - screenskipby4[2]));

    percent = percent * 31.0;
    int bit = int(percent);

    float answer = y;
    while (checkbit(sector, bit)) {
        bit -= 1;
    }
    if (bit < 0) {
        bit = 0;
        //run back up (0 -> 31 is upwards)
        while (checkbit(sector, bit)) {
            bit += 1;
        }
        if (bit > 31) {
            //was all full

            //set it to the default
            return y;
        } else {
            //set the flaot to this %
            percent = (float(bit) / float(31));
            if (markreturned == 1) {
                dosetbit(sector, bit);
            }
        }
    } else {
        //set the float to this %
        percent = (float(bit) / float(31));
        if (markreturned == 1) {
            dosetbit(sector, bit);
        }
    }
    answer = screenskipby4[2] + ((screenskipby4[3] - screenskipby4[2]) * percent);
    return answer;
}

