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
#include "gfxlib.h"
#include <queue>
using namespace std;

#ifndef GFXSTAT
#define GFXSTAT
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

#define MAX_NUM_LIGHTS 4
#define MAX_NUM_MATERIAL 4
#define TEXTURE_CUBE_MAP_ARB                0x8513


//extern Matrix model;
//extern Matrix view;
#ifdef WIN32
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

extern PFNGLLOCKARRAYSSGIPROC glLockArraysEXT_p;
extern PFNGLUNLOCKARRAYSSGIPROC glUnlockArraysEXT_p;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCOLORTABLEEXTPROC glColorTable;
//extern PFNGLSELECTTEXTURESGISPROC glSelectTextureSGIS ;
//extern PFNGLMULTITEXCOORD2FSGISPROC glMultiTexCoord2fSGIS ;
//extern PFNGLMTEXCOORDPOINTERSGISPROC glMTexCoordPointerSGIS ;
#else
extern PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p;
extern PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;
#endif
//extern int sharedcolortable;
extern GLenum GFXStage0;
extern GLenum GFXStage1;
extern queue<GFXStats> statsqueue;
extern int Stage0Texture;
extern int Stage0TextureName;
extern int Stage1Texture;
extern int Stage1TextureName;



