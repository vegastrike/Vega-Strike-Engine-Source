#include "gfx/quadtree.h"
class Unit;
class Vector;
class Terrain : public QuadTree {
  float mass;
  int whichstage;
  void ApplyForce (Unit * un, const Vector &norm, float distance);
 public:
  Terrain (const char * filename);
  ~Terrain();
  void Collide (Unit * un);
  void Collide();
  void SetTransformation (Matrix mat);
  static void CollideAll();
  static void DeleteAll ();
  static void RenderAll ();
  static void UpdateAll (int resolution);
};
