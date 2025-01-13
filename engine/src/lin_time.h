/*
 * lin_time.h
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
#ifndef VEGA_STRIKE_ENGINE_LIN_TIME_H
#define VEGA_STRIKE_ENGINE_LIN_TIME_H

#include "vegastrike.h"

#define VEGA_EPOCH 1136073600. //Jan 1, 2006 GMT

void InitTime();
double GetElapsedTime();
void UpdateTime();
void micro_sleep(unsigned int n);
double getNewTime();
void setNewTime(double newnewtime);

//Essentially calling UpdateTime();getNewTime() without modifying any state.
//Always use this except at the beginning of a frame.
double queryTime();

//like query time, but returns a monotonous increasing sequence
double realTime();

extern float getTimeCompression();
extern void setTimeCompression(float tc);

#endif //VEGA_STRIKE_ENGINE_LIN_TIME_H
