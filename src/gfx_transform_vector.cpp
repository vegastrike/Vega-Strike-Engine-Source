/* © 1998 Chris Fry & Daniel Horn*/

#include <math.h>
#include "gfx_transform_vector.h"
//#include "glob_externs.h"
#define _CZ 761.465325527
//extern Vector	_LightVector;
//extern float _LVRed;
//extern float _LVGreen;
//extern float _LVBlue;
//extern Vector _CamCoord;
//extern Vector _CamP;
//extern Vector _CamQ;
//extern Vector _CamR;


//extern float _CamTransConst;


/*FLOATING POINT ASSEMBLY NOTES*
 *FLDCW load control word
 */
float DotProduct(const Vector &a,const Vector &b)
{
	return (a.i*b.i+a.j*b.j+a.k*b.k);
}

void CrossProduct(const Vector &a,const Vector &b, Vector &RES) 
{
		RES.i = a.j*b.k-a.k*b.j; 
    	RES.j = a.k*b.i-a.i*b.k;
    	RES.k = a.i*b.j-a.j*b.i;
}

void ScaledCrossProduct(const Vector &a, const Vector &b, Vector &r) 
{
	r.i = a.j*b.k-a.k*b.j; 
    	r.j = a.k*b.i-a.i*b.k;
    	r.k = a.i*b.j-a.j*b.i;
	float size = (float)sqrt(r.i*r.i+r.j*r.j+r.k*r.k);
	r.i /= size;
	r.j /= size;
	r.k /= size;
}

void Normalize(Vector &r)
{
	float size = (float)sqrt(DotProduct (r,r));
	r.i /= size;
	r.j /= size;
	r.k /= size;
}


/////////////////////////////////////////////////////////////
//   Yaws a unit vector
/////////////////////////////////////////////////////////////

void Vector::Yaw(float rad) //only works with unit vector
{
	float theta;
	
	if (i>0)
		theta = (float)atan(k/i);
	else if (i<0)
		theta = PI+(float)atan(k/i);
	else if (k<=0 && i==0)
		theta = -PI/2;
	else if (k>0 && i==0)
		theta = PI/2;

	theta += rad;
	i = (float)cos(theta);
	k = (float)sin(theta); 
}

void Vector::Roll(float rad)
{
	float theta;
	
	if (i>0)
		theta = (float)atan(j/i);
	else if (i<0)
		theta = PI+(float)atan(j/i);
	else if (j<=0 && i==0)
		theta = -PI/2;
	else if (j>0 && i==0)
		theta = PI/2;
	
	theta += rad; 
	i = (float)cos(theta);
	j = (float)sin(theta); 
}

void Vector::Pitch(float rad)
{
	float theta;
	
	if (k>0)
		theta = (float)atan(j/k);
	else if (k<0)
		theta = PI+(float)atan(j/k);
	else if (j<=0 && k==0)
		theta = -PI/2;
	else if (j>0 && k==0)
		theta = PI/2;
	
	theta += rad;
	k = (float)cos(theta);
	j = (float)sin(theta);
}

void Yaw (float rad, Vector &p,Vector &q, Vector &r)
{
	Vector temp1, temp2, temp3;
	temp1.i=1;
	temp1.j =0;
	temp1.k =0;
	temp1.Yaw (rad);
	temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp2.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	temp1.i= 0;
	temp1.j =0;
	temp1.k =1;
	temp1.Yaw (rad);
	temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp3.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	p = temp2;
	r = temp3;
}

void Pitch (float rad,Vector &p, Vector &q, Vector &r)
{
	Vector temp1, temp2, temp3;
	temp1.i=0;
	temp1.j =1;
	temp1.k =0;
	temp1.Pitch (rad);
	temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp2.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	temp1.i= 0;
	temp1.j =0;
	temp1.k =1;
	temp1.Pitch (rad);
	temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp3.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	q = temp2;
	r = temp3;
}
void Roll (float rad,Vector &p, Vector &q, Vector &r)
{
	Vector temp1, temp2, temp3;
	temp1.i=1;
	temp1.j =0;
	temp1.k =0;
	temp1.Roll (rad);
	temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp2.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	temp1.i= 0;
	temp1.j =1;
	temp1.k =0;
	temp1.Roll (rad);
	temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp3.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	p = temp2;
	q = temp3;
}
void ResetVectors (Vector &p, Vector &q, Vector &r)
{
	p.i = q.j = r.k = 1;
	p.j = p.k= q.i = q.k = r.i = r.j = 0;
}

void Orthogonize(Vector &p, Vector &q, Vector &r)
{
	Normalize(r);
	ScaledCrossProduct (r,p,q); //result of scaled cross put into q
	ScaledCrossProduct (q,r,p); //result of scaled cross put back into p
}
