/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel Horn
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
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "vegastrike.h"
#include "alglib.h"
#include "gauss.h"

vector_t make_vector( scalar_t x, scalar_t y, scalar_t z )
{
    vector_t result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

point_t make_point( scalar_t x, scalar_t y, scalar_t z )
{
    point_t result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

point2d_t make_point2d( scalar_t x, scalar_t y )
{
    point2d_t result;
    result.x = x;
    result.y = y;
    return result;
}

index2d_t make_index2d( int i, int j )
{
    index2d_t result;
    result.i = i;
    result.j = j;
    return result;
}

scalar_t dot_product( vector_t v1, vector_t v2 )
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vector_t scale_vector( scalar_t s, vector_t v )
{
    vector_t rval;
    rval.x = s * v.x;
    rval.y = s * v.y;
    rval.z = s * v.z;
    return rval;
}

vector_t add_vectors( vector_t v1, vector_t v2 )
{
    vector_t result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

point_t move_point( point_t p1, vector_t v2 )
{
    point_t result;
    result.x = p1.x + v2.x;
    result.y = p1.y + v2.y;
    result.z = p1.z + v2.z;
    return result;
}

vector_t subtract_points( point_t p1, point_t p2 )
{
    vector_t result;
    result.x = p1.x - p2.x;
    result.y = p1.y - p2.y;
    result.z = p1.z - p2.z;
    return result;
}

vector_t subtract_vectors( vector_t v1, vector_t v2 )
{
    vector_t result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

point_t point_minus_vector( point_t p1, vector_t v2 )
{
    point_t result;
    result.x = p1.x - v2.x;
    result.y = p1.y - v2.y;
    result.z = p1.z - v2.z;
    return result;

}

/* 
 * returns the component of v in the plane with normal nml. 
 * nml should be normalized.
 */
vector_t project_into_plane( vector_t nml, vector_t v )
{
    vector_t nmlComp;
    scalar_t dotProd;

    dotProd = dot_product( nml, v );
    nmlComp = scale_vector( dotProd, nml );

    return subtract_vectors( v, nmlComp );
} 

scalar_t normalize_vector( vector_t *v )
{
    scalar_t denom = 1.;
    scalar_t x = ( v->x > 0.0 ) ? v->x : - v->x;
    scalar_t y = ( v->y > 0.0 ) ? v->y : - v->y;
    scalar_t z = ( v->z > 0.0 ) ? v->z : - v->z;

    if( x > y ) {
        if( x > z ) {
            if( 1.0+x > 1.0 ) {
                y = y/x;
                z = z/x;
                denom = 1.0/(x*sqrt(1.0+y*y+z*z));
            }
        } else { 
            /* z > x > y */ 
            if( 1.0+z > 1.0 ) {
                y = y/z;
                x = x/z;
                denom = 1.0/(z*sqrt(1.0+y*y+x*x));
            }
        }
    } else {
        if( y > z ) {
            if( 1.0+y > 1.0 ) {
                z = z/y;
                x = x/y;
                denom = 1.0/(y*sqrt(1.0+z*z+x*x));
            }
        }
        else {
            /* x < y < z */
            if( 1.0+z > 1.0 ) {
                y = y/z;
                x = x/z;
                denom = 1.0/(z*sqrt(1.0+y*y+x*x));
            }
        }
    }

    if( 1.0 + x + y + z > 1.0 ) {
        *v = scale_vector( denom, *v );
        return 1.0/denom;
    } else {
        return 0.0;
    } 
}

vector_t cross_product(vector_t u, vector_t v)
{
    vector_t ret;
    ret.x = u.y * v.z - u.z * v.y;
    ret.y = u.z * v.x - u.x * v.z;
    ret.z = u.x * v.y - u.y * v.x;
    return ret;
}

vector_t transform_vector(matrixgl_t mat, vector_t v)
{
    vector_t r;
    r.x = v.x * mat[0][0] + v.y * mat[1][0] + v.z * mat[2][0];
    r.y = v.x * mat[0][1] + v.y * mat[1][1] + v.z * mat[2][1];
    r.z = v.x * mat[0][2] + v.y * mat[1][2] + v.z * mat[2][2];
    return r;
}

vector_t transform_normal(vector_t n, matrixgl_t mat)
{
    vector_t r;
    r.x = n.x * mat[0][0] + n.y * mat[0][1] + n.z * mat[0][2];
    r.y = n.x * mat[1][0] + n.y * mat[1][1] + n.z * mat[1][2];
    r.z = n.x * mat[2][0] + n.y * mat[2][1] + n.z * mat[2][2];
    return r;
}

point_t transform_point(matrixgl_t mat, point_t p)
{
    point_t r;
    r.x = p.x * mat[0][0] + p.y * mat[1][0] + p.z * mat[2][0];
    r.y = p.x * mat[0][1] + p.y * mat[1][1] + p.z * mat[2][1];
    r.z = p.x * mat[0][2] + p.y * mat[1][2] + p.z * mat[2][2];
    r.x += mat[3][0];
    r.y += mat[3][1];
    r.z += mat[3][2];
    return r;
}

plane_t make_plane( scalar_t nx, scalar_t ny, scalar_t nz, scalar_t d )
{
    plane_t tmp;
    tmp.nml.x = nx;
    tmp.nml.y = ny;
    tmp.nml.z = nz;
    tmp.d = d;

    return tmp;
}

bool_t intersect_planes( plane_t s1, plane_t s2, plane_t s3, point_t *p )
{
    double A[3][4];
    double x[3];
    scalar_t retval;

    A[0][0] =  s1.nml.x;
    A[0][1] =  s1.nml.y;
    A[0][2] =  s1.nml.z;
    A[0][3] = -s1.d;

    A[1][0] =  s2.nml.x;
    A[1][1] =  s2.nml.y;
    A[1][2] =  s2.nml.z;
    A[1][3] = -s2.d;

    A[2][0] =  s3.nml.x;
    A[2][1] =  s3.nml.y;
    A[2][2] =  s3.nml.z;
    A[2][3] = -s3.d;

    retval = gauss( (double*) A, 3, x);

    if ( retval != 0 ) {
	/* Matrix is singular */
	return False;
    } else {
	/* Solution found */
	p->x = x[0];
	p->y = x[1];
	p->z = x[2];
	return True;
    }
}

scalar_t distance_to_plane( plane_t plane, point_t pt ) 
{
    return 
	plane.nml.x * pt.x +
	plane.nml.y * pt.y +
	plane.nml.z * pt.z +
	plane.d;
}

void make_identity_matrix(matrixgl_t h)
{
    int i,j;
  
    for( i= 0 ; i< 4 ; i++ )
	for( j= 0 ; j< 4 ; j++ )
	    h[i][j]= (i==j);
}

void multiply_matrices(matrixgl_t ret, matrixgl_t mat1, matrixgl_t mat2)
{
    int i,j;
    matrixgl_t r;

    for( i= 0 ; i< 4 ; i++ )
	for( j= 0 ; j< 4 ; j++ )
	    r[j][i]= mat1[0][i] * mat2[j][0] +
		mat1[1][i] * mat2[j][1] +
		mat1[2][i] * mat2[j][2] +
		mat1[3][i] * mat2[j][3];

    for( i= 0 ; i< 4 ; i++ )
	for( j= 0 ; j< 4 ; j++ )
	    ret[i][j] = r[i][j];
}

void transpose_matrix( matrixgl_t mat, matrixgl_t trans )
{
    int i,j;
    matrixgl_t r;

    for( i= 0 ; i< 4 ; i++ )
	for( j= 0 ; j< 4 ; j++ )
	    r[j][i]= mat[i][j];

    for( i= 0 ; i< 4 ; i++ )
	for( j= 0 ; j< 4 ; j++ )
	    trans[i][j] = r[i][j];
}

void make_rotation_matrix( matrixgl_t mat, scalar_t angle, char axis )
{
    scalar_t sinv, cosv;
    sinv = sin( ANGLES_TO_RADIANS( angle ) );
    cosv = cos( ANGLES_TO_RADIANS( angle ) );

    make_identity_matrix( mat );

    switch( axis ) {
    case 'x':
        mat[1][1] = cosv;
        mat[2][1] = -sinv;
        mat[1][2] = sinv;
        mat[2][2] = cosv;
        break;

    case 'y':
        mat[0][0] = cosv;
        mat[2][0] = sinv;
        mat[0][2] = -sinv;
        mat[2][2] = cosv;
        break;

    case 'z': 
        mat[0][0] = cosv;
        mat[1][0] = -sinv;
        mat[0][1] = sinv;
        mat[1][1] = cosv;
        break;

    default:
        code_not_reached();  /* shouldn't get here */

    }
} 

void make_translation_matrix( matrixgl_t mat, 
			      scalar_t x, scalar_t y, scalar_t z )
{
    make_identity_matrix( mat );
    mat[3][0] = x;
    mat[3][1] = y;
    mat[3][2] = z;
} 

void make_scaling_matrix( matrixgl_t mat, 
			  scalar_t x, scalar_t y, scalar_t z )
{
    make_identity_matrix( mat );
    mat[0][0] = x;
    mat[1][1] = y;
    mat[2][2] = z;
} 

/* transforms from (w1, w2, w3) orthonormal basis to (i,j,k) orthonormal basis
 */
void make_change_of_basis_matrix( matrixgl_t mat, matrixgl_t invMat, 
				  vector_t w1, vector_t w2, vector_t w3 )
{
    make_identity_matrix( mat );
    mat[0][0] = w1.x;
    mat[0][1] = w1.y;
    mat[0][2] = w1.z;
    mat[1][0] = w2.x;
    mat[1][1] = w2.y;
    mat[1][2] = w2.z;
    mat[2][0] = w3.x;
    mat[2][1] = w3.y;
    mat[2][2] = w3.z;

    make_identity_matrix( invMat );
    invMat[0][0] = w1.x;
    invMat[1][0] = w1.y;
    invMat[2][0] = w1.z;
    invMat[0][1] = w2.x;
    invMat[1][1] = w2.y;
    invMat[2][1] = w2.z;
    invMat[0][2] = w3.x;
    invMat[1][2] = w3.y;
    invMat[2][2] = w3.z;
} 

void make_rotation_about_vector_matrix( matrixgl_t mat, 
					vector_t u, scalar_t angle )
{
    matrixgl_t rx, irx, ry, iry;
    scalar_t a, b, c, d;

    a = u.x;
    b = u.y;
    c = u.z;

    d = sqrt( b*b + c*c );

    if ( d < EPS ) {
        if ( a < 0 ) 
            make_rotation_matrix( mat, -angle, 'x' );
        else
            make_rotation_matrix( mat, angle, 'x' );
        return;
    } 

    make_identity_matrix( rx );
    make_identity_matrix( irx );
    make_identity_matrix( ry );
    make_identity_matrix( iry );

    rx[1][1] = c/d;
    rx[2][1] = -b/d;
    rx[1][2] = b/d;
    rx[2][2] = c/d;

    irx[1][1] = c/d;
    irx[2][1] = b/d;
    irx[1][2] = -b/d;
    irx[2][2] = c/d;

    ry[0][0] = d;
    ry[2][0] = -a;
    ry[0][2] = a;
    ry[2][2] = d;

    iry[0][0] = d;
    iry[2][0] = a;
    iry[0][2] = -a;
    iry[2][2] = d;

    make_rotation_matrix( mat, angle, 'z' );

    multiply_matrices( mat, mat, ry );
    multiply_matrices( mat, mat, rx );
    multiply_matrices( mat, iry, mat );
    multiply_matrices( mat, irx, mat );
} 

/*
 * Quaternion operations
 */
quaternion_t make_quaternion( scalar_t x, scalar_t y, scalar_t z, scalar_t w )
{
    quaternion_t q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

quaternion_t multiply_quaternions( quaternion_t q, quaternion_t r )
{
    quaternion_t res; /* the result */
    res.x = q.y * r.z - q.z * r.y + r.w * q.x + q.w * r.x;
    res.y = q.z * r.x - q.x * r.z + r.w * q.y + q.w * r.y;
    res.z = q.x * r.y - q.y * r.x + r.w * q.z + q.w * r.z;
    res.w = q.w * r.w - q.x * r.x - q.y * r.y - q.z * r.z;
    return res;
}

quaternion_t add_quaternions( quaternion_t q, quaternion_t r )
{
    quaternion_t res; /* the result */
    res.x = q.x + r.x;
    res.y = q.y + r.y;
    res.z = q.z + r.z;
    res.w = q.w + r.w;
    return res;
}

/* Return conjugate of q.  If q is a unit quaternion, this is the
   multiplicative inverse of q. */
quaternion_t quaternion_conjugate( quaternion_t q )
{
    quaternion_t res; /* the result */
    res.x = -1 * q.x;
    res.y = -1 * q.y;
    res.z = -1 * q.z;
    res.w = q.w;

    return res;
}

quaternion_t scale_quaternion( scalar_t s, quaternion_t q )
{
    quaternion_t res; /* the result */
    res.x = s * q.x;
    res.y = s * q.y;
    res.z = s * q.z;
    res.w = s * q.w;

    return res;
}

/* 
 * Make the transformation matrix corresponding to the *unit* quaternion _q_
 */
void make_matrix_from_quaternion( matrixgl_t mat, quaternion_t q )
{
    mat[0][0] = 1.0 - 2.0 * ( q.y * q.y + q.z * q.z );
    mat[1][0] =       2.0 * ( q.x * q.y - q.w * q.z );
    mat[2][0] =       2.0 * ( q.x * q.z + q.w * q.y );

    mat[0][1] =       2.0 * ( q.x * q.y + q.w * q.z );
    mat[1][1] = 1.0 - 2.0 * ( q.x * q.x + q.z * q.z );
    mat[2][1] =       2.0 * ( q.y * q.z - q.w * q.x );

    mat[0][2] =       2.0 * ( q.x * q.z - q.w * q.y );
    mat[1][2] =       2.0 * ( q.y * q.z + q.w * q.x );
    mat[2][2] = 1.0 - 2.0 * ( q.x * q.x + q.y * q.y );

    mat[3][0] = mat[3][1] = mat[3][2] = 0.0;
    mat[0][3] = mat[1][3] = mat[2][3] = 0.0;
    mat[3][3] = 1.0;
}

/* 
 * Convert an orthogonal matrix to a quaternion
 * 
 * From Game Developer article by Nick Bobick, 
 * July 3, 1998 Vol. 2: Issue 26
 */
quaternion_t make_quaternion_from_matrix( matrixgl_t m )
{
    quaternion_t res;
    scalar_t  tr, s, q[4];
    int    i, j, k;

    static int nxt[3] = {1, 2, 0};

    tr = m[0][0] + m[1][1] + m[2][2];

    /* check the diagonal */
    if (tr > 0.0) {
	s = sqrt (tr + 1.0);
	res.w = 0.5 * s;
	s = 0.5 / s;
	res.x = (m[1][2] - m[2][1]) * s;
	res.y = (m[2][0] - m[0][2]) * s;
	res.z = (m[0][1] - m[1][0]) * s;
    } else {                
	/* diagonal is negative */
	i = 0;
	if (m[1][1] > m[0][0]) i = 1;
	if (m[2][2] > m[i][i]) i = 2;
	j = nxt[i];
	k = nxt[j];

	s = sqrt (m[i][i] - m[j][j] - m[k][k] + 1.0);
                       
	q[i] = s * 0.5;
                             
	if (s != 0.0) s = 0.5 / s;

	q[3] = (m[j][k] - m[k][j]) * s;
	q[j] = (m[i][j] + m[j][i]) * s;
	q[k] = (m[i][k] + m[k][i]) * s;

	res.x = q[0];
	res.y = q[1];
	res.z = q[2];
	res.w = q[3];
    }

    return res;
}

/*
 * Makes a quaternion that rotates s into t, where s and t are unit vectors
 */
quaternion_t make_rotation_quaternion( vector_t s, vector_t t )
{
    quaternion_t res;
    vector_t u;
    scalar_t cos2phi, sin2phi;
    scalar_t cosphi, sinphi;

    u = cross_product( s, t );
    sin2phi = normalize_vector( &u );

    if ( sin2phi < EPS ) {
	res = make_quaternion( 0., 0., 0., 1. );
    } else {
	cos2phi = dot_product( s, t );

	/* 
	 * calculate sinphi and cosphi using half-angle relations 
	 */
	sinphi = sqrt( ( 1 - cos2phi ) / 2.0 );
	cosphi = sqrt( ( 1 + cos2phi ) / 2.0 );

	res.x = sinphi * u.x;
	res.y = sinphi * u.y;
	res.z = sinphi * u.z;
	res.w = cosphi;
    }

    return res;
}

/*
 * Interpolates from q to r by factor t in [0,1].  
 * This is also known as slerp(q, r, t) (spherical linear interpolation)
 */
quaternion_t interpolate_quaternions( quaternion_t q, quaternion_t r, 
				      scalar_t t )
{
    quaternion_t res;
    scalar_t cosphi;
    scalar_t sinphi;
    scalar_t phi;
    scalar_t scale0, scale1;

    cosphi = q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w;

    /* adjust signs (if necessary) */
    if ( cosphi < 0.0 ) {
	cosphi = -cosphi;
	r.x = -r.x;
	r.y = -r.y;
	r.z = -r.z;
	r.w = -r.w;
    }

    if ( 1.0 - cosphi > EPS ) {
	/* standard case -- slerp */
	phi = acos( cosphi );
	sinphi = sin( phi );
	scale0 = sin( phi * ( 1.0 - t ) ) / sinphi;
	scale1 = sin( phi * t ) / sinphi;
    } else {
	/* use linear interpolation to avoid division by zero */
	scale0 = 1.0 - t;
	scale1 = t;
    }

    res.x = scale0 * q.x + scale1 * r.x; 
    res.y = scale0 * q.y + scale1 * r.y; 
    res.z = scale0 * q.z + scale1 * r.z; 
    res.w = scale0 * q.w + scale1 * r.w; 

    return res;
}

vector_t rotate_vector( quaternion_t q, vector_t v )
{
    vector_t res;
    quaternion_t res_q;
    quaternion_t qs;
    quaternion_t p;

    p.x = v.x;
    p.y = v.y;
    p.z = v.z;
    p.w = 1.0;

    qs.x = -q.x;
    qs.y = -q.y;
    qs.z = -q.z;
    qs.w =  q.w;

    res_q = multiply_quaternions( q, 
				  multiply_quaternions( p, qs ) );
    res.x = res_q.x;
    res.y = res_q.y;
    res.z = res_q.z;

    return res;
}

void cut_line( point_t p1, point_t p2, point2d_t t1, point2d_t t2, scalar_t ratio,
	       point_t *result, point2d_t *result_tex )
{
    *result = move_point( p1, scale_vector( 1 - ratio, subtract_points( p2, p1 ) ) );
    result_tex->x = (t1.x * ratio) + ((1 - ratio)*t2.x);
    result_tex->y = (t1.y * ratio) + ((1 - ratio)*t2.y);
}


int cut_triangle( triangle_t *in_tri, triangle_t *out_tri2, triangle_t *out_tri3, 
		  line_t cut )
{
    triangle_t out_tri1;
    int result;             /* number of triangles */
    scalar_t side0, side1, side2;
    scalar_t ratio01, ratio02, ratio12;

    side0 = dot_product( cut.nml, 
			 subtract_points( in_tri->p[0], cut.pt ) );
    side1 = dot_product( cut.nml, 
			 subtract_points( in_tri->p[1], cut.pt ) );
    side2 = dot_product( cut.nml, 
			 subtract_points( in_tri->p[2], cut.pt ) );
    ratio01 = fabs(side1/(side0-side1));
    ratio02 = fabs(side2/(side0-side2));
    ratio12 = fabs(side2/(side1-side2));


    out_tri1.p[0] = in_tri->p[0];out_tri1.t[0] = in_tri->t[0];
    if (side0 > 0) {
	if (side1 > 0) {
	    out_tri1.p[1] = in_tri->p[1]; out_tri1.t[1] = in_tri->t[1];
	    if (side2 >= 0) {
		out_tri1.p[2] = in_tri->p[2]; out_tri1.t[2] = in_tri->t[2];
		result = 1;
	    } else {
		cut_line(in_tri->p[1], in_tri->p[2], in_tri->t[1], in_tri->t[2], 
			 ratio12, &out_tri1.p[2], &out_tri1.t[2]);
                out_tri2->p[0] = out_tri1.p[2];out_tri2->t[0] = out_tri1.t[2];
		cut_line(in_tri->p[0], in_tri->p[2], in_tri->t[0], in_tri->t[2], 
			 ratio02, &out_tri2->p[1], &out_tri2->t[1]);
		out_tri2->p[2] = in_tri->p[0];out_tri2->t[2] = in_tri->t[0];
		out_tri3->p[0] = out_tri2->p[1];out_tri3->t[0] = out_tri2->t[1];
		out_tri3->p[1] = out_tri2->p[0];out_tri3->t[1] = out_tri2->t[0];
		out_tri3->p[2] = in_tri->p[2];out_tri3->t[2] = in_tri->t[2];
		result = 3;
	    }

	} else if (side1 == 0) {
	    out_tri1.p[1] = in_tri->p[1];out_tri1.t[1] = in_tri->t[1];
	    if (side2 >= 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		result = 1;
	    } else {
		cut_line(in_tri->p[0], in_tri->p[2], in_tri->t[0], in_tri->t[2], 
			 ratio02, &out_tri1.p[2], &out_tri1.t[2]);
		out_tri2->p[0] = out_tri1.p[2];out_tri2->t[0] = out_tri1.t[2];
		out_tri2->p[1] = in_tri->p[1];out_tri2->t[1] = in_tri->t[1];
		out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
		result = 2;
	    }

	} else { /* (side1 < 0) */
	    cut_line(in_tri->p[0], in_tri->p[1], in_tri->t[0], in_tri->t[1], 
		     ratio01, &out_tri1.p[1], &out_tri1.t[1]);
	    if (side2 > 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		out_tri2->p[0] = in_tri->p[2];out_tri2->t[0] = in_tri->t[2];
		out_tri2->p[1] = out_tri1.p[1];out_tri2->t[1] = out_tri1.t[1];
		cut_line(in_tri->p[1], in_tri->p[2], in_tri->t[1], in_tri->t[2], 
			 ratio12, &out_tri2->p[2], &out_tri2->t[2]);
		out_tri3->p[0] = out_tri2->p[1];out_tri3->t[0] = out_tri2->t[1];
		out_tri3->p[1] = in_tri->p[1];out_tri3->t[1] = in_tri->t[1];
		out_tri3->p[2] = out_tri2->p[2];out_tri3->t[2] = out_tri2->t[2];
		result = 3;
	    } else if (side2 == 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		out_tri2->p[0] = out_tri1.p[1];out_tri2->t[0] = out_tri1.t[1];
		out_tri2->p[1] = in_tri->p[1];out_tri2->t[1] = in_tri->t[1];
		out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
		result = 2;
	    } else {  /* (side2 < 0) */
		cut_line(in_tri->p[0], in_tri->p[2], in_tri->t[0], in_tri->t[2], 
			 ratio02, &out_tri1.p[2], &out_tri1.t[2]);
		out_tri2->p[0] = out_tri1.p[2];out_tri2->t[0] = out_tri1.t[2];
		out_tri2->p[1] = out_tri1.p[1];out_tri2->t[1] = out_tri1.t[1];
		out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
		out_tri3->p[0] = in_tri->p[1];out_tri3->t[0] = in_tri->t[1];
		out_tri3->p[1] = in_tri->p[2];out_tri3->t[1] = in_tri->t[2];
		out_tri3->p[2] = out_tri1.p[1];out_tri3->t[2] = out_tri1.t[1];
		result = 3;
	    }
	}

    } else if (side0 == 0) {
	out_tri1.p[1] = in_tri->p[1];out_tri1.t[1] = in_tri->t[1];
	if (((side1 > 0) && (side2 > 0)) ||
	    ((side1 < 0) && (side2 < 0))) {
	    out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
	    result = 1;
	} else {
	    cut_line(in_tri->p[1], in_tri->p[2], in_tri->t[1], in_tri->t[2], 
		     ratio12, &out_tri1.p[2], &out_tri1.t[2]);
	    out_tri2->p[0] = in_tri->p[0];out_tri2->t[0] = in_tri->t[0];
	    out_tri2->p[1] = out_tri1.p[2];out_tri2->t[1] = out_tri1.t[2];
	    out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
	    result = 2;
	}

    } else { /* (side0 < 0) */
	if (side1 < 0) {
	    out_tri1.p[1] = in_tri->p[1];out_tri1.t[1] = in_tri->t[1];
	    if (side2 <= 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		result = 1;
	    } else {
		cut_line(in_tri->p[1], in_tri->p[2], in_tri->t[1], in_tri->t[2], 
			 ratio12, &out_tri1.p[2], &out_tri1.t[2]);
		out_tri2->p[0] = out_tri1.p[2];out_tri2->t[0] = out_tri1.t[2];
		cut_line(in_tri->p[0], in_tri->p[2], in_tri->t[0], in_tri->t[2], 
			 ratio02, &out_tri2->p[1], &out_tri2->t[1]);
		out_tri2->p[2] = in_tri->p[0];out_tri2->t[2] = in_tri->t[0];
		out_tri3->p[0] = out_tri2->p[1];out_tri3->t[0] = out_tri2->t[1];
		out_tri3->p[1] = out_tri2->p[0];out_tri3->t[1] = out_tri2->t[0];
		out_tri3->p[2] = in_tri->p[2];out_tri3->t[2] = in_tri->t[2];
		result = 3;
	    }

	} else if (side1 == 0) {
	    out_tri1.p[1] = in_tri->p[1];out_tri1.t[1] = in_tri->t[1];
	    if (side2 <= 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		result = 1;
	    } else {
		cut_line(in_tri->p[0], in_tri->p[2], in_tri->t[0], in_tri->t[2], 
			 ratio02, &out_tri1.p[2], &out_tri1.t[2]);
		out_tri2->p[0] = out_tri1.p[2];out_tri2->t[0] = out_tri1.t[2];
		out_tri2->p[1] = in_tri->p[1];out_tri2->t[1] = in_tri->t[1];
		out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
		result = 2;
	    }

	} else { /* (side1 > 0) */
	    cut_line(in_tri->p[0], in_tri->p[1], in_tri->t[0], in_tri->t[1], 
		     ratio01, &out_tri1.p[1], &out_tri1.t[1]);
	    if (side2 < 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		out_tri2->p[0] = in_tri->p[2];out_tri2->t[0] = in_tri->t[2];
		out_tri2->p[1] = out_tri1.p[1];out_tri2->t[1] = out_tri1.t[1];
		cut_line(in_tri->p[1], in_tri->p[2], in_tri->t[1], in_tri->t[2], 
			 ratio12, &out_tri2->p[2], &out_tri2->t[2]);
		out_tri3->p[0] = out_tri2->p[1];out_tri3->t[0] = out_tri2->t[1];
		out_tri3->p[1] = in_tri->p[1];out_tri3->t[1] = in_tri->t[1];
		out_tri3->p[2] = out_tri2->p[2];out_tri3->t[2] = out_tri2->t[2];
		result = 3;
	    } else if (side2 == 0) {
		out_tri1.p[2] = in_tri->p[2];out_tri1.t[2] = in_tri->t[2];
		out_tri2->p[0] = out_tri1.p[1];out_tri2->t[0] = out_tri1.t[1];
		out_tri2->p[1] = in_tri->p[1];out_tri2->t[1] = in_tri->t[1];
		out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
		result = 2;
	    } else {  /* (side2 > 0) */
		cut_line(in_tri->p[0], in_tri->p[2], in_tri->t[0], in_tri->t[2], 
			 ratio02, &out_tri1.p[2], &out_tri1.t[2]);
		out_tri2->p[0] = out_tri1.p[2];out_tri2->t[0] = out_tri1.t[2];
		out_tri2->p[1] = out_tri1.p[1];out_tri2->t[1] = out_tri1.t[1];
		out_tri2->p[2] = in_tri->p[2];out_tri2->t[2] = in_tri->t[2];
		out_tri3->p[0] = in_tri->p[1];out_tri3->t[0] = in_tri->t[1];
		out_tri3->p[1] = in_tri->p[2];out_tri3->t[1] = in_tri->t[2];
		out_tri3->p[2] = out_tri1.p[1];out_tri3->t[2] = out_tri1.t[1];
		result = 3;
	    }
	}
    }
    *in_tri = out_tri1;
    return result;
}
