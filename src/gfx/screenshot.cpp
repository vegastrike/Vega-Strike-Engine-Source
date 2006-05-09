#include "in_kb.h"
#include "vs_globals.h"
#include "vsimage.h"
#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif
using namespace VSFileSystem;
void Screenshot (const KBData&, KBSTATE state) {
  if (state==PRESS) {
    GLint xywh[4]={0,0,0,0};
    xywh[2]=g_game.x_resolution;
    xywh[3]=g_game.y_resolution;
    glGetIntegerv(GL_VIEWPORT,xywh);
    unsigned char * tmp = (unsigned char*) malloc (xywh[2]*xywh[3]*4*sizeof(unsigned char));    
    //memset(tmp,0x7f,xywh[2]*xywh[3]*4*sizeof(char));
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ROW_LENGTH,xywh[2]);

    glFinish();
    glReadPixels(0,0,xywh[2],xywh[3],GL_RGB,GL_UNSIGNED_BYTE,tmp);
    glPixelStorei(GL_PACK_ROW_LENGTH,0);
    ::VSImage image;
    VSFileSystem::VSFile f;
    static int count=0;
    std::string filename="Screenshot"+XMLSupport::tostring(count)+".png";
    for (;;) {
      if (f.OpenReadOnly( filename, TextureFile)<=VSFileSystem::Ok) {
        f.Close();
        filename="Screenshot"+XMLSupport::tostring(++count)+".png";
      }else break;
    }
    char * tmpchar=strdup(filename.c_str());
    image.WriteImage(tmpchar, tmp, PngImage, xywh[2], xywh[3], false, 8, TextureFile,true);
    free(tmpchar);
  }
}
