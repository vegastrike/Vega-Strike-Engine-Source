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
#include <GL/glut.h>
#include <vector>
#ifndef GFXBOOL
#define GFXBOOL unsigned char
#endif
#define GFXTRUE 1
#define GFXFALSE 0


using std::vector;

#include "gfx_transform_vector.h"

#include "gfxlib_struct.h"

//Init functions
void  /*GFXDRVAPI*/ GFXInit(int, char **);
void /*GFXDRVAPI*/ GFXLoop(void main_loop ());
void /*GFXDRVAPI*/ GFXShutdown();

//Misc functions
void /*GFXDRVAPI*/ GFXBeginScene();
void /*GFXDRVAPI*/ GFXEndScene();
void /*GFXDRVAPI*/ GFXClear();

//Light

void /*GFXDRVAPI*/ GFXCreateLightContext(int &con_number);
void /*GFXDRVAPI*/ GFXDeleteLightContext(int con_number);
void /*GFXDRVAPI*/ GFXSetLightContext(int con_number);
GFXBOOL /*GFXDRVAPI*/ GFXLightContextAmbient (const GFXColor &amb);

void /*GFXDRVAPI*/ GFXPickLights(const float *);
GFXBOOL /*GFXDRVAPI*/ GFXSetSeparateSpecularColor (GFXBOOL spec);
GFXBOOL /*GFXDRVAPI*/ GFXSetCutoff (float cutoff);
void /*GFXDRVAPI*/ GFXSetOptimalIntensity (float newint, float saturatevalue);
GFXBOOL /*GFXDRVAPI*/ GFXSetOptimalNumLights (int numlights);

void /*GFXDRVAPI*/ GFXCreateLight (int &light);
void /*GFXDRVAPI*/ GFXDeleteLight (int light);
GFXBOOL /*GFXDRVAPI*/ GFXEnableLight(int light);
GFXBOOL /*GFXDRVAPI*/ GFXDisableLight(int light);

GFXBOOL /*GFXDRVAPI*/ GFXSetPower(int light, float power);
GFXBOOL /*GFXDRVAPI*/ GFXSetLight(int light, enum LIGHT_TARGET, const GFXColor &color);








//Materials
void /*GFXDRVAPI*/ GFXSetMaterial(unsigned int &number, const GFXMaterial &material);
void /*GFXDRVAPI*/ GFXModifyMaterial (unsigned int number, const GFXMaterial &material);
GFXBOOL /*GFXDRVAPI*/ GFXGetMaterial(unsigned int number, GFXMaterial &material);
void /*GFXDRVAPI*/ GFXSelectMaterial(unsigned int number);

//Matrix
void /*GFXDRVAPI*/ GFXMultMatrix(MATRIXMODE mode, const Matrix matrix);
void  /*GFXDRVAPI*/ GFXLoadMatrix(MATRIXMODE mode, const Matrix matrix);
void /*GFXDRVAPI*/ GFXLoadIdentity(MATRIXMODE mode);
void /*GFXDRVAPI*/ GFXGetMatrix(MATRIXMODE mode, Matrix matrix);
float /*GFXDRVAPI*/GFXGetXInvPerspective();
float /*GFXDRVAPI*/GFXGetYInvPerspective();
void /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar);
void /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float znear, float zfar);
void /*GFXDRVAPI*/ GFXLookAt(Vector eye, Vector center, Vector up);
void /*GFXDRVAPI*/ GFXGetFrustum (float f[6][4]);
void /*GFXDRVAPI*/ GFXCalculateFrustum();
void /*GFXDRVAPI*/ GFXCalculateFrustum(float frustum[6][4],float *modlmatrix, float *projection);
void /*GFXDRVAPI*/ GFXGetFrustumVars (bool, float *l,float *r, float *b, float *t, float *n, float *f);
float /*GFXDRVAPI*/ GFXSphereInFrustum(const Vector &Center, float Radius);
float /*GFXDRVAPI*/ GFXSphereInFrustum(float f[6][4],const Vector &Center, float Radius);
void /*GFXDRVAPI*/ GFXFrustum (float * mat, float *inv, float left,float right, float bottom, float top, float nearval, float farval);

//Textures

GFXBOOL /*GFXDRVAPI*/ GFXCreateTexture(int width, int height, TEXTUREFORMAT textureformat, int *handle, char *palette = 0, int texturestage = 0, enum FILTER mipmap = MIPMAP, enum TEXTURE_TARGET texture_target = TEXTURE2D);
void /*GFXDRVAPI*/ GFXPrioritizeTexture (unsigned int handle, float priority);
void /*GFXDRVAPI*/ GFXAttachPalette(unsigned char *palette, int handle);
GFXBOOL /*GFXDRVAPI*/ GFXTransferTexture(unsigned char *buffer, int handle, enum TEXTURE_IMAGE_TARGET image2D=TEXTURE_2D);
void /*GFXDRVAPI*/ GFXDeleteTexture(int handle);
void /*GFXDRVAPI*/ GFXDestroyAllTextures();
void  /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage=0);



//Screen capture
GFXBOOL /*GFXDRVAPI*/ GFXCapture(char *filename);

//State
void /*GFXDRVAPI*/ GFXEnable(enum STATE);
void /*GFXDRVAPI*/ GFXDisable(enum STATE);
void /*GFXDRVAPI*/ GFXTextureAddressMode(ADDRESSMODE);
void /*GFXDRVAPI*/ GFXBlendMode(enum BLENDFUNC src, enum BLENDFUNC dst);
void /*GFXDRVAPI*/ GFXPushBlendMode();
void /*GFXDRVAPI*/ GFXPopBlendMode();
void /*GFXDRVAPI*/ GFXActiveTexture (int stage);
void /*GFXDRVAPI*/ GFXSelectTexcoordSet(int stage, int texset);

//primitive Begin/End 
void /*GFXDRVAPI*/ GFXBegin(enum PRIMITIVE);
void /*GFXDRVAPI*/ GFXColor4f(float r, float g, float b, float a = 1.0);
void /*GFXDRVAPI*/ GFXTexCoord2f(float s, float t);
void /*GFXDRVAPI*/ GFXTexCoord4f(float s, float t, float u, float v);
void /*GFXDRVAPI*/ GFXNormal3f(float i, float j, float k);
void /*GFXDRVAPI*/ GFXNormal(Vector n);
void /*GFXDRVAPI*/ GFXVertex3f(float x, float y, float z = 1.0);
void /*GFXDRVAPI*/ GFXEnd();

//display list
int GFXCreateList();
GFXBOOL GFXEndList();
void GFXCallList(int list);
void GFXDeleteList (int list);

//pick mode
void GFXBeginPick(int x, int y, int xsize, int ysize);
void GFXSetPickName(int name);
//bool GFXCheckPicked();
vector<PickData> *GFXEndPick();

//Subwindow management
void GFXSubwindow(int x, int y, int xsize, int ysize);
void GFXSubwindow(float x, float y, float xsize, float ysize);

//Coordinate system conversion
Vector GFXDeviceToEye(int x, int y);

#endif
