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

void GFXVertexList::Init (enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColorVertex * colors, int numlists, int *offsets, bool Mutable, int tess) {
  isColor = (colors!=NULL)?GFXTRUE:GFXFALSE;
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
    if (vertices) {
      data.vertices = (GFXVertex*)malloc (sizeof (GFXVertex)*numVertices);
      memcpy(data.vertices, vertices, sizeof(GFXVertex)*numVertices);
    } else if (colors) {
      data.colors = (GFXColorVertex*)malloc( sizeof (GFXColorVertex)*numVertices);
      memcpy (data.colors, colors, sizeof (GFXColorVertex)*numVertices);
    } 
  }else {
      data.vertices=NULL;
      data.colors = NULL;
  }
  
  this->offsets = new int [numlists];
  memcpy(this->offsets, offsets, sizeof(int)*numlists);
  
  display_list = 0;
  if (Mutable)
    changed = CHANGE_MUTABLE;
  else 
    changed = CHANGE_CHANGE;
  RefreshDisplayList();
  if (Mutable)
    changed = CHANGE_MUTABLE;//for display lists
  else
    changed = 0;
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
  ///Returns the array of vertices to be mutated
union GFXVertexList::VDAT * GFXVertexList::BeginMutate (int offset) {
  return &data;
}
///Ends mutation and refreshes display list
void GFXVertexList::EndMutate () {
  if (changed!=CHANGE_MUTABLE) {
    changed = CHANGE_CHANGE;
  }
  RefreshDisplayList();
  if (changed==CHANGE_CHANGE) {
    changed=0;
  }
}



void GFXVertexList::RefreshDisplayList () {
#ifdef USE_DISPLAY_LISTS
  if ((!g_game.display_lists)||(display_list&&!(changed&CHANGE_CHANGE))||(changed&CHANGE_MUTABLE)) {
    return;//don't used lists if they're mutable
  }
  if (display_list) {
    GFXDeleteList (display_list);
  }
  int a;
  int offset =0;
  display_list = GFXCreateList();
  if (isColor) {
    for (int i=0;i<numlists;i++) {
      glBegin(mode[i]);
      for(a=0; a<offsets[i]; a++) {
	glTexCoord2fv(&data.colors[offset+a].s);
	glColor3fv (&data.colors[offset+a].r);
	glNormal3fv(&data.colors[offset+a].i);
	glVertex3fv(&data.colors[offset+a].x);
      }
      offset +=offsets[i];
      glEnd();
    }
  }else {
    for (int i=0;i<numlists;i++) {
      glBegin(mode[i]);
      for(a=0; a<offsets[i]; a++) {
	glNormal3fv(&data.vertices[offset+a].i);
	glTexCoord2fv(&data.vertices[offset+a].s);
	glVertex3fv(&data.vertices[offset+a].x);
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

GFXVertexList::~GFXVertexList() {
  if (display_list)
    GFXDeleteList (display_list); //delete dis
  if (offsets)
    delete [] offsets;
  if(isColor) {
    if (data.colors) {
      free (data.colors);
    }
  } else {
    if (data.vertices) {
      free (data.vertices);
    }
  }
}

static void VtxCopy (GFXVertex *dst, const unsigned char *src, int howmany) {
  memcpy (dst,src,sizeof (GFXVertex)*howmany);
}
static void ColVtxCopy (GFXVertex *dst, const unsigned char *src, int howmany) {
  for (int i=0;i<howmany;i++) {
    ((GFXVertex *)dst)[i].
      SetTexCoord (((GFXColorVertex *)src)[i].s,((GFXColorVertex *)src)[i].t).
      SetNormal (Vector (((GFXColorVertex *)src)[i].i,((GFXColorVertex *)src)[i].j,((GFXColorVertex *)src)[i].k)).
      SetVertex (Vector (((GFXColorVertex *)src)[i].x,((GFXColorVertex *)src)[i].y,((GFXColorVertex *)src)[i].z));
  }
}

void GFXVertexList::GetPolys (GFXVertex **vert, int *numpolys, int *numtris) {
  void (*vtxcpy) (GFXVertex *dst,const unsigned char *stc, int howmany);
  vtxcpy = isColor?ColVtxCopy:VtxCopy;
  int offst = isColor?sizeof(GFXColorVertex):sizeof(GFXVertex);
  unsigned char *myVertices =  (isColor?(unsigned char*)data.colors:(unsigned char*)data.vertices);
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
      (*vtxcpy) (&res[curtri],&myVertices [offst*cur],offsets[i]);
      curtri+=offsets[i];
      break;
    case GL_TRIANGLE_FAN:
    case GL_POLYGON:
      for (j=1;j<offsets[i]-1;j++) {
	(*vtxcpy) (&res[curtri],&myVertices [offst*cur],1);
	curtri++;
	(*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j)],1);
	curtri++;
	(*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j+1)],1);
	curtri++;
      }	    
      break;
    case GL_TRIANGLE_STRIP:
      for (j=2;j<offsets[i];j+=2) {
	(*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j-2)],1);
	curtri++;
	(*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j-1)],1);
	curtri++;
	(*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j)],1);
	curtri++;
	if (j+1<offsets[i]) {//copy reverse
	  (*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j)],1);
	  curtri++;
	  (*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j-1)],1);
	  curtri++;
	  (*vtxcpy) (&res[curtri],&myVertices [offst*(cur+j+1)],1);
	  curtri++;
	}
      }	    
      break;
    case GL_QUADS:
      (*vtxcpy) (&res[curquad],&myVertices [offst*(cur)],offsets[i]);
	    curquad+=offsets[i];
	    break;
    case GL_QUAD_STRIP:
      for (j=2;j<offsets[i]-1;j+=2) {
	(*vtxcpy) (&res[curquad],&myVertices [offst*(cur+j-2)],1);
	curquad++;
	(*vtxcpy) (&res[curquad],&myVertices [offst*(cur+j-1)],1);
	curquad++;
	(*vtxcpy) (&res[curquad],&myVertices [offst*(cur+j+1)],1);
	curquad++;
	(*vtxcpy) (&res[curquad],&myVertices [offst*(cur+j)],1);
	curquad++;
      }
      break;
    default:
      break;
    }
    cur +=offsets[i];
  }
}




void GFXVertexList::LoadDrawState() {

}
void GFXVertexList::BeginDrawState(GFXBOOL lock) {
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    
  } else 
#endif
    {      
      if (isColor) {
	glInterleavedArrays (GL_T2F_C4F_N3F_V3F,sizeof(GFXColorVertex),&data.colors[0]);
      } else {
	glInterleavedArrays (GL_T2F_N3F_V3F,sizeof(GFXVertex),&data.vertices[0]);
      }
      if (lock&&glLockArraysEXT_p)
	(*glLockArraysEXT_p) (0,numVertices);
  }
}
void GFXVertexList::EndDrawState(GFXBOOL lock) {
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    
  } else
#endif
    {
      if (lock&&glUnlockArraysEXT_p)
	(*glUnlockArraysEXT_p) ();
    }
  if (isColor) {
    GFXColor4f(1,1,1,1);
  }
}
void GFXVertexList::Draw()
{
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    GFXCallList(display_list);
  } else 
#endif
    {
      int totoffset=0;
      for (int i=0;i<numlists;i++) {
	  glDrawArrays(mode[i], totoffset, offsets[i]);
	  totoffset += offsets[i];
      }
    }
}


