/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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
#ifndef _GL_GLOBALS_H_
#define _GL_GLOBALS_H_
#include <queue>

using namespace std;

#ifndef GFXSTAT
#define GFXSTAT
#ifdef STATS_QUEUE
#include <time.h>

struct GFXStats{
	int drawnTris;
	int drawnQuads;
	int drawnPoints;
  time_t ztime;
        GFXStats() {drawnTris = drawnQuads = drawnPoints = 0; time(&ztime);}
	GFXStats(int tri, int quad, int point) {drawnTris = tri; drawnQuads = quad; drawnPoints = point;}
	GFXStats &operator+=(const GFXStats &rval) { drawnTris+=rval.drawnTris; drawnQuads+=rval.drawnQuads; drawnPoints+=rval.drawnPoints; return *this;}
	int total() {return drawnTris*3+drawnQuads*4+drawnPoints;}
  int elapsedTime() {time_t t; time (&t); return (int)(t-ztime);}
};
#endif
#endif

#define MAX_NUM_LIGHTS 4
#define MAX_NUM_MATERIAL 4
#define TEXTURE_CUBE_MAP_ARB                0x8513

//extern Matrix model;
//extern Matrix view;
#if defined(__CYGWIN__)
#define GL_EXT_color_subtable 1
#endif

#ifndef _WIN32
// #define GL_GLEXT_PROTOTYPES

#endif
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#include <GL/gl.h>
#endif
#if defined(__APPLE__) || defined(MACOSX)
    #include <GLUT/glut.h>
#ifdef GL_INIT_CPP
#define GL_GLEXT_PROTOTYPES
#endif
    #include <OpenGL/glext.h>
#else
    #include <GL/glut.h>
#include "gl_undefined_extensions.h"
    #include <GL/glext.h>
#endif
#ifdef _WIN32
#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1
#define GL_TEXTURE_CUBE_MAP_ARB           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C
#define GL_TEXTURE_CUBE_MAP_EXT           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT  0x851C
#endif

extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB_p;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB_p;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB_p;
extern PFNGLCOLORTABLEEXTPROC glColorTable_p;
extern PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p;
extern PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;

//extern int sharedcolortable;
extern GLenum GFXStage0;
extern GLenum GFXStage1;
#ifdef STATS_QUEUE
extern queue<GFXStats> statsqueue;
#endif
extern int Stage0Texture;
extern int Stage0TextureName;
extern int Stage1Texture;
extern int Stage1TextureName;
typedef struct {
  int fullscreen;
  int Multitexture;
  int PaletteExt;
  int display_lists;
  int mipmap;//0 = nearest 1 = linear 2 = mipmap
  int color_depth;
  int cubemap;  
  int compression;
  char wireframe;
  char smooth_shade;
  int max_texture_dimension;
  bool s3tc;
} gl_options_t;
extern gl_options_t gl_options;

// Maximum number of things that can be returned in a pick operation
#define MAX_PICK 2048
#define GFX_SCALE 1./1024.
#endif
