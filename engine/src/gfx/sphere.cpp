/*
 * sphere.cpp
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


#include "gfx_generic/sphere.h"
#include "gfx/ani_texture.h"
#include "src/vegastrike.h"
#include "src/config_xml.h"
#include "root_generic/vs_globals.h"
#include "root_generic/vsfilesystem.h"
#include "root_generic/xml_support.h"
#ifndef M_PI
#define M_PI (3.1415926536F)
#endif
#include "gfx/camera.h"

using XMLSupport::tostring;

void SphereMesh::ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    static GFXColor spherecol = vs_config->getColor("planet_ambient");
    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(spherecol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

SphereMesh::~SphereMesh() = default;

void SphereMesh::SelectCullFace(int whichdrawqueue) {
    GFXEnable(CULLFACE);
}

void CityLights::ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    const GFXColor citycol(1, 1, 1, 1);
    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(citycol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

CityLights::~CityLights() = default;
