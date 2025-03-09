/*
 * beam_server.cpp
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


#include "vegastrike.h"
#include <vector>
#include "beam.h"
#include "unit_generic.h"
//#include "unit_collide.h"
#include "images.h"

Beam::Beam(const Transformation &trans, const weapon_info &clne, void *own, Unit *self, int sound)
        : vlist(NULL), Col(clne.r, clne.g, clne.b, clne.a) {
    VSCONSTRUCT2('B')
    listen_to_owner = false;     //warning this line of code is also present in beam.cpp change one, change ALL

    Init(trans, clne, own, self);
    impact = UNSTABLE;
}

Beam::~Beam() {
    VSDESTRUCT2
    //VSFileSystem::vs_fprintf (stderr,"Deleting %x",this);
#ifdef BEAMCOLQ
    RemoveFromSystem( true );
#endif
//delete vlist;  // Do not delete shared vlist.
}

void Beam::Draw(const Transformation &trans, const Matrix &m, Unit *targ, float tracking_cone) {
    //hope that the correct transformation is on teh stack
}

void Beam::ProcessDrawQueue() {
}

