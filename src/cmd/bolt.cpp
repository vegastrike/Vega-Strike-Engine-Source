#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"

struct bolt_appearence {
  float r,g,b,a;
  float rad,len;
  bolt_appearence (const struct weapon_info &tmp) {
    r = tmp.r;g=tmp.g;b=tmp.b;a=tmp.a;rad=tmp.Radius;len=tmp.Length;
  }
};

static vector <bolt_appearence> BoltLook;
static vector <GFXVertexList *>vlist;
static vector <int> BoltRef;
static vector <vector <MeshDrawContext> > drawqueue;

 
