#ifndef _STAR_H_
#define _STAR_H_
#include "gfxlib.h"

#include "gfxlib_struct.h"
const int STARnumvlist = 27;
#include <string>

class StarVlist {
	GFXVertexList * vlist;
	GFXVertexList * nonstretchvlist;
	float spread;
	Vector newcamr;
	Vector newcamq;
    Vector camr;
	Vector camq;
	double lasttime;
public:
	StarVlist (int num, float spread,const std::string &our_system_name);
	~StarVlist();
	bool BeginDrawState(const QVector &center, const Vector & vel,const Vector & angular_vel,bool rotate, bool yawpitch);
	void Draw(bool);
	void EndDrawState(bool);
	void UpdateGraphics();
};
class Stars {
private:
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
