#ifndef TERRAIN_H_
#define TERRAIN_H_
#include "gfx/quadtree.h"
class Unit;
class Vector;


class Terrain : public QuadTree {
  updateparity *updatetransform;
  float TotalSizeX;
  float TotalSizeZ;
  float mass;
  int whichstage;
  bool draw;
  void ApplyForce (Unit * un, const Vector &norm, float distance);
 public:
  Terrain (const char * filename, const Vector & Scales, const float mass, const float radius, updateparity *updatetransform=identityparity);
  void SetTotalSize (float X, float Z) {TotalSizeX =X;TotalSizeZ=Z;}
  ~Terrain();
  void Collide (Unit * un);
  void Collide();
  void SetTransformation (Matrix mat);
  static void CollideAll();
  static void DeleteAll ();
  static void RenderAll ();
  static void UpdateAll (int resolution);
};
#endif
