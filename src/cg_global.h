#ifndef _CG_GLOBAL_
#define _CG_GLOBAL_

#if defined(CG_SUPPORT)
#include "gldrv/gl_globals.h"
#include <Cg/cg.h>
#include <Cg/cgGL.h>

#define CG_REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
"GL_NV_texture_shader " \
"GL_ARB_vertex_program " \
"GL_SGIS_generate_mipmap "

#include <glh/glh_extensions.h>
#include <glh/glh_genext.h>
#include <glh/glh_obs.h>
#include <glh/glh_cube_map.h>
using namespace glh;

class CG_Cloak
{
 public:
 CGprogram vertexProgram;

 CGcontext shaderContext;

 CGprofile vertexProfile;

 // float3
 CGparameter VecPosition;
 CGparameter VecNormal;

 // float4x4
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
 void cgLoadMedia(string pathname, string filename);
};

extern CG_Cloak *cloak_cg;
#endif
#endif
