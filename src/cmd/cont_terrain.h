#include "terrain.h"

const int numcontterr=4;
class ContinuousTerrain {
  Vector Scales;
  float sizeX;
  float sizeZ;
  Terrain *data[numcontterr];
  Matrix transformation;
  Vector location[numcontterr];
  bool dirty[numcontterr];
 public:
  ContinuousTerrain (const char* filenameUL, const char * filenameUR, const char * filenameLL, const char *filenameLR,  const Vector & Scales, const float mass);
  void SetTransform (Matrix transformation);
  /**
   *  Centers terrain around camera
   */
  void AdjustTerrain();
  ~ContinuousTerrain();
};
