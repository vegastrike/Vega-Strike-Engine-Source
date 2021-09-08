/*
 * gl_matrix_hack.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn and Alan Shieh
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "gfxlib.h"
#include "gfx/vec.h"
#include <stdio.h>
//typedef float GLdouble;
#include <math.h>
#include <string.h>
#include <assert.h>
#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#ifndef M_PI
# define M_PI 3.14159265358979323846            /* pi */
#endif
#endif
#include "gl_matrix.h"

#include "vs_globals.h"
#include "vsfilesystem.h"
#include "vs_logging.h"

//#include <GL/glu.h>

inline void Zero( float matrix[] )
{
    matrix[0]  = 0;
    matrix[1]  = 0;
    matrix[2]  = 0;
    matrix[3]  = 0;

    matrix[4]  = 0;
    matrix[5]  = 0;
    matrix[6]  = 0;
    matrix[7]  = 0;

    matrix[8]  = 0;
    matrix[9]  = 0;
    matrix[10] = 0;
    matrix[11] = 0;

    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 0;
}

inline void Identity( float matrix[] )
{
    matrix[0]  = 1;
    matrix[1]  = 0;
    matrix[2]  = 0;
    matrix[3]  = 0;

    matrix[4]  = 0;
    matrix[5]  = 1;
    matrix[6]  = 0;
    matrix[7]  = 0;

    matrix[8]  = 0;
    matrix[9]  = 0;
    matrix[10] = 1;
    matrix[11] = 0;

    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

float Magnitude( Vector v )
{
    return sqrtf( v.i*v.i+v.j*v.j+v.k*v.k );
}

/*
 *  Vector operator*(float left, const Vector &right)
 *  {
 *       return Vector(right.i*left, right.j*left, right.k*left);
 *  }*/

float DotProduct( Vector &a, Vector &b )
{
    return a.Dot( b );
}

inline void MultMatrix( float dest[], const float m1[], const float m2[] )
{
    dest[0]  = m1[0]*m2[0]+m1[4]*m2[1]+m1[8]*m2[2]+m1[12]*m2[3];
    dest[1]  = m1[1]*m2[0]+m1[5]*m2[1]+m1[9]*m2[2]+m1[13]*m2[3];
    dest[2]  = m1[2]*m2[0]+m1[6]*m2[1]+m1[10]*m2[2]+m1[14]*m2[3];
    dest[3]  = m1[3]*m2[0]+m1[7]*m2[1]+m1[11]*m2[2]+m1[15]*m2[3];

    dest[4]  = m1[0]*m2[4]+m1[4]*m2[5]+m1[8]*m2[6]+m1[12]*m2[7];
    dest[5]  = m1[1]*m2[4]+m1[5]*m2[5]+m1[9]*m2[6]+m1[13]*m2[7];
    dest[6]  = m1[2]*m2[4]+m1[6]*m2[5]+m1[10]*m2[6]+m1[14]*m2[7];
    dest[7]  = m1[3]*m2[4]+m1[7]*m2[5]+m1[11]*m2[6]+m1[15]*m2[7];

    dest[8]  = m1[0]*m2[8]+m1[4]*m2[9]+m1[8]*m2[10]+m1[12]*m2[11];
    dest[9]  = m1[1]*m2[8]+m1[5]*m2[9]+m1[9]*m2[10]+m1[13]*m2[11];
    dest[10] = m1[2]*m2[8]+m1[6]*m2[9]+m1[10]*m2[10]+m1[14]*m2[11];
    dest[11] = m1[3]*m2[8]+m1[7]*m2[9]+m1[11]*m2[10]+m1[15]*m2[11];

    dest[12] = m1[0]*m2[12]+m1[4]*m2[13]+m1[8]*m2[14]+m1[12]*m2[15];
    dest[13] = m1[1]*m2[12]+m1[5]*m2[13]+m1[9]*m2[14]+m1[13]*m2[15];
    dest[14] = m1[2]*m2[12]+m1[6]*m2[13]+m1[10]*m2[14]+m1[14]*m2[15];
    dest[15] = m1[3]*m2[12]+m1[7]*m2[13]+m1[11]*m2[14]+m1[15]*m2[15];
    /*	Zero(dest);
     *     for(int rowcount = 0; rowcount<4; rowcount++)
     *             for(int colcount = 0; colcount<4; colcount++)
     *                     for(int mcount = 0; mcount <4; mcount ++)
     *                             dest[colcount*4+rowcount] += m1[mcount*4+rowcount]*m2[colcount*4+mcount];
     */
}

inline void CopyMatrix( Matrix dest, const Matrix source )
{
    dest[0]  = source[0];
    dest[1]  = source[1];
    dest[2]  = source[2];
    dest[3]  = source[3];
    dest[4]  = source[4];
    dest[5]  = source[5];
    dest[6]  = source[6];
    dest[7]  = source[7];
    dest[8]  = source[8];
    dest[9]  = source[9];
    dest[10] = source[10];
    dest[11] = source[11];
    dest[12] = source[12];
    dest[13] = source[13];
    dest[14] = source[14];
    dest[15] = source[15];
    //memcpy(dest, source, sizeof(Matrix));

    /*	for(int matindex = 0; matindex<16; matindex++)
     *             dest[matindex] = source[matindex];
     */
}

using namespace GFXMatrices;  //causes problems with g_game

float centerx, centery, centerz;
void evaluateViews()
{
    //Identity(transview);
    Identity( rotview );
#define M( row, col ) rotview[col*4+row]
    rotview[0]  = view[0];
    rotview[1]  = view[1];
    rotview[2]  = view[2];
    //transview[3]=view[3];
    rotview[4]  = view[4];
    rotview[5]  = view[5];
    rotview[6]  = view[6];
    //transview[7]=view[7];
    rotview[8]  = view[8];
    rotview[9]  = view[9];
    rotview[10] = view[10];
    //transview[11]=view[11];
#undef M
}

Vector eye, center, up;

void getInverseProjection( float* &inv )
{
    inv = invprojection;
}

/**
 * GFXGetXPerspective () returns the number that x/z is multiplied by to
 * land a pixel on the screen.
 * | xs 0  a 0 |[x]   [xs + az]          [1/xs 0   0  a/xs][x]   [x/xs+ aw/xs]
 * | 0  ys b 0 |[y] = [ys + bz]    ^-1   [ 0  1/ys 0  b/ys][y] = [y/ys+ bw/ys]
 * | 0  0  c d |[z]   [cz + dw]          [ 0   0   0  -1  ][z]   [0          ]
 * | 0  0 -1 0 |[w]   [-z     ]          [ 0   0  1/d c/d ][w]   [z/d + cw/d ]
 * therefore   return 1/(xs *d) and 1/(ys * d)
 * I'm not good with matrix math...tell me if I should ret 1/xs+c/d instead
 * for test cases I can think of, it doesn't matter--- */
float GFXGetZPerspective( const float z )
{
    float left, right, bottom, top, nearval, farval;
    GFXGetFrustumVars( true, &left, &right, &bottom, &top, &nearval, &farval );

    VS_LOG(info, (boost::format("nearval: %1%, left: %2%, right: %3%, z: %4%") % nearval % left % right % z));

    float xs = 2*nearval/(right-left);
    float a  = (right+left)/(right-left);

    //Compute homogeneus x,w for (1,0,z,0)
    float hx = xs+z*a;
    float hw = -z;

    //Translate into euclidean coordinates and return euclidean x
    return hx/hw;
}

float GFXGetXInvPerspective()
{
    return /*invprojection[11]*  */ invprojection[0];     //invprojection[15];//should be??  c/d == invproj[15]
}

float GFXGetYInvPerspective()
{
    return /*invprojection[11]*  */ invprojection[5];     //invprojection[15];//should be??  c/d == invproj[15]
}

void /*GFXDRVAPI*/ GFXTranslate( const MATRIXMODE mode, const Vector &a )
{
    switch (mode)
    {
    case MODEL:
        model[12] += a.i*model[0]+a.j*model[4]+a.k*model[8];
        model[13] += a.i*model[1]+a.j*model[5]+a.k*model[9];
        model[14] += a.i*model[2]+a.j*model[6]+a.k*model[10];
        glMatrixMode( GL_MODELVIEW );
        glTranslatef( a.i, a.j, a.k );
        break;
    case VIEW:
        view[12] += a.i*view[0]+a.j*view[4]+a.k*view[8];
        view[13] += a.i*view[1]+a.j*view[5]+a.k*view[9];
        view[14] += a.i*view[2]+a.j*view[6]+a.k*view[10];
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
        glLoadIdentity();
        glTranslatef( -view[12], -view[13], -view[14] );
        glPushMatrix();
        glMultMatrixf( model );
        break;
    case PROJECTION:
        projection[12] += a.i*projection[0]+a.j*projection[4]+a.k*projection[8];
        projection[13] += a.i*projection[1]+a.j*projection[5]+a.k*projection[9];
        projection[14] += a.i*projection[2]+a.j*projection[6]+a.k*projection[10];
        {
            Matrix t;
            MultMatrix( t, projection, rotview );
            glMatrixMode( GL_PROJECTION );
            glLoadMatrixf( t );
        }
        break;
    }
}

void /*GFXDRVAPI*/ GFXMultMatrix( const MATRIXMODE mode, const Matrix matrix )
{
    Matrix t;
    switch (mode)
    {
    case MODEL:
        MultMatrix( t, model, matrix );
        CopyMatrix( model, t );
        glMatrixMode( GL_MODELVIEW );
        //glPopMatrix();
        //glPushMatrix();
        glMultMatrixf( matrix );
        break;
    case VIEW:
        MultMatrix( t, view, matrix );
        CopyMatrix( view, t );
        evaluateViews();
        MultMatrix( t, projection, rotview );
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( t );
        /*FIXME1233
         *  glMatrixMode(GL_MODELVIEW);
         *  glPopMatrix();
         *  glLoadIdentity();
         *  glTranslatef(-centerx,-centery,-centerz);
         *  glPushMatrix();
         *  glMultMatrixf(model);
         */
        break;
    case PROJECTION:
        MultMatrix( t, projection, matrix );
        CopyMatrix( projection, t );
        MultMatrix( t, projection, rotview );
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( t );
        break;
    }
}

void /*GFXDRVAPI*/ GFXLoadMatrix( const MATRIXMODE mode, const Matrix matrix )
{
    Matrix t;
    switch (mode)
    {
    case MODEL:
        CopyMatrix( model, matrix );
        model[12] -= centerx;
        model[13] -= centery;
        model[14] -= centerz;
        glMatrixMode( GL_MODELVIEW );
        //glPopMatrix();
        //glPushMatrix();
        glLoadMatrixf( model );
        model[12] = matrix[12];
        model[13] = matrix[13];
        model[14] = matrix[14];

        break;
    case VIEW:
        CopyMatrix( view, matrix );
        evaluateViews();
        //MultMatrix(t, transview, model);
        /* FIXME1233
         *  glMatrixMode(GL_MODELVIEW);
         *  glPopMatrix();
         *  glLoadIdentity();
         *  glTranslatef(-centerx,-centery,-centerz);
         *  glPushMatrix();
         *
         *  glMultMatrixf(model);
         */
        glMatrixMode( GL_PROJECTION );
        MultMatrix( t, projection, rotview );
        glLoadMatrixf( t );
        break;
    case PROJECTION:
        CopyMatrix( projection, matrix );
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( projection );
        glMultMatrixf( rotview );
        break;
    }
}

void GFXViewPort( int minx, int miny, int maxx, int maxy )
{
    glViewport( minx, miny, maxx, maxy );
}

void GFXHudMode( const bool Enter )
{
    if (Enter) {
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
    } else {
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
    }
}

void /*GFXDRVAPI*/ GFXLoadIdentity( const MATRIXMODE mode )
{
    switch (mode)
    {
    case MODEL:
        Identity( model );
        glMatrixMode( GL_MODELVIEW );
        //glLoadMatrixf(transview);
        glPopMatrix();
        glPushMatrix();
        break;
    case VIEW:
        Identity( view );
        Identity( rotview );
        //Identity (transview);
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
        glLoadMatrixf( model );
        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( projection );
        break;
    case PROJECTION:
        Identity( projection );
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( rotview );
        break;
    }
}

void /*GFXDRVAPI*/ GFXGetMatrix( const MATRIXMODE mode, Matrix matrix )
{
    Matrix translation;
    switch (mode)
    {
    case MODEL:
        assert( 0 );
        break;
    case VIEW:
        Identity( translation );
        translation[12] = -centerx;
        translation[13] = -centery;
        translation[14] = -centerz;
        MultMatrix( matrix, rotview, translation );
        //CopyMatrix(matrix, view);
        break;
    case PROJECTION:
        CopyMatrix( matrix, projection );
        break;
    }
}

static void gl_Frustum( float left, float right, float bottom, float top, float nearval, float farval )
{
    GFXGetFrustumVars( false, &left, &right, &bottom, &top, &nearval, &farval );
    GFXFrustum( projection, invprojection, left, right, bottom, top, nearval, farval );
}

void GFXGetFrustumVars( bool retr, float *l, float *r, float *b, float *t, float *n, float *f )
{
    static float nnear, ffar, left, right, bot, top;     //Visual C++ reserves near and far
    if (!retr) {
        nnear = *n;
        ffar  = *f;
        left  = *l;
        right = *r;
        bot   = *b;
        top   = *t;
    } else {
        *l = left;
        *r = right;
        *b = bot;
        *t = top;
        *n = nnear;
        *f = ffar;
    }
}

void GFXFrustum( float *m, float *i, float left, float right, float bottom, float top, float nearval, float farval )
{
    GLfloat x, y, a, b, c, d;
    x = ( ( (float) 2.0 )*nearval )/(right-left);
    y = ( ( (float) 2.0 )*nearval )/(top-bottom);
    a = (right+left)/(right-left);
    b = (top+bottom)/(top-bottom);
    //If farval == 0, we'll build an infinite-farplane projection matrix.
    if (farval == 0) {
        c = -1.0;
        d = -1.99*nearval;         //-2*nearval, but using exactly -2 might create artifacts
    } else {
        c = -(farval+nearval)/(farval-nearval);
        d = -( ( (float) 2.0 )*farval*nearval )/(farval-nearval);
    }
#define M( row, col ) m[col*4+row]
    M( 0, 0 ) = x;
    M( 0, 1 ) = 0.0F;
    M( 0, 2 ) = a;
    M( 0, 3 ) = 0.0F;
    M( 1, 0 ) = 0.0F;
    M( 1, 1 ) = y;
    M( 1, 2 ) = b;
    M( 1, 3 ) = 0.0F;
    M( 2, 0 ) = 0.0F;
    M( 2, 1 ) = 0.0F;
    M( 2, 2 ) = c;
    M( 2, 3 ) = d;
    M( 3, 0 ) = 0.0F;
    M( 3, 1 ) = 0.0F;
    M( 3, 2 ) = -1.0F;
    M( 3, 3 ) = 0.0F;
#undef M
#define M( row, col ) i[col*4+row]
    M( 0, 0 ) = 1./x;
    M( 0, 1 ) = 0.0F;
    M( 0, 2 ) = 0.0F;
    M( 0, 3 ) = a/x;
    M( 1, 0 ) = 0.0F;
    M( 1, 1 ) = 1./y;
    M( 1, 2 ) = 0.0F;
    M( 1, 3 ) = b/y;
    M( 2, 0 ) = 0.0F;
    M( 2, 1 ) = 0.0F;
    M( 2, 2 ) = 0.0F;
    M( 2, 3 ) = -1.0F;
    M( 3, 0 ) = 0.0F;
    M( 3, 1 ) = 0.0F;
    M( 3, 2 ) = 1.F/d;
    M( 3, 3 ) = (float) c/d;
#undef M
}

void /*GFXDRVAPI*/ GFXPerspective( float fov, float aspect, float znear, float zfar, float cockpit_offset )
{
    //gluPerspective (fov,aspect,znear,zfar);
    float xmin, xmax, ymin, ymax;
    ymax  = znear*tanf( fov*M_PI/( (float) 360.0 ) );       //78.0 --> 4.7046
    ymin  = -ymax;     //-4.7046
    xmin  = (ymin-cockpit_offset/2)*aspect;       //-6.2571
    xmax  = (ymax+cockpit_offset/2)*aspect;       //6.2571
    ymin -= cockpit_offset;
    gl_Frustum( xmin, xmax, ymin, ymax, znear, zfar );
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( projection );
}

void /*GFXDRVAPI*/ GFXParallel( float left, float right, float bottom, float top, float nearval, float farval )
{
    float *m = projection, x, y, z, tx, ty, tz;
    x  = 2.0/(right-left);
    y  = 2.0/(top-bottom);
    z  = -2.0/(farval-nearval);
    tx = -(right+left)/(right-left);
    ty = -(top+bottom)/(top-bottom);
    tz = -(farval+nearval)/(farval-nearval);
#define M( row, col ) m[col*4+row]
    M( 0, 0 ) = x;
    M( 0, 1 ) = 0.0F;
    M( 0, 2 ) = 0.0F;
    M( 0, 3 ) = tx;
    M( 1, 0 ) = 0.0F;
    M( 1, 1 ) = y;
    M( 1, 2 ) = 0.0F;
    M( 1, 3 ) = ty;
    M( 2, 0 ) = 0.0F;
    M( 2, 1 ) = 0.0F;
    M( 2, 2 ) = z;
    M( 2, 3 ) = tz;
    M( 3, 0 ) = 0.0F;
    M( 3, 1 ) = 0.0F;
    M( 3, 2 ) = 0.0F;
    M( 3, 3 ) = 1.0F;
#undef M
    GFXLoadMatrix( PROJECTION, projection );
    GFXGetFrustumVars( false, &left, &right, &bottom, &top, &nearval, &farval );
}

static void LookAtHelper( float eyex,
                          float eyey,
                          float eyez,
                          float centerx,
                          float centery,
                          float centerz,
                          float upx,
                          float upy,
                          float upz )
{
    float m[16];
    float x[3], y[3], z[3];
    float mag;
    /* Make rotation matrix */
    /* Z vector */
    z[0] = eyex-centerx;
    z[1] = eyey-centery;
    z[2] = eyez-centerz;
    mag  = sqrtf( z[0]*z[0]+z[1]*z[1]+z[2]*z[2] );
    if (mag) {
        /* mpichler, 19950515 */
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }
    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;
    /* X vector = Y cross Z */
    //x[0] =  z[1]*y[2] - z[2]*y[1];
    //x[1] = -z[0]*y[2] + z[2]*y[0];
    //x[2] =  z[0]*y[1] - z[1]*y[0];
    x[0] = y[1]*z[2]-y[2]*z[1];
    x[1] = -y[0]*z[2]+y[2]*z[0];
    x[2] = y[0]*z[1]-y[1]*z[0];
    /* Recompute Y = Z cross X */
    //y[0] =  x[1]*z[2] - x[2]*z[1];
    //y[1] = -x[0]*z[2] + x[2]*z[0];
    //y[2] =  x[0]*z[1] - x[1]*z[0];
    y[0] = z[1]*x[2]-z[2]*x[1];
    y[1] = -z[0]*x[2]+z[2]*x[0];
    y[2] = z[0]*x[1]-z[1]*x[0];
    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */
    mag = sqrtf( x[0]*x[0]+x[1]*x[1]+x[2]*x[2] );
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }
    mag = sqrtf( y[0]*y[0]+y[1]*y[1]+y[2]*y[2] );
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }
#define M( row, col ) m[col*4+row]
    M( 0, 0 ) = x[0];
    M( 0, 1 ) = x[1];
    M( 0, 2 ) = x[2];
    M( 0, 3 ) = 0.0;
    M( 1, 0 ) = y[0];
    M( 1, 1 ) = y[1];
    M( 1, 2 ) = y[2];
    M( 1, 3 ) = 0.0;
    M( 2, 0 ) = z[0];
    M( 2, 1 ) = z[1];
    M( 2, 2 ) = z[2];
    M( 2, 3 ) = 0.0;
    M( 3, 0 ) = 0.0;
    M( 3, 1 ) = 0.0;
    M( 3, 2 ) = 0.0;
    M( 3, 3 ) = 1.0;
    float tm[16];
#ifdef WIN32
    ZeroMemory( tm, sizeof (tm) );
#else
    bzero( tm, sizeof (tm) );
#endif
#undef M
#define M( row, col ) tm[col*4+row]
    M( 0, 0 ) = 1.0;
    M( 0, 3 ) = -eyex;
    M( 1, 1 ) = 1.0;
    M( 1, 3 ) = -eyey;
    M( 2, 2 ) = 1.0;
    M( 2, 3 ) = -eyez;
    M( 3, 3 ) = 1.0;
#undef M
    MultMatrix( view, m, tm );
/***
 *   float dis = sqrtf(upx*upx+upy*upy);
 *  Identity (tm);
 *  if (eyez-centerz > 0) {
 *    upx = -upx;
 *  }
 * #define M(row,col)  tm[col*4+row]
 *  M(0,0) = upy/dis;
 *  M(0,1) = -upx/dis;
 *  M(1,1) = upy/dis;
 *  M(1,0) = upx/dis;
 *  M(2,2) = 1.0;
 *  M(3,3) = 1.0;
 * #undef M
 ***/                                                                                                                                                                                                                                                                                                    //old hack to twiddle the texture in the xy plane
#ifdef NV_CUBE_MAP
    //FIXME--ADD CAMERA MATRICES
    //the texture matrix must be used to rotate the texgen-computed
    //reflection or normal vector texture coordinates to match the orinetation
    //of the cube map.  Teh rotation can be computed based on two vectors
    //1) the direction vector from the cube map center to the eye position
    //and 2 the cube map orientation in world coordinates.
    //the axis is the cross product of these two vectors...teh angle is arcsin
    //of the dot of these two vectors
    GFXActiveTexture( 1 );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    //Vector (centerx,centery,centerz).Cross (Vector (1,0,0));  DID NOT TRANSFORM THE ORIENTATION VECTOR TO REVERSE CAMERASPACE
    Vector axis( centerx, centery, centerz );
    Vector cubemapincamspace( eyex, eyey, eyez );
    cubemapincamspace.Normalize();
    axis.Normalize();
    //float theta = arcsinf (Vector (centerx,centery,centerz).Normalize().Dot (Vector (1,0,0)));  DID NOT TRANSFORM THE ORIENTATION VECTOR TO REVERSE CAMERASPACE
    float theta = asinf( axis.Dot( cubemapincamspace ) );
    axis = axis.Cross( axis.Cross( cubemapincamspace ) );
    glRotatef( theta, axis.i, axis.j, axis.k );
    //ok do matrix math to rotate by theta on axis  those ..
    GFXActiveTexture( 0 );
#else
    /*	glTranslatef(.5f,.5f,.4994f);
     *    glMultMatrixf(tm);
     *    glTranslatef(-.5f,-.5f,-.4994f);
     */
#endif
}

void /*GFXDRVAPI*/ GFXLookAt( Vector eye, Vector center, Vector up )
{
    LookAtHelper( eye.i, eye.j, eye.k, center.i, center.j, center.k, up.i, up.j, up.k );

    //Identity(transview);
    //transview[3]=center.i;
    //transview[7]=center.j;
    //transview[11]=center.k;
    centerx = center.i;
    centery = center.j;
    centerz = center.k;
    GFXLoadMatrix( VIEW, view );
}

float frust[6][4];

float /*GFXDRVAPI*/ GFXSphereInFrustum( const Vector &Cnt, float radius )
{
    return GFXSphereInFrustum( frust, Cnt, radius );
}

float /*GFXDRVAPI*/ GFXSphereInFrustum( float f[6][4], const Vector &Cnt, float radius )
{
    int   p;
    float d;
    for (p = 0; p < 5; p++) {
        //does not evaluate for yon
        d = f[p][0]*Cnt.i+f[p][1]*Cnt.j+f[p][2]*Cnt.k+f[p][3];
        if (d <= -radius)
            return 0;
    }
    return d;
}

void /*GFXDRVAPI*/ GFXGetFrustum( float f[6][4] )
{
    f = frust;
}

void /*GFXDRVAPI*/ GFXCalculateFrustum()
{
    GFXCalculateFrustum( frust, view, projection );
}

void /*GFXDRVAPI*/ GFXCalculateFrustum( float frustum[6][4], float *modl, float *proj )
{
////float   *proj=projection;
////float   *modl=view;
    float clip[16];
    float t;

    /* Get the current PROJECTION matrix from OpenGL */
    //glGetFloatv( GL_PROJECTION_MATRIX, proj );

    /* Get the current MODELVIEW matrix from OpenGL */
    //glGetFloatv( GL_MODELVIEW_MATRIX, modl );

    /* Combine the two matrices (multiply projection by modelview) */
    clip[0]  = modl[0]*proj[0]+modl[1]*proj[4]+modl[2]*proj[8]+modl[3]*proj[12];
    clip[1]  = modl[0]*proj[1]+modl[1]*proj[5]+modl[2]*proj[9]+modl[3]*proj[13];
    clip[2]  = modl[0]*proj[2]+modl[1]*proj[6]+modl[2]*proj[10]+modl[3]*proj[14];
    clip[3]  = modl[0]*proj[3]+modl[1]*proj[7]+modl[2]*proj[11]+modl[3]*proj[15];

    clip[4]  = modl[4]*proj[0]+modl[5]*proj[4]+modl[6]*proj[8]+modl[7]*proj[12];
    clip[5]  = modl[4]*proj[1]+modl[5]*proj[5]+modl[6]*proj[9]+modl[7]*proj[13];
    clip[6]  = modl[4]*proj[2]+modl[5]*proj[6]+modl[6]*proj[10]+modl[7]*proj[14];
    clip[7]  = modl[4]*proj[3]+modl[5]*proj[7]+modl[6]*proj[11]+modl[7]*proj[15];

    clip[8]  = modl[8]*proj[0]+modl[9]*proj[4]+modl[10]*proj[8]+modl[11]*proj[12];
    clip[9]  = modl[8]*proj[1]+modl[9]*proj[5]+modl[10]*proj[9]+modl[11]*proj[13];
    clip[10] = modl[8]*proj[2]+modl[9]*proj[6]+modl[10]*proj[10]+modl[11]*proj[14];
    clip[11] = modl[8]*proj[3]+modl[9]*proj[7]+modl[10]*proj[11]+modl[11]*proj[15];

    clip[12] = modl[12]*proj[0]+modl[13]*proj[4]+modl[14]*proj[8]+modl[15]*proj[12];
    clip[13] = modl[12]*proj[1]+modl[13]*proj[5]+modl[14]*proj[9]+modl[15]*proj[13];
    clip[14] = modl[12]*proj[2]+modl[13]*proj[6]+modl[14]*proj[10]+modl[15]*proj[14];
    clip[15] = modl[12]*proj[3]+modl[13]*proj[7]+modl[14]*proj[11]+modl[15]*proj[15];

    /* Extract the numbers for the RIGHT plane */
    frustum[0][0] = clip[3]-clip[0];
    frustum[0][1] = clip[7]-clip[4];
    frustum[0][2] = clip[11]-clip[8];
    frustum[0][3] = clip[15]-clip[12];

    /* Normalize the result */
    t = sqrtf( frustum[0][0]*frustum[0][0]+frustum[0][1]*frustum[0][1]+frustum[0][2]*frustum[0][2] );
    frustum[0][0] /= t;
    frustum[0][1] /= t;
    frustum[0][2] /= t;
    frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    frustum[1][0]  = clip[3]+clip[0];
    frustum[1][1]  = clip[7]+clip[4];
    frustum[1][2]  = clip[11]+clip[8];
    frustum[1][3]  = clip[15]+clip[12];

    /* Normalize the result */
    t = sqrtf( frustum[1][0]*frustum[1][0]+frustum[1][1]*frustum[1][1]+frustum[1][2]*frustum[1][2] );
    frustum[1][0] /= t;
    frustum[1][1] /= t;
    frustum[1][2] /= t;
    frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    frustum[2][0]  = clip[3]+clip[1];
    frustum[2][1]  = clip[7]+clip[5];
    frustum[2][2]  = clip[11]+clip[9];
    frustum[2][3]  = clip[15]+clip[13];

    /* Normalize the result */
    t = sqrtf( frustum[2][0]*frustum[2][0]+frustum[2][1]*frustum[2][1]+frustum[2][2]*frustum[2][2] );
    frustum[2][0] /= t;
    frustum[2][1] /= t;
    frustum[2][2] /= t;
    frustum[2][3] /= t;

    /* Extract the TOP plane */
    frustum[3][0]  = clip[3]-clip[1];
    frustum[3][1]  = clip[7]-clip[5];
    frustum[3][2]  = clip[11]-clip[9];
    frustum[3][3]  = clip[15]-clip[13];

    /* Normalize the result */
    t = sqrtf( frustum[3][0]*frustum[3][0]+frustum[3][1]*frustum[3][1]+frustum[3][2]*frustum[3][2] );
    frustum[3][0] /= t;
    frustum[3][1] /= t;
    frustum[3][2] /= t;
    frustum[3][3] /= t;

    /* Extract the FAR plane */
    frustum[5][0]  = clip[3]-clip[2];
    frustum[5][1]  = clip[7]-clip[6];
    frustum[5][2]  = clip[11]-clip[10];
    frustum[5][3]  = clip[15]-clip[14];

    /* Normalize the result */
    t = sqrtf( frustum[5][0]*frustum[5][0]+frustum[5][1]*frustum[5][1]+frustum[5][2]*frustum[5][2] );
    frustum[5][0] /= t;
    frustum[5][1] /= t;
    frustum[5][2] /= t;
    frustum[5][3] /= t;

    /* Extract the NEAR plane */
    frustum[4][0]  = clip[3]+clip[2];
    frustum[4][1]  = clip[7]+clip[6];
    frustum[4][2]  = clip[11]+clip[10];
    frustum[4][3]  = clip[15]+clip[14];

    /* Normalize the result */
    t = sqrtf( frustum[4][0]*frustum[4][0]+frustum[4][1]*frustum[4][1]+frustum[4][2]*frustum[4][2] );
    frustum[4][0] /= t;
    frustum[4][1] /= t;
    frustum[4][2] /= t;
    frustum[4][3] /= t;
}

