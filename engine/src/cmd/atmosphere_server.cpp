/*
 * atmosphere_server.cpp
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


#include "atmosphere.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"

Atmosphere::Atmosphere(const Parameters &params) : user_params(params) {
}

void Atmosphere::Update(const QVector &position, const Matrix &tmatrix) {
}

void Atmosphere::SetMatricesAndDraw(const QVector &pos, const Matrix mat) {
}

void Atmosphere::ProcessDrawQueue() {
}

void Atmosphere::Draw() {
}

void Atmosphere::DrawAtmospheres() {
}
