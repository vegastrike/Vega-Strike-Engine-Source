#include "gl_globals.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "config_xml.h"
#include "gfxlib.h"
#include "lin_time.h"
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
 
int GFXCreateProgramConst(const char*vprogram,const char* fprogram) {
  if (vprogram[0]=='\0'&&fprogram[0]=='\0') return 0;
#ifndef __APPLE__
  if (glGetProgramInfoLog_p==NULL||glCreateShader_p==NULL||glShaderSource_p==NULL||glCompileShader_p==NULL||glAttachShader_p==NULL||glLinkProgram_p==NULL||glGetShaderiv_p==NULL||glGetProgramiv_p==NULL)
    return 0;
#else
#ifdef OSX_LOWER_THAN_10_4
    return 0;
#endif
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
  if (vperr>Ok&&fperr>Ok) return 0;
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
    GLint successp=0;
    glGetShaderiv_p(vproghandle,GL_COMPILE_STATUS,&successp);
    if (successp==0) {
      printLog(vproghandle,true);
      fprintf(stderr,"Vertex Program Error: Failed to compile %s\n",vprogram);
      return 0;
    }
          
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
    GLint successp=0;
    glGetShaderiv_p(fproghandle,GL_COMPILE_STATUS,&successp);
    if (successp==0) {
      printLog(fproghandle,true);
      fprintf(stderr,"Fragment Program Error: Failed to compile %s\n",fprogram);
      return 0;
    }
    printLog(fproghandle,true);
    glAttachShader_p(sp,fproghandle);
  }
  glLinkProgram_p(sp);
  printLog(sp,false);
  GLint successp=0;
  glGetProgramiv_p(sp,GL_LINK_STATUS,&successp);
  if (successp==0) {
    fprintf(stderr,"Shader Program Error: Failed to link %s to %s\n",vprogram,fprogram);
    return 0;
  }
  /* only for dev work
  glGetProgramiv_p(sp,GL_VALIDATE_STATUS,&successp);
  if (successp==0) {
    fprintf(stderr,"Shader Program Error: Failed to validate %s linking to %s\n",vprogram,fprogram);
    return 0;
  }
  */
  while((errCode=glGetError())!=GL_NO_ERROR) {
    printf ("Error code %s\n",gluErrorString(errCode));
    sp=0;//no proper vertex prog support
  }
  return sp;
}
int GFXCreateProgram(char*vprogram,char* fprogram) {
  return GFXCreateProgramConst(vprogram,fprogram);
}
static int programChanged=false;
static int defaultprog=0;
static int lowfiprog=0;
static int hifiprog=0;
#ifdef __APPLE__
std::string hifiProgramName="mac";
std::string lowfiProgramName="maclite";
#else
std::string hifiProgramName="default";
std::string lowfiProgramName="lite";
#endif
int getDefaultProgram() {
  static bool initted=false;
  if (!initted){
#ifdef __APPLE__
    hifiProgramName=vs_config->getVariable("graphics","mac_shader_name","mac");
#else
    hifiProgramName=vs_config->getVariable("graphics","shader_name","default");
#endif
    if (hifiProgramName.length()==0) {
      lowfiprog=hifiprog=0;
    }else {
      lowfiprog=GFXCreateProgramConst(lowfiProgramName.c_str(),lowfiProgramName.c_str());
      if (lowfiprog==0)lowfiprog=GFXCreateProgramConst(hifiProgramName.c_str(),hifiProgramName.c_str());
      hifiprog=GFXCreateProgramConst(hifiProgramName.c_str(),hifiProgramName.c_str());
      if (hifiprog==0)hifiprog=GFXCreateProgramConst(lowfiProgramName.c_str(),lowfiProgramName.c_str());
    }
    defaultprog=hifiprog;
    programChanged=true;
    initted=true;
  }
  return defaultprog;
}
void GFXReloadDefaultShader() {
  bool islow=(lowfiprog==defaultprog);
  if (glDeleteProgram_p&&defaultprog) {
    glDeleteProgram_p(lowfiprog);
    glDeleteProgram_p(hifiprog);
  }
  programChanged=true;
  if (islow) {
    hifiprog=GFXCreateProgramConst(hifiProgramName.c_str(),hifiProgramName.c_str());
    if (hifiprog==0)hifiprog=GFXCreateProgramConst(lowfiProgramName.c_str(),lowfiProgramName.c_str());
    lowfiprog=GFXCreateProgramConst(lowfiProgramName.c_str(),lowfiProgramName.c_str());
    if (lowfiprog==0)lowfiprog=GFXCreateProgramConst(hifiProgramName.c_str(),hifiProgramName.c_str());
    defaultprog=lowfiprog;
  }else{
    lowfiprog=GFXCreateProgramConst(lowfiProgramName.c_str(),lowfiProgramName.c_str());
    if (lowfiprog==0)lowfiprog=GFXCreateProgramConst(hifiProgramName.c_str(),hifiProgramName.c_str());
    hifiprog=GFXCreateProgramConst(hifiProgramName.c_str(),hifiProgramName.c_str());
    if (hifiprog==0)hifiprog=GFXCreateProgramConst(lowfiProgramName.c_str(),lowfiProgramName.c_str());
    defaultprog=hifiprog;
  }
}
enum GameSpeed {
  JUSTRIGHT,
  TOOSLOW,
  TOOFAST
};
unsigned int gpdcounter=(1<<30);
#define NUMFRAMESLOOK 128
GameSpeed gameplaydata[NUMFRAMESLOOK]={JUSTRIGHT};
GameSpeed GFXGetFramerate() {
  GameSpeed retval=JUSTRIGHT;
  static double lasttime=queryTime();
  double thistime=queryTime();
  double framerate=1./(thistime-lasttime);
  static double toofast=80;
  static double tooslow=29;
  static int lim=10;
  static int penalty=10;
  static float lowratio=.125;
  static float highratio=.75;
  GameSpeed curframe;
  if (framerate>toofast)curframe=TOOFAST;
  else if (framerate>tooslow) curframe=JUSTRIGHT;
  else curframe=TOOSLOW;
  gameplaydata[((unsigned int)gpdcounter++)%NUMFRAMESLOOK]=curframe;
  unsigned int i=0;
  if(!(curframe==JUSTRIGHT||(curframe==TOOFAST&&defaultprog==hifiprog)||(curframe==TOOSLOW&&defaultprog==0))) {
    for (;i<lim;++i) {
      if (curframe!=gameplaydata[((unsigned int)(gpdcounter-i))%NUMFRAMESLOOK]) {
        break;
      }
    }
    if(i==lim) {
      int correct=0;
      int incorrect=0;
      for (unsigned int j=0;j<NUMFRAMESLOOK;++j) {
        if (gameplaydata[j]==curframe) correct++;
        if (gameplaydata[j]!=curframe) incorrect++;        
        if (curframe==TOOFAST&&gameplaydata[j]==TOOSLOW)
          incorrect+=penalty;
      }
      double myratio=(double)correct/(double)(correct+incorrect);

      if (curframe==TOOFAST&&myratio>highratio) {
        static int toomanyswitches=3;
        toomanyswitches-=1;
        if (toomanyswitches>=0)
          retval=curframe;//only switch back and forth so many times
      }else if (myratio>lowratio) {
        retval=curframe;
      }
    }
  }
  lasttime=thistime;
  if (retval!=JUSTRIGHT) {
    for (unsigned int i=0;i<NUMFRAMESLOOK;++i) {
      gameplaydata[i]=JUSTRIGHT;
    }
  }
  return retval;
}
bool GFXShaderReloaded() {
  bool retval=programChanged;
  static bool framerate_changes_shader=XMLSupport::parse_bool(vs_config->getVariable("graphics","framerate_changes_shader","false"));
  if (framerate_changes_shader) {
    switch (GFXGetFramerate()) {
    case TOOSLOW:
      if (defaultprog) {
        retval=true;
        if (defaultprog==hifiprog) 
          defaultprog=lowfiprog;
        else 
          defaultprog=0;
        GFXActivateShader(NULL);
      }
      break;
    case TOOFAST:
      if (defaultprog!=hifiprog){
        retval=true;
        if (defaultprog==0)
          defaultprog=lowfiprog;
        else
          defaultprog=hifiprog;
        GFXActivateShader(NULL);    
      }
      break;
    default:
      break;
    }
  }
  programChanged=false;
  return retval;
}
bool GFXDefaultShaderSupported() {
  return getDefaultProgram()!=0;
}
int GFXActivateShader(char *program) {
  int defaultprogram=getDefaultProgram();
  int curprogram=defaultprogram;
  static char* lastprogram=program;//check for null
  if (program!=lastprogram) programChanged=true;
  lastprogram=program;
  if (program) {
    std::map<std::string,int>::iterator where=loadedprograms.find(std::string(program));
    if (where==loadedprograms.end()) {
      curprogram=GFXCreateProgramConst(program,program);
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


int GFXShaderConstantv(int name,int count, float*values) {
  if (1
#ifndef __APPLE__
      &&glUniform1fv_p
#endif
      ) {
    glUniform1fv_p(name,count,values);
    return 1;
  }
  return 0;  
}
int GFXShaderConstant4v(int name,int count, float*values) {
  if (1
#ifndef __APPLE__
      &&glUniform4fv_p
#endif
      ) {
    glUniform4fv_p(name,count,values);
    return 1;
  }
  return 0;  
}


int GFXShaderConstant(int name,int value) {
  if (1
#ifndef __APPLE__
      &&glUniform1i_p
#endif
      ) {
    glUniform1i_p(name,value);
    return 1;
  }
  return 0;  
}


int GFXShaderConstantv(int name,unsigned int count, int *value) {
  if (1
#ifndef __APPLE__
      &&glUniform1i_p
#endif
      ) {
    glUniform1iv_p(name,count,(GLint*)value);
    return 1;
  }
  return 0;  
}


int GFXNamedShaderConstant(char*progID,char*name) {
  int programname=defaultprog;
  if(progID)
    programname=loadedprograms[progID];
  if (1
#ifndef __APPLE__
      &&glGetUniformLocation_p
#endif
      ) {
    int varloc=glGetUniformLocation_p(programname,name);
    return varloc;
  }
  return 0;
}



int GFXNamedShaderConstant(char*progID,char*name,int value) {
  int programname=defaultprog;
  if(progID)
    programname=loadedprograms[progID];
  if (1
#ifndef __APPLE__
      &&glGetUniformLocation_p
#endif
      ) {
    int varloc=glGetUniformLocation_p(programname,name);
    GFXShaderConstant(varloc,value);
    return varloc;
  }
  return 0;
}
