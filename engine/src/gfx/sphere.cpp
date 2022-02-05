/**
 * sphere.cpp
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


#include "sphere.h"
#include "ani_texture.h"
#include "vegastrike.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "xml_support.h"
#ifndef M_PI
#define M_PI (3.1415926536F)
#endif
#include "gfx/camera.h"

using XMLSupport::tostring;

void SphereMesh::ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr)
{
    static GFXColor spherecol = vs_config->getColor("planet_ambient");
    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(spherecol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

void SphereMesh::SelectCullFace(int whichdrawqueue)
{
    GFXEnable(CULLFACE);
}

void CityLights::ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr)
{
    const GFXColor citycol(1, 1, 1, 1);
    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(citycol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

