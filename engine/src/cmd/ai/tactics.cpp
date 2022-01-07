/*
 * tactics.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
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


#include "tactics.h"
#include "vegastrike.h"
#include "vs_logging.h"
#include "cmd/unit_generic.h"

void CloakFor::Execute()
{
    if (time == 0) {
        parent->Cloak(enable);
    }
    time += SIMULATION_ATOM;
    if (time > maxtime) {
        done = true;
        if (maxtime != 0) {
            parent->Cloak(!enable);
        }
        return;
    }
}

CloakFor::~CloakFor()
{
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("clk%1$x") % this));
#endif
    if (parent && time <= maxtime) {
        parent->Cloak(!enable);
    }
}

