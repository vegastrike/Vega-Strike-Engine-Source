#ifndef TERRAIN_H_
#define TERRAIN_H_
#include "gfx/quadtree.h"
class Unit;
class Vector;

#define TERRAINRENDER 1
#define TERRAINUPDATE 2
class Terrain : public QuadTree {
  updateparity *updatetransform;
  float TotalSizeX;
  float TotalSizeZ;
  float mass;
  int whichstage;
  char draw;
  void ApplyForce (Unit * un, const Vector &norm, float distance);
 public:
  Terrain (const char * filename, const Vector & Scales, const float mass, const float radius, updateparity *updatetransform=identityparity);
  void SetTotalSize (float X, float Z) {TotalSizeX =X;TotalSizeZ=Z;}
  ~Terrain();
  void Collide (Unit * un, Matrix t);
  void Collide (Unit * un);
  void Collide();
  void SetTransformation (Matrix mat);
  static void CollideAll();
  static void DeleteAll ();
  void Render();
  static void RenderAll ();
  static void UpdateAll (int resolution);
  void DisableDraw();
  void EnableDraw();
  void DisableUpdate();
  void EnableUpdate();
  Vector GetUpVector (const Vector &pos);
};
#endif
