#include "nonlinear_transform.h"
#include "matrix.h"
#include "quaternion.h"
//#include "config_xml.h"
//#include "vs_globals.h"

inline Vector SwizzleIt (const Vector &v) {return Vector(-v.i,-v.j,v.k);}//return Vector (v.i,v.k,v.j);}

class PlanetaryTransform: public SphericalTransform{
  float scaleheight;
  ///make sure ~Planet destructors alloc memory for this one so it survives planet
  float *xform;
 public:
  Vector ReverseReverseX (const Vector & v) const {return Vector (v.i,scaleheight*v.j,v.k);}
  Vector ReverseX (const Vector & v) const {return Vector (v.i,v.j/scaleheight,v.k);}
  PlanetaryTransform (float radius, float xsize, float ysize, int ratio, float scaleatmos): SphericalTransform (xsize*ratio,radius,ysize*ratio) {
    //    static float sh =XMLSupport::parse_float(vs_config->getVariable ("physics","atmosphereheight","10"));
    scaleheight=scaleatmos;
    xform=NULL;
  }
  void SetTransformation (float * t) {xform = t;}
  virtual ~PlanetaryTransform () {while (1);}
  Vector Transform (const Vector & v) const {return ::Transform (xform,SwizzleIt (SphericalTransform::Transform(ReverseReverseX(v))));}
  Vector TransformNormal (const Vector &p, const Vector & n) const {
    return this->Transform (p+n)-this->Transform (p);
  }
  Vector InvTransform (const Vector &v) const{
    return ReverseX(SphericalTransform::InvTransform (SwizzleIt(::InvTransform(xform,v))));
  }  
  Vector InvTransformNormal (const Vector & p, const Vector &n) const{
    return 
      this->InvTransform (p+n) -
      this->InvTransform(p);
  }
  void InvTransformBasis (Matrix basis, Vector p, Vector q, Vector r, Vector coord) {
    Matrix yaw;
    Matrix pitchandyaw;
    coord =::InvTransform (xform,coord);
    p = ::InvTransformNormal (xform,p);
    q = ::InvTransformNormal (xform,q);
    r = ::InvTransformNormal (xform,r);
    RotateAxisAngle (yaw,Vector (0,1,0), -atan2 (coord.i,coord.k));//will figure out how far to get to the z axis
    RotateAxisAngle (basis,Vector (1,0,0),-atan2 (sqrtf (coord.i*coord.i+coord.k*coord.k),coord.j));//deals with pitch to get it literally "on the top of the world"
    MultMatrix (pitchandyaw,basis,yaw);
    VectorAndPositionToMatrix (basis,
			       ::TransformNormal (pitchandyaw,p),
			       ::TransformNormal (pitchandyaw,q),
			       ::TransformNormal (pitchandyaw,r),
			       ReverseX(SphericalTransform::InvTransform(SwizzleIt(coord))));
  }
  void GrabPerpendicularOrigin (const Vector &m, Matrix trans) const{
    static int counter=0;
    Vector pos =::InvTransform (xform,m);
    Vector q(pos);
    q.Normalize();
    Vector Intersection (q*GetR());
    Vector p(q.Cross (Vector(0,1,0)));
    Vector r(p.Cross (q));
    p.Normalize();
    r.Normalize();
    if (counter%50==0) {
      fprintf (stderr,"V<%f, %f, %f>",pos.i,pos.j,pos.k);
    }
    signed char posit = (pos.i>0)?-1:1;
    pos = SphericalTransform::InvTransform(Vector (pos.i,pos.k,pos.j));
    if (counter++%50==0) {
      float tmp = GetX();
      float tmp2=GetZ();
      
      fprintf (stderr,"Spherical<%f, %f, %f>\n",pos.i/tmp,pos.j,pos.k/tmp2);
    }

    /*    
    Matrix tmpmat;
    static int blooh=1;
    if (blooh) {
      RotateAxisAngle (tmpmat,q,M_PI*.5+pos.i*scalex);
      p= ::TransformNormal (tmpmat,p);
      r= ::TransformNormal (tmpmat,r);
      }*/
    //fprintf (stderr,"Spherical <%f,%f,%f>",pos.i,pos.j,pos.k);
    Intersection = Intersection -posit*r*pos.i-posit*p*pos.k/*-sinf(pos.i*scalex)*r*pos.i;-r*cosf(pos.i*scalex)*pos.k+p*sinf(pos.i*scalex)*pos.k*/;
    p= ::TransformNormal (xform,p);
    q= ::TransformNormal (xform,q);
    r= ::TransformNormal (xform,r);
    Intersection  =::Transform (xform,Intersection);
    VectorAndPositionToMatrix (trans,p,q,r,Intersection);
  }

};
struct PlanetaryOrbitData {
  ///Planetary Transform that will move to and from planet space
  PlanetaryTransform * trans;
  ///Current Physical State
  Transformation cps;
  ///Previous Physical State
  Transformation pps;
  ///Has this starship collided with a planet any more 
  bool dirty;
};
