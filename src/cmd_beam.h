#ifndef _CMD_BEAM_H_
#define _CMD_BEAM_H_
#include "cmd_weapon_xml.h"
#include "gfx_transform_matrix.h"
#include "gfx_primitive.h"
#include <vector>

using std::vector;

class Beam {
private:
  Transformation local_transformation;
  unsigned int decal;
  GFXVertexList *vlist;
  unsigned int numframes;
  float speed;//lite speed
  float texturespeed;
  float curlength;
  float range;
  float radialspeed;
  float curthick;
  float thickness;
  float lastthick;
  float lastlength;
  
  float stability;
  float energy;
  float damagerate;
  float rangepenalty;
  GFXColor Col;
  enum {
    ALIVE=0,
    IMPACT=1,
    UNSTABLE=2
  } impact;//is it right now blowing the enemy to smitherie
  void * owner;//may be a dead pointer...never dereferenced
  Vector center;//in world coordinates as of last physics frame...
  Vector direction;
  
  void RecalculateVertices();
  //static vector <Texture *> BeamDecal;
  //static vector <int> DecalRef;
  //static vector <vector <DrawContext> > drawqueue;
public:
  Beam (const Transformation & trans, const weapon_info & clne, void * own);
  void Init (const Transformation & trans, const weapon_info & clne, void * own);
  ~Beam();
  Vector &Position();
  void SetPosition (float, float, float);
  void SetPosition (const Vector &);
  void SetOrientation(const Vector &p, const Vector &q, const Vector &r);
  void UpdatePhysics(const Transformation & =identity_transformation, const Matrix = identity_matrix);
  void Draw(const Transformation & =identity_transformation, const float [] = identity_matrix );
  void Destabilize () {impact=UNSTABLE;}
  bool Dissolved () {return curthick==0;} 
  bool Collide (class Unit * target);
  static void ProcessDrawQueue();

};
#endif
