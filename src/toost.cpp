#include "quaternion.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#define     GFXEPSILON         ((float)10e-6)

void Quaternion::to_matrix(Matrix mat) const {
  float W = v.i*v.i+v.j*v.j+v.k*v.k+s*s; //norm
  W = (W<0+GFXEPSILON&&W>0-GFXEPSILON)?0:2.0/W;

  float xw = v.i*W;
  float yw = v.j*W;
  float zw = v.k*W;
  
  float sx = s*xw;
  float sy = s*yw;
  float sz = s*zw;
 
  float xx = v.i*xw;
  float xy = v.i*yw;
  float xz = v.i*zw;
  
  float yy= v.j*yw;
  float yz= v.j*zw;
  float zz= v.k*zw;
  

#define M(B,A) mat[B*4+A]
  M(0,0)  = 1 - ( yy + zz );
  M(1,0)  =     ( xy + sz );
  M(2,0)  =     ( xz - sy );

  M(0,1)  =     ( xy - sz );
  M(1,1)  = 1 - ( xx + zz );
  M(2,1)  =     ( yz + sx );

  M(0,2)  =     ( xz + sy );
  M(1,2)  =     ( yz - sx );
  M(2,2) = 1 - ( xx + yy );

  M(3,0)  = M(3,1) = M(3,2) = M(0,3) = M(1,3) = M(2,3) = 0;
  M(3,3) = 1;

#undef M
  /*
  clog << "Quaternion " << *this << " converted to matrix: \n";
  float *temp = mat;
  for(int a=0; a<3; a++, temp++) {
    clog.form("%f %f %f\n", temp[0], temp[4], temp[8]);
  }
  clog << endl;
  */
}

Quaternion Quaternion::from_vectors(const Vector &v1, const Vector &v2, const Vector &v3) {
  float T = v1.i + v2.j + v3.k + 1, S, W, X, Y, Z;
  
  if(T>0) {
    
    S = 0.5 / sqrtf(T);
    
    W = 0.25 / S;
      
    X = ( v3.j - v2.k ) * S;
      
    Y = ( v1.k - v3.i ) * S;
    
    Z = ( v2.i - v1.j ) * S;
  }
  else {
    int max = (v1.i>v2.j)?1:2;
    if(max==1)
      max = (v1.i>v3.k)?1:3;
    else 
      max = (v2.j>v3.k)?2:3;

    switch(max) {
    case 1:
      //column 0
      S = sqrtf ( (v2.i - (v2.j + v3.k)) + 1);
      X = S * .5;
      S = .5/S;
      W = (v3.j - v2.k)*S;
      Y = (v2.i + v1.j)*S;
      Z = (v3.i + v1.k)*S;
      break;
    case 2:
      //column 1
      S  = sqrtf( (v3.j - (v3.k + v1.i)) +1);
      Y = 0.5 *  S;
      S = .5 / S;
      W = (v1.k - v3.i);
      Z = (v3.j + v2.k);
      X = (v1.j + v2.i);
      break;
    case 3:
      //column 2    
      S  = sqrtf( (v1.k - (v1.i + v2.j))+1);
      Z = 0.5 *  S;
      S = .5 / S;
      W = (v2.i - v1.j);
      X = (v1.k + v3.i);
      Y = (v2.k + v3.j);
      break;
    }

    /*
    switch(max) {
    case 1:
      //column 0
      S  = sqrtf( (v1.j - (v2.j + v3.k ))+1);
      Y = 0.5 *  S;
      S = .5 / S;
      Z = (v1.j + v2.i ) * S;
      X = (v1.k + v3.i ) * S;
      W = (v2.k - v3.j ) * S;
  
      break;
    case 2:
      //column 1
      S  = sqrtf( v2.k - (v3.k + v1.i )+1);
      Y = 0.5 *  S;
      S = .5 / S;
      Z = (v3.j + v2.k ) * S;
      X = (v2.i + v1.j ) * S;
      W = (v3.i - v1.k ) * S;
      break;
    case 3:
      //column 2    
      S  = sqrtf( v3.i - (v1.i + v2.j )+1);
      Z = 0.5 *  S;
      S = .5 / S;
      X = (v1.k + v3.i ) * S;
      Y = (v3.j + v2.k ) * S;
      W = (v1.j - v2.i ) * S;
      break;
    }
    */
  }
  return Quaternion(W, Vector(X,Y,Z));
}

Quaternion Quaternion::from_axis_angle(const Vector &axis, float angle) {
  float sin_a = sinf( angle / 2 );
  float cos_a = cosf( angle / 2 );
  
  return Quaternion(cos_a, Vector(axis.i / sin_a, 
		axis.j / sin_a,
		axis.k / sin_a));
}

void CrossProduct(const Vector &a,const Vector &b, Vector &RES) 
{
		RES.i = a.j*b.k-a.k*b.j; 
    	RES.j = a.k*b.i-a.i*b.k;
    	RES.k = a.i*b.j-a.j*b.i;
}

float DotProduct(const Vector &a,const Vector &b)
{
	return (a.i*b.i+a.j*b.j+a.k*b.k);
}
void Normalize(Vector &r)
{
	float size = sqrtf(DotProduct (r,r));
	r.i /= size;
	r.j /= size;
	r.k /= size;
}

float psilon=.00001;
char equiv (float x, float y) {
  return x > y-psilon &&x<y+psilon&&((x>-.001&&y>-.001)||(x<.001&&y<.001));
}

int main(int argc, char ** argv) {
  float m [16];
  int tests;
  sscanf (argv[1],"%d",&tests);
  sscanf (argv[2],"%f",&psilon);
  for (int i=0;i<tests;i++) {
  Vector x;
  Vector y(rand()%1000-500,rand()%1000-500,rand()%1000-500);
  Vector z(rand()%1000-500,rand()%1000-500,rand()%1000-500);
  CrossProduct(y,z,x);
  CrossProduct(z,x,y);
  Normalize(x);
  Normalize(y);
  Normalize(z);
  Transformation tmp(Quaternion::from_vectors(x,y,z),Vector(0,0,0));
  tmp.to_matrix(m);
  
  if (!(equiv(x.i,m[0])&&equiv(x.j,m[1])&&equiv(x.k,m[2]) &&
	equiv(y.i,m[4])&&equiv(y.j,m[5])&&equiv(y.k,m[6]) &&
	equiv(z.i,m[8])&&equiv(z.j,m[9])&&equiv(z.k,m[10]))){
  printf ("<%f %f %f>\n",x.i,x.j,x.k);
  printf ("<%f %f %f>\n",y.i,y.j,y.k);
  printf ("<%f %f %f>\n",z.i,z.j,z.k);

    tmp = Transformation(Quaternion::from_vectors(x,y,z),Vector(0,0,0));
    tmp.to_matrix(m);
  for (int i=0;i<16;i++) {
    printf ("%f ",m[i]);
    if (i%4==3)
      printf ("\n");
  }

  }
  
  }
}
