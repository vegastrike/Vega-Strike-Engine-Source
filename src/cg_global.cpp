#if defined(CG_SUPPORT)
#define GLH_EXT_SINGLE_FILE 1
#include "cg_global.h"


#include <glh/glh_extensions.h>
#include <iostream>
#include <string>
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
  

      this->vertexProgram = cgCreateProgramFromFile(this->shaderContext,
        CG_SOURCE, mediafile.data(),
        this->vertexProfile, NULL, NULL);

cgGLLoadProgram(this->vertexProgram);


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

 return;
}
#endif
