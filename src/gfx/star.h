#ifndef _STAR_H_
#define _STAR_H_
#include "gfxlib.h"

#include "gfxlib_struct.h"
const int STARnumvlist = 27;
#include <string>

class StarVlist {
protected:
  float spread;
  Vector newcamr;
  Vector newcamq;
  Vector camr;
  Vector camq;
  double lasttime;
public:
  StarVlist(float spread);
  void UpdateGraphics();
  virtual ~StarVlist(){}
  virtual bool BeginDrawState(const QVector &center, const Vector & vel,const Vector & angular_vel,bool rotate, bool yawpitch){}
  virtual void Draw(bool){}
  virtual void EndDrawState(bool){}

};

class PointStarVlist:public StarVlist{
  GFXVertexList * vlist;
  GFXVertexList * nonstretchvlist;
public:
  PointStarVlist (int num, float spread,const std::string &our_system_name);
  ~PointStarVlist();
  bool BeginDrawState(const QVector &center, const Vector & vel,const Vector & angular_vel,bool rotate, bool yawpitch);
  void Draw(bool);
  void EndDrawState(bool);
};

class SpriteStarVlist:public StarVlist{
  GFXVertexList * vlist;
  class Texture *decal;
public:
  SpriteStarVlist (int num, float spread, std::string our_system_name, std::string texturename,float size);
  ~SpriteStarVlist();
  bool BeginDrawState(const QVector &center, const Vector & vel,const Vector & angular_vel,bool rotate, bool yawpitch);
  void Draw(bool);
  void EndDrawState(bool);
};

class Stars {
private:
  StarVlist *vlist;
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
  ~Stars();
};
#endif
