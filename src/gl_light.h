#ifndef _GL_LIGHT_H_
#define _GL_LIGHT_H_
#include "gfxlib.h"

int GFX_MAX_LIGHTS=8;
int GFX_OPTIMAL_LIGHTS=4;
GFXBOOL GFXLIGHTING=GFXFALSE;
#define GFX_DIFFUSE 1
#define GFX_SPECULAR 2
#define GFX_AMBIENT 4
#define GFX_LIGHT_POS 8
#define GFX_ATTENUATED 16
#define GFX_LIGHT_ENABLED 32
#define GFX_LOCAL_LIGHT 64

class gfx_light: public GFXLight {
 public:
  gfx_light():GFXLight(){}
  int lightNum();
  bool LocalLight() {return (options&GFX_LOCAL_LIGHT)!=0;}
  bool enabled () {return (options&GFX_LIGHT_ENABLED)!=0;}
  int& Target () {return target;}
  void Create (const GFXLight &, bool global);//if global, puts it into GLlights (if space ||enabled) <clobber?>
  //for local lights, if enabled, call Enable().
  void Kill (); // Disables it (may remove from table), trashes it from GLlights. sets target to -2 (dead)  

  void ClobberGLLight (const int target);//replaces target GL light in the implementation. Sets this->target! Checks for -1 and calls ContextSwitch to clobber completely
  void ContextSwitchEnableLight (const int target);//replaces target GL light, copying all state sets this->target!
  void Enable ();//for global lights, clobbers SOMETHING for sure, calls GLenable
  //for local lights, puts it into the light table
  void Disable ();// for global lights, GLdisables it.
  //for local lights, removes it from the table. and trashes it form GLlights.
  void ResetProperties (const enum LIGHT_TARGET, const GFXColor&color);//sets properties, making minimum GL state changes for global,
  //for local lights, removes it from table, trashes it from GLlights, 
  //if enabled, puts it bakc in table.
  LineCollide CalculateBounds ();//calculates bounds for the table!
};
struct OpenGLLights {
  int index;//-1 == unassigned
  enum {
    GLL_OFF=0,
    GLL_ON=1,
    GLL_LOCAL=2,
    GLL_LOCAL_ON=3
    
  } options;
};


#endif
