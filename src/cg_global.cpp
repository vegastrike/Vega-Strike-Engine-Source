#define GLX_GLXEXT_PROTOTYPES 1
#define GLH_EXT_SINGLE_FILE 1

#if defined(__APPLE__) || defined(MACOSX)
#define MACOS
#elif !defined(WIN32)
#define UNIX 1
#endif

#include <iostream>
#include "cg_global.h"

#if defined(CG_SUPPORT)
#include <shared/data_path.h>

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


  if (vertex)
    {
      /*
      if (this->vertexProgram) delete this->vertexProgram;
      if (this->VertexPosition) delete this->VertexPosition;
      if (this->VertexTexCoord) delete this->VertexTexCoord;
      if (this->WorldViewProj) this->WorldViewProj = NULL;
      if (this->Camera) delete this->Camera;
      if (this->BumpScale) delete this->BumpScale;
      if (this->VertexX) delete this->VertexX;
      if (this->VertexY) delete this->VertexY;
      if (this->VertexZ) delete this->VertexZ;
      */
      this->vertexProgram = NULL;
      this->VertexPosition = NULL;
      this->VertexTexCoord = NULL;
      this->WorldViewProj = NULL;
      this->Camera = NULL;
      this->BumpScale = NULL;
      this->VertexX = NULL;
      this->VertexY = NULL;
      this->VertexZ = NULL;


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
      /*
      if (this->pixelProgram) delete this->pixelProgram;
      if (this->PixelPosition) delete this->PixelPosition;
      if (this->PixelTexCoord) delete this->PixelTexCoord;
      if (this->NormalMap) delete this->NormalMap;
      if (this->EnvironmentMap) delete this->EnvironmentMap
      if (this->CameraVector) delete this->CameraVector;
      if (this->PixelX) delete this->PixelX;
      if (this->PixelY) delete this->PixelY;
      if (this->PixelZ) delete this->PixelZ;
      */

      this->pixelProgram = NULL;
      this->PixelPosition = NULL;
      this->PixelTexCoord = NULL;
      this->NormalMap = NULL;
      this->EnvironmentMap = NULL;
      this->CameraVector = NULL;
      this->PixelX = NULL;
      this->PixelY = NULL;
      this->PixelZ = NULL;

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
