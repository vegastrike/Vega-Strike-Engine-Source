#include "gfxlib.h"
#include "gfxlib_struct.h"
const int STARnumvlist = 27;
class Stars {
private:

  GFXVertexList * vlist;
  QVector pos[STARnumvlist];
  float spread;
  bool blend;
  bool fade;
  void ResetPosition(const QVector & cent);
  void UpdatePosition(const QVector & cp);
public:
  Stars (int num, float spread);
  void SetBlend(bool blendit, bool fadeit);
  ~Stars ();
  void Draw();
};
