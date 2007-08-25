#include "gl_globals.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "config_xml.h"
#include "gfxlib.h"
#include <map>
std::map<std::string,int> loadedprograms;

void printLog(GLuint obj, bool shader)
{
  GLsizei infologLength = 0;
  char infoLog[1024];
  if (shader)
    glGetShaderInfoLog_p(obj, 1024, &infologLength, infoLog);
  else
    glGetProgramInfoLog_p(obj, 1024, &infologLength, infoLog); 
  if (infologLength > 0)
    fprintf(stderr,"%s\n", infoLog);
}
 
int GFXCreateProgram(char*vprogram,char* fprogram) {
#ifndef __APPLE__
  if (glGetProgramInfoLog_p==NULL||glCreateShader_p==NULL||glShaderSource_p==NULL||glCompileShader_p==NULL||glAttachShader_p==NULL||glLinkProgram_p==NULL)
    return 0;
#endif
  GLenum errCode;
  while((errCode=glGetError())!=GL_NO_ERROR) {
    printf ("Error code %s\n",gluErrorString(errCode));
  }
  VSFileSystem::VSFile vf,ff;
  std::string vpfilename="programs/";
  vpfilename+=vprogram;
  vpfilename+=".vp";
  std::string fpfilename="programs/";
  fpfilename+=fprogram;
  fpfilename+=".fp";
  VSFileSystem::VSError vperr = vf.OpenReadOnly(vpfilename.c_str(), UnknownFile);  
  VSFileSystem::VSError fperr = ff.OpenReadOnly(fpfilename.c_str(), UnknownFile);  
  GLint vproghandle=0;
  GLint fproghandle=0;
  GLint sp=glCreateProgram_p();
  if (vperr<=Ok) {
    std::string vertexprg;
    vertexprg=vf.ReadFull();
    vf.Close();
    vproghandle=glCreateShader_p(GL_VERTEX_SHADER);
    const char *tmp=vertexprg.c_str();
    glShaderSource_p(vproghandle,1,&tmp,NULL);
    glCompileShader_p(vproghandle);
    printLog(vproghandle,true);
    glAttachShader_p(sp,vproghandle);
  }
  if (fperr<=Ok) {
    std::string fragprg;
    fragprg=ff.ReadFull();
    ff.Close();
    fproghandle=glCreateShader_p(GL_FRAGMENT_SHADER);
    const char*tmp=fragprg.c_str();
    glShaderSource_p(fproghandle,1,&tmp,NULL);
    glCompileShader_p(fproghandle);
    printLog(fproghandle,true);
    glAttachShader_p(sp,fproghandle);
  }
  glLinkProgram_p(sp);
  printLog(sp,false);
  
  while((errCode=glGetError())!=GL_NO_ERROR) {
    printf ("Error code %s\n",gluErrorString(errCode));
    sp=0;//no proper vertex prog support
  }
  return sp;
}
static int defaultprog=0;
int getDefaultProgram() {
  static int defaultprogram=defaultprog=GFXCreateProgram("default","default");
  return defaultprogram;
}
bool GFXDefaultShaderSupported() {
  return getDefaultProgram()!=0;
}
int GFXActivateShader(char *program) {
  int defaultprogram=getDefaultProgram();
  int curprogram=defaultprogram;
  if (program) {
    std::map<std::string,int>::iterator where=loadedprograms.find(std::string(program));
    if (where==loadedprograms.end()) {
      curprogram=GFXCreateProgram(program,program);
      loadedprograms[program]=curprogram;
    }else {
      curprogram=where->second;
    }
  }
  if(1
#ifndef __APPLE__
     &&glUseProgram_p
#endif
     )
    glUseProgram_p(curprogram);
  else return 0;
  return curprogram;
  
}
void GFXDeactivateShader() {
  if(1
#ifndef __APPLE__
     &&glUseProgram_p
#endif
     )
    glUseProgram_p(0);
}
int GFXShaderConstant(int name,float*values) {
  if (1
#ifndef __APPLE__
      &&glUniform4f_p
#endif
      ) {
    glUniform4f_p(name,values[0],values[1],values[2],values[3]);
    return 1;
  }
  return 0;  
}
int GFXNamedShaderConstant(char*progID,char*name,float*values) {
  int programname=defaultprog;
  if(progID)
    programname=loadedprograms[progID];
  if (1
#ifndef __APPLE__
      &&glGetUniformLocation_p
#endif
      ) {
    int varloc=glGetUniformLocation_p(programname,name);
    GFXShaderConstant(varloc,values);
    return varloc;
  }
  return 0;
}
