#include "gfxlib.h"
#include "gfxlib_struct.h"
const int STARnumvlist = 27;
class Stars {
private:

  GFXVertexList * vlist;
  Vector pos[STARnumvlist];
  float spread;
  bool blend;
  void ResetPosition(const Vector & cent);
  void UpdatePosition(const Vector & cp);
public:
  Stars (int num, float spread);
  void SetBlend(bool blendit);
  ~Stars ();
  void Draw();
};
