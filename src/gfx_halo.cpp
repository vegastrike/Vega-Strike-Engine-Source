#include "gfx_halo.h"
Halo::Halo() local_transformation(identity_transformation){
  

}


void Halo::SetPosition (float x,float y, float z) {
  local_transformation.position = Vector (x,y,z);
}
void Halo::SetPosition (const Vector &k) {
  local_transformation.position = k;
}
void Halo::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  local_transformation.orientation = Quaternion::from_vectors(p,q,r);
}

Vector &Halo::Position()
{
  return local_transformation.position;
}

