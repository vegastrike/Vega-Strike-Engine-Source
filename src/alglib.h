/* 
 * Vega Strike 
 * Copyright (C) 2000-2001 Daniel Horn
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

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ALGLIB_H
#define ALGLIB_H

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef ROUND_TO_NEAREST
#   define ROUND_TO_NEAREST(x) ( (int) ((x)+0.5) )
#endif

#define ANGLES_TO_RADIANS(x) ( M_PI / 180.0 * (x) )
#define RADIANS_TO_ANGLES(x) ( 180.0 / M_PI * (x) )

/* points and vectors */
vector_t  make_vector( scalar_t x, scalar_t y, scalar_t z );
point_t   make_point( scalar_t x, scalar_t y, scalar_t z );
point2d_t make_point2d( scalar_t x, scalar_t y );
index2d_t make_index2d( int i, int j );

scalar_t  dot_product( vector_t v1, vector_t v2 );
vector_t  scale_vector( scalar_t s, vector_t v );

vector_t  add_vectors( vector_t v1, vector_t v2 );
point_t   move_point( point_t v1, vector_t v2 );

vector_t  subtract_points( point_t v1, point_t v2 );
vector_t  subtract_vectors( vector_t v1, vector_t v2 );
point_t   point_minus_vector( point_t v1, vector_t v2 );
vector_t  project_into_plane( vector_t nml, vector_t v );

scalar_t  normalize_vector( vector_t *v );

vector_t  cross_product(vector_t u, vector_t v);
vector_t  transform_vector(matrixgl_t mat, vector_t v);
vector_t  transform_normal(vector_t n, matrixgl_t mat);
point_t   transform_point(matrixgl_t mat, point_t p);

/* planes */
plane_t make_plane( scalar_t nx, scalar_t ny, scalar_t nz, scalar_t d );
bool_t intersect_planes( plane_t s1, plane_t s2, plane_t s3, point_t *p );
scalar_t distance_to_plane( plane_t plane, point_t pt );

/* matrices */
void make_identity_matrix(matrixgl_t h);
void multiply_matrices(matrixgl_t ret, matrixgl_t mat1, matrixgl_t mat2);
void transpose_matrix( matrixgl_t mat, matrixgl_t trans );

/* transformation matrices */
void make_rotation_matrix( matrixgl_t mat, scalar_t angle, char axis );
void make_translation_matrix( matrixgl_t mat,
				 scalar_t x, scalar_t y, scalar_t z );
void make_scaling_matrix( matrixgl_t mat, 
			     scalar_t x, scalar_t y, scalar_t z );
void make_change_of_basis_matrix( matrixgl_t mat, matrixgl_t invMat, 
				vector_t w1, vector_t w2, vector_t w3 );

void make_rotation_about_vector_matrix( matrixgl_t mat, vector_t u, scalar_t angle );

quaternion_t make_quaternion( scalar_t x, scalar_t y, scalar_t z, scalar_t w );
quaternion_t multiply_quaternions( quaternion_t q, quaternion_t r );
quaternion_t add_quaternions( quaternion_t q, quaternion_t r );
quaternion_t scale_quaternion( scalar_t s, quaternion_t q );
quaternion_t quaternion_conjugate( quaternion_t q );
void make_matrix_from_quaternion( matrixgl_t mat, quaternion_t q );
quaternion_t make_quaternion_from_matrix( matrixgl_t mat );
quaternion_t make_rotation_quaternion( vector_t s, vector_t t );
quaternion_t interpolate_quaternions( quaternion_t q, quaternion_t r, 
				      scalar_t t );
vector_t rotate_vector( quaternion_t q, vector_t v );

#define MAG_SQD( vec ) ( (vec).x * (vec).x + \
                         (vec).y * (vec).y + \
                         (vec).z * (vec).z )

/* polygons */
int cut_triangle( triangle_t *replace, triangle_t *new1, triangle_t *new2, 
		  line_t cut_line );

#endif /* ALGLIB_H */

#ifdef __cplusplus
} /* extern "C" */
#endif
