#include "gfxlib_struct.h"
#include "gldrv/gl_globals.h"
GLenum PolyLookup (POLYTYPE poly) {
  switch (poly) {
  case GFXTRI:    return GL_TRIANGLES;
  case GFXQUAD:    return GL_QUADS;
  case GFXTRISTRIP:    return GL_TRIANGLE_STRIP;
  case GFXQUADSTRIP:    return GL_QUAD_STRIP;
  case GFXTRIFAN:    return GL_TRIANGLE_FAN;
  case GFXPOLY:    return GL_POLYGON;
  case GFXLINE:    return GL_LINES;
  case GFXLINESTRIP:    return GL_LINE_STRIP;
  case GFXPOINT:    return GL_POINTS;
  default:    return GL_TRIANGLES;
  }
}
void GFXVertexList::RefreshDisplayList () {
#ifdef USE_DISPLAY_LISTS
  if ((!gl_options.display_lists)||(display_list&&!(changed&CHANGE_CHANGE))||(changed&CHANGE_MUTABLE)) {
    return;//don't used lists if they're mutable
  }
  if (display_list) {
    GFXDeleteList (display_list);
  }
  int a;
  int offset =0;
  display_list = GFXCreateList();
  if (changed&HAS_COLOR) {
    for (int i=0;i<numlists;i++) {
      glBegin(PolyLookup(mode[i]));
      if (changed&HAS_INDEX) {
	for(a=0; a<offsets[i]; a++) {
	  glTexCoord2fv(&data.colors[GetIndex(offset+a)].s);
	  glColor3fv (&data.colors[GetIndex(offset+a)].r);
	  glNormal3fv(&data.colors[GetIndex(offset+a)].i);
	  glVertex3fv(&data.colors[GetIndex(offset+a)].x);	
	}
      }else {
	for(a=0; a<offsets[i]; a++) {
	  glTexCoord2fv(&data.colors[offset+a].s);
	  glColor3fv (&data.colors[offset+a].r);
	  glNormal3fv(&data.colors[offset+a].i);
	  glVertex3fv(&data.colors[offset+a].x);
	}
      }
      offset +=offsets[i];
      glEnd();
    }
  }else {
    for (int i=0;i<numlists;i++) {
      glBegin(PolyLookup(mode[i]));
      if (changed&HAS_INDEX) {
	for(a=0; a<offsets[i]; a++) {
	  glNormal3fv(&data.vertices[GetIndex(offset+a)].i);
	  glTexCoord2fv(&data.vertices[GetIndex(offset+a)].s);
	  glVertex3fv(&data.vertices[GetIndex(offset+a)].x);
	}
      }else {
	for(a=0; a<offsets[i]; a++) {
	  glNormal3fv(&data.vertices[offset+a].i);
	  glTexCoord2fv(&data.vertices[offset+a].s);
	  glVertex3fv(&data.vertices[offset+a].x);
	}
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

void GFXVertexList::BeginDrawState(GFXBOOL lock) {
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    
  } else 
#endif
    {      
      if (changed&HAS_COLOR) {
	glInterleavedArrays (GL_T2F_C4F_N3F_V3F,sizeof(GFXColorVertex),&data.colors[0]);
      } else {
	glInterleavedArrays (GL_T2F_N3F_V3F,sizeof(GFXVertex),&data.vertices[0]);
      }
      if (lock&&glLockArraysEXT_p)
	(*glLockArraysEXT_p) (0,numVertices);
  }
}
extern void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a);

void GFXVertexList::EndDrawState(GFXBOOL lock) {
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    
  } else
#endif
    {
      if (lock&&glUnlockArraysEXT_p) {
	//	glFlush();
	(*glUnlockArraysEXT_p) ();
      }
    }
  if (changed&HAS_COLOR) {
    GFXColor4f(1,1,1,1);
  }
}

extern GLenum PolyLookup (POLYTYPE poly);

void GFXVertexList::Draw (enum POLYTYPE poly, int numV) {
  //  GLenum tmp = PolyLookup(poly);
  INDEX index;
  index.b = NULL;
  Draw (&poly,index,1,&numV);
}
void GFXVertexList::Draw (enum POLYTYPE poly, int numV, unsigned char *index) {
  char tmpchanged = changed;
  changed = sizeof (unsigned char) | ((~HAS_INDEX)&changed);
  //  GLenum myenum = PolyLookup (poly);
  INDEX tmp; tmp.b =(index);
  Draw (&poly,tmp,1,&numV);
  changed = tmpchanged;
}
void GFXVertexList::Draw (enum POLYTYPE poly, int numV, unsigned short *index) {
  char tmpchanged = changed;
  changed = sizeof (unsigned short) | ((~HAS_INDEX)&changed);
  //  GLenum myenum = PolyLookup (poly);
  INDEX tmp; tmp.s =(index);
  Draw (&poly,tmp,1,&numV);
  changed = tmpchanged;
}
void GFXVertexList::Draw (enum POLYTYPE poly, int numV, unsigned int *index) {
  char tmpchanged = changed;
  changed = sizeof (unsigned int) | ((~HAS_INDEX)&changed);
  //  GLenum myenum = PolyLookup (poly);
  INDEX tmp;tmp.i= (index);
  Draw (&poly,tmp,1,&numV);
  changed = tmpchanged;
}

void GFXVertexList::DrawOnce (){LoadDrawState();BeginDrawState(GFXFALSE);Draw();EndDrawState(GFXFALSE);}

void GFXVertexList::Draw()
{
  Draw (mode,index,numlists,offsets);
}
extern void GFXCallList(int list);
void GFXVertexList::Draw (enum POLYTYPE *mode,const INDEX index, const int numlists, const int *offsets) {
#ifdef USE_DISPLAY_LISTS
  if(display_list!=0) {
    GFXCallList(display_list);
  } else 
#endif
    {
      int totoffset=0;
      if (changed&HAS_INDEX) {
	char stride = changed&HAS_INDEX;
	GLenum indextype = (changed & INDEX_BYTE)
	  ?GL_UNSIGNED_BYTE
	  : ((changed & INDEX_SHORT) 
	     ? GL_UNSIGNED_SHORT 
	     : GL_UNSIGNED_INT);
	for (int i=0;i<numlists;i++) {
	  glDrawElements (PolyLookup(mode[i]),offsets[i], indextype, &index.b[stride*totoffset]);//changed&INDEX_BYTE == stride!
	  totoffset +=offsets[i];
	}
      } else {
	for (int i=0;i<numlists;i++) {
	  glDrawArrays(PolyLookup(mode[i]), totoffset, offsets[i]);
	  totoffset += offsets[i];
	}
      }
    }
}
