#include "gfxlib.h"


void GFXFogMode (const FOGMODE fog) {
  switch (fog) {
  case FOG_OFF:
    glDisable (GL_FOG);
    return;
  case FOG_EXP:
    glEnable(GL_FOG);
    glFogi (GL_FOG_MODE,GL_EXP);
    break;
  case FOG_EXP2:
    glEnable (GL_FOG);
    glFogi (GL_FOG_MODE,GL_EXP2);
    break;
  case FOG_LINEAR:
    glEnable (GL_FOG);
    glFogi (GL_FOG_MODE,GL_LINEAR);
    break;
  }
  //glHint (GL_FOG_HINT,GL_NICEST );
}
void GFXFogDensity (const float fogdensity) {
  glFogf (GL_FOG_DENSITY,fogdensity); 
}
void GFXFogLimits (const float fognear, const float fogfar) {
  glFogf (GL_FOG_START,fognear); 
  glFogf (GL_FOG_END,fogfar); 
}
void GFXFogColor (GFXColor c) {
  glFogfv (GL_FOG_COLOR, &c.r);
}
void GFXFogIndex (const int index){
  glFogi (GL_FOG_INDEX, index);
}
