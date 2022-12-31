/*
 * sphere.cpp
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "vega_cast_utils.h"
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
#include "preferred_types.h"

using XMLSupport::tostring;
using namespace vega_types;

void SphereMesh::ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    static SharedPtr<GFXColor> spherecol = MakeShared<GFXColor>(vs_config->getColor("planet_ambient"));
    SharedPtr<GFXColor> const tmpcol = MakeShared<GFXColor>(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(spherecol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

void SphereMesh::SelectCullFace(int whichdrawqueue) {
    GFXEnable(CULLFACE);
}

vega_types::SharedPtr<SphereMesh>
SphereMesh::constructSphereMesh(float radius, int stacks, int slices, const char *texture, const string &technique,
                                const char *alpha, bool insideout, const BLENDFUNC a, const BLENDFUNC b, bool envMap,
                                float rho_min, float rho_max, float theta_min, float theta_max, FILTER mipmap,
                                bool reverse_normals) {
    SphereMesh return_value;
    return_value.setConvex(true);
    std::string hash_name = calculateSphereMeshHashName();
//    SphereMesh return_value{radius, stacks, slices, texture, technique, alpha, insideout, a, b, envMap, rho_min, rho_max, theta_min, theta_max, mipmap, reverse_normals};
    return vega_dynamic_cast_shared_ptr<SphereMesh>(return_value.shared_from_this());
}

void CityLights::ProcessDrawQueue(int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    SharedPtr<GFXColor> const citycol = MakeShared<GFXColor>(1, 1, 1, 1);
    SharedPtr<GFXColor> const tmpcol = MakeShared<GFXColor>(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(citycol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

