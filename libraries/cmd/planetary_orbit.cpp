/*
 * planetary_orbit.cpp
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
    orbit_phase_initialized = false;
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

//A point on the orbit, as an offset from the orbit's centre.
static QVector OrbitOffset(double t, const QVector &x_size, const QVector &y_size) {
    return (std::cos(t) * x_size) + (std::sin(t) * y_size);
}

//The phase whose point on the orbit lies nearest to a given offset from the orbit's centre.
//The two axes are not always perpendicular - some systems author them anti-parallel, which
//collapses the orbit to a line segment - so this searches the orbit rather than projecting
//the offset onto each axis.
static double NearestOrbitPhase(const QVector &x_size, const QVector &y_size, const QVector &offset) {
    const int kCoarseSteps = 64;
    int nearest_step = 0;
    double nearest_distance = -1.0;
    for (int step = 0; step < kCoarseSteps; ++step) {
        const double candidate = (2.0 * PI * step) / kCoarseSteps;
        const double distance = (OrbitOffset(candidate, x_size, y_size) - offset).MagnitudeSquared();
        if (nearest_distance < 0.0 || distance < nearest_distance) {
            nearest_distance = distance;
            nearest_step = step;
        }
    }
    //A coarse step is a whole degree of the orbit, which at orbital distances is far too
    //coarse to leave a body where it stands, so narrow the phase down within the
    //neighbourhood of the nearest step.
    const double step_size = (2.0 * PI) / kCoarseSteps;
    double low = (nearest_step - 1) * step_size;
    double high = (nearest_step + 1) * step_size;
    const double kGoldenRatio = 0.6180339887498949;
    double left = high - (kGoldenRatio * (high - low));
    double right = low + (kGoldenRatio * (high - low));
    for (int step = 0; step < 64; ++step) {
        const double left_distance = (OrbitOffset(left, x_size, y_size) - offset).MagnitudeSquared();
        const double right_distance = (OrbitOffset(right, x_size, y_size) - offset).MagnitudeSquared();
        if (left_distance < right_distance) {
            high = right;
            right = left;
            left = high - (kGoldenRatio * (high - low));
        } else {
            low = left;
            left = right;
            right = low + (kGoldenRatio * (high - low));
        }
    }
    return (low + high) / 2.0;
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
    const float orbit_centroid_averaging = configuration().physics.orbit_averaging_flt;
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
    if (!orbit_phase_initialized) {
        // theta starts at the "position" attribute, and a saved game does not carry an
        // orbit's phase - so when a unit loads, theta restarts at its initial value while
        // the unit keeps the position it was saved at. Recover the phase from that position
        // instead, so the body stays where it is rather than being moved onto the orbit.
        orbit_phase_initialized = true;
        const QVector orbit_centre = origin - focus + sum_orbiting_average;
        theta = NearestOrbitPhase(x_size, y_size, parent->LocalPosition() - orbit_centre);
    }
    const double div2pi = (1.0 / (2.0 * PI));
    const double theta_rate = velocity * div2pi;      //radians per second
    theta += theta_rate * simulation_atom_var;

    QVector destination = origin - focus + sum_orbiting_average
            + OrbitOffset(theta, x_size, y_size);

    //The velocity is the orbit's own motion in closed form: the derivative of
    //cos(theta)*x_size + sin(theta)*y_size, plus whatever the body being orbited is doing
    //(which is how the body is carried along with its parent). Taking it instead from the
    //difference between this body's position and the orbit point would turn a position
    //error into speed, so any body that had drifted off its orbit - a loaded game, or a
    //system that places a body off the orbit it gives it - would fling itself, and
    //anything docked to it, across the system.
    QVector orbit_velocity = ((-std::sin(theta) * x_size) + (std::cos(theta) * y_size)) * theta_rate;
    Unit *orbitee = (subtype & SSELF) ? group.GetUnit() : nullptr;
    if (orbitee != nullptr) {
        orbit_velocity += orbitee->Velocity.Cast();
    }
    parent->Velocity = parent->cumulative_velocity = orbit_velocity.Cast();
    const float Unreasonable_value = configuration().physics.planet_ejection_stophack_flt;
    float v2 = parent->Velocity.Dot(parent->Velocity);
    if (v2 > Unreasonable_value * Unreasonable_value) {
        VS_LOG(debug,
                (boost::format(
                        "void PlanetaryOrbit::Execute(): A velocity value considered unreasonable was calculated for planet %1%; zeroing it out")
                        % this->parent->name));
        parent->Velocity.Set(0, 0, 0);
        parent->cumulative_velocity.Set(0, 0, 0);
    }
    //The orbit is the authority for where this body is, and that correction stays
    //positional: routing it back through Velocity is what made it a speed.
    parent->SetCurPosition(destination);
}
