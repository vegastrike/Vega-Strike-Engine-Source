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
#include <math.h>
#include <iostream.h>
#ifdef __cplusplus
class Vector;
void Normalize(Vector &);
float DotProduct(const Vector &,const Vector &);

void CrossProduct(const Vector &, const Vector &, Vector &);

void ScaledCrossProduct(const Vector &, const Vector &, Vector &); 

inline Vector operator* (const Vector &lval, const float obj);

inline Vector operator* (const float obj, const Vector &rval);

inline Vector operator+= (Vector &lval, const Vector &obj);

class Vector {
	private:
		
	public:
		Vector()
		{
			i = 0;
			j = 0;
			k = 0;
		}
		Vector(float i,float j,float k)
		{
			this->i = i;
			this->j = j;
			this->k = k;
		}
		float i,j,k;
		void Yaw(float rad);
		void Roll(float rad);
		void Pitch(float rad);

		Vector Transform ( const Vector &p, const Vector &q, const Vector &r)
		{
			Vector tvect = Vector ( DotProduct(*this, p), DotProduct(*this,q), DotProduct(*this,r));
			*this = tvect;
			return *this;
		}
			
		//Vector operator= (const Vector &obj) {i = obj.i; j = obj.j; k = obj.k; return *this;}
		Vector operator+ (const Vector &obj) const {Vector retval(i + obj.i, j + obj.j, k + obj.k); return retval;}
		//Vector operator+= (const Vector &obj) {i += obj.i; j += obj.j; k += obj.k; return *this;}
		Vector operator- (const Vector &obj) const {Vector retval(i - obj.i, j - obj.j, k - obj.k); return retval;}
		//Vector operator* (const float obj) const {Vector retval(i * obj, j * obj, k * obj); return retval;}
		Vector Normalize(){::Normalize (*this); return *this;};
		Vector operator- () const {Vector retval(-i, -j, -k); return retval;}

		Vector Cross(const Vector &v) const {Vector t; CrossProduct(*this, v, t); return t;};
		
		float operator* (const Vector &b) const {return (i*b.i+j*b.j+k*b.k);};
		//Vector operator* (const Vector &b) {return Vector(i*b.i, j*b.j, k*b.k);};
		float Dot(const Vector &b) const {return *this * b;};
		//float Dot(const Vector &b) {return i*b.i+j*b.j+k*b.k;};

		/*
		Vector operator/(const Vector &b) {return Vector(i*b.i, );};
		Vector Transform(const Vector &b) {return *this / b;};
		*/

		float Magnitude() const {return sqrtf(i*i+j*j+k*k);};
		float MagnitudeSquared() const { return i*i + j*j + k*k; };


		//friend Vector operator+(const Vector &lval, const Vector &rval);
		//friend Vector operator+(const Vector &lval, const Vector &rval);
		inline const Vector Transform(float m1[]) const
{
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

inline ostream &operator<<(ostream &os, const Vector &obj) {
  return os << "(" << obj.i << "," << obj.j << "," << obj.k << ")";
}


/*
inline Vector operator+(const Vector &lval, const Vector &rval)
{
	Vector retval = lval; 
	return retval+rval;
}
*/
inline Vector operator* (const Vector &lval, const float obj) {Vector retval(lval.i * obj, lval.j * obj, lval.k * obj); return retval;}

inline Vector operator* (const float obj, const Vector &rval) {Vector retval(rval.i * obj, rval.j * obj, rval.k * obj); return retval;}

inline Vector operator+= (Vector &lval, const Vector &obj) {lval.i += obj.i; lval.j += obj.j; lval.k += obj.k; return lval;}

inline Vector operator*= (Vector &lval, const float &obj) {lval.i *= obj; lval.j *= obj, lval.k *= obj; return lval;}

/*
inline Vector operator*(const Vector &lval, const float &rval)
{
	return Vector(lval.i * rval, lval.j * rval, lval.k * rval);
}

inline Vector operator*(const float &lval, const Vector &rval)
{
	return rval * lval;
}
*/
/*
struct Light {
	float Red;
	float Green;
	float Blue;
	Vector Direction;
};
*/

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

void Yaw (float rad, Vector &p,Vector &q, Vector &r);
void Pitch (float rad,Vector &p, Vector &q, Vector &r);
void Roll (float rad,Vector &p, Vector &q, Vector &r);
void ResetVectors (Vector &p, Vector &q, Vector &r);
void MakeRVector (Vector &p, Vector &q, Vector &r);
void Orthogonize(Vector &p, Vector &q, Vector &r);;
#endif
#endif
