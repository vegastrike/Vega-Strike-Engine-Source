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

class VSCG
{
 public:
CGprogram vertexProgram;
CGprogram pixelProgram;

CGcontext shaderContext;

CGprofile vertexProfile;
CGprofile pixelProfile;

CGparameter VertexPosition;
CGparameter VertexTexCoord;
CGparameter VertexX;
CGparameter VertexY;
CGparameter VertexZ;

CGparameter WorldViewProj;
CGparameter ObjToCubeSpace;
CGparameter Camera;
CGparameter BumpScale;


CGparameter PixelPosition;
CGparameter PixelTexCoord;
CGparameter PixelX;
CGparameter PixelY;
CGparameter PixelZ;
CGparameter NormalMap;
CGparameter EnvironmentMap;
CGparameter CameraVector;

 VSCG() {}
void cgLoadMedia(string pathname, string filename, bool vertex);
};

extern VSCG *defaultcg;
#endif
#endif
