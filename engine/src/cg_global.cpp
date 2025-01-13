/*
 * cg_global.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#if defined (CG_SUPPORT)
#define GLH_EXT_SINGLE_FILE 1
#include "cg_global.h"
#include "vs_globals.h"

#include <glh/glh_extensions.h>
// #include <iostream>
#include <string>
#include <shared/data_path.h>

#include "vs_logging.h"

// using std::string;

void CG_Cloak::cgLoadMedia( std::string pathname, std::string filename )
{
    data_path media;
    media.path.push_back( "." );
    media.path.push_back( pathname );
    std::string mediafile = media.get_file( filename );
    if (mediafile == "") {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("Unable to load %1%, exiting...") % filename), 0);
    }
    this->vertexProgram = cgCreateProgramFromFile( this->shaderContext,
                                                   CG_SOURCE, mediafile.data(),
                                                   this->vertexProfile, NULL, NULL );

    cgGLLoadProgram( this->vertexProgram );

    this->ModelViewProj    = cgGetNamedParameter( this->vertexProgram, "matModelViewProj" );
    this->ModelViewIT      = cgGetNamedParameter( this->vertexProgram, "matModelView" );
    this->ModelView        = cgGetNamedParameter( this->vertexProgram, "matModelView" );

    this->MaterialDiffuse  = cgGetNamedParameter( this->vertexProgram, "materialDiffuse" );
    this->MaterialAmbient  = cgGetNamedParameter( this->vertexProgram, "materialAmbient" );
    this->MaterialSpecular = cgGetNamedParameter( this->vertexProgram, "materialSpecular" );
    this->MaterialEmissive = cgGetNamedParameter( this->vertexProgram, "materialEmissive" );

    this->VecPower = cgGetNamedParameter( this->vertexProgram, "vecPower" );
    this->VecCenter        = cgGetNamedParameter( this->vertexProgram, "vecCenter" );
    this->VecBlendParams   = cgGetNamedParameter( this->vertexProgram, "vecBlendParams" );
    this->VecLightDir      = cgGetNamedParameter( this->vertexProgram, "vecLightDir" );
    this->VecEye  = cgGetNamedParameter( this->vertexProgram, "vecEye" );
    this->VecTime = cgGetNamedParameter( this->vertexProgram, "VecTime" );
}
#endif

