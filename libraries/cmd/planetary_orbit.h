/*
 * planetary_orbit.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_PLANETARY_ORBIT_H
#define VEGA_STRIKE_ENGINE_CMD_PLANETARY_ORBIT_H

#include "gfx_generic/vec.h"
#include "src/star_system.h"
#include "cmd/ai/order.h"

class PlanetaryOrbit : public Order {
private:
    double velocity;
    double theta;
    double inittheta;

    QVector x_size;
    QVector y_size;
    QVector focus;
#define ORBIT_PRIORITY 8
#define NUM_ORBIT_AVERAGE (SIM_QUEUE_SIZE/ORBIT_PRIORITY)
    QVector orbiting_average[NUM_ORBIT_AVERAGE];
    float orbiting_last_simatom;
    int current_orbit_frame;
    bool orbit_list_filled;
protected:
///A vector containing all lihgts currently activated on current planet
    std::vector<int> lights;

public:
    PlanetaryOrbit(Unit *p,
            double velocity,
            double initpos,
            const QVector &x_axis,
            const QVector &y_axis,
            const QVector &Centre,
            Unit *target = NULL);
    ~PlanetaryOrbit();
    void Execute();
};

#endif //VEGA_STRIKE_ENGINE_CMD_PLANETARY_ORBIT_H
