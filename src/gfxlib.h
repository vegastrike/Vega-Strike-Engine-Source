/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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

#ifndef _GFXLIB_H
#define _GFXLIB_H
/**
#ifdef GLDRV_EXPORTS
#define GFXDRVAPI  __declspec(dllexport)
#else
#define GFXDRVAPI  __declspec(dllimport)
#endif
*/
#include <GL/gl.h>
typedef int BOOL;
#define TRUE 1
#define FALSE 0



#include "gfx_transform_vector.h"

#include "gfxlib_struct.h"

//Init functions
BOOL /*GFXDRVAPI*/ GFXInit(int, char **);
BOOL /*GFXDRVAPI*/ GFXLoop(void main_loop ());
BOOL /*GFXDRVAPI*/ GFXShutdown();

//Misc functions
BOOL /*GFXDRVAPI*/ GFXBeginScene();
BOOL /*GFXDRVAPI*/ GFXEndScene();
BOOL /*GFXDRVAPI*/ GFXClear();

//Light
BOOL /*GFXDRVAPI*/ GFXEnableLight(int light);
BOOL /*GFXDRVAPI*/ GFXDisableLight(int light);

//BOOL GFXSetPower(int light, float power);
BOOL /*GFXDRVAPI*/ GFXSetLightPosition(int light, const Vector &position);
BOOL /*GFXDRVAPI*/ GFXSetLightDirection(int light, const Vector &direction);
BOOL /*GFXDRVAPI*/ GFXSetLightColor(int light, const GFXColor &color);
BOOL /*GFXDRVAPI*/ GFXSetLightColor(int light, const float color[4]);

//Materials
BOOL /*GFXDRVAPI*/ GFXSetMaterial(int number, const GFXMaterial &material);
BOOL /*GFXDRVAPI*/ GFXGetMaterial(int number, GFXMaterial &material);
BOOL /*GFXDRVAPI*/ GFXSelectMaterial(int number);

//Matrix
BOOL /*GFXDRVAPI*/ GFXMultMatrix(MATRIXMODE mode, Matrix matrix);
BOOL /*GFXDRVAPI*/ GFXLoadMatrix(MATRIXMODE mode, Matrix matrix);
BOOL /*GFXDRVAPI*/ GFXLoadIdentity(MATRIXMODE mode);
BOOL /*GFXDRVAPI*/ GFXGetMatrix(MATRIXMODE mode, Matrix matrix);
float /*GFXDRVAPI*/GFXGetXInvPerspective();
float /*GFXDRVAPI*/GFXGetYInvPerspective();
BOOL /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar);
BOOL /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float znear, float zfar);
BOOL /*GFXDRVAPI*/ GFXLookAt(Vector eye, Vector center, Vector up);
BOOL /*GFXDRVAPI*/ GFXGetFrustum (float f[6][4]);
BOOL /*GFXDRVAPI*/ GFXCalculateFrustum();
BOOL /*GFXDRVAPI*/ GFXCalculateFrustum(float frustum[6][4],float *modlmatrix, float *projection);
BOOL /*GFXDRVAPI*/ GFXGetFrustumVars (bool, float *l,float *r, float *b, float *t, float *n, float *f);
float /*GFXDRVAPI*/ GFXSphereInFrustum(const Vector &Center, float Radius);
float /*GFXDRVAPI*/ GFXSphereInFrustum(float f[6][4],const Vector &Center, float Radius);
BOOL /*GFXDRVAPI*/ GFXFrustum (float * mat, float *inv, float left,float right, float bottom, float top, float nearval, float farval);

//Textures

BOOL /*GFXDRVAPI*/ GFXCreateTexture(int width, int height, TEXTUREFORMAT textureformat, int *handle, char *palette = 0, int texturestage = 0, enum TEXTURE_TARGET texture_target = TEXTURE2D);
BOOL /*GFXDRVAPI*/ GFXPrioritizeTexture (unsigned int handle, float priority);
BOOL /*GFXDRVAPI*/ GFXAttachPalette(unsigned char *palette, int handle);
BOOL /*GFXDRVAPI*/ GFXTransferTexture(unsigned char *buffer, int handle, enum TEXTURE_IMAGE_TARGET image2D=TEXTURE_2D);
BOOL /*GFXDRVAPI*/ GFXDeleteTexture(int handle);
BOOL /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage=0);


//Light
BOOL /*GFXDRVAPI*/ GFXEnableLight(int light);
BOOL /*GFXDRVAPI*/ GFXDisableLight(int light);

BOOL /*GFXDRVAPI*/ GFXSetLightPosition(int light, const Vector &position);
BOOL /*GFXDRVAPI*/ GFXSetLightDirection(int light, const Vector &direction);
BOOL /*GFXDRVAPI*/ GFXSetLightColor(int light, const GFXColor &color);
BOOL /*GFXDRVAPI*/ GFXSetLightColor(int light, const float num[4]);
//BOOL GFXSetSpecular(int light, const float &(num[4=));


//Screen capture
BOOL /*GFXDRVAPI*/ GFXCapture(char *filename);

//State
BOOL /*GFXDRVAPI*/ GFXEnable(enum STATE);
BOOL /*GFXDRVAPI*/ GFXDisable(enum STATE);
BOOL /*GFXDRVAPI*/ GFXTextureAddressMode(ADDRESSMODE);
BOOL /*GFXDRVAPI*/ GFXBlendMode(enum BLENDFUNC src, enum BLENDFUNC dst);
BOOL /*GFXDRVAPI*/ GFXPushBlendMode();
BOOL /*GFXDRVAPI*/ GFXPopBlendMode();
BOOL /*GFXDRVAPI*/ GFXSelectTexcoordSet(int stage, int texset);

//primitive Begin/End 
BOOL /*GFXDRVAPI*/ GFXBegin(enum PRIMITIVE);
void /*GFXDRVAPI*/ GFXColor4f(float r, float g, float b, float a = 1.0);
void /*GFXDRVAPI*/ GFXTexCoord2f(float s, float t);
void /*GFXDRVAPI*/ GFXTexCoord4f(float s, float t, float u, float v);
void /*GFXDRVAPI*/ GFXNormal3f(float i, float j, float k);
void /*GFXDRVAPI*/ GFXNormal(Vector n);
void /*GFXDRVAPI*/ GFXVertex3f(float x, float y, float z = 1.0);
BOOL /*GFXDRVAPI*/ GFXEnd();

//display list
int GFXCreateList();
void GFXEndList();
void GFXCallList(int list);

#endif
