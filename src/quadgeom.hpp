//Vega Strike
// Copyright 2001-2002 Daniel Horn
// geometry.hpp	-thatcher 1/28/1998 Copyright Thatcher Ulrich

// Some basic geometric types.

// This code may be freely modified and redistributed.  I make no
// warrantees about it; use at your own risk.  If you do incorporate
// this code into a project, I'd appreciate a mention in the credits.
//
// Thatcher Ulrich <tu@tulrich.com>


#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP


class	vector
// 3-element vector class, for 3D math.
{
public:
	vector() {}
	vector(float X, float Y, float Z) { x[0] = X; x[1] = Y; x[2] = Z; }
	vector(const vector& v) { x[0] = v.x[0]; x[1] = v.x[1]; x[2] = v.x[2]; }

	operator	const float*() const { return &x[0]; }
			
	float	Get(int element) const { return x[element]; }
	void	Set(int element, float NewValue) { x[element] = NewValue; }
	float	X() const { return x[0]; }
	float	Y() const { return x[1]; }
	float	Z() const { return x[2]; }
	void	SetX(float newx) { x[0] = newx; }
	void	SetY(float newy) { x[1] = newy; }
	void	SetZ(float newz) { x[2] = newz; }
	void	SetXYZ(float newx, float newy, float newz) { x[0] = newx; x[1] = newy; x[2] = newz; }
	
	vector	operator+(const vector& v) const;
	vector	operator-(const vector& v) const;
	vector	operator-() const;
	float	operator*(const vector& v) const;
	vector	operator*(float f) const;
	vector	operator/(float f) const { return this->operator*(1.0 / f); }
	vector	cross(const vector& v) const;

	vector&	normalize();
	vector&	operator=(const vector& v) { x[0] = v.x[0]; x[1] = v.x[1]; x[2] = v.x[2]; return *this; }
	vector&	operator+=(const vector& v);
	vector& operator-=(const vector& v);
	vector&	operator*=(float f);
	vector&	operator/=(float f) { return this->operator*=(1.0 / f); }

	float	magnitude() const;
	float	sqrmag() const;
//	float	min() const;
//	float	max() const;
//	float	minabs() const;
//	float	maxabs() const;

	bool	checknan() const;	// Returns true if any component is nan.
private:
	float	x[3];
};


#define INLINE_VECTOR


#ifdef INLINE_VECTOR


inline float	vector::operator*(const vector& v) const
// Dot product.
{
	float	result;
	result = x[0] * v.x[0];
	result += x[1] * v.x[1];
	result += x[2] * v.x[2];
	return result;
}


inline vector&	vector::operator+=(const vector& v)
// Adds a vector to *this.
{
	x[0] += v.x[0];
	x[1] += v.x[1];
	x[2] += v.x[2];
	return *this;
}


inline vector&	vector::operator-=(const vector& v)
// Subtracts a vector from *this.
{
	x[0] -= v.x[0];
	x[1] -= v.x[1];
	x[2] -= v.x[2];
	return *this;
}


#endif // INLINE_VECTOR




extern vector	ZeroVector, XAxis, YAxis, ZAxis;


class	quaternion;


class	matrix
// 3x4 matrix class, for 3D transformations.
{
public:
	matrix() { Identity(); }

	void	Identity();
	void	View(const vector& ViewNormal, const vector& ViewUp, const vector& ViewLocation);
	void	Orient(const vector& ObjectDirection, const vector& ObjectUp, const vector& ObjectLocation);

	static void	Compose(matrix* dest, const matrix& left, const matrix& right);
	vector	operator*(const vector& v) const;
	matrix	operator*(const matrix& m) const;
//	operator*=(const quaternion& q);

	matrix&	operator*=(float f);
	matrix&	operator+=(const matrix& m);
	
	void	Invert();
	void	InvertRotation();
	void	NormalizeRotation();
	void	Apply(vector* result, const vector& v) const;
	void	ApplyRotation(vector* result, const vector& v) const;
	void	ApplyInverse(vector* result, const vector& v) const;
	void	ApplyInverseRotation(vector* result, const vector& v) const;
	void	Translate(const vector& v);
	void	SetOrientation(const quaternion& q);
	quaternion	GetOrientation() const;
	
	void	SetColumn(int column, const vector& v) { m[column] = v; }
	const vector&	GetColumn(int column) const { return m[column]; }
private:
	vector	m[4];
};


// class quaternion -- handy for representing rotations.

class quaternion {
public:
	quaternion() : S(1), V(ZeroVector) {}
	quaternion(const quaternion& q) : S(q.S), V(q.V) {}
	quaternion(float s, const vector& v) : S(s), V(v) {}

	quaternion(const vector& Axis, float Angle);	// Slightly dubious: semantics varies from other constructor depending on order of arg types.

	float	GetS() const { return S; }
	const vector&	GetV() const { return V; }
	void	SetS(float s) { S = s; }
	void	SetV(const vector& v) { V = v; }

	float	Get(int i) const { if (i==0) return GetS(); else return V.Get(i-1); }
	void	Set(int i, float f) { if (i==0) S = f; else V.Set(i-1, f); }

	quaternion	operator*(const quaternion& q) const;
	quaternion&	operator*=(float f) { S *= f; V *= f; return *this; }
	quaternion&	operator+=(const quaternion& q) { S += q.S; V += q.V; return *this; }

	quaternion&	operator=(const quaternion& q) { S = q.S; V = q.V; return *this; }
	quaternion&	normalize();
	quaternion&	operator*=(const quaternion& q);
	void	ApplyRotation(vector* result, const vector& v);
	
	quaternion	lerp(const quaternion& q, float f) const;
private:
	float	S;
	vector	V;
};


namespace Geometry {
	vector	Rotate(float Angle, const vector& Axis, const vector& Point);
};



#endif // GEOMETRY_HPP
