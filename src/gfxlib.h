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
#ifdef _WIN32
#ifdef GLDRV_EXPORTS
#define GFXDRVAPI  __declspec(dllexport)
#else
#define GFXDRVAPI  __declspec(dllimport)
#endif
#else
#define GFXDRVAPI
#endif

#include <GL/glut.h>
#include <vector>
#ifndef GFXBOOL
#define GFXBOOL unsigned char
#endif
#define GFXTRUE 1
#define GFXFALSE 0


using std::vector;

#include "gfx/vec.h"

#include "gfxlib_struct.h"

//Init functions
void  /*GFXDRVAPI*/ GFXInit(int, char **);
void /*GFXDRVAPI*/ GFXLoop(void main_loop ());
void /*GFXDRVAPI*/ GFXShutdown();

//Misc functions
void /*GFXDRVAPI*/ GFXBeginScene();
void /*GFXDRVAPI*/ GFXEndScene();
void /*GFXDRVAPI*/ GFXClear(const GFXBOOL ColorBuffer);

//Light

void /*GFXDRVAPI*/ GFXCreateLightContext(int &con_number);
void /*GFXDRVAPI*/ GFXDeleteLightContext(const int con_number);
void /*GFXDRVAPI*/ GFXSetLightContext(const int con_number);
GFXBOOL /*GFXDRVAPI*/ GFXLightContextAmbient (const GFXColor &amb);

void /*GFXDRVAPI*/ GFXPickLights(const Vector &center, const float radius);
GFXBOOL /*GFXDRVAPI*/ GFXSetSeparateSpecularColor (const GFXBOOL spec);
GFXBOOL /*GFXDRVAPI*/ GFXSetCutoff (const float cutoff);
void /*GFXDRVAPI*/ GFXSetOptimalIntensity (const float newint, const float saturatevalue);
GFXBOOL /*GFXDRVAPI*/ GFXSetOptimalNumLights (const int numlights);

GFXBOOL /*GFXDRVAPI*/ GFXCreateLight (int &light, const GFXLight &, const bool global);
void /*GFXDRVAPI*/ GFXDeleteLight (const int light);
GFXBOOL /*GFXDRVAPI*/ GFXEnableLight(const int light);
GFXBOOL /*GFXDRVAPI*/ GFXDisableLight(const int light);

GFXBOOL /*GFXDRVAPI*/ GFXSetLight(int light, enum LIGHT_TARGET, const GFXColor &color);
void /*GFXDRVAPI*/ GFXPushGlobalEffects ();
GFXBOOL /*GFXDRVAPI*/ GFXPopGlobalEffects();







//Materials
void /*GFXDRVAPI*/ GFXSetMaterial(unsigned int &number, const GFXMaterial &material);
void /*GFXDRVAPI*/ GFXModifyMaterial (const unsigned int number, const GFXMaterial &material);
GFXBOOL /*GFXDRVAPI*/ GFXGetMaterial(const unsigned int number, GFXMaterial &material);
void /*GFXDRVAPI*/ GFXSelectMaterial(const unsigned int number);

//Matrix
void /*GFXDRVAPI*/ GFXHudMode(const bool Enter);
void /*GFXDRVAPI*/ GFXTranslate(const MATRIXMODE mode, const Vector & r);
void /*GFXDRVAPI*/ GFXMultMatrix(const MATRIXMODE mode, const Matrix matrix);
void  /*GFXDRVAPI*/ GFXLoadMatrix(const MATRIXMODE mode, const Matrix matrix);
void /*GFXDRVAPI*/ GFXLoadIdentity(const MATRIXMODE mode);
void /*GFXDRVAPI*/ GFXGetMatrix(const MATRIXMODE mode, Matrix matrix);
float /*GFXDRVAPI*/ GFXGetZPerspective (const float z);
float /*GFXDRVAPI*/GFXGetXInvPerspective();
float /*GFXDRVAPI*/GFXGetYInvPerspective();
void /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar, float cockpit_offset);
void /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float znear, float zfar);
void /*GFXDRVAPI*/ GFXViewPort (int minx, int miny, int maxx, int maxy);
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
void /*GFXDRVAPI*/ GFXEnable(const enum STATE);
void /*GFXDRVAPI*/ GFXDisable(const enum STATE);
void /*GFXDRVAPI*/ GFXTextureAddressMode(const ADDRESSMODE);
void /*GFXDRVAPI*/ GFXBlendMode(const enum BLENDFUNC src, const enum BLENDFUNC dst);
void /*GFXDRVAPI*/ GFXPushBlendMode();
void /*GFXDRVAPI*/ GFXPopBlendMode();
void /*GFXDRVAPI*/ GFXActiveTexture (const int stage);
void /*GFXDRVAPI*/ GFXSelectTexcoordSet(int stage, int texset);
void /*GFXDRVALP*/ GFXAlphaTest (const enum DEPTHFUNC,const float ref);
//primitive Begin/End 
void /*GFXDRVAPI*/ GFXBegin(const enum POLYTYPE);
void /*GFXDRVAPI*/ GFXColorf (const GFXColor & col); 
void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a = 1.0);
void /*GFXDRVAPI*/ GFXTexCoord2f(const float s, const float t);
void /*GFXDRVAPI*/ GFXTexCoord4f(const float s, const float t, const float u, const float v);
void /*GFXDRVAPI*/ GFXNormal3f(const float i, const float j, const float k);
void /*GFXDRVAPI*/ GFXNormal(const Vector &n);
void /*GFXDRVAPI*/ GFXVertex3f(const float x, const float y, const float z = 1.0);
void /*GFXDRVAPI*/ GFXVertexf (const Vector &v);
void /*GFXDRVAPI*/ GFXEnd();
 
//display list
int /*GFXDRVAPI*/ GFXCreateList();
GFXBOOL /*GFXDRVAPI*/ GFXEndList();
void /*GFXDRVAPI*/ GFXCallList(int list);
void /*GFXDRVAPI*/ GFXDeleteList (int list);

//pick mode
void /*GFXDRVAPI*/ GFXBeginPick(int x, int y, int xsize, int ysize);
void /*GFXDRVAPI*/ GFXSetPickName(int name);
//bool GFXCheckPicked();
vector<PickData> * /*GFXDRVAPI*/ GFXEndPick();

//Subwindow management
void /*GFXDRVAPI*/ GFXSubwindow(int x, int y, int xsize, int ysize);
void /*GFXDRVAPI*/ GFXSubwindow(float x, float y, float xsize, float ysize);

//Coordinate system conversion
Vector /*GFXDRVAPI*/ GFXDeviceToEye(int x, int y);

#endif
