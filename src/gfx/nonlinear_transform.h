#ifndef NONLINEAR_TRANSFORM_H_
#define NONLINEAR_TRANSFORM_H_
#include <math.h>
#ifndef M_PI
#define M_PI 3.1415926536
#endif
/**
 * We could make it virtual and ahve a sphere-map or cube-map version of this
 *
 */
class IdentityTransform {
 public:
  ///Transforms in a possibly nonlinear way the point to some new space
  virtual Vector Transform (const Vector &v) {return v;} 
  ///transforms a direction to some new space
  virtual Vector TransformNormal (const Vector & v, const Vector &n) {return n;}
  ///Transforms in reverse the vector into quadsquare space
  virtual Vector InvTransform (const Vector &v) {return v;}
  ///Transforms a min and a max vector and figures out what is bigger
  virtual CLIPSTATE BoxInFrustum (Vector &min, Vector &max,const Vector & campos) {return GFXBoxInFrustum(min,max);}
  float TransformS (float x, float scale) {return x*scale;}
  float TransformT (float y, float scale) {return y*scale;}
};

extern float SphereTransformRenderlevel;
class SphericalTransform:public IdentityTransform {
  float scalex,scalez,r;
 public:
  SphericalTransform (float a,float b, float c):IdentityTransform() {SetXZ (a,c); SetR (b);}
  void SetXZ (float x,float z) {this->scalex = 2*M_PI/x;this->scalez= M_PI/z;}//x ranges from 0 to 2PI x ranges from -PI/2 to PI/2
  void SetR (float rr) {r =rr;}
  float GetR() {return r;}
  float GetX () {return 2*M_PI/scalex;}
  float GetZ () {return M_PI/scalez;} 
  Vector Transform (const Vector &v) {
    Vector T (v.i*scalex, r+v.j,v.k *scalez-.5*M_PI);
    float cosphi = cos (T.k);
    return Vector (T.j*cosphi*cos (T.i),T.j*sin (T.k),T.j*cosphi*sin(T.i));
  }
  Vector TransformNormal (const Vector &point, const Vector & n) {
    return SphericalTransform::Transform (n+point)-Transform (point);
    
  }
  Vector InvTransform (const Vector &v) {
    float rplusy = v.Magnitude();
    //    float lengthxypln = sqrtf (rplusy*rplusy-v.j*v.j);//pythagorus
    return Vector ((atan2 (-v.k,-v.i)+M_PI)/scalex,rplusy-r,(asin(v.j/rplusy)+M_PI*.5)/scalez);
  }
  CLIPSTATE BoxInFrustum (Vector &min, Vector &max, const Vector & campos) {
    const float rendermin=3;
    /*
    float tmpx = fabs(campos.i-min.i);float maxx = fabs(campos.i-max.i);
    if (tmpx>.35*GetX()&&tmpx<.65*GetX()&&maxx>.25*GetX()&&maxx<.75*GetX()) {return GFX_NOT_VISIBLE;}
    tmpx = fabs(campos.k-min.k); maxx = fabs(campos.k-max.k);
    if (tmpx>.25*GetZ()&&tmpx<.75*GetZ()&&maxx>.25*GetZ()&&maxx<.75*GetZ()) {      return GFX_NOT_VISIBLE;//i/f it's on the other side of the hemisphere} */
    if (SphereTransformRenderlevel<rendermin) {
      return GFX_PARTIALLY_VISIBLE;
    }
    Vector tmin= SphericalTransform::Transform (min);
    Vector tmax = SphericalTransform::Transform (max);
    tmax = .5*(tmax+tmin);//center
    float rad = 1.8*(tmax-tmin).Magnitude();

    return GFXSpherePartiallyInFrustum (tmax,rad);
  }
};
/*
class PlanetaryTransform:public SphericalTransform {
  Vector Origin;
 public:
  PlanetaryTransform (Vector loc, float r, float scalex, float scaley): SphericalTransform (float r,float scalex, float scaley), origin(loc) {}  
  void SetOrigin (const Vector &t) {Origin = t;}
  ~PlanetaryTransform () {while (1);}
  Vector Transform (const Vector & v) {return Origin+SphericalTransform::Transform(v);}
  Vector TransformNormal (const Vector &p, const vector & n){return SphericalTransform::TransformNormal (p,n);}
  Vector InvTransform (const Vector &v) {return SphericalTransform::InvTransform (v-Origin);}  
  void GrabPerpendicularOrigin (const Vector &m, Matrix trans){
    Vector norm (m-Origin);
    Normalize(norm);
    Vector Intersection (norm*r);
    


  }
  CLIPSTATE BoxInFrustum (Vector &min, Vector &max, const Vector & campos) {
    min = min-Origin;
    max = max-Origin;
    return SphericalTransform::BoxInFrustum(min,max,campos-Origin);
  }  
};
*/
#endif
