#ifndef _GFX_BOX_H_
#define _GFX_BOX_H_

#include "mesh.h"

class Box : public Mesh {
 private:
  Vector corner_min, corner_max;
 public:
  Box(const Vector &cornermin, const Vector &cornermax);
  void ProcessDrawQueue(int whichdrawqueue);
};

#endif

