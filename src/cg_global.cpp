#define GLX_GLXEXT_PROTOTYPES 1
#define GLH_EXT_SINGLE_FILE 1

#if defined(__APPLE__) || defined(MACOSX)
#define MACOS 1
#elif !defined(WIN32)
#define UNIX 1
#endif

#include <iostream>
#include <string>
using std::string;
#if defined(CG_SUPPORT)

#include "cg_global.h"
#include <shared/data_path.h>

using namespace std;

void CG_Cloak::cgLoadMedia(string pathname, string filename)
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
  /*
      this->VecPosition = NULL;
      this->VecNormal = NULL;
      this->ModelViewProj = NULL;
      this->ModelViewIT = NULL;
      this->ModelView = NULL;

      this->MaterialDiffuse = NULL;
      this->MaterialAmbient = NULL;
      this->MaterialSpecular = NULL;
      this->MaterialEmissive = NULL;

      this->VecPower = NULL;
      this->VecCenter = NULL;
      this->VecBlendParams = NULL;
      this->VecLightDir = NULL;
      this->VecEye = NULL;
      this->VecTime = NULL;
  */

      this->vertexProgram = cgCreateProgramFromFile(this->shaderContext,
        CG_SOURCE, mediafile.data(),
        this->vertexProfile, NULL, NULL);

cgGLLoadProgram(this->vertexProgram);

 this->VecPosition = cgGetNamedParameter(this->vertexProgram, "I.vecPosition");
 this->VecNormal = cgGetNamedParameter(this->vertexProgram, "I.vecNormal");

 this->ModelViewProj = cgGetNamedParameter(this->vertexProgram, "matModelViewProj");
 this->ModelViewIT = cgGetNamedParameter(this->vertexProgram, "matModelView");
 this->ModelView = cgGetNamedParameter(this->vertexProgram, "matModelView");

 this->MaterialDiffuse = cgGetNamedParameter(this->vertexProgram, "materialDiffuse");
 this->MaterialAmbient = cgGetNamedParameter(this->vertexProgram, "materialAmbient");
 this->MaterialSpecular = cgGetNamedParameter(this->vertexProgram, "materialSpecular");
 this->MaterialEmissive = cgGetNamedParameter(this->vertexProgram, "materialEmissive");

 this->VecPower = cgGetNamedParameter(this->vertexProgram, "vecPower");
 this->VecCenter = cgGetNamedParameter(this->vertexProgram, "vecCenter");
 this->VecBlendParams = cgGetNamedParameter(this->vertexProgram, "vecBlendParams");
 this->VecLightDir = cgGetNamedParameter(this->vertexProgram, "vecLightDir");
 this->VecEye = cgGetNamedParameter(this->vertexProgram, "vecEye");
 this->VecTime = cgGetNamedParameter(this->vertexProgram, "VecTime");

 
}

#endif
