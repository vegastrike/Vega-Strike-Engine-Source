#include "gfx_transform_matrix.h"
#include "gfx_bounding_box.h"


BoundingBox::BoundingBox(Vector LX, Vector MX,Vector LY,Vector MY,Vector LZ,Vector MZ){
  lx = LX;
  ly = LY;
  lz = LZ;
  mx = MX;
  my = MY;
  mz = MZ;

}
//transform expects one to transform from inward out.... so turret then ship then unit then space... not general...but much easier to read code
void BoundingBox::Transform (Matrix t) {  
  lx = ::Transform (t,lx);
  ly = ::Transform (t,ly);
  lz = ::Transform (t,lz);
  mx = ::Transform (t,mx);
  my = ::Transform (t,my);
  mz = ::Transform (t,mz);
}

void BoundingBox::Transform (const Vector &p,const Vector &q,const Vector &r) {
  lx = ::Transform (p,q,r,lx);
  ly = ::Transform (p,q,r,ly);
  lz = ::Transform (p,q,r,lz);
  mx = ::Transform (p,q,r,mx);
  my = ::Transform (p,q,r,my);
  mz = ::Transform (p,q,r,mz);
}
void BoundingBox::Transform (const Vector &Pos) {
  lx +=Pos;
  ly +=Pos;
  lz +=Pos;
  mx +=Pos;
  my +=Pos;
  mz +=Pos;
}

bool BoundingBox::Within (Vector query, float err) {
  err = -err;
  Vector tquery =query -lx; // now it's in coords with respect to the minimum x;
  Vector tmp = mx-lx;
  if (tquery.Dot (tmp) <err)
    return false;
  tquery = query - mx;
  tmp = lx - mx;
  if (tquery.Dot(tmp)<err)
    return false;
  tquery = query - ly;
  tmp = my - ly;
  if (tquery.Dot (tmp)<err)
    return false;
  tquery = query - my;
  tmp = ly - my;
  if (tquery.Dot (tmp) <err)
    return false;
  tquery = query - lz;
  tmp = mz - lz;
  if (tquery.Dot (tmp)<err)
    return false;
  tquery = query - mz;
  tmp = lz - mz;
  if (tquery.Dot (tmp) <err)
    return false;
  return true; //within bounding box;
}
