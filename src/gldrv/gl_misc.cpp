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
#include <string.h>
#define GL_MISC_CPP
#include "gl_globals.h"
#undef GL_MISC_CPP
#include "vegastrike.h"
#include "gfxlib.h"
#include "vs_globals.h"
#include "gl_light.h"
#include "config_xml.h"
#include "winsys.h"
extern GFXBOOL bTex0;
extern GFXBOOL bTex1;

bool GFXMultiTexAvailable() {
    return gl_options.Multitexture!=0;
}
void GFXCircle (float x, float y, float wid, float hei) {
	static float aaccuracy=XMLSupport::parse_float(vs_config->getVariable("graphics","circle_accuracy","100"));
	int accuracy = aaccuracy*(wid>hei?wid:hei);
	if (accuracy<8)
		accuracy=8;
	//	const int accuracy=((wid*g_game.x_resolution)+(hei*g_game.y_resolution))*M_PI;
	GFXBegin (GFXLINESTRIP);
	for (int i=0;i<=accuracy;i++) {
		GFXVertex3f (x+wid*cos (i*2.*M_PI/accuracy),y+hei*sin (i*2.*M_PI/accuracy),0);
	}
	GFXEnd ();

}

void /*GFXDRVAPI*/ GFXBeginScene()
{
	GFXLoadIdentity(MODEL); // bad this should instead load the cached view matrix
	light_rekey_frame();
}

void /*GFXDRVAPI*/ GFXEndScene()
{
        glFlush();
	winsys_swap_buffers(); //swap the buffers
	
}

void /*GFXDRVAPI*/ GFXClear(const GFXBOOL colorbuffer)
{
	glClear((colorbuffer?GL_COLOR_BUFFER_BIT:0) | GL_DEPTH_BUFFER_BIT);
}

GFXBOOL /*GFXDRVAPI*/ GFXCapture(char *filename)
{
	return GFXFALSE;
}
static float last_factor=0;
static float last_units=0;
void GFXGetPolygonOffset(float *factor, float *units) {
  *factor = last_factor;
  *units=last_units;
}
void /*GFXDRVAPI*/ GFXPolygonOffset (float factor, float units) {
  last_factor=factor;
  last_units=units;
  if (!factor&&!units) {
    glDisable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (0,0);
  } else {
    glPolygonOffset (factor*4,units*4);
    glEnable (GL_POLYGON_OFFSET_FILL);
  }


}
void GFXPointSize (const float size) {
  glPointSize (size);
}
void /*GFXDRVAPI*/ GFXBegin(const enum POLYTYPE ptype)
{
	GLenum mode;
	switch(ptype)
	{
	case GFXTRI:
	  mode = GL_TRIANGLES;
	  break;
	case GFXLINE:
	  mode = GL_LINES;
	  break;
	case GFXTRISTRIP:
	  mode = GL_TRIANGLE_STRIP;
	  break;
	case GFXTRIFAN:
	  mode = GL_TRIANGLE_FAN;
	  break;
	case GFXQUAD:
	  mode = GL_QUADS;
	  break;
	case GFXQUADSTRIP:
	  mode = GL_QUAD_STRIP;
	  break;
	case GFXLINESTRIP:
	  mode = GL_LINE_STRIP;
	  break;
	case GFXPOLY:
	  mode = GL_POLYGON;
	  break;
	case GFXPOINT:
	  mode = GL_POINTS;
	  break;
	}
	glBegin(mode);


}
void /*GFXDRVAPI*/ GFXColorf (const GFXColor & col) {
  glColor4fv (&col.r);
}
#if 0
//HELL slow on the TNT...we can't have it
void /*GFXDRVAPI*/ GFXBlendColor (const GFXColor &col) {
#ifndef WIN32
  glBlendColor (col.r,col.g,col.b,col.a);
#endif
}
#endif

void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a)
{
	glColor4f(r,g,b,a);
}

void /*GFXDRVAPI*/ GFXTexCoord2f(const float s, const float t)
{
	glTexCoord2f(s,t);
}

void /*GFXDRVAPI*/ GFXTexCoord4f(const float s, const float t, const float u, const float v)
{
#if !defined(IRIX)
	if(gl_options.Multitexture)
	{
		glMultiTexCoord2fARB_p(GL_TEXTURE0_ARB, s,t);
		glMultiTexCoord2fARB_p(GL_TEXTURE1_ARB, u,v);
	}
	else
#endif
	{
		glTexCoord2f(s,t);
	}
}

void /*GFXDRVAPI*/ GFXNormal3f(const float i, const float j, const float k)
{
  glNormal3f(i,j,k);
}

void /*GFXDRVAPI*/ GFXNormal(const Vector &n)
{
  glNormal3fv(&n.i);
}

void /*GFXDRVAPI*/ GFXVertex3f(const float x, const float y, const float z)
{
  glVertex3f(x,y,z);
}
void /*GFXDRVAPI*/ GFXVertex3f(const double x, const double y, const double z)
{
  glVertex3d(x,y,z);
}
void /*GFXDRVAPI*/ GFXVertex3d(const double x, const double y, const double z)
{
  glVertex3d(x,y,z);
}
void GFXVertexf (const Vector &v) {
  glVertex3f(v.i,v.j,v.k);
}
void GFXVertexf (const QVector &v) {
  glVertex3d(v.i,v.j,v.k);
}
void /*GFXDRVAPI*/ GFXEnd()
{
	glEnd();
}

int GFXCreateList() {
  glGetError();
  int list = glGenLists(1);
  glNewList(list, GL_COMPILE);
  return list;
}

GFXBOOL GFXEndList() {
  glEndList();
  return (glGetError ()!= GL_OUT_OF_MEMORY);
}

void GFXCallList(int list) {
//	VSFileSystem::Fprintf (stderr,"CallListStart");///causes crash with GF2 privaledge instruction on Win2k in certain instances
//	fflush (stderr);
  glCallList(list);
//	VSFileSystem::Fprintf (stderr,"CallListEnd");
//	fflush (stderr);
  
}

void GFXDeleteList (int list) {
  if (glIsList(list)) {
    glDeleteLists (list,1);
  }
}

void GFXSubwindow(int x, int y, int xsize, int ysize) {
  glViewport(x,y,xsize,ysize);
  glScissor(x,y,xsize,ysize);
  if(x==0&&y==0&&xsize==g_game.x_resolution&&ysize==g_game.y_resolution) {
    glDisable(GL_SCISSOR_TEST);
  } else {
    glEnable(GL_SCISSOR_TEST);
  }
}

void GFXSubwindow(float x, float y, float xsize, float ysize) {
  GFXSubwindow(int(x*g_game.x_resolution), int(y*g_game.y_resolution), int(xsize*g_game.x_resolution), int(ysize*g_game.y_resolution));
}

Vector GFXDeviceToEye(int x, int y) {
  float l, r, b, t , n, f;
  GFXGetFrustumVars (true,&l,&r,&b,&t,&n,&f);
  return Vector ((l + (r-l) * float(x)/g_game.x_resolution),
		 (t + (b-t) * float(y)/g_game.y_resolution),
		 n);
}
