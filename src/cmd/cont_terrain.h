#include "terrain.h"

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
  ContinuousTerrain (char** filenameUL, const int numwidth,  const Vector & Scales, const float mass);
  void SetTransform (Matrix transformation);
  /**
   *  Centers terrain around camera
   */
  void AdjustTerrain();
  ~ContinuousTerrain();
};
