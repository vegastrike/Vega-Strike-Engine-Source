#include "gfxlib.h"
#include "gfxlib_struct.h"
const int STARnumvlist = 8;
class Stars {
private:

  GFXVertexList * vlists[STARnumvlist];
  Vector pos[STARnumvlist];
  Vector campos;
  float spread;
  void ResetPosition(const Vector & cent);
  void UpdatePosition(const Vector & cp);
public:
  Stars (int num, float spread);
  ~Stars ();
  void Draw();
};
