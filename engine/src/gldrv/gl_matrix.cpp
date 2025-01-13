/*
 * gl_matrix.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
#include "gl_globals.h"
#include "gfxlib.h"
#include "gfx/vec.h"
#include <stdio.h>
//typedef float GLdouble;
#include <math.h>
#include <string.h>
#include <assert.h>
//#include "vegastrike.h"
#include "gfx/matrix.h"
#include "vs_globals.h"
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

using namespace GFXMatrices;  //causes problems with g_game

void getInverseProjection(float *&inv) {
    inv = invprojection;
}

float GFXGetXInvPerspective() {
    return /*invprojection[11]*  */ invprojection[0];     //invprojection[15];//should be??  c/d == invproj[15]
}

float GFXGetYInvPerspective() {
    return /*invprojection[11]*  */ invprojection[5];     //invprojection[15];//should be??  c/d == invproj[15]
}

void MatrixToDoubles(double t[], const Matrix &m) {
    t[0] = m.r[0];     //possible performance hit?!?!
    t[1] = m.r[1];
    t[2] = m.r[2];
    t[3] = 0;
    t[4] = m.r[3];
    t[5] = m.r[4];
    t[6] = m.r[5];
    t[7] = 0;
    t[8] = m.r[6];
    t[9] = m.r[7];
    t[10] = m.r[8];
    t[11] = 0;
    t[12] = (m.p.i);
    t[13] = (m.p.j);
    t[14] = (m.p.k);
    t[15] = 1;
}

inline void ViewToModel() {
    double t[16];
    t[0] = model.r[0] * GFX_SCALE;     //possible performance hit?!?!
    t[1] = model.r[1] * GFX_SCALE;
    t[2] = model.r[2] * GFX_SCALE;
    t[3] = 0;
    t[4] = model.r[3] * GFX_SCALE;
    t[5] = model.r[4] * GFX_SCALE;
    t[6] = model.r[5] * GFX_SCALE;
    t[7] = 0;
    t[8] = model.r[6] * GFX_SCALE;
    t[9] = model.r[7] * GFX_SCALE;
    t[10] = model.r[8] * GFX_SCALE;
    t[11] = 0;
    t[12] = (model.p.i - view.p.i) * GFX_SCALE;
    t[13] = (model.p.j - view.p.j) * GFX_SCALE;
    t[14] = (model.p.k - view.p.k) * GFX_SCALE;
    t[15] = 1;
    //MatrixToDoubles (t,model);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(t);
}

static void IdentityFloat(float id[]) {
    id[0] = id[5] = id[10] = id[15] = 1;
    id[1] = id[2] = id[3] = id[4] = id[6] = id[7] = id[8] = id[9] = id[11] = id[12] = id[13] = id[14] = 0;
}

void MultFloatMatrix(float dest[], const float m1[], const Matrix &m2) {
    dest[0] = m1[0] * m2.r[0] + m1[4] * m2.r[1] + m1[8] * m2.r[2];
    dest[1] = m1[1] * m2.r[0] + m1[5] * m2.r[1] + m1[9] * m2.r[2];
    dest[2] = m1[2] * m2.r[0] + m1[6] * m2.r[1] + m1[10] * m2.r[2];
    dest[3] = m1[3] * m2.r[0] + m1[7] * m2.r[1] + m1[11] * m2.r[2];

    dest[4] = m1[0] * m2.r[3] + m1[4] * m2.r[4] + m1[8] * m2.r[5];
    dest[5] = m1[1] * m2.r[3] + m1[5] * m2.r[4] + m1[9] * m2.r[5];
    dest[6] = m1[2] * m2.r[3] + m1[6] * m2.r[4] + m1[10] * m2.r[5];
    dest[7] = m1[3] * m2.r[3] + m1[7] * m2.r[4] + m1[11] * m2.r[5];

    dest[8] = m1[0] * m2.r[6] + m1[4] * m2.r[7] + m1[8] * m2.r[8];
    dest[9] = m1[1] * m2.r[6] + m1[5] * m2.r[7] + m1[9] * m2.r[8];
    dest[10] = m1[2] * m2.r[6] + m1[6] * m2.r[7] + m1[10] * m2.r[8];
    dest[11] = m1[3] * m2.r[6] + m1[7] * m2.r[7] + m1[11] * m2.r[8];

    dest[12] = m1[0] * m2.p.i + m1[4] * m2.p.j + m1[8] * m2.p.k + m1[12];
    dest[13] = m1[1] * m2.p.i + m1[5] * m2.p.j + m1[9] * m2.p.k + m1[13];
    dest[14] = m1[2] * m2.p.i + m1[6] * m2.p.j + m1[10] * m2.p.k + m1[14];
    dest[15] = m1[3] * m2.p.i + m1[7] * m2.p.j + m1[11] * m2.p.k + m1[15];
}

static void RotateFloatMatrix(float dest[], const float m1[], const Matrix &m2) {
    dest[0] = (m1[0] * m2.r[0] + m1[4] * m2.r[1] + m1[8] * m2.r[2]);
    dest[1] = (m1[1] * m2.r[0] + m1[5] * m2.r[1] + m1[9] * m2.r[2]);
    dest[2] = (m1[2] * m2.r[0] + m1[6] * m2.r[1] + m1[10] * m2.r[2]);
    dest[3] = (m1[3] * m2.r[0] + m1[7] * m2.r[1] + m1[11] * m2.r[2]);

    dest[4] = (m1[0] * m2.r[3] + m1[4] * m2.r[4] + m1[8] * m2.r[5]);
    dest[5] = (m1[1] * m2.r[3] + m1[5] * m2.r[4] + m1[9] * m2.r[5]);
    dest[6] = (m1[2] * m2.r[3] + m1[6] * m2.r[4] + m1[10] * m2.r[5]);
    dest[7] = (m1[3] * m2.r[3] + m1[7] * m2.r[4] + m1[11] * m2.r[5]);

    dest[8] = (m1[0] * m2.r[6] + m1[4] * m2.r[7] + m1[8] * m2.r[8]);
    dest[9] = (m1[1] * m2.r[6] + m1[5] * m2.r[7] + m1[9] * m2.r[8]);
    dest[10] = (m1[2] * m2.r[6] + m1[6] * m2.r[7] + m1[10] * m2.r[8]);
    dest[11] = (m1[3] * m2.r[6] + m1[7] * m2.r[7] + m1[11] * m2.r[8]);

    dest[12] = m1[12];
    dest[13] = m1[13];
    dest[14] = m1[14];
    dest[15] = m1[15];
}

void ConstructAndLoadProjection() {
    float t[16];
    RotateFloatMatrix(t, projection, view);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(t);
}

void /*GLDRVAPI*/ GFXTranslateView(const QVector &a) {
    view.p += TransformNormal(view, a);
    //glPopMatrix();
    //glLoadIdentity();
    //glTranslatef(-view[12],-view[13],-view[14]);
    //glPushMatrix();
    ViewToModel();
}

void /*GFXDRVAPI*/ GFXTranslateModel(const QVector &a) {
    model.p += TransformNormal(model, a);
    ViewToModel();
}

void /*GFXDRVAPI*/ GFXTranslateProjection(const Vector &a) {
    projection[12] += a.i * projection[0] + a.j * projection[4] + a.k * projection[8];
    projection[13] += a.i * projection[1] + a.j * projection[5] + a.k * projection[9];
    projection[14] += a.i * projection[2] + a.j * projection[6] + a.k * projection[10];
    ConstructAndLoadProjection();
}

void /*GFXDRVAPI*/ GFXMultMatrixModel(const Matrix &matrix) {
    Matrix t;
    MultMatrix(t, model, matrix);
    CopyMatrix(model, t);
    ViewToModel();
}

//Matrix *mm = model;
//Matrix *vv = view;

void GFXLoadMatrixView(const Matrix &matrix) {
    CopyMatrix(view, matrix);
    ViewToModel();
    ConstructAndLoadProjection();
}

void /*GFXDRVAPI*/ GFXLoadMatrixModel(const Matrix &matrix) {
    CopyMatrix(model, matrix);
    ViewToModel();
}

void /*GFXDRVAPI*/ GFXLoadMatrixProjection(const float matrix[16]) {
    memcpy(projection, matrix, 16 * sizeof(float));
    ConstructAndLoadProjection();
}

void /*GFXDRVAPI*/ GFXViewPort(int minx, int miny, int maxx, int maxy) {
    glViewport(minx, miny, maxx, maxy);
}

void /*GFXDRVAPI*/ GFXCenterCamera(bool Enter) {
    static QVector tmp;
    if (Enter) {
        tmp = view.p;
        view.p.Set(0, 0, 0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    } else {
        view.p = tmp;
        GFXLoadIdentity(MODEL);
    }
}

void GFXRestoreHudMode() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

void GFXHudMode(const bool Enter) {
    if (Enter) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
    } else {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void /*GFXDRVAPI*/ GFXLoadIdentity(const MATRIXMODE mode) {
    switch (mode) {
        case MODEL:
            Identity(model);
            glMatrixMode(GL_MODELVIEW);
            //glLoadMatrixf(transview);
            glLoadIdentity();
            glTranslated(-view.p.i * GFX_SCALE, -view.p.j * GFX_SCALE, -view.p.k * GFX_SCALE);
            glScalef(GFX_SCALE, GFX_SCALE, GFX_SCALE);
            break;
        case PROJECTION:
            IdentityFloat(projection);
            ConstructAndLoadProjection();
            break;
        case VIEW:
            Identity(view);
            ViewToModel();
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(projection);
            break;
    }
}

void /*GFXDRVAPI*/ GFXGetMatrixView(Matrix &matrix) {
    CopyMatrix(matrix, view);
}

void /*GFXDRVAPI*/ GFXGetMatrixModel(Matrix &matrix) {
    CopyMatrix(matrix, model);
}

static void gl_Frustum(float left, float right, float bottom, float top, float nearval, float farval) {
    GFXGetFrustumVars(false, &left, &right, &bottom, &top, &nearval, &farval);
    GFXFrustum(projection, invprojection, left, right, bottom, top, nearval, farval);
}

void GFXFrustum(float *m, float *i, float left, float right, float bottom, float top, float nearval, float farval) {
    GLfloat x, y, a, b, c, d;
    x = (((float) 2.0) * nearval) / (right - left);
    y = (((float) 2.0) * nearval) / (top - bottom);
    a = (right + left) / (right - left);
    b = (top + bottom) / (top - bottom);
    //If farval == 0, we'll build an infinite-farplane projection matrix.
    if (farval == 0) {
        c = -1.0;
        d = -1.99 * nearval;         //-2*nearval, but using exactly -2 might create artifacts
    } else {
        c = -(farval + nearval) / (farval - nearval);
        d = -(((float) 2.0) * farval * nearval) / (farval - nearval);
    }
#define M(row, col) m[col*4+row]
    M(0, 0) = x;
    M(0, 1) = 0.0F;
    M(0, 2) = a;
    M(0, 3) = 0.0F;
    M(1, 0) = 0.0F;
    M(1, 1) = y;
    M(1, 2) = b;
    M(1, 3) = 0.0F;
    M(2, 0) = 0.0F;
    M(2, 1) = 0.0F;
    M(2, 2) = c;
    M(2, 3) = d;
    M(3, 0) = 0.0F;
    M(3, 1) = 0.0F;
    M(3, 2) = -1.0F;
    M(3, 3) = 0.0F;
#undef M
#define M(row, col) i[col*4+row]
    M(0, 0) = 1. / x;
    M(0, 1) = 0.0F;
    M(0, 2) = 0.0F;
    M(0, 3) = a / x;
    M(1, 0) = 0.0F;
    M(1, 1) = 1. / y;
    M(1, 2) = 0.0F;
    M(1, 3) = b / y;
    M(2, 0) = 0.0F;
    M(2, 1) = 0.0F;
    M(2, 2) = 0.0F;
    M(2, 3) = -1.0F;
    M(3, 0) = 0.0F;
    M(3, 1) = 0.0F;
    M(3, 2) = 1.F / d;
    M(3, 3) = (float) c / d;
#undef M
}

void /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar, float cockpit_offset) {
    znear *= GFX_SCALE;
    zfar *= GFX_SCALE;
    cockpit_offset *= GFX_SCALE;
    //gluPerspective (fov,aspect,znear,zfar);

    float xmin, xmax, ymin, ymax;

    ymax = znear * tanf(fov * M_PI / ((float) 360.0));       //78.0 --> 4.7046

    ymin = -ymax;     //-4.7046

    xmin = (ymin - cockpit_offset / 2) * aspect;       //-6.2571
    xmax = (ymax + cockpit_offset / 2) * aspect;       //6.2571
    ymin -= cockpit_offset;
    gl_Frustum(xmin, xmax, ymin, ymax, znear, zfar);
    ConstructAndLoadProjection();
}

void /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float nearval, float farval) {
    float *m = projection, x, y, z, tx, ty, tz;
    x = 2.0 / (right - left);
    y = 2.0 / (top - bottom);
    z = -2.0 / (farval - nearval);
    tx = -(right + left) / (right - left);
    ty = -(top + bottom) / (top - bottom);
    tz = -(farval + nearval) / (farval - nearval);

#define M(row, col) m[col*4+row]
    M(0, 0) = x;
    M(0, 1) = 0.0F;
    M(0, 2) = 0.0F;
    M(0, 3) = tx;
    M(1, 0) = 0.0F;
    M(1, 1) = y;
    M(1, 2) = 0.0F;
    M(1, 3) = ty;
    M(2, 0) = 0.0F;
    M(2, 1) = 0.0F;
    M(2, 2) = z;
    M(2, 3) = tz;
    M(3, 0) = 0.0F;
    M(3, 1) = 0.0F;
    M(3, 2) = 0.0F;
    M(3, 3) = 1.0F;
#undef M
    GFXLoadMatrixProjection(projection);
    GFXGetFrustumVars(false, &left, &right, &bottom, &top, &nearval, &farval);
}

static void LookAtHelper(float eyex,
        float eyey,
        float eyez,
        double centerx,
        double centery,
        double centerz,
        float upx,
        float upy,
        float upz) {
    //Matrix m;
    double x[3], y[3], z[3];
    double mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = eyex;
    z[1] = eyey;
    z[2] = eyez;
    mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
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
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Recompute Y = Z cross X */
    //y[0] =  x[1]*z[2] - x[2]*z[1];
    //y[1] = -x[0]*z[2] + x[2]*z[0];
    //y[2] =  x[0]*z[1] - x[1]*z[0];
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */

    mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }
    mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }
#define M(row, col) view.r[col*3+row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];                                  //M(0,3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];                                  //M(1,3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];                                  //M(2,3) = 0.0;
#undef M
    //M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;

    //Matrix tm;
    //Identity(tm);
    view.p.i = centerx + eyex;
    view.p.j = centery + eyey;
    view.p.k = centerz + eyez;
    //CopyMatrix (view,m);
    //MultMatrix(view, m, tm);
}

void /*GFXDRVAPI*/ GFXLookAt(Vector eye, QVector center, Vector up) {
    LookAtHelper(eye.i, eye.j, eye.k, center.i, center.j, center.k, up.i, up.j, up.k);
    GFXLoadMatrixView(view);
}

#ifdef SELF_TEST
int main()
{
    float  m1[16];
    double m2[16];
    Matrix m3[16];
    float  res[16];

    return 0;
}

#endif

#if 0
void GFXMultMatrixView( const &Matrix )
{
    const int MULTMATRIXVIEWNOTIMPLEMENTED = 0;
    assert( MULTMATRIXVIEWNOTIMPLEMENTED );
    MultMatrix( t, view, matrix );
    CopyMatrix( view, t );
    ConstructAndLoadProjection();
    glMatrixMode( GL_MODELVIEW );
    //glPopMatrix();
    //glLoadIdentity();
    //glTranslatef(-centerx,-centery,-centerz);
    //glPushMatrix();
    ViewToModel( true );
    glLoadMatrixf( model );
    ViewToModel( false );
}
#endif

#if 0
LOOKATHELPER under MultMatrix( view, m, tm );
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
#error

//Vector (centerx,centery,centerz).Cross (Vector (1,0,0));  DID NOT TRANSFORM THE ORIENTATION VECTOR TO REVERSE CAMERASPACE
Vector axis( centerx, centery, centerz );
Vector cubemapincamspace( eyex+centerx, eyey+centery, eyez+centerz );
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
#endif

