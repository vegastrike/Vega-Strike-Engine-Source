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


#ifdef __cplusplus
class XVector;

inline XVector operator* (const XVector &lval, const QFLOAT obj);

inline XVector operator* (const QFLOAT obj, const XVector &rval);

inline XVector operator+= (XVector &lval, const XVector &obj);

inline QFLOAT DotProduct (const XVector &a, const XVector &b);
inline void Normalize(XVector &r);
class YVector;
class XVector {
 public:
  QFLOAT i,j,k;
  XVector () {}
 private:
  friend class Quadsquare;
  friend class QuadTree;
  friend class CoordinateSelect;
  friend class AIScript;
  friend class PlanetaryTransform;
  friend class SphericalTransform;
  inline YVector operator = (const YVector &);
 public:
  inline XVector (const YVector &);
  inline YVector Cast() const;
  XVector(QFLOAT i,QFLOAT j,QFLOAT k) {
    this->i = i;
    this->j = j;
    this->k = k;
  }
  inline void Set (QFLOAT x, QFLOAT y, QFLOAT z) {i=x;j=y;k=z;}
  void netswap()
  {
  	this->i = NetSwap( this->i);
  	this->j = NetSwap( this->j);
  	this->k = NetSwap( this->k);
  }
  void Yaw(QFLOAT rad);
  void Roll (QFLOAT rad);
  void Pitch(QFLOAT rad);
  XVector Scale (QFLOAT s)const  {return XVector (s*i,s*j,s*k);}
  XVector Transform ( const XVector &p, const XVector &q, const XVector &r) {
    XVector tvect = XVector ( DotProduct(*this, p), DotProduct(*this,q), DotProduct(*this,r));
    *this = tvect;
    return *this;
  }
  XVector operator+ (const XVector &obj) const {return XVector (i + obj.i, j + obj.j, k + obj.k);}
  XVector operator- (const XVector &obj) const {return XVector (i - obj.i, j - obj.j, k - obj.k);}
  XVector Normalize(){::Normalize (*this); return *this;};
  XVector operator- () const {return XVector (-i, -j, -k);}
  bool operator== (const XVector &b)const {return (i==b.i && j==b.j && k==b.k);};
  XVector Cross(const XVector &v) const {return XVector ( this->j*v.k-this->k*v.j, 
						       this->k*v.i-this->i*v.k,
						       this->i*v.j-this->j*v.i);}
  QFLOAT operator* (const XVector &b) const {return (i*b.i+j*b.j+k*b.k);};
  QFLOAT Dot(const XVector &b) const {return DotProduct(*this, b);}
  QFLOAT Magnitude() const {return XSQRT(i*i+j*j+k*k);};
  QFLOAT MagnitudeSquared() const { return i*i + j*j + k*k; };
  XVector Vabs()const {
    return XVector(i>=0?i:-i,
				   j>=0?j:-j,
				   k>=0?k:-k);
  }
  inline const XVector Transform(const class Matrix  &m1) const;

  XVector Min(const XVector &other) const{
    return XVector((i<other.i)?i:other.i,
		  (j<other.j)?j:other.j,
		  (k<other.k)?k:other.k);
  }
  XVector Max(const XVector &other) const{
    return XVector((i>other.i)?i:other.i,
		  (j>other.j)?j:other.j,
		  (k>other.k)?k:other.k);
  }
  XVector (struct _object *);
};


inline XVector operator/ (const XVector &lval, const QFLOAT obj) {XVector retval(lval.i / obj, lval.j / obj, lval.k / obj); return retval;}

inline XVector operator+= (XVector &lval, const XVector &obj) {lval.i += obj.i; lval.j += obj.j; lval.k += obj.k; return lval;}

inline XVector operator*= (XVector &lval, const QFLOAT &obj) {lval.i *= obj; lval.j *= obj, lval.k *= obj; return lval;}

inline void Normalize(XVector &r)
{
	QFLOAT size = XSQRT(r.i*r.i+r.j*r.j+r.k*r.k);
	if( size>0.00001)
	{
		r.i /= size;
		r.j /= size;
		r.k /= size;
	}
}

inline QFLOAT DotProduct(const XVector &a,const XVector &b)
{
	return (a.i*b.i+a.j*b.j+a.k*b.k);
}

inline XVector operator* (const XVector &lval, const double obj) {XVector retval(lval.i * obj, lval.j * obj, lval.k * obj); return retval;}
inline XVector operator* (const XVector &lval, const float obj) {XVector retval(lval.i * obj, lval.j * obj, lval.k * obj); return retval;}

inline XVector operator* (const double obj, const XVector &rval) {return XVector(rval.i * obj, rval.j * obj, rval.k * obj); }

inline XVector operator* (const float obj, const XVector &rval) {return XVector(rval.i * obj, rval.j * obj, rval.k * obj); }
inline XVector operator* (const XVector &lval, const int obj) {XVector retval(lval.i * obj, lval.j * obj, lval.k * obj); return retval;}

inline XVector operator* (const int obj, const XVector &rval) {return XVector(rval.i * obj, rval.j * obj, rval.k * obj); }




inline void ScaledCrossProduct(const XVector &a, const XVector &b, XVector &r) {
	r.i = a.j*b.k-a.k*b.j; 
    	r.j = a.k*b.i-a.i*b.k;
    	r.k = a.i*b.j-a.j*b.i;
	QFLOAT size = XSQRT(r.i*r.i+r.j*r.j+r.k*r.k);
	if( size<0.00001)
	{
		r.i = r.j = r.k = 0;
	}
	else
	{
		r.i /= size;
		r.j /= size;
		r.k /= size;
	}
}


inline XVector PolygonNormal(XVector v1, XVector v2, XVector v3)
{
	XVector temp;
	ScaledCrossProduct(v2-v1, v3-v1, temp);
	return temp;
}

inline XVector Transform(const XVector &p, const XVector &q, const XVector &r, const XVector &v) 
{
	return XVector(p.i * v.i + q.i * v.j + r.i * v.k, 
		p.j * v.i + q.j * v.j + r.j * v.k, 
		p.k * v.i + q.k * v.j + r.k * v.k);
}
inline XVector CrossProduct(const XVector& v1, const XVector& v2) {
	XVector result;
    result.i = v1.j * v2.k - v1.k * v2.j;
    result.j = v1.k * v2.i - v1.i * v2.k;
    result.k = v1.i * v2.j  - v1.j * v2.i;     
	return result;
}
inline void CrossProduct(const XVector & a, const XVector & b, XVector & RES) {RES = a.Cross(b);}

void Yaw (QFLOAT rad, XVector &p,XVector &q, XVector &r);
void Pitch (QFLOAT rad,XVector &p, XVector &q, XVector &r);
void Roll (QFLOAT rad,XVector &p, XVector &q, XVector &r);
void ResetVectors (XVector &p, XVector &q, XVector &r);
void MakeRVector (XVector &p, XVector &q, XVector &r);
void Orthogonize(XVector &p, XVector &q, XVector &r);
#endif

