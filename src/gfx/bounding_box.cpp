#include "matrix.h"
#include "bounding_box.h"
#include <stdio.h>

BoundingBox::BoundingBox(const QVector &LX, 
			 const QVector &MX,
			 const QVector &LY,
			 const QVector &MY,
			 const QVector &LZ,
			 const QVector &MZ){
  lx = LX;
  ly = LY;
  lz = LZ;
  mx = MX;
  my = MY;
  mz = MZ;

}
//transform expects one to transform from inward out.... so turret then ship then unit then space... not general...but much easier to read code
void BoundingBox::Transform (const Matrix &t) {  
  lx = ::Transform (t,lx);
  ly = ::Transform (t,ly);
  lz = ::Transform (t,lz);
  mx = ::Transform (t,mx);
  my = ::Transform (t,my);
  mz = ::Transform (t,mz);
}

void BoundingBox::Transform (const Transformation &trans) {
  Matrix m;
  trans.to_matrix(m);
  Transform(m);
}


/*  Determines whether the ray intersects the box...returns -1 if the -ray does, 1 if
 *  the ray itself does. 0 if the ray misses the box
 *  To Compute the intersection of a ray from eye to pnt and a bounding box
 *  solve for 'r' 
 *  (r* pnt.i+eye.i-PlaneAffine.i) *PlaneNorm.i +
 *  (r* pnt.j+eye.j-PlaneAffine.j) *PlaneNorm.j +
 *  (r* pnt.k+eye.k-PlaneAffine.k) *PlaneNorm.k ==0 
 *  
 *  r (pnt.i*PlaneNorm.i+pnt.j*PlaneNorm.j+pnt.k*PlaneNorm.k) ==
 *  (PlaneAffine.i-eye.i)*PlaneNorm.i +(PlaneAffine.j-eye.j)*PlaneNorm.j+(PlaneAffine.k-eye.k)*PlaneNorm.k
 *  r =   pnt.Dot (PlaneNorm)/PlaneNorm.Dot(PlaneAffine-eye) 
 *  xyz = r * pnt + eye;
 */
int BoundingBox::Intersect (const QVector &eye, const QVector &pnt,float err) {
  QVector IntersectXYZ (eye.i,eye.j,eye.k);
  QVector Normal (mx - lx);
  double divisor=pnt.Dot (Normal);
  if (divisor!=0) {
    IntersectXYZ+= (Normal.Dot(lx-eye)/divisor)*pnt;//point of intersection with lx plane
    //VSFileSystem::Fprintf (stderr,"Intersect::%f,%f,%f>\n",IntersectXYZ.i,IntersectXYZ.j,IntersectXYZ.k);
    if (OpenWithin (IntersectXYZ,err,0)) 
      return Normal.Dot(lx-eye)*divisor>=0?1:-1;
  }
  Normal = -Normal;  
  divisor=pnt.Dot(Normal);
  if (divisor!=0) {
    IntersectXYZ = eye+( Normal.Dot(mx-eye)/divisor)*pnt; //point of interscetion with mx plane
    //VSFileSystem::Fprintf (stderr,"Intersect::%f,%f,%f>\n",IntersectXYZ.i,IntersectXYZ.j,IntersectXYZ.k);
    if (OpenWithin (IntersectXYZ,err,0)) 
      return Normal.Dot(mx-eye)*divisor>=0?1:-1;
  }
  Normal = my - ly;
  divisor = pnt.Dot(Normal);
  if (divisor!=0) {
    IntersectXYZ = eye + (Normal.Dot(ly-eye)/divisor)*pnt;//point of intersection with the ly plane
    //VSFileSystem::Fprintf (stderr,"Intersect::%f,%f,%f>\n",IntersectXYZ.i,IntersectXYZ.j,IntersectXYZ.k);
    if (OpenWithin (IntersectXYZ,err,1))
      return Normal.Dot(ly-eye)*divisor>=0?1:-1;
  }
  Normal = -Normal;
  divisor = pnt.Dot(Normal);
  if (divisor!=0) {

    IntersectXYZ = eye + (Normal.Dot(my-eye)/divisor)*pnt;//point of intersection with my plane
    //VSFileSystem::Fprintf (stderr,"Intersect::%f,%f,%f>\n",IntersectXYZ.i,IntersectXYZ.j,IntersectXYZ.k);
    if (OpenWithin (IntersectXYZ,err,1)) 
      return Normal.Dot(my-eye)*divisor>=0?1:-1;
  }
  Normal = (mz - lz);
  divisor = pnt.Dot(Normal);
  if (divisor!=0) {
    IntersectXYZ = eye + (Normal.Dot(lz-eye)/divisor)*pnt;//point of intersection with the ly plane
    //VSFileSystem::Fprintf (stderr,"Intersect::%f,%f,%f>\n",IntersectXYZ.i,IntersectXYZ.j,IntersectXYZ.k);
    if (OpenWithin (IntersectXYZ,err,2)) 
      return Normal.Dot(lz-eye)*divisor>=0?1:-1;
  }

  Normal = -Normal;
  divisor = pnt.Dot(Normal);
  if (divisor!=0) {
    IntersectXYZ = eye + (Normal.Dot (mz-eye)/divisor)*pnt;//point of intersection with my plane
    //VSFileSystem::Fprintf (stderr,"Intersect::%f,%f,%f>\n",IntersectXYZ.i,IntersectXYZ.j,IntersectXYZ.k);
    if (OpenWithin (IntersectXYZ,err,2)) 
      return Normal.Dot(mz-eye)*divisor>=0?1:-1;
  }
  return false;
}

/** queries the bounding box, excluding a select edge 
 *  0 = lx, 1=mx, 2=ly 3=my 4=lz 5=mz
 */
bool BoundingBox::OpenWithin (const QVector &query, float err, int excludeWhich) {
  err = -err;
  QVector tquery (query -lx); // now it's in coords with respect to the minimum x;
  QVector tmp (mx-lx);
  if (excludeWhich!=0){
    if (tquery.Dot (tmp) <err) {
      //VSFileSystem::Fprintf (stderr,"tqueryreturn mx-lx%f Exc %d\n",tquery.Dot(tmp),excludeWhich);
      return false;
    }
  }
  if (excludeWhich!=0) {
    tquery = query - mx;
    tmp = lx - mx;
    if (tquery.Dot(tmp)<err){
      //VSFileSystem::Fprintf (stderr,"tqueryreturn lx-mx, %fExc %d\n",tquery.Dot(tmp),excludeWhich);
      return false;
    }
  }
  if (excludeWhich!=1) {
    tquery = query - ly;
    tmp = my - ly;
    if (tquery.Dot (tmp)<err) {
      //VSFileSystem::Fprintf (stderr,"tqueryreturn my-ly%f Exc %d\n",tquery.Dot(tmp),excludeWhich);
      return false;
    }
  }
  if (excludeWhich!=1) {
    tquery = query - my;
    tmp = ly - my;
    if (tquery.Dot (tmp) <err) {
      //VSFileSystem::Fprintf (stderr,"tqueryreturn ly-my%f Exc %d\n",tquery.Dot(tmp),excludeWhich);
      return false;
    }
  }
  if (excludeWhich!=2) {
    tquery = query - lz;
    tmp = mz - lz;
    if (tquery.Dot (tmp)<err) {
      //VSFileSystem::Fprintf (stderr,"tqueryreturn mz-lz%f Exc%d\n",tquery.Dot(tmp),excludeWhich);
      return false;
    }
  }
  if (excludeWhich!=2) {
    tquery = query - mz;
    tmp = lz - mz;
    if (tquery.Dot (tmp) <err) {
      //VSFileSystem::Fprintf (stderr,"tqueryreturn lz-mz%f Ignore%d\n",tquery.Dot(tmp),excludeWhich);
      return false;
    }
  }
  return true; //within bounding box;
}

/** Queries the bounding box whether the query vector is within it*/
bool BoundingBox::Within (const QVector &query, float err) {
  err = -err;
  QVector tquery =query -lx; // now it's in coords with respect to the minimum x;
  QVector tmp = mx-lx;
  if (tquery.Dot (tmp) <err) {
    //VSFileSystem::Fprintf (stderr,"tqueryreturn mx-lx%f",tquery.Dot(tmp));
    return false;
  }
  tquery = query - mx;
  tmp = lx - mx;
  if (tquery.Dot(tmp)<err){
    //VSFileSystem::Fprintf (stderr,"tqueryreturn lx-mx, %f",tquery.Dot(tmp));
    return false;
  }
  tquery = query - ly;
  tmp = my - ly;
  if (tquery.Dot (tmp)<err) {
    //VSFileSystem::Fprintf (stderr,"tqueryreturn my-ly%f",tquery.Dot(tmp));
    return false;
  }
  tquery = query - my;
  tmp = ly - my;
  if (tquery.Dot (tmp) <err) {
    //VSFileSystem::Fprintf (stderr,"tqueryreturn ly-my%f",tquery.Dot(tmp));
    return false;
  }
  tquery = query - lz;
  tmp = mz - lz;
  if (tquery.Dot (tmp)<err) {
    //VSFileSystem::Fprintf (stderr,"tqueryreturn mz-lz%f",tquery.Dot(tmp));
    return false;
  }
  tquery = query - mz;
  tmp = lz - mz;
  if (tquery.Dot (tmp) <err) {
    //VSFileSystem::Fprintf (stderr,"tqueryreturn lz-mz%f",tquery.Dot(tmp));
    return false;
  }
  return true; //within bounding box;
}

