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
//#include "gl_globals.h"

#include "gfxlib_struct.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include <assert.h>
// Untransformed and transformed data 

#ifndef GFX_SCALE
#define GFX_SCALE 1./1024.
#endif

GFXVertexList *next;

extern GFXBOOL bTex0;
extern GFXBOOL bTex1;

static bool iseq (float a, float b) {
  const float eps=.001;
  return (fabs(a-b) <eps);
}
static GFXBOOL GFXCompareVertex (GFXVertex *old, GFXVertex * n) {
  return (old->x==n->x&&old->y==n->y&&old->z==n->z&&iseq(old->s,n->s)&&iseq (old->t,n->t)&&((old->i*n->i+old->j*n->j+old->k*n->k)>=0));
}
static void GFXUpdateVertex (GFXVertex *old, GFXVertex *n, unsigned int *ijk) {
  //  old->i*=ijk;
  //  old->j*=ijk;
  //  old->k*=ijk;
  old->i+=n->i;
  old->j+=n->j;
  old->k+=n->k;
  ijk++;
  //  old->i/=ijk;
  //  old->j/=ijk;
  //  old->k/=ijk;
}

void GFXNormalizeVert (GFXVertex *v) {
  float mag =sqrtf( v->i*v->i+v->j*v->j+v->k*v->k);
  if (mag>0) {
    v->i/=mag;
    v->j/=mag;
    v->k/=mag;
  }
}
void GFXOptimizeList (GFXVertex * old, int numV, GFXVertex ** nw, int * nnewV, unsigned int **ind) {
  unsigned int *ijk = (unsigned int *)malloc (sizeof (unsigned int)*numV);
  *ind = (unsigned int *)malloc (sizeof (unsigned int) * numV);
  *nw = (GFXVertex *)malloc (numV*sizeof (GFXVertex));
  *nnewV = 0;
  int i;
  for (i=0;i<numV;i++) {
    int j;
    for (j=0;j<(*nnewV);j++) {
      if (GFXCompareVertex ((*nw)+j,old+i)==GFXTRUE) {
	GFXUpdateVertex ((*nw)+j,old+i,ijk+j);
	(*ind)[i]=j;
	break;
      }
    }
    if (j==(*nnewV)) {
      ijk[*nnewV]=1;
      memcpy ((*nw)+(*nnewV),old+i,sizeof (GFXVertex));
      ((*ind)[i])=(*nnewV);
      (*nnewV)=(*nnewV)+1;
    }
  }
  for (i=0;i<*nnewV;i++) {
    GFXNormalizeVert ((*nw)+i);
  }
  free (ijk);  
}



void GFXVertexList::Init (enum POLYTYPE *poly, int numVertices, const GFXVertex *vertices, const GFXColorVertex * colors, int numlists, int *offsets, bool Mutable, unsigned int * indices) {
  VSCONSTRUCT1('v')
  int stride=0;
  changed = HAS_COLOR*((colors!=NULL)?1:0);
  mode = new POLYTYPE [numlists];
  for (int pol=0;pol<numlists;pol++) {
      mode[pol]=poly[pol];//PolyLookup (poly[pol]);
  }
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
  int i;
  unsigned int numindices=0;
  for (i=0;i<numlists;i++) {
    numindices+=offsets[i];
  }
  display_list = 0;
  if (Mutable)
    changed |= CHANGE_MUTABLE;
  else 
    changed |= CHANGE_CHANGE;
  if (indices) {
    stride=INDEX_BYTE;
    if (numVertices>255)
      stride=INDEX_SHORT;
    if (numVertices>65535)
      stride=INDEX_INT;
    index.b = (unsigned char *) malloc (stride*numindices);
    for (unsigned int i=0;i<numindices;i++) {
      switch (stride) {
      case INDEX_BYTE:
	index.b[i]=indices[i];
	break;
      case INDEX_SHORT:
	index.s[i]=indices[i];
	break;
      case INDEX_INT:
	index.i[i]=indices[i];
	break;
      }
    }
  } else {
    index.b=NULL;
  }
  
  changed |= stride;
  RenormalizeNormals ();  
  RefreshDisplayList();
  if (Mutable)
    changed |= CHANGE_MUTABLE;//for display lists
  else
    changed &= (~CHANGE_CHANGE);

}

int GFXVertexList::numTris () {
    int tot=0;
    for (int i=0;i<numlists;i++) {
	switch (mode[i]) {
	case GFXTRI:
	    tot+= offsets[i]/3;
	    break;
	case GFXTRISTRIP:
	case GFXTRIFAN:
	case GFXPOLY:
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
	case GFXQUAD:
	    tot+=offsets[i]/4;
	    break;
	case GFXQUADSTRIP:
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
void GFXVertexList::EndMutate (int newvertexsize) {
  if (!(changed&CHANGE_MUTABLE)) {
    changed |= CHANGE_CHANGE;
  }
  RenormalizeNormals ();
  RefreshDisplayList();
  if (changed&CHANGE_CHANGE) {
    changed&=(~CHANGE_CHANGE);
  }
  if (newvertexsize) {
    numVertices = newvertexsize;
  }

}




GFXVertexList::~GFXVertexList() {
  VSDESTRUCT1
  if (display_list)
    GFXDeleteList (display_list); //delete dis
  if (offsets)
    delete [] offsets;
  if (mode)
    delete [] mode;
  if(changed&HAS_COLOR) {
    if (data.colors) {
      free (data.colors);
    }
  } else {
    if (data.vertices) {
      free (data.vertices);
    }
  }
}

void GFXVertexList::VtxCopy (GFXVertexList * thus, GFXVertex *dst, int offset, int howmany) {
  memcpy (dst,&thus->data.vertices[offset],sizeof (GFXVertex)*howmany);
}
void GFXVertexList::ColVtxCopy (GFXVertexList * thus, GFXVertex *dst, int offset, int howmany) {
  for (int i=0;i<howmany;i++) {
    dst[i].
      SetTexCoord (thus->data.colors[i+offset].s,thus->data.colors[i+offset].t).
      SetNormal (Vector (thus->data.colors[i+offset].i,thus->data.colors[i+offset].j,thus->data.colors[i+offset].k)).
      SetVertex (Vector (thus->data.colors[i+offset].x,thus->data.colors[i+offset].y,thus->data.colors[i+offset].z));
  }
}

void GFXVertexList::RenormalizeNormals () {

  if (numVertices>0) {
    Vector firstNormal;
    if (changed&HAS_COLOR) {
      firstNormal=    data.colors[0].GetNormal();
    }else {
      firstNormal=    data.vertices[0].GetNormal();
    }
    float mag = firstNormal.Magnitude();
    if (mag>GFX_SCALE/1.5&&mag<GFX_SCALE*1.5) {
      return;
    }
    if (mag<GFX_SCALE/100&&mag<.00001) {
      firstNormal.Set(1,0,0);
    }
    firstNormal.Normalize();
    if (changed&HAS_COLOR) {
      data.colors[0].SetNormal(firstNormal);
    }else {
      data.vertices[0].SetNormal(firstNormal);
    }
    if (changed&HAS_COLOR) {
      for (int i=0;i<numVertices;i++) {
	//	data.colors[i].SetNormal(data.colors[i].GetNormal().Normalize());
	data.colors[i].i*=GFX_SCALE;
	data.colors[i].j*=GFX_SCALE;
	data.colors[i].k*=GFX_SCALE;
      }
    }else {
      for (int i=0;i<numVertices;i++) {
	//	data.vertices[i].SetNormal(data.vertices[i].GetNormal().Normalize());
	data.vertices[i].i*=GFX_SCALE;
	data.vertices[i].j*=GFX_SCALE;
	data.vertices[i].k*=GFX_SCALE;
      }
    }
  }
}
unsigned int GFXVertexList::GetIndex (int offset) const {
  return (changed&sizeof(unsigned char))
      ? (unsigned int) (index.b[offset]) 
      : ((changed&sizeof(unsigned short)) 
	 ? (unsigned int) (index.s[offset])
	 : index.i[offset]);
}
void GFXVertexList::ColIndVtxCopy (GFXVertexList * thus, GFXVertex *dst, int offset, int howmany) {
  for (int i=0;i<howmany;i++) {
    unsigned int j = thus->GetIndex (i+offset);
    dst[i].
      SetTexCoord (thus->data.colors[j].s,thus->data.colors[j].t).
      SetNormal (Vector (thus->data.colors[j].i,thus->data.colors[j].j,thus->data.colors[j].k)).
      SetVertex (Vector (thus->data.colors[j].x,thus->data.colors[j].y,thus->data.colors[j].z));
  }
}
void GFXVertexList::IndVtxCopy (GFXVertexList * thus, GFXVertex *dst, int offset, int howmany) {
  for (int i=0;i<howmany;i++) {
    unsigned int j = thus->GetIndex (i+offset);
    dst[i].
      SetTexCoord (thus->data.vertices[j].s,thus->data.vertices[j].t).
      SetNormal (Vector (thus->data.vertices[j].i,thus->data.vertices[j].j,thus->data.vertices[j].k)).
      SetVertex (Vector (thus->data.vertices[j].x,thus->data.vertices[j].y,thus->data.vertices[j].z));
  }
}

const GFXVertex * GFXVertexList::GetVertex (int index) {
  return data.vertices+index;
}
const GFXColorVertex * GFXVertexList::GetColorVertex (int index) {
  return data.colors+index;
}


void GFXVertexList::GetPolys (GFXVertex **vert, int *numpolys, int *numtris) {
  void (*vtxcpy) (GFXVertexList *thus, GFXVertex *dst,int offset, int howmany);

  vtxcpy = (changed&HAS_COLOR)
    ? ((changed&HAS_INDEX)
       ?ColIndVtxCopy
       :ColVtxCopy)
    : ((changed&HAS_INDEX)
       ? IndVtxCopy
       : VtxCopy);
  //  int offst = (changed&HAS_COLOR)?sizeof(GFXColorVertex):sizeof(GFXVertex);
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
    case GFXTRI:
      (*vtxcpy) (this,&res[curtri],cur,offsets[i]);
      curtri+=offsets[i];
      break;
    case GFXTRIFAN:
    case GFXPOLY:
      for (j=1;j<offsets[i]-1;j++) {
	(*vtxcpy) (this,&res[curtri],cur,1);
	curtri++;
	(*vtxcpy) (this,&res[curtri],(cur+j),1);
	curtri++;
	(*vtxcpy) (this,&res[curtri],(cur+j+1),1);
	curtri++;
      }	    
      break;
    case GFXTRISTRIP:
      for (j=2;j<offsets[i];j+=2) {
	(*vtxcpy) (this,&res[curtri],(cur+j-2),1);
	curtri++;
	(*vtxcpy) (this,&res[curtri],(cur+j-1),1);
	curtri++;
	(*vtxcpy) (this,&res[curtri],(cur+j),1);
	curtri++;
	if (j+1<offsets[i]) {//copy reverse
	  (*vtxcpy) (this,&res[curtri],(cur+j),1);
	  curtri++;
	  (*vtxcpy) (this,&res[curtri],(cur+j-1),1);
	  curtri++;
	  (*vtxcpy) (this,&res[curtri],(cur+j+1),1);
	  curtri++;
	}
      }	    
      break;
    case GFXQUAD:
      (*vtxcpy) (this,&res[curquad],(cur),offsets[i]);
	    curquad+=offsets[i];
	    break;
    case GFXQUADSTRIP:
      for (j=2;j<offsets[i]-1;j+=2) {
	(*vtxcpy) (this,&res[curquad],(cur+j-2),1);
	curquad++;
	(*vtxcpy) (this,&res[curquad],(cur+j-1),1);
	curquad++;
	(*vtxcpy) (this,&res[curquad],(cur+j+1),1);
	curquad++;
	(*vtxcpy) (this,&res[curquad],(cur+j),1);
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
