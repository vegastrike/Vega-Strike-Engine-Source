// Vega Strike
// Copyright 2001-2002 Daniel Horn
// geometry.cpp	-thatcher 1/28/1998 Copyright Thatcher Ulrich

// Implementations for the basic geometric types.

// This code may be freely modified and redistributed.  I make no
// warrantees about it; use at your own risk.  If you do incorporate
// this code into a project, I'd appreciate a mention in the credits.
//
// Thatcher Ulrich <tu@tulrich.com>


#include <math.h>
#include <float.h>
#include "quadgeom.hpp"
#include "vegastrike.h"

vector	vector::operator+(const vector& v) const
// Adds two vectors.  Creates a temporary for the return value.
{
	vector	result;
	result.x[0] = x[0] + v.x[0];
	result.x[1] = x[1] + v.x[1];
	result.x[2] = x[2] + v.x[2];
	return result;
}


vector	vector::operator-(const vector& v) const
// Subtracts two vectors.  Creates a temporary for the return value.
{
	vector	result;
	result.x[0] = x[0] - v.x[0];
	result.x[1] = x[1] - v.x[1];
	result.x[2] = x[2] - v.x[2];
	return result;
}


vector	vector::operator-() const
// Returns the negative of *this.  Creates a temporary for the return value.
{
	vector	result;
	result.x[0] = -x[0];
	result.x[1] = -x[1];
	result.x[2] = -x[2];
	return result;
}


#ifndef INLINE_VECTOR


float	vector::operator*(const vector& v) const
// Dot product.
{
	float	result;
	result = x[0] * v.x[0];
	result += x[1] * v.x[1];
	result += x[2] * v.x[2];
	return result;
}


vector&	vector::operator+=(const vector& v)
// Adds a vector to *this.
{
	x[0] += v.x[0];
	x[1] += v.x[1];
	x[2] += v.x[2];
	return *this;
}


vector&	vector::operator-=(const vector& v)
// Subtracts a vector from *this.
{
	x[0] -= v.x[0];
	x[1] -= v.x[1];
	x[2] -= v.x[2];
	return *this;
}


#endif // INLINE_VECTOR


vector	vector::operator*(float f) const
// Scale a vector by a scalar.  Creates a temporary for the return value.
{
	vector	result;
	result.x[0] = x[0] * f;
	result.x[1] = x[1] * f;
	result.x[2] = x[2] * f;
	return result;
}


vector	vector::cross(const vector& v) const
// Cross product.  Creates a temporary for the return value.
{
	vector	result;
	result.x[0] = x[1] * v.x[2] - x[2] * v.x[1];
	result.x[1] = x[2] * v.x[0] - x[0] * v.x[2];
	result.x[2] = x[0] * v.x[1] - x[1] * v.x[0];
	return result;
}


vector&	vector::normalize()
// Scales the vector to unit length.  Preserves its direction.
{
	float	f = magnitude();
	if (f < 0.0000001) {
		// Punt.
		x[0] = 1;
		x[1] = 0;
		x[2] = 0;
	} else {
		this->operator/=(f);
	}
	return *this;
}


vector&	vector::operator*=(float f)
// Scales *this by the given scalar.
{
	x[0] *= f;
	x[1] *= f;
	x[2] *= f;
	return *this;
}


float	vector::magnitude() const
// Returns the length of *this.
{
	return sqrt(sqrmag());
}


float	vector::sqrmag() const
// Returns the square of the length of *this.
{
	return x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
}


bool	vector::checknan() const
// Returns true if any component is nan.
{
	if (fabs(x[0]) > 10000000 || fabs(x[1]) > 10000000 || fabs(x[2]) > 10000000) {
		return true;//xxxxxxx
	}
	if (!FINITE(x[0]) || !FINITE(x[1]) || !FINITE(x[2])) {
		return true;
	}
	else return false;
}


vector	ZeroVector(0, 0, 0);
vector	XAxis(1, 0, 0);
vector	YAxis(0, 1, 0);
vector	ZAxis(0, 0, 1);


// class matrix


void	matrix::Identity()
// Sets *this to be an identity matrix.
{
	SetColumn(0, XAxis);
	SetColumn(1, YAxis);
	SetColumn(2, ZAxis);
	SetColumn(3, ZeroVector);
}


void	matrix::View(const vector& ViewNormal, const vector& ViewUp, const vector& ViewLocation)
// Turns *this into a view matrix, given the direction the camera is
// looking (ViewNormal) and the camera's up vector (ViewUp), and its
// location (ViewLocation) (all vectors in world-coordinates).  The
// resulting matrix will transform points from world coordinates to view
// coordinates, which is a right-handed system with the x axis pointing
// left, the y axis pointing up, and the z axis pointing into the scene.
{
	vector	ViewX = ViewUp.cross(ViewNormal);

	// Construct the view-to-world matrix.
	Orient(ViewX, ViewUp, ViewLocation);

	// Turn it around, to make it world-to-view.
	Invert();
}


void	matrix::Orient(const vector& Direction, const vector& Up, const vector& Location)
// Turns *this into a transformation matrix, that transforms vectors
// from object coordinates to world coordinates, given the object's Direction, Up,
// and Location in world coordinates.
{
	vector	ZAxis = Direction.cross(Up);

	SetColumn(0, Direction);
	SetColumn(1, Up);
	SetColumn(2, ZAxis);
	SetColumn(3, Location);
}


vector	matrix::operator*(const vector& v) const
// Applies *this to the given vector, and returns the transformed vector.
{
	vector	result;
	Apply(&result, v);
	return result;
}


matrix	matrix::operator*(const matrix& a) const
// Composes the two matrices, returns the product.  Creates a temporary
// for the return value.
{
	matrix result;

	Compose(&result, *this, a);

	return result;
}


void	matrix::Compose(matrix* dest, const matrix& left, const matrix& right)
// Multiplies left * right, and puts the result in *dest.
{
	left.ApplyRotation(&const_cast<vector&>(dest->GetColumn(0)), right.GetColumn(0));
	left.ApplyRotation(&const_cast<vector&>(dest->GetColumn(1)), right.GetColumn(1));
	left.ApplyRotation(&const_cast<vector&>(dest->GetColumn(2)), right.GetColumn(2));
	left.Apply(&const_cast<vector&>(dest->GetColumn(3)), right.GetColumn(3));
}


matrix&	matrix::operator*=(float f)
// Scalar multiply of a matrix.
{
	int	i;
	for (i = 0; i < 4; i++) m[i] *= f;
	return *this;
}


matrix&	matrix::operator+=(const matrix& mat)
// Memberwise matrix addition.
{
	int	i;
	for (i = 0; i < 4; i++) m[i] += mat.m[i];
	return *this;
}


void	matrix::Invert()
// Inverts *this.  Uses transpose property of rotation matrices.
{
	InvertRotation();

	// Compute the translation part of the inverted matrix, by applying
	// the inverse rotation to the original translation.
	vector	TransPrime;
	ApplyRotation(&TransPrime, GetColumn(3));

	SetColumn(3, -TransPrime);	// Could optimize the negation by doing it in-place.
}


void	matrix::InvertRotation()
// Inverts the rotation part of *this.  Ignores the translation.
// Uses the transpose property of rotation matrices.
{
	float	f;

	// Swap elements across the diagonal.
	f = m[1].Get(0);
	m[1].Set(0, m[0].Get(1));
	m[0].Set(1, f);

	f = m[2].Get(0);
	m[2].Set(0, m[0].Get(2));
	m[0].Set(2, f);

	f = m[2].Get(1);
	m[2].Set(1, m[1].Get(2));
	m[1].Set(2, f);
}


void	matrix::NormalizeRotation()
// Normalizes the rotation part of the matrix.
{
	m[0].normalize();
	m[1] = m[2].cross(m[0]);
	m[1].normalize();
	m[2] = m[0].cross(m[1]);
}


void	matrix::Apply(vector* result, const vector& v) const
// Applies v to *this, and puts the transformed result in *result.
{
	// Do the rotation.
	ApplyRotation(result, v);
	// Do the translation.
	*result += m[3];
}


void	matrix::ApplyRotation(vector* result, const vector& v) const
// Applies the rotation portion of *this, and puts the transformed result in *result.
{
	result->Set(0, m[0].Get(0) * v.Get(0) + m[1].Get(0) * v.Get(1) + m[2].Get(0) * v.Get(2));
	result->Set(1, m[0].Get(1) * v.Get(0) + m[1].Get(1) * v.Get(1) + m[2].Get(1) * v.Get(2));
	result->Set(2, m[0].Get(2) * v.Get(0) + m[1].Get(2) * v.Get(1) + m[2].Get(2) * v.Get(2));
}


void	matrix::ApplyInverse(vector* result, const vector& v) const
// Applies v to the inverse of *this, and puts the transformed result in *result.
{
	ApplyInverseRotation(result, v - m[3]);
}


void	matrix::ApplyInverseRotation(vector* result, const vector& v) const
// Applies v to the inverse rotation part of *this, and puts the result in *result.
{
	result->Set(0, m[0] * v);
	result->Set(1, m[1] * v);
	result->Set(2, m[2] * v);
}


void	matrix::Translate(const vector& v)
// Composes a translation on the right of *this.
{
	vector	newtrans;
	Apply(&newtrans, v);
	SetColumn(3, newtrans);
}


void	matrix::SetOrientation(const quaternion& q)
// Sets the rotation part of the matrix to the values which correspond to the given
// quaternion orientation.
{
	float	S = q.GetS();
	const vector&	V = q.GetV();
	
	m[0].Set(0, 1 - 2 * V.Y() * V.Y() - 2 * V.Z() * V.Z());
	m[0].Set(1, 2 * V.X() * V.Y() + 2 * S * V.Z());
	m[0].Set(2, 2 * V.X() * V.Z() - 2 * S * V.Y());

	m[1].Set(0, 2 * V.X() * V.Y() - 2 * S * V.Z());
	m[1].Set(1, 1 - 2 * V.X() * V.X() - 2 * V.Z() * V.Z());
	m[1].Set(2, 2 * V.Y() * V.Z() + 2 * S * V.X());

	m[2].Set(0, 2 * V.X() * V.Z() + 2 * S * V.Y());
	m[2].Set(1, 2 * V.Y() * V.Z() - 2 * S * V.X());
	m[2].Set(2, 1 - 2 * V.X() * V.X() - 2 * V.Y() * V.Y());
}


quaternion	matrix::GetOrientation() const
// Converts the rotation part of *this into a quaternion, and returns it.
{
	// Code adapted from Baraff, "Rigid Body Simulation", from SIGGRAPH 95 course notes for Physically Based Modeling.
	quaternion	q;
	float	tr, s;

	tr = m[0].Get(0) + m[1].Get(1) + m[2].Get(2);	// trace

	if (tr >= 0) {
		s = sqrt(tr + 1);
		q.SetS(0.5 * s);
		s = 0.5 / s;
		q.SetV(vector(m[1].Get(2) - m[2].Get(1), m[2].Get(0) - m[0].Get(2), m[0].Get(1) - m[1].Get(0)) * s);
	} else {
		int	i = 0;

		if (m[1].Get(1) > m[0].Get(0)) {
			i = 1;
		}
		if (m[2].Get(2) > m[i].Get(i)) {
			i = 2;
		}

		float	qr=0.f, qi=0.f, qj=0.f, qk=0.f;
		switch (i) {
		case 0:
			s = sqrt((m[0].Get(0) - (m[1].Get(1) + m[2].Get(2))) + 1);
			qi = 0.5 * s;
			s = 0.5 / s;
			qj = (m[1].Get(0) + m[0].Get(1)) * s;
			qk = (m[0].Get(2) + m[2].Get(0)) * s;
			qr = (m[1].Get(2) - m[2].Get(1)) * s;
			break;

		case 1:
			s = sqrt((m[1].Get(1) - (m[2].Get(2) + m[0].Get(0))) + 1);
			qj = 0.5 * s;
			s = 0.5 / s;
			qk = (m[2].Get(1) + m[1].Get(2)) * s;
			qi = (m[1].Get(0) + m[0].Get(1)) * s;
			qr = (m[2].Get(0) - m[0].Get(2)) * s;
			break;

		case 2:
			s = sqrt((m[2].Get(2) - (m[0].Get(0) + m[1].Get(1))) + 1);
			qk = 0.5 * s;
			s = 0.5 / s;
			qi = (m[0].Get(2) + m[2].Get(0)) * s;
			qj = (m[2].Get(1) + m[1].Get(2)) * s;
			qr = (m[0].Get(1) - m[1].Get(0)) * s;
			break;
		}
		q.SetS(qr);
		q.SetV(vector(qi, qj, qk));
	}

	return q;
}


//
// class quaternion
//


quaternion::quaternion(const vector& Axis, float Angle)
// Constructs the quaternion defined by the rotation about the given axis of the given angle (in radians).
{
	S = cos(Angle / 2);
	V = Axis;
	V *= sin(Angle / 2);
}


quaternion	quaternion::operator*(const quaternion& q) const
// Multiplication of two quaternions.  Returns a new quaternion
// result.
{
	return quaternion(S * q.S - V * q.V, q.V * S + V * q.S + V.cross(q.V));
}


quaternion&	quaternion::normalize()
// Ensures that the quaternion has magnitude 1.
{
	float	mag = sqrt(S * S + V * V);
	if (mag > 0.0000001) {
		float	inv = 1.0 / mag;
		S *= inv;
		V *= inv;
	} else {
		// Quaternion is messed up.  Turn it into a null rotation.
		S = 1;
		V = ZeroVector;
	}

	return *this;
}


quaternion&	quaternion::operator*=(const quaternion& q)
// In-place quaternion multiplication.
{
	*this = *this * q;	// I don't think we can avoid making temporaries.

	return *this;
}


void	quaternion::ApplyRotation(vector* result, const vector& v)
// Rotates the given vector v by the rotation represented by this quaternion.
// Stores the result in the given result vector.
{
	quaternion	q(*this * quaternion(0, v) * quaternion(S, -V));	// There's definitely a shortcut here.  Deal with later.

	*result = q.V;
}


quaternion	quaternion::lerp(const quaternion& q, float f) const
// Does a spherical linear interpolation between *this and q, using f as
// the blend factor.  f == 0 --> result == *this, f == 1 --> result == q.
{
	quaternion	result;

	float	f0, f1;

	float	cos_omega = V * q.V + S * q.S;
	quaternion	qtemp(q);

	// Adjust signs if necessary.
	if (cos_omega < 0) {
		cos_omega = -cos_omega;
		qtemp.V = -qtemp.V;
		qtemp.S = -qtemp.S;
	}

	if (cos_omega < 0.99) {
		// Do the spherical interp.
		float	omega = acos(cos_omega);
		float	sin_omega = sin(omega);
		f0 = sin((1 - f) * omega) / sin_omega;
		f1 = sin(f * omega) / sin_omega;
	} else {
		// Quaternions are close; just do straight lerp and avoid division by near-zero.
		f0 = 1 - f;
		f1 = f;
	}
	
	result.S = S * f0 + qtemp.S * f1;
	result.V = V * f0 + qtemp.V * f1;
	result.normalize();

	return result;
}



#ifdef NOT
QuatSlerp(QUAT * from, QUAT * to, float t, QUAT * res)
      {
        float           to1[4];
        double        omega, cosom, sinom, scale0, scale1;

        // calc cosine
        cosom = from->x * to->x + from->y * to->y + from->z * to->z
			       + from->w * to->w;

        // adjust signs (if necessary)
        if ( cosom <0.0 ){ cosom = -cosom; to1[0] = - to->x;
		to1[1] = - to->y;
		to1[2] = - to->z;
		to1[3] = - to->w;
        } else  {
		to1[0] = to->x;
		to1[1] = to->y;
		to1[2] = to->z;
		to1[3] = to->w;
        }

        // calculate coefficients

       if ( (1.0 - cosom) > DELTA ) {
                // standard case (slerp)
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = sin((1.0 - t) * omega) / sinom;
                scale1 = sin(t * omega) / sinom;

        } else {        
    // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
                scale0 = 1.0 - t;
                scale1 = t;
        }
	// calculate final values
	res->x = scale0 * from->x + scale1 * to1[0];
	res->y = scale0 * from->y + scale1 * to1[1];
	res->z = scale0 * from->z + scale1 * to1[2];
	res->w = scale0 * from->w + scale1 * to1[3];
}
 
 
#endif // NOT


#ifdef NOT
void	bitmap32::ProcessForColorKeyZero()
// Examine alpha channel, and set pixels that have alpha==0 to color 0.
// Pixels that have alpha > 0.5, but have color 0, should be tweaked slightly
// so they don't get knocked out when blitting.
{
	uint32	Key32 = 0;
	uint32*	p = GetData();
	int	pixels = GetHeight() * GetWidth();
	for (int i = 0; i < pixels; i++, p++) {
		if ((*p >> 24) >= 128) {
			// Alpha >= 0.5.  Make sure color isn't equal to color key.
			if ((*p & 0x00FFFFFF) == Key32) {
				*p ^= 8;	// Twiddle a low blue bit.
			}
		} else {
			// Set pixel's color equal to color key.
			*p = (*p & 0xFF000000) | Key32;
		}
	}		
}
#endif // NOT


namespace Geometry {
;


vector	Rotate(float Angle, const vector& Axis, const vector& Point)
// Rotates the given point through the given angle (in radians) about the given
// axis.
{
	quaternion	q(cos(Angle/2), Axis * sin(Angle/2));

	vector	result;
	q.ApplyRotation(&result, Point);

	return result;
}


};
