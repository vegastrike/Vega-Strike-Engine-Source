#include "quaternion.h"
class Terrain {
  
 public:
  Terrain (const char * filename, const char * texture);
  ~Terrain();
  bool intersects (const Vector & point, const float err, Vector & norm, float & dist) const;
  float intersects (const Vector & start, const Vector & end, Vector & norm);
  void Draw (const Transformation &quat=identity_transformation, const Matrix=identity_matrix);
  
};
