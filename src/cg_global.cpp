#define GLX_GLXEXT_PROTOTYPES 1
#define GLH_EXT_SINGLE_FILE 1
#define UNIX 1
#include <iostream>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "cg_global.h"
#include <shared/data_path.h>

#if defined(CG_SUPPORT)

using namespace std;

void VSCG::cgLoadMedia(string pathname, string filename, bool vertex)
{
  data_path media;
  media.path.push_back(".");
  media.path.push_back(pathname);
  string mediafile = media.get_file(filename);
  if (mediafile == "")
    {
      cout << "Unable to load " << filename << ", exiting..." << endl;
     exit(0);
    }
    this->shaderContext = cgCreateContext();




  if (vertex)
    {
      this->vertexProgram = cgCreateProgramFromFile(this->shaderContext,
        CG_SOURCE, mediafile.data(),
        this->vertexProfile, NULL, NULL);

cgGLLoadProgram(this->vertexProgram);

 this->VertexPosition = cgGetNamedParameter(this->vertexProgram, "IN.POSITION");
 this->VertexTexCoord = cgGetNamedParameter(this->vertexProgram, "IN.TEXCOORD0");
 this->WorldViewProj = cgGetNamedParameter(this->vertexProgram, "WorldViewProj");
 this->Camera = cgGetNamedParameter(this->vertexProgram, "EyePosition");
this->BumpScale = cgGetNamedParameter(this->vertexProgram, "BumpScale");
    }
 else
    {
  this->pixelProgram = cgCreateProgramFromFile(this->shaderContext,
        CG_SOURCE, mediafile.data(),
        this->pixelProfile, NULL, NULL);

  cgGLLoadProgram(this->pixelProgram);
    
      this->PixelPosition = cgGetNamedParameter(this->pixelProgram, "IN.POSITION");
      this->PixelTexCoord = cgGetNamedParameter(this->pixelProgram, "IN.TEXCOORD0");
      this->NormalMap = cgGetNamedParameter(this->pixelProgram, "NormalMap");
      this->EnvironmentMap = cgGetNamedParameter(this->pixelProgram, "EnvironmentMap");
      this->CameraVector = cgGetNamedParameter(this->pixelProgram, "EyeVector");
    }
}

#endif
