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
#include "gl_globals.h"

#include "vegastrike.h"
#include "gfxlib.h"
#include "vs_globals.h"
#include "gl_light.h"
extern GFXBOOL bTex0;
extern GFXBOOL bTex1;



void /*GFXDRVAPI*/ GFXBeginScene()
{
	GFXClear(GFXFALSE);
	GFXLoadIdentity(MODEL); // bad this should instead load the cached view matrix
	light_rekey_frame();
}

void /*GFXDRVAPI*/ GFXEndScene()
{
        glFlush();
	glutSwapBuffers(); //swap the buffers
	
}

void /*GFXDRVAPI*/ GFXClear(GFXBOOL colorbuffer)
{
	glClear((colorbuffer?GL_COLOR_BUFFER_BIT:0) | GL_DEPTH_BUFFER_BIT);
}

GFXBOOL /*GFXDRVAPI*/ GFXCapture(char *filename)
{
	return GFXFALSE;
}

void /*GFXDRVAPI*/ GFXBegin(enum PRIMITIVE ptype)
{
	GLenum mode;
	switch(ptype)
	{
	case TRIANGLES:
#ifdef STATS_QUEUE
	        statsqueue.back() += GFXStats(1, 0, 0);
#endif
		mode = GL_TRIANGLES;
		break;
	case TRIANGLE_STRIP:
		mode = GL_TRIANGLE_STRIP;
		break;
	case TRIANGLE_FAN:
		mode = GL_TRIANGLE_FAN;
		break;
	case QUADS:
#ifdef STATS_QUEUE
		statsqueue.back() += GFXStats(0, 1, 0);
#endif
		mode = GL_QUADS;
		break;
	case POLYGON:
		mode = GL_POLYGON;
		break;
	}
	glBegin(mode);


}

void /*GFXDRVAPI*/ GFXColor4f(float r, float g, float b, float a)
{
	glColor4f(r,g,b,a);
}

void /*GFXDRVAPI*/ GFXTexCoord2f(float s, float t)
{
	glTexCoord2f(s,t);
}

void /*GFXDRVAPI*/ GFXTexCoord4f(float s, float t, float u, float v)
{
	if(g_game.Multitexture)
	{
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, s,t);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u,v);
	}
	else {
		glTexCoord2f(s,t);
	}
}

void /*GFXDRVAPI*/ GFXNormal3f(float i, float j, float k)
{
	glNormal3f(i,j,k);
}

void /*GFXDRVAPI*/ GFXNormal(Vector n)
{
	glNormal3f(n.i,n.j,n.k);
}

void /*GFXDRVAPI*/ GFXVertex3f(float x, float y, float z)
{
	glVertex3f(x,y,z);
}

void /*GFXDRVAPI*/ GFXEnd()
{
	glEnd();
}

int GFXCreateList() {
  int list = glGenLists(1);
  glNewList(list, GL_COMPILE);
  return list;
}

GFXBOOL GFXEndList() {
  glEndList();
  return (glGetError ()!= GL_OUT_OF_MEMORY);
}

void GFXCallList(int list) {
  glCallList(list);
}

void GFXDeleteList (int list) {
    glDeleteLists (list,1);
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
  //fprintf (stderr, "N: %f\n",n);
  //Vector mousePoint (MouseCoordinate (mouseX,mouseY,1));
  /*  cerr.form("%f, %f, %f, %f\n", l,r,b,t);
  cerr << "top - bottom " <<  t-b << endl;
  cerr << "right - left " <<  r-l << endl;
  */
  return Vector ((l + (r-l) * float(x)/g_game.x_resolution),
		 (t + (b-t) * float(y)/g_game.y_resolution),
		 n);
}
