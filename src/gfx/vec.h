/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Chris Fry
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef _3DMANIP_H_
#define _3DMANIP_H_
#include "macosx_math.h"
#include <math.h>
#ifndef WIN32
#include <iostream>
#endif
#ifdef __cplusplus
class Vector;

inline Vector operator* (const Vector &lval, const float obj);

inline Vector operator* (const float obj, const Vector &rval);

inline Vector operator+= (Vector &lval, const Vector &obj);

inline float DotProduct (const Vector &a, const Vector &b);
inline void Normalize(Vector &r);
class QVector;
class Vector {
 public:
  float i,j,k;
  Vector () {}
  inline Vector (const QVector &);
  Vector(float i,float j,float k) {
    this->i = i;
    this->j = j;
    this->k = k;
  }
  inline void Set (float x, float y, float z) {i=x;j=y;k=z;}
  void Yaw(float rad);
  void Roll(float rad);
  void Pitch(float rad);
  Vector Transform ( const Vector &p, const Vector &q, const Vector &r) {
    Vector tvect = Vector ( DotProduct(*this, p), DotProduct(*this,q), DotProduct(*this,r));
    *this = tvect;
    return *this;
  }
  Vector operator+ (const Vector &obj) const {return Vector (i + obj.i, j + obj.j, k + obj.k);}
  Vector operator- (const Vector &obj) const {return Vector (i - obj.i, j - obj.j, k - obj.k);}
  Vector Normalize(){::Normalize (*this); return *this;};
  Vector operator- () const {return Vector (-i, -j, -k);}
  Vector Cross(const Vector &v) const {return Vector ( this->j*v.k-this->k*v.j, 
						       this->k*v.i-this->i*v.k,
						       this->i*v.j-this->j*v.i);}
  float operator* (const Vector &b) const {return (i*b.i+j*b.j+k*b.k);};
  float Dot(const Vector &b) const {return DotProduct(*this, b);}
  float Magnitude() const {return sqrtf(i*i+j*j+k*k);};
  float MagnitudeSquared() const { return i*i + j*j + k*k; };
  
  inline const Vector Transform(const float m1[16]) const {
    return Vector(m1[0] * i + m1[4] * j + m1[8] * k + m1[12],
		  m1[1] * i + m1[5] * j + m1[9] * k + m1[13],
		  m1[2] * i + m1[6] * j + m1[10] * k + m1[14]);
  }

  Vector Min(const Vector &other) {
    return Vector((i<other.i)?i:other.i,
		  (j<other.j)?j:other.j,
		  (k<other.k)?k:other.k);
  }
  Vector Max(const Vector &other) {
    return Vector((i>other.i)?i:other.i,
		  (j>other.j)?j:other.j,
		  (k>other.k)?k:other.k);
  }
};
#ifndef WIN32
inline std::ostream &operator<<(std::ostream &os, const Vector &obj) {
  return os << "(" << obj.i << "," << obj.j << "," << obj.k << ")";
}
#endif

inline Vector operator* (const Vector &lval, const float obj) {Vector retval(lval.i * obj, lval.j * obj, lval.k * obj); return retval;}

inline Vector operator/ (const Vector &lval, const float obj) {Vector retval(lval.i / obj, lval.j / obj, lval.k / obj); return retval;}

inline Vector operator* (const float obj, const Vector &rval) {Vector retval(rval.i * obj, rval.j * obj, rval.k * obj); return retval;}

inline Vector operator+= (Vector &lval, const Vector &obj) {lval.i += obj.i; lval.j += obj.j; lval.k += obj.k; return lval;}

inline Vector operator*= (Vector &lval, const float &obj) {lval.i *= obj; lval.j *= obj, lval.k *= obj; return lval;}

inline void Normalize(Vector &r)
{
	float size = sqrtf(r.i*r.i+r.j*r.j+r.k*r.k);
	r.i /= size;
	r.j /= size;
	r.k /= size;
}

inline float DotProduct(const Vector &a,const Vector &b)
{
	return (a.i*b.i+a.j*b.j+a.k*b.k);
}





inline void ScaledCrossProduct(const Vector &a, const Vector &b, Vector &r) {
	r.i = a.j*b.k-a.k*b.j; 
    	r.j = a.k*b.i-a.i*b.k;
    	r.k = a.i*b.j-a.j*b.i;
	float size = sqrtf(r.i*r.i+r.j*r.j+r.k*r.k);
	r.i /= size;
	r.j /= size;
	r.k /= size;
}


inline Vector PolygonNormal(Vector v1, Vector v2, Vector v3)
{
	Vector temp;
	ScaledCrossProduct(v2-v1, v3-v1, temp);
	return temp;
}

inline Vector Transform(const Vector &p, const Vector &q, const Vector &r, const Vector &v) 
{
	return Vector(p.i * v.i + q.i * v.j + r.i * v.k, 
		p.j * v.i + q.j * v.j + r.j * v.k, 
		p.k * v.i + q.k * v.j + r.k * v.k);
}
inline Vector CrossProduct(const Vector& v1, const Vector& v2) {
	Vector result;
    result.i = v1.j * v2.k - v1.k * v2.j;
    result.j = v1.k * v2.i - v1.i * v2.k;
    result.k = v1.i * v2.j  - v1.j * v2.i;     
	return result;
}
inline void CrossProduct(const Vector & a, const Vector & b, Vector & RES) {RES = a.Cross(b);}

void Yaw (float rad, Vector &p,Vector &q, Vector &r);
void Pitch (float rad,Vector &p, Vector &q, Vector &r);
void Roll (float rad,Vector &p, Vector &q, Vector &r);
void ResetVectors (Vector &p, Vector &q, Vector &r);
void MakeRVector (Vector &p, Vector &q, Vector &r);
void Orthogonize(Vector &p, Vector &q, Vector &r);
#endif
#endif

