#ifndef QUADTREE_H_
#define QUADTREE_H_
#include "quadsquare.h"
class QuadTree {
  Matrix transformation;
  quadcornerdata RootCornerData;
  quadsquare *root;
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

};

#endif
