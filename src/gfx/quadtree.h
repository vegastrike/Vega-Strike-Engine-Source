#ifndef QUADTREE_H_
#define QUADTREE_H_
#include "quadsquare.h"
class Texture;
/** 
 * This class is a wrapper class for quadsquare
 * It takes care of activating textures, having possession of the vlist
 * and having posession of the static vars that get set
 */
class QuadTree {
  IdentityTransform * nonlinear_transform;
  Matrix transformation;
  quadcornerdata RootCornerData;
  quadsquare *root;
  std::vector <Texture *> textures;
  std::vector <unsigned int> unusedvertices;
  GFXVertexList vertices;
  unsigned int VertexAllocated;
  unsigned int VertexCount;
  void LoadData();
 public:
  QuadTree ();
  ~QuadTree();
  void Render();
  void Update();
  void SetTransformation (const Matrix transformation);
  float GetHeight (Vector Location);
};

#endif
