#include "gfx/nonlinear_transform.h"
#include "gfx/matrix.h"



class PlanetaryTransform: public SphericalTransform{
  ///make sure ~Planet destructors alloc memory for this one so it survives planet
  float *xform;
 public:
  
  PlanetaryTransform (float radius, float xsize, float ysize, int ratio): SphericalTransform (1,radius,1) {
    SetXZ (xsize*ratio,ysize*ratio);
  }
  void SetTransformation (float * t) {xform = t;}
  virtual ~PlanetaryTransform () {while (1);}
  Vector Transform (const Vector & v) {return SphericalTransform::Transform(::Transform(xform,v));}
  Vector TransformNormal (const Vector &p, const Vector & n){return SphericalTransform::TransformNormal (::Transform(xform,p),::TransformNormal(xform,n));}
  Vector InvTransform (const Vector &v) {return SphericalTransform::InvTransform (::InvTransform (xform,v));}  
  void GrabPerpendicularOrigin (const Vector &m, Matrix trans){
    static int counter=0;
    Vector pos =::InvTransform (xform,m);
    Vector q(pos);
    q.Normalize();
    Vector Intersection (q*GetR());
    Vector p(q.Cross (Vector(0,1,0)));
    Vector r(p.Cross (q));
    p.Normalize();
    r.Normalize();
    if (counter%100==0) {
      fprintf (stderr,"V<%f, %f, %f>",pos.i,pos.j,pos.k);
    }
    signed char posit = (pos.i>0)?-1:1;
    pos = SphericalTransform::InvTransform(Vector (pos.i,pos.k,pos.j));
    if (counter++%100==0) {
      float tmp = GetX();
      float tmp2=GetZ();
      
      fprintf (stderr,"Spherical<%f, %f, %f>\n",pos.i/tmp,pos.j,pos.k/tmp2);
    }

    //fprintf (stderr,"Spherical <%f,%f,%f>",pos.i,pos.j,pos.k);
    Intersection = Intersection + posit*r*pos.i+posit*p*pos.k;
    p= ::TransformNormal (xform,p);
    q= ::TransformNormal (xform,q);
    r= ::TransformNormal (xform,r);
    Intersection  =::Transform (xform,Intersection);
    VectorAndPositionToMatrix (trans,p,q,r,Intersection);
  }

};
