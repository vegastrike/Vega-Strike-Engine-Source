#ifndef _CMD_BEAM_H_
#define _CMD_BEAM_H_
#include "weapon_xml.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include <vector>
class GFXVertexList;
struct Texture;
struct GFXColor;
using std::vector;

class Beam {
private:
  Transformation local_transformation;
  unsigned int decal;
  GFXVertexList *vlist;
  LineCollide CollideInfo;
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
  float damagerate;
  float rangepenalty;
  float refire;
  float refiretime;
  GFXColor Col;
  enum Impact{
    ALIVE=0,
    IMPACT=1,
    UNSTABLE=2,
    IMPACTANDUNSTABLE=3
  };//is it right now blowing the enemy to smitheri
  unsigned char impact;
  void * owner;//may be a dead pointer...never dereferenced
  Vector center;//in world coordinates as of last physics frame...
  Vector direction;
  
  void RecalculateVertices();
  void CollideHuge(const LineCollide &);
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
  bool Ready () {return curthick==0&&refiretime>refire;}
  bool Collide (class Unit * target);
  static void ProcessDrawQueue();

};
#endif

