#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include <vector>

#include <string>
#include <algorithm>
#include "unit_generic.h"
#include "configxml.h"
GFXVertexList * bolt_draw::boltmesh=NULL;

extern void AUDAdjustSound(int i, QVector const & qv, Vector const & vv);
extern bool AUDIsPlaying(int snd);
bolt_draw::~bolt_draw () {

  unsigned int i;
  for (i=0;i<balls.size();i++) {
    for (int j=balls[i].size()-1;j>=0;j--) {
      balls[i][j].Destroy(j);
    }
  }
  for (i=0;i<bolts.size();i++) {
    for (int j=balls[i].size()-1;j>=0;j--) {
      bolts[i][j].Destroy(j);
    }
  }
}

bolt_draw::bolt_draw () {
  boltmesh=NULL;
  boltdecals=NULL;
}
int Bolt::AddTexture(bolt_draw *q, std::string file) {
  int decal=0;
  if (decal>=(int)q->bolts.size()) {
    q->bolts.push_back (vector <Bolt>());
  }
  return decal;
}  
int Bolt::AddAnimation(bolt_draw *q, std::string file, QVector cur_position) {
  int decal=0;
  if (decal>=(int)q->balls.size()) {
    q->balls.push_back (vector <Bolt>());
  }
  return decal;
}


void Bolt::Draw () {
}
extern void BoltDestroyGeneric(Bolt * whichbolt, int index, int decal, bool isBall);
void Bolt::Destroy(int index) {
  BoltDestroyGeneric(this,index,decal,type->type!=weapon_info::BOLT);
}
