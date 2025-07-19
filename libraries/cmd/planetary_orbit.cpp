/*
 * planetary_orbit.cpp
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


#include "cmd/planetary_orbit.h"

#include "cmd/unit_generic.h"
#include "src/vs_logging.h"
#include "root_generic/configxml.h"

PlanetaryOrbit::PlanetaryOrbit(Unit *p,
        double velocity,
        double initpos,
        const QVector &x_axis,
        const QVector &y_axis,
        const QVector &centre,
        Unit *targetunit) : Order(MOVEMENT, 0),
        velocity(velocity),
        theta(initpos),
        inittheta(initpos),
        x_size(x_axis),
        y_size(y_axis),
        current_orbit_frame(0) {
    for (unsigned int t = 0; t < NUM_ORBIT_AVERAGE; ++t) {
        orbiting_average[t] = QVector(0, 0, 0);
    }
    orbiting_last_simatom = simulation_atom_var;
    orbit_list_filled = false;
    p->SetResolveForces(false);
    double delta = x_size.Magnitude() - y_size.Magnitude();
    if (delta == 0) {
        focus = QVector(0, 0, 0);
    } else if (delta > 0) {
        focus = x_size * (delta / x_size.Magnitude());
    } else {
        focus = y_size * (-delta / y_size.Magnitude());
    }
    if (targetunit) {
        type = (MOVEMENT);
        subtype = (SSELF);
        AttachSelfOrder(targetunit);
    } else {
        type = (MOVEMENT);
        subtype = (SLOCATION);
        AttachOrder(centre);
    }

    this->SetParent(p);
}

PlanetaryOrbit::~PlanetaryOrbit() {
    parent->SetResolveForces(true);
}

void PlanetaryOrbit::Execute() {
    bool mining = parent->rSize() > 1444 && parent->rSize() < 1445;
    bool done = this->done;
    this->Order::Execute();
    this->done = done;     //we ain't done till the cows come home
    if (done) {
        return;
    }
    QVector origin(targetlocation);
    const float orbit_centroid_averaging = configuration()->physics.orbit_averaging;
    float averaging = (float) orbit_centroid_averaging / (float) (parent->predicted_priority + 1.0f);
    if (averaging < 1.0f) {
        averaging = 1.0f;
    }
    if (subtype & SSELF) {
        Unit *unit = group.GetUnit();
        if (unit) {
            unsigned int o = current_orbit_frame++;
            current_orbit_frame %= NUM_ORBIT_AVERAGE;
            if (current_orbit_frame == 0) {
                orbit_list_filled = true;
            }
            QVector desired = unit->prev_physical_state.position;
            if (orbiting_average[o].i == 0 && orbiting_average[o].j == 0 && orbiting_average[o].k == 0) {
                //clear all of them.
                for (o = 0; o < NUM_ORBIT_AVERAGE; o++) {
                    orbiting_average[o] = desired;
                }
                orbiting_last_simatom = simulation_atom_var;
                current_orbit_frame = 2;
                orbit_list_filled = false;
            } else {
                if (simulation_atom_var != orbiting_last_simatom) {
                    VS_LOG(trace,
                            (boost::format(
                                    "void PlanetaryOrbit::Execute(): simulation_atom_var, %1$.6f, != orbiting_last_simatom, %2$.6f, for planet %3$s")
                                    % simulation_atom_var % orbiting_last_simatom % this->parent->name));
                    QVector sum_diff(0, 0, 0);
                    QVector sum_position;
                    int limit;
                    if (orbit_list_filled) {
                        sum_position = orbiting_average[o];
                        limit = NUM_ORBIT_AVERAGE - 1;
                        o = (o + 1) % NUM_ORBIT_AVERAGE;
                    } else {
                        sum_position = orbiting_average[0];
                        limit = o;
                        o = 1;
                    }
                    for (int i = 0; i < limit; i++) {
                        sum_diff += (orbiting_average[o]
                                - orbiting_average[(o + NUM_ORBIT_AVERAGE - 1) % NUM_ORBIT_AVERAGE]);
                        sum_position += orbiting_average[o];
                        o = (o + 1) % NUM_ORBIT_AVERAGE;
                    }
                    if (limit != 0) {
                        sum_diff *= (1. / (limit));
                    }
                    sum_position *= (1. / (limit + 1));

                    float ratio_simatom = (simulation_atom_var / orbiting_last_simatom);
                    sum_diff *= ratio_simatom;
                    unsigned int number_to_fill;
                    number_to_fill = (int) ((NUM_ORBIT_AVERAGE / ratio_simatom) + .99);
                    if (number_to_fill > NUM_ORBIT_AVERAGE) {
                        number_to_fill = NUM_ORBIT_AVERAGE;
                    }
                    if (ratio_simatom <= 1) {
                        number_to_fill = NUM_ORBIT_AVERAGE;
                    }
                    //subtract it so the average remains the same.
                    sum_position += (sum_diff * (number_to_fill / -2.));
                    for (o = 0; o < number_to_fill; o++) {
                        orbiting_average[o] = sum_position;
                        sum_position += sum_diff;
                    }
                    orbit_list_filled = (o >= NUM_ORBIT_AVERAGE - 1);
                    o %= NUM_ORBIT_AVERAGE;
                    current_orbit_frame = (o + 1) % NUM_ORBIT_AVERAGE;
                    orbiting_last_simatom = simulation_atom_var;
                }
                orbiting_average[o] = desired;
            }
        } else {
            done = true;
            parent->SetResolveForces(true);
            return;             //flung off into space.
        }
    }
    QVector sum_orbiting_average(0, 0, 0);
    {
        int limit;
        if (orbit_list_filled) {
            limit = NUM_ORBIT_AVERAGE;
        } else {
            limit = current_orbit_frame;
        }
        for (int o = 0; o < limit; o++) {
            sum_orbiting_average += orbiting_average[o];
        }
        sum_orbiting_average *= 1. / (limit == 0 ? 1 : limit);
    }
    const double div2pi = (1.0 / (2.0 * PI));
    theta += velocity * simulation_atom_var * div2pi;

    QVector x_offset = cos(theta) * x_size;
    QVector y_offset = sin(theta) * y_size;

    QVector destination = origin - focus + sum_orbiting_average + x_offset + y_offset;
    double mag = (destination - parent->LocalPosition()).Magnitude();
    parent->Velocity = parent->cumulative_velocity =
            (((destination - parent->LocalPosition()) * (1. / simulation_atom_var)).Cast());
    const float Unreasonable_value = configuration()->physics.planet_ejection_stophack;
    float v2 = parent->Velocity.Dot(parent->Velocity);
    if (v2 > Unreasonable_value * Unreasonable_value) {
        VS_LOG(debug,
                (boost::format(
                        "void PlanetaryOrbit::Execute(): A velocity value considered unreasonable was calculated for planet %1%; zeroing it out")
                        % this->parent->name));
        parent->Velocity.Set(0, 0, 0);
        parent->cumulative_velocity.Set(0, 0, 0);
        parent->SetCurPosition(origin - focus + sum_orbiting_average + x_offset + y_offset);
    }
}
