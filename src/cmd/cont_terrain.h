#include "terrain.h"
class StarSystem;
const int numcontterr=4;
class ContinuousTerrain {
  Vector Scales;
  float sizeX;
  float sizeZ;
  int width;
  int numcontterr;
  Terrain **data;
  Matrix transformation;
  Vector *location;
  bool *dirty;
  bool checkInvScale(float &pos, float campos, float size);
 public:
  ContinuousTerrain (const char* filenameUL, const Vector & Scales=Vector (0,0,0), const float mass=0);
  void SetTransformation (Matrix transformation);
  /**
   *  Centers terrain around camera
   */
  void AdjustTerrain(StarSystem *);
  ~ContinuousTerrain();
  void DisableDraw();
  void EnableDraw();
  Vector GetGroundPos(Vector ShipPos, Vector & norm);
};
