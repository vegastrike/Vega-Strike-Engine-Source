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
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _VS_TYPES_H_
#define _VS_TYPES_H_

#include "vegastrike.h"

typedef double scalar_t;

/* Not sure where these are getting defined, but it shouldn't hurt to
   undef them. */
#ifdef True
#undef True
#endif

#ifdef False
#undef False
#endif

typedef enum {
    False = 0,
    True = 1
} bool_t;

typedef struct {
    scalar_t x;   
    scalar_t y;
} point2d_t;

typedef struct {
    scalar_t x;   
    scalar_t y;
    scalar_t z;
} point_t;

typedef struct {
    int i;   
    int j;
} index2d_t;

typedef struct {
    scalar_t x;   
    scalar_t y;
    scalar_t z;
} vector_t; 

typedef struct {
    scalar_t x;
    scalar_t y;
} vector2d_t;

typedef struct {
    vector_t nml;
    scalar_t d;
} plane_t;

typedef scalar_t matrixgl_t[4][4]; 

typedef struct {
    int num_vertices;
    int *vertices;
} polygon_t;

typedef struct {
    point_t p[3];
    point2d_t t[3];
} triangle_t;

typedef struct {
    int num_vertices;
    int num_polygons;
    point_t *vertices;
    polygon_t *polygons;
} polyhedron_t;

/* Ray (half-line) */
typedef struct { 
    point_t pt;
    vector_t vec;
} ray_t;

typedef struct {
    point_t pt;
    vector_t nml;
} line_t;

/* Quaternion */
typedef struct {
    scalar_t x;
    scalar_t y;
    scalar_t z;
    scalar_t w;
} quaternion_t;

/* Colour */
typedef struct {
    scalar_t r;
    scalar_t g;
    scalar_t b;
    scalar_t a;
} colour_t;

/* Material */
typedef struct {
    colour_t diffuse;
    colour_t specular_colour;
    double specular_exp;
} material_t;

/* Light */
typedef struct {
    bool_t is_on;
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat position[4];
    GLfloat spot_direction[3];
    GLfloat spot_exponent;
    GLfloat spot_cutoff;
    GLfloat constant_attenuation;
    GLfloat linear_attenuation;
    GLfloat quadratic_attenuation;
} light_t;

/* Key frame for animation sequences */
typedef struct {
    scalar_t time;
    point_t pos;
    scalar_t yaw;        /* angle of rotation about y axis */
    scalar_t pitch;      /* angle of rotation about x axis */
    scalar_t l_shldr;
    scalar_t r_shldr;
    scalar_t l_hip;
    scalar_t r_hip;
} key_frame_t; 

/* Scene graph node types. */
typedef enum { 
    Empty, Sphere
} geometry_t;

/* Data for Sphere node type. */
typedef struct {
    double radius;

    /* How many divisions do we use to draw a sphere? */
    int divisions;
} sphere_t;

  
/* Scene graph node. */
typedef struct scene_node_struct {
    struct scene_node_struct* parent;
    struct scene_node_struct* next;
    struct scene_node_struct* child;

    geometry_t geom;   /* type of node */

    union {
        sphere_t sphere;   
    } param;
      
    material_t* mat;


    /* Do we draw the shadow of this node? */
    bool_t render_shadow;

    /* The forward and inverse transforms */
    matrixgl_t trans;
    matrixgl_t invtrans;   

    /* name of node (for debugging) */
    char *name;
} scene_node_t;

typedef enum {
    Ice = 0,
    Rock = 1,
    Snow = 2,
    NumTerrains
} terrain_t;

typedef struct {
    ray_t ray;
    scalar_t height;
    scalar_t diam;
    int tree_type;
} tree_t;

typedef struct {
    ray_t ray;
    scalar_t height;
    scalar_t diam;
    int item_type;
    int collectable;
    bool_t drawable;
} item_t;

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
