#ifndef _CG_GLOBAL_
#define _CG_GLOBAL_

#if defined(CG_SUPPORT)

#if defined(__APPLE__) || defined(MACOSX)
#define MACOS 1
#elif !defined(WIN32)
#define UNIX 1
#endif

#include "gldrv/gl_globals.h"
#if defined(UNIX)
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include "gldrv/glx_undefined_extensions.h"
#include <GL/glxext.h>
#endif
#include <Cg/cg.h>
#include <Cg/cgGL.h>

#define CG_REQUIRED_EXTENSIONS "GL_ARB_multitexture " \
"GL_NV_texture_shader " \
"GL_ARB_vertex_program " \
"GL_SGIS_generate_mipmap "
#include <glh/glh_extensions.h>

#include <glh/glh_obs.h>
#include <glh/glh_cube_map.h>
using namespace glh;

class CG_Cloak
{
 public:
 CGprogram vertexProgram;

 CGcontext shaderContext;

 CGprofile vertexProfile;

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
