#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include <vector>
namespace bolt_draw {
  static std::vector <vector <Bolt *> > drawqueue;
  static GFXVertexList * boltmesh=NULL;
  static int boltrefcount=0;
  static vector <vector <MeshDrawContext> > drawq;
  void CreateBoltMesh () {

  }

}
using namespace bolt_draw;
Bolt::Bolt (const Matrix orientationpos, float r, float g, float b, float a, float rad, float len) {
  type = weapon_info::BOLT;
  CopyMatrix (curr_physical_state,orientationpos);
  ScaleMatrix (curr_physical_state,Vector (rad,rad,len));
  if (boltrefcount==0||boltmesh==NULL) {
    CreateBoltMesh();
  }
}

Bolt::~Bolt () {
  boltrefcount--;
  if (boltrefcount==0) {
    if (boltmesh) {
      delete boltmesh;
      boltmesh = NULL;
    }
  }
}
