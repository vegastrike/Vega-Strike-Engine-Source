#ifndef _STAR_H_
#define _STAR_H_
#include "gfxlib.h"

#include "gfxlib_struct.h"
const int STARnumvlist = 27;


class StarVlist {
	GFXVertexList * vlist;
	float spread;
    Vector camr;
	Vector camq;
public:
	StarVlist (int num, float spread);
	~StarVlist();
	void BeginDrawState(const QVector &center, const Vector & vel,bool rotate);
	void Draw();
	void EndDrawState();
};
class Stars {
private:
  QVector campos;
  StarVlist vlist;
  QVector pos[STARnumvlist];
  float spread;
  bool blend;
  bool fade;
  void ResetPosition(const QVector & cent);
  void UpdatePosition(const QVector & cp);
public:
  Stars (int num, float spread);
  void SetBlend(bool blendit, bool fadeit);
  void Draw();
  ~Stars() {}
};
#endif
