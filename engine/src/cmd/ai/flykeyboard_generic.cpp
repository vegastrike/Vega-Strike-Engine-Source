/*
 * flykeyboard_generic.cpp
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


#include "flykeyboard.h"
#include "cmd/unit_generic.h"
#include "autodocking.h"
#include "config_xml.h"
#include "xml_support.h"
#include "vs_globals.h"
#include "lin_time.h"

float FlyByKeyboard::clamp_axis(float v) {
    static int axis_scale = XMLSupport::parse_int(vs_config->getVariable("physics", "slide_start", "3"));
    int as = parent->GetComputerData().slide_start;
    if (as == 0) {
        as = axis_scale;
    }
    if (as) {
        v /= as;
        if (v > 1) {
            return 1;
        }
        if (v < -1) {
            return -1;
        }
    }
    return v;
}

float FlyByKeyboard::reduce_axis(float v) {
    static int axis_scale = XMLSupport::parse_int(vs_config->getVariable("physics", "slide_end", "2"));
    int as = parent->GetComputerData().slide_end;
    if (as == 0) {
        as = axis_scale;
    }
    if (as && fabs(v) > as) {
        if (v > 0) {
            v -= as;
        } else {
            v += as;
        }
    } else {
        v = 0;
    }
    return v;
}

void FlyByKeyboard::Destroy() {
    if (autopilot) {
        autopilot->Destroy();
    }
    Order::Destroy();
}

FlyByKeyboard::~FlyByKeyboard() {
}

void FlyByKeyboard::KeyboardUp(float v) {
    if (v == 0) {
        axis_key.i = reduce_axis(axis_key.i);
    } else {
        if ((v > 0) == (axis_key.i >= 0)) {
            axis_key.i += v;
        } else {
            axis_key.i = v;
        }
    }
    Up(clamp_axis(axis_key.i));
}

void FlyByKeyboard::KeyboardRight(float v) {
    if (v == 0) {
        axis_key.j = reduce_axis(axis_key.j);
    } else {
        if ((v > 0) == (axis_key.j >= 0)) {
            axis_key.j += v;
        } else {
            axis_key.j = v;
        }
    }
    Right(clamp_axis(axis_key.j));
}

void FlyByKeyboard::KeyboardRollRight(float v) {
    if (v == 0) {
        axis_key.k = reduce_axis(axis_key.k);
    } else {
        if ((v > 0) == (axis_key.k >= 0)) {
            axis_key.k += v;
        } else {
            axis_key.k = v;
        }
    }
    RollRight(clamp_axis(axis_key.k));
}

void FlyByKeyboard::Execute() {
    FlyByKeyboard::Execute(true);
}

