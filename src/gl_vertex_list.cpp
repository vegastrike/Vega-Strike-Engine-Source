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
#include "vegastrike.h"
#include "gl_globals.h"
#include <assert.h>
// Untransformed and transformed data 
//LPDIRECT3DVERTEXBUFFER lpd3dvbUntransformed;
//LPDIRECT3DVERTEXBUFFER lpd3dvbTransformed;

//int numVertices;
//int numPolygons;
GFXVertexList *next;

extern BOOL bTex0;
extern BOOL bTex1;


#define USE_DISPLAY_LISTS
GFXVertexList::GFXVertexList():numVertices(0),myVertices(NULL),display_list(0), numlists(0) { }
/*
GFXVertexList::GFXVertexList (enum POLYTYPE poly, int numVertices, GFXVertex *vertices) {
  Init (&poly, numVertices, vertices, 1, &numVertices);
}
GFXVertexList::GFXVertexList(enum POLYTYPE *poly, int numVertices, GFXVertex *vertices, int numlists, int *offsets) // TODO: Add in features to accept flags for what's
{
  Init (poly,numVertices, vertices, numlists, offsets);
}
*/
void GFXVertexList::Init (enum POLYTYPE *poly, int numVertices, GFXVertex *vertices, int numlists, int *offsets, int tess) {
  mode = new GLenum [numlists];
  for (int pol=0;pol<numlists;pol++) {
    switch (poly[pol]) {
    case GFXTRI:
      mode[pol] = GL_TRIANGLES;
      break;
    case GFXQUAD:
      mode[pol] = GL_QUADS;
      break;
    case GFXTRISTRIP:
      mode[pol] = GL_TRIANGLE_STRIP;
      break;
    case GFXQUADSTRIP:
      mode[pol] = GL_QUAD_STRIP;
      break;
    case GFXTRIFAN:
      mode[pol] = GL_TRIANGLE_FAN;
      break;
    case GFXPOLY:
      mode[pol] = GL_POLYGON;
      break;
    case GFXLINE:
      mode[pol] = GL_LINES;
      break;
    case GFXLINESTRIP:
      mode[pol] = GL_LINE_STRIP;
      break;
    }
  }
  if (numlists==1) {
    offsets = new int[1];
    offsets[0]= numVertices;
  }
  assert (offsets!=NULL);
  
  this->numlists = numlists;
  this->numVertices = numVertices;
  myVertices = new GFXVertex[numVertices];
  memcpy(myVertices, vertices, sizeof(GFXVertex)*numVertices);
  this->offsets = new int [numlists];
  memcpy(this->offsets, offsets, sizeof(int)*numlists);
  display_list = 0;
  this->tessellation = tess;
  changed = true;
  if (tess) 
    Tess (tess);
  else {
    tesslist = NULL;
    RefreshDisplayList();
  }
  changed = false;//for display lists


}
void GFXVertexList::Tess (int tess) {
  if (tessellation ==tess)
    return;
  if (tesslist) {
    delete tesslist;
    tesslist = NULL;
  }
  if (tess==0) {
    RefreshDisplayList();
    tessellation = 0;
    return;
  }

  for(;tess>0;tess--) {
    


  }
}
void GFXVertexList::RefreshDisplayList () {
#ifdef USE_DISPLAY_LISTS
  if (display_list&&!changed) {
      return;
  }
  /*
	display_list = GFXCreateList();
	glVertexPointer(3, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
	glNormalPointer(GL_FLOAT, sizeof(GFXVertex), &myVertices[0].i);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	glClientActiveTextureARB (GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTextureARB (GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  		
	glDrawArrays(mode, 0, numVertices);
  */
	int a;
	int offset =0;
	for (int i=0;i<numlists;i++) {
	  glBegin(mode[i]);
	  for(a=0; a<offsets[i]; a++) {
	    glNormal3fv(&myVertices[offset+a].i);
	    glTexCoord2fv(&myVertices[offset+a].s);
	    glVertex3fv(&myVertices[offset+a].x);
	  }
	  offset +=offsets[i];
	  glEnd();
	}

	glEndList();
	/*
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTextureARB (GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	*/
#endif
}

GFXVertexList::~GFXVertexList()
{
  if (display_list)
    glDeleteLists (display_list, 1); //delete dis
  if (tessellation)
    delete tesslist;
  if (offsets)
    delete [] offsets;
  if(myVertices)
    delete [] myVertices;
}

GFXTVertex *GFXVertexList::LockTransformed()
{
  if (tessellation)
    tesslist->LockTransformed();
	return NULL;
} // Stuff to support environment mapping

void GFXVertexList::UnlockTransformed()
{
  if (tessellation)
    tesslist->UnlockTransformed();

}

GFXVertex *GFXVertexList::LockUntransformed()
{
  if (tessellation)
    tesslist->LockUntransformed();
  return myVertices;
}// Stuff to support environment mapping

void GFXVertexList::UnlockUntransformed()
{
  if (tessellation)
    tesslist->UnlockUntransformed();
}

BOOL GFXVertexList::SetNext(GFXVertexList *vlist)
{
  
	return FALSE;
}

BOOL GFXVertexList::SwapUntransformed()
{
  if (tessellation)
    tesslist->SwapUntransformed();
  
	return FALSE;
}

BOOL GFXVertexList::SwapTransformed()
{
  if (tessellation)
    tesslist->SwapTransformed();

	return FALSE;
}
BOOL GFXVertexList::Draw()
{
  if (tessellation&&tesslist) {
    tesslist->Draw();
    return TRUE;
  }
#ifdef STATS_QUEUE
  //  statsqueue.back() += GFXStats(numTriangles, numQuads, 0);
#endif
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
	if(g_game.Multitexture) {
	  glActiveTextureARB(GL_TEXTURE0_ARB);	
	  if(bTex0) 
	    glEnable (GL_TEXTURE_2D);		
	  else
	    glDisable(GL_TEXTURE_2D);
	  
	  glActiveTextureARB(GL_TEXTURE1_ARB);	
	  if(bTex1)
#ifdef NV_CUBE_MAP
	    glEnable (GL_TEXTURE_CUBE_MAP_EXT);////FIXME--have some gneeral state that holds CUBE MAPPING values
#else	
	  glEnable (GL_TEXTURE_2D);
#endif
	  else
	    glDisable(GL_TEXTURE_2D);
	  glActiveTextureARB(GL_TEXTURE0_ARB);
	} 
	GFXCallList(display_list);
  } else 
#endif
    {
    if(g_game.Multitexture) {
      glActiveTextureARB(GL_TEXTURE0_ARB);	
      if(bTex0) 
	glEnable (GL_TEXTURE_2D);		
      else
	glDisable(GL_TEXTURE_2D);
      
      glActiveTextureARB(GL_TEXTURE1_ARB);	
      if(bTex1)
#ifdef NV_CUBE_MAP
	glEnable (GL_TEXTURE_CUBE_MAP_EXT);////FIXME--have some gneeral state that holds CUBE MAPPING values
#else	
      glEnable (GL_TEXTURE_2D);
#endif	
      else
	glDisable(GL_TEXTURE_2D);
    }
#ifdef STATS_QUEUE
    //    statsqueue.back() += GFXStats(numTriangles, numQuads, 0);
#endif
    //float *texcoords = NULL;
    if (g_game.Multitexture){
      /*texcoords = new float[numVertices*4];
	for(int a=0; a<numVertices; a++) {
	texcoords[4*a] = myVertices[a].s;
	texcoords[4*a+1] = myVertices[a].t;
	texcoords[4*a+2] = myVertices[a].u;
	texcoords[4*a+3] = myVertices[a].v;
	}*/ 
      glVertexPointer(3, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].x);
      glNormalPointer(GL_FLOAT, sizeof(GFXVertex), &myVertices[0].i);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      
      glClientActiveTextureARB (GL_TEXTURE0_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
      
      glClientActiveTextureARB (GL_TEXTURE1_ARB);
      
      /*
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage1*2);
      */
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      int totoffset=0;
      for (int i=0;i<numlists;i++) {
	glDrawArrays(mode[i], totoffset, offsets[i]);
	totoffset += offsets[i];
      }
    }else{ 
      /*transfer vertex, texture coords, and normal pointer*/
      //GLenum err;
      glVertexPointer(3, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].x);
      glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
      glNormalPointer(GL_FLOAT, sizeof(GFXVertex), &myVertices[0].i);
      
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      
      if(bTex0) {
	int totoffset=0;
	for (int i=0;i<numlists;i++) {
	  glDrawArrays(mode[i], totoffset, offsets[i]);
	  totoffset += offsets[i];
	}
      }
      if (Stage1Texture&&bTex1) {
	/* int ssrc,ddst;
	   glGetIntegerv (GL_BLEND_SRC, &ssrc);
	   glGetIntegerv (GL_BLEND_DST, &ddst);
	   glBindTexture(GL_TEXTURE_2D, Stage1TextureName);
	   glEnable (GL_BLEND);
	   glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	   
	   //now transfer textures that correspond to second set of coords
	   glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage1*2);
	   int totoffset=0;
	   for (int i=0;i<numlists;i++) {
	   glDrawArrays(mode[i], totoffset, offsets[i]);
	   totoffset += offsets[i];
	   }
	   
	   glBlendFunc (ssrc,ddst);
	   glBindTexture(GL_TEXTURE_2D, Stage0TextureName);
	   //glDisable (GL_BLEND);
	   
	   //reload the old texture pointer
	   glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);
	*/  //if hardware doesn't support multitexturem don't even try it mon
      }
      
    }
	

    }
  return TRUE;
}



