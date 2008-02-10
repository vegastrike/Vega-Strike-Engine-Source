#ifdef OPCODE_COLLIDER
#include "cmd/collide2/Stdafx.h"
#include "cmd/collide2/CSopcodecollider.h"
#endif
#include "terrain.h"
class StarSystem;
const int numcontterr=4;
struct MeshDat {
  class Mesh * mesh;
#ifndef OPCODE_COLLIDER
  class csRapidCollider * collider;
#else
  class csOPCODECollider * collider;
#endif
  Matrix mat;
};
class ContinuousTerrain {
  Vector Scales;
  float sizeX;
  float sizeZ;
  int width;
  int numcontterr;
  Terrain **data;
  MeshDat * md;
  Matrix transformation;
  QVector *location;
  bool *dirty;
  bool checkInvScale(double &pos, double campos, float size);
 public:
  ContinuousTerrain (const char* filenameUL, const Vector & Scales=Vector (0,0,0), const float mass=0);
  void SetTransformation (const Matrix &transformation);
  /**
   *  Centers terrain around camera
   */
  void AdjustTerrain(StarSystem *);

  void AdjustTerrain(Matrix &transform, const Matrix &transformation, const QVector &unitpos,int i);
  ~ContinuousTerrain();
  void DisableDraw();
  void EnableDraw();
  void DisableUpdate();
  void EnableUpdate();
  void Draw();
  QVector GetGroundPosIdentTrans (QVector ShipPos, Vector &norm);
  QVector GetGroundPos(QVector ShipPos, Vector & norm);
  Vector GetUpVector (const Vector & pos);
  void Collide (Unit * un, Matrix trans);
  void Collide (Unit * un);
  void Collide ();
  void GetTotalSize (float &X, float &Z) {X = sizeX;Z = sizeZ;}
};
