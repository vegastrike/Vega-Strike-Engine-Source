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
#include "gl_globals.h"

#include "gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include <assert.h>
// Untransformed and transformed data 

GFXVertexList *next;

extern GFXBOOL bTex0;
extern GFXBOOL bTex1;

#define CHANGE_MUTABLE 1
#define CHANGE_CHANGE 2
#define USE_DISPLAY_LISTS
GFXVertexList::GFXVertexList():numVertices(0),myVertices(NULL),display_list(0), numlists(0) { }

void GFXVertexList::Init (enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColor * colors, int numlists, int *offsets, bool Mutable, int tess) {
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
    case GFXPOINT:
      mode[pol]=GL_POINTS;
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
  if (numVertices) {
    myVertices = (GFXVertex*)malloc (sizeof (GFXVertex)*numVertices);
    memcpy(myVertices, vertices, sizeof(GFXVertex)*numVertices);
    if (colors) {
      myColors = (GFXColor*)malloc( sizeof (GFXColor)*numVertices);
      memcpy (myColors, colors, sizeof (GFXColor)*numVertices);
    }else {
      myColors = NULL;
    }
  }	
  this->offsets = new int [numlists];
  memcpy(this->offsets, offsets, sizeof(int)*numlists);
  
  display_list = 0;
  this->tessellation = tess;
  if (Mutable)
    changed = CHANGE_MUTABLE;
  else
    changed = CHANGE_CHANGE;
  if (tess) 
    Tess (tess);
  else {
    tesslist = NULL;
    RefreshDisplayList();
  }
  if (Mutable)
    changed = CHANGE_MUTABLE;//for display lists
  else
    changed = 0;

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
int GFXVertexList::numTris () {
    int tot=0;
    for (int i=0;i<numlists;i++) {
	switch (mode[i]) {
	case GL_TRIANGLES:
	    tot+= offsets[i]/3;
	    break;
	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
	case GL_POLYGON:
	    tot+= offsets[i]-2;
	    break;
	default:
	    break;
	}
    }
    return tot;
}
int GFXVertexList::numQuads () {
    int tot=0;
    for (int i=0;i<numlists;i++) {
	switch (mode[i]) {
	case GL_QUADS:
	    tot+=offsets[i]/4;
	    break;
	case GL_QUAD_STRIP:
	    tot+= (offsets[i]-2)/2;
	    break;
	default:
	    break;
	}
    }
    return tot;
}



void GFXVertexList::RefreshDisplayList () {
#ifdef USE_DISPLAY_LISTS
  if ((display_list&&!(changed&CHANGE_CHANGE))||(changed&CHANGE_MUTABLE)) {
    return;//don't used lists if they're mutable
  }
	int a;
	int offset =0;
	display_list = GFXCreateList();
	if (myColors !=NULL) {
	  for (int i=0;i<numlists;i++) {
	    glBegin(mode[i]);
	    for(a=0; a<offsets[i]; a++) {
	      glColor3fv (&myColors[offset+a].r);
	      glNormal3fv(&myVertices[offset+a].i);
	      glTexCoord2fv(&myVertices[offset+a].s);
	      glVertex3fv(&myVertices[offset+a].x);
	    }
	    offset +=offsets[i];
	    glEnd();
	  }
	}else {
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
	}
	if (!GFXEndList()){
	    GFXDeleteList ( display_list);
	    display_list=0;
	}
#endif
}

GFXVertexList::~GFXVertexList()
{
  if (display_list)
    GFXDeleteList (display_list); //delete dis
  if (tessellation)
    delete tesslist;
  if (offsets)
    delete [] offsets;
  if(myVertices)
    free (myVertices);
  if (myColors) 
    free (myColors);
}

void GFXVertexList::GetPolys (GFXVertex **vert, int *numpolys, int *numtris) {
    int i;
    int cur=0;
    GFXVertex *res;
    *numtris = numTris();
    *numpolys = *numtris + numQuads();
    int curtri=0;
    int curquad=3*(*numtris);
    res = (GFXVertex *)malloc (((*numtris)*3+4*(*numpolys-(*numtris)))*sizeof(GFXVertex));
    *vert = res;
    for (i=0;i<numlists;i++) {
	int j;
	switch (mode[i]) {
	case GL_TRIANGLES:
	    memcpy (&res[curtri],&myVertices [cur],offsets[i]*sizeof (GFXVertex));
	    curtri+=offsets[i];
	    break;
	case GL_TRIANGLE_FAN:
	case GL_POLYGON:
	    for (j=1;j<offsets[i]-1;j++) {
		memcpy (&res[curtri],&myVertices [cur],sizeof (GFXVertex));
		curtri++;
		memcpy (&res[curtri],&myVertices [cur+j],sizeof (GFXVertex));
		curtri++;
		memcpy (&res[curtri],&myVertices [cur+j+1],sizeof (GFXVertex));
		curtri++;
	    }	    
	    break;
	case GL_TRIANGLE_STRIP:
	    for (j=2;j<offsets[i];j+=2) {
		memcpy (&res[curtri],&myVertices [cur+j-2],sizeof (GFXVertex));
		curtri++;
		memcpy (&res[curtri],&myVertices [cur+j-1],sizeof (GFXVertex));
		curtri++;
		memcpy (&res[curtri],&myVertices [cur+j],sizeof (GFXVertex));
		curtri++;
		if (j+1<offsets[i]) {//copy reverse
		    memcpy (&res[curtri],&myVertices [cur+j],sizeof (GFXVertex));
		    curtri++;
		    memcpy (&res[curtri],&myVertices [cur+j-1],sizeof (GFXVertex));
		    curtri++;
		    memcpy (&res[curtri],&myVertices [cur+j+1],sizeof (GFXVertex));
		    curtri++;
		}
	    }	    
	    break;
	case GL_QUADS:
	    memcpy (&res[curquad],&myVertices [cur],offsets[i]*sizeof (GFXVertex));
	    curquad+=offsets[i];
	    break;
	case GL_QUAD_STRIP:
	    for (j=2;j<offsets[i]-1;j+=2) {
		memcpy (&res[curquad],&myVertices [cur+j-2],sizeof (GFXVertex));
		curquad++;
		memcpy (&res[curquad],&myVertices [cur+j-1],sizeof (GFXVertex));
		curquad++;
		memcpy (&res[curquad],&myVertices [cur+j+1],sizeof (GFXVertex));
		curquad++;
		memcpy (&res[curquad],&myVertices [cur+j],sizeof (GFXVertex));
		curquad++;
	    }
	    break;
	default:
	    break;
	}
	cur +=offsets[i];
    }
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


GFXBOOL GFXVertexList::SwapUntransformed()
{
  if (tessellation)
    tesslist->SwapUntransformed();
  
	return GFXFALSE;
}

GFXBOOL GFXVertexList::SwapTransformed()
{
  if (tessellation)
    tesslist->SwapTransformed();

	return GFXFALSE;
}

GFXBOOL GFXVertexList::Mutate (int offset, const GFXVertex *vlist, int number, const GFXColor *color){  
  if (offset+number>numVertices)
    return GFXFALSE;
  memcpy (&myVertices[offset].x, vlist, number*sizeof(GFXVertex));
  if (myColors&&color) {
    memcpy (&myColors[offset].r,color,number*sizeof(GFXColor));
  }
  RefreshDisplayList();
  return GFXTRUE;
} 

void GFXVertexList::LoadDrawState() {
#ifdef USE_DISPLAY_LISTS
  if (display_list!=0) return;
#endif
    if (g_game.Multitexture){
      if (myColors!=NULL) {
	glEnableClientState(GL_COLOR_ARRAY);
      }else {
	glDisableClientState (GL_COLOR_ARRAY);
      }
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glClientActiveTextureARB (GL_TEXTURE0_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glClientActiveTextureARB (GL_TEXTURE1_ARB);
      /*
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      */
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glClientActiveTextureARB (GL_TEXTURE0_ARB);
    }else{ 
      if (myColors!=NULL) {
	glEnableClientState(GL_COLOR_ARRAY);
      }else {
	glDisableClientState (GL_COLOR_ARRAY);
      }
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
    }
}
void GFXVertexList::Draw()
{
  if (tessellation&&tesslist) {
    tesslist->Draw();
    return;
  }
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    GFXCallList(display_list);
  } else 
#endif
    {
      if (myColors!=NULL) {
	glColorPointer (4,GL_FLOAT, sizeof (GFXColor), &myColors[0].r);
      }
      glVertexPointer(3, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].x);
      glNormalPointer(GL_FLOAT, sizeof(GFXVertex), &myVertices[0].i);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &myVertices[0].s+GFXStage0*2);

      int totoffset=0;
      for (int i=0;i<numlists;i++) {
	  glDrawArrays(mode[i], totoffset, offsets[i]);
	  totoffset += offsets[i];
      }
    }
  if (myColors!=NULL) {
    GFXColor4f(1,1,1,1);
  }
}


