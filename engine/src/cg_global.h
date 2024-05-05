/*
 * cg_global.h
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, and other Vega Strike contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_CG_GLOBAL_H
#define VEGA_STRIKE_ENGINE_CG_GLOBAL_H

#if defined (CG_SUPPORT)

#if defined(__APPLE__) && defined(__MACH__)
#define MACOS 1
#elif !defined (WIN32)
#define UNIX 1

#endif //defined (__APPLE__) || defined (MACOSX)

#include "gldrv/gl_globals.h"

#if defined (UNIX)

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#define GLX_GLXEXT_PROTOTYPES 1
#include <glx.h>
#include "gldrv/glx_undefined_extensions.h"
#include <glxext.h>

#endif //defined (UNIX)

#include <string>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#define CG_REQUIRED_EXTENSIONS \
    "GL_ARB_multitexture "     \
    "GL_NV_texture_shader "    \
    "GL_ARB_vertex_program "   \
    "GL_SGIS_generate_mipmap "

#include <glh/glh_extensions.h>
#include <glh/glh_obs.h>
#include <glh/glh_cube_map.h>

// using namespace glh; DONT DO THIS IN HEADERS

class CG_Cloak
{
public:
    CGprogram   vertexProgram;
    CGcontext   shaderContext;
    CGprofile   vertexProfile;
//float4x4
    CGparameter ModelViewProj;
    CGparameter ModelViewIT;
    CGparameter ModelView;
//float4
    CGparameter MaterialDiffuse;
    CGparameter MaterialAmbient;
    CGparameter MaterialSpecular;
    CGparameter MaterialEmissive;
    CGparameter VecPower;
    CGparameter VecCenter;
    CGparameter VecBlendParams;
    CGparameter VecLightDir;
    CGparameter VecEye;
    CGparameter VecTime;
    CG_Cloak() {}
    void cgLoadMedia( std::string pathname, std::string filename );
};

extern CG_Cloak *cloak_cg;

#endif //defined (CG_SUPPORT)

#endif //VEGA_STRIKE_ENGINE_CG_GLOBAL_H
