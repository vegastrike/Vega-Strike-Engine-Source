 #ifndef _GL_LIGHT_H_
#define _GL_LIGHT_H_
#include "gfxlib.h"
#include "hashtable_3d.h"

extern int GFX_MAX_LIGHTS;
extern int GFX_OPTIMAL_LIGHTS;
extern GFXBOOL GFXLIGHTING;
#define GFX_ATTENUATED 1
//#define GFX_DIFFUSE 2
//#define GFX_SPECULAR 4
//#define GFX_AMBIENT 8
//#define GFX_LIGHT_POS 16
#define GFX_LIGHT_ENABLED 32
#define GFX_LOCAL_LIGHT 64

class gfx_light: public GFXLight {
 public:
  gfx_light():GFXLight(){}
  GFXLight operator =(const GFXLight &tmp);
  int lightNum();
  bool LocalLight() {return (options&GFX_LOCAL_LIGHT)!=0;}
  bool enabled () {return (options&GFX_LIGHT_ENABLED)!=0;}
  int& Target () {return target;}
  bool Create (const GFXLight &, bool global);//if global, puts it into GLlights (if space ||enabled) <clobber?>
  //for local lights, if enabled, call Enable().
  void Kill (); // Disables it (may remove from table), trashes it from GLlights. sets target to -2 (dead)  
  void SendGLPosition (const GLenum target);//properly utilizes union.
  void ClobberGLLight (const int target);//replaces target GL light in the implementation. Sets this->target! Checks for -1 and calls ContextSwitch to clobber completely
  
  inline void FinesseClobberLight (const GLenum target, const int original);//replaces target GL light, copying all state sets this->target!
  inline void ContextSwitchClobberLight (const GLenum target);//replaces target GL light, copying all state sets this->target!
  void Enable ();//for global lights, clobbers SOMETHING for sure, calls GLenable
  //for local lights, puts it into the light table
  void Disable ();// for global lights, GLdisables it.
  //for local lights, removes it from the table. and trashes it form GLlights.
  void ResetProperties (const enum LIGHT_TARGET, const GFXColor&color);//sets properties, making minimum GL state changes for global,
  //for local lights, removes it from table, trashes it from GLlights, 
  //if enabled, puts it bakc in table.
  void AddToTable();
  void RemoveFromTable();
  void TrashFromGLLights();
  static void dopickenables();
  LineCollide CalculateBounds (bool & err);//calculates bounds for the table!
};
namespace OpenGLL {
const  char GLL_OFF=0;
const      char GLL_ON=1;
const      char GLL_LOCAL=2;
const      char GL_ENABLED= 4;
}
struct OpenGLLights {
    int index;//-1 == unassigned
    char options;
};

void light_rekey_frame();
void unpicklights();
extern int _currentContext;
extern vector <vector <gfx_light> > _local_lights_dat;
extern vector <GFXColor> _ambient_light;
extern vector <gfx_light> * _llights;
extern int _GLLightsEnabled;
//currently stored GL lights!
extern OpenGLLights* GLLights;
struct LineCollideStar {
  LineCollide* lc;
  LineCollideStar () {lc = NULL;}
  bool operator == (const LineCollideStar & b) {
    return lc->object==b.lc->object;
  }
  bool operator < (const LineCollideStar &b) {
      return (*((int *)&(lc->object)) < *((int *)&(b.lc->object)));
  }
  inline int GetIndex () {return *((int *)(&lc->object));}
};

int findLocalClobberable();
//table to store local lights, numerical pointers to _llights (eg indices)
#define CTACC 200
extern Hashtable3d <LineCollideStar, char[20],char[CTACC]> lighttable;

//optimization globals
extern float intensity_cutoff;//something that would normally round down
extern float optintense;
extern float optsat;

#endif
