#include "gfxlib_struct.h"
#include "gfxlib.h"
#include "gldrv/gl_globals.h"
#include <stdio.h>
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"


//#define GFX_BUFFER_MAP_UNMAP
static GLenum gl_error;
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
static void BindBuf(unsigned int vbo_data) {
    (*glBindBufferARB_p)(GL_ARRAY_BUFFER_ARB,vbo_data);  
}
static void BindInd(unsigned int element_data) {
    (*glBindBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,element_data);
}
void GFXVertexList::RefreshDisplayList () {
  static bool use_vbo=XMLSupport::parse_bool(vs_config->getVariable("graphics","vbo","false"));
  static bool smooth_lines = XMLSupport::parse_bool( vs_config->getVariable("graphics/mesh","smooth_lines","true") );
  static bool smooth_points= XMLSupport::parse_bool( vs_config->getVariable("graphics/mesh","smooth_points","true") ); 

  if (use_vbo&&!vbo_data) {
    if (glGenBuffersARB_p==0||glBindBufferARB_p==0||glBufferDataARB_p==0||glMapBufferARB_p==0||glUnmapBufferARB_p==0) {
      use_vbo=0;
    }else {
      (*glGenBuffersARB_p)(1,(GLuint *)&vbo_data);
      if (changed&HAS_INDEX){
        (*glGenBuffersARB_p)(1,(GLuint*)&display_list);
      }
    }   
  }
  if (vbo_data) {
    BindBuf(vbo_data);
    (*glBufferDataARB_p)(GL_ARRAY_BUFFER_ARB, numVertices*((changed&HAS_COLOR)?sizeof(GFXColorVertex):sizeof(GFXVertex)), data.vertices, (changed&CHANGE_MUTABLE)?GL_DYNAMIC_DRAW_ARB:GL_STATIC_DRAW_ARB);
    if (changed&HAS_INDEX) {
      BindInd(display_list);
      unsigned int tot=0;
      for (int i=0;i<numlists;++i) {
        tot+=offsets[i];
      }
      unsigned int indexsize = (changed & INDEX_BYTE)
        ?sizeof(char)
        : ((changed & INDEX_SHORT) 
           ? sizeof (unsigned short) 
           : sizeof(unsigned int));
      (*glBufferDataARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB, tot*indexsize, &index.b[0], (changed&CHANGE_MUTABLE)?GL_DYNAMIC_DRAW_ARB:GL_STATIC_DRAW_ARB);      
    }
    return;
  }
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
          const GFXColorVertex& vtx=data.colors[GetIndex(offset+a)];
          GFXTexCoord4f(vtx.s,vtx.t,vtx.s,vtx.t);
	      glColor4fv (&vtx.r);
	      glNormal3fv(&vtx.i);
	      glVertex3fv(&vtx.x);	
	    }
      }else {
	    for(a=0; a<offsets[i]; a++) {
          const GFXColorVertex& vtx=data.colors[offset+a];
          GFXTexCoord4f(vtx.s,vtx.t,vtx.s,vtx.t);
	      glColor4fv (&vtx.r);
	      glNormal3fv(&vtx.i);
	      glVertex3fv(&vtx.x);
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
          const GFXVertex& vtx=data.vertices[GetIndex(offset+a)];
	      glNormal3fv(&vtx.i);
          GFXTexCoord4f(vtx.s,vtx.t,vtx.s,vtx.t);
	      glVertex3fv(&vtx.x);
	    }
      }else {
	    for(a=0; a<offsets[i]; a++) {
          const GFXVertex& vtx=data.vertices[offset+a];
	      glNormal3fv(&vtx.i);
          GFXTexCoord4f(vtx.s,vtx.t,vtx.s,vtx.t);
	      glVertex3fv(&vtx.x);
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
}

void GFXVertexList::BeginDrawState(GFXBOOL lock) {
  if (vbo_data) {
    BindBuf(vbo_data);
    if (changed&HAS_INDEX)
      BindInd(display_list);
    if (changed&HAS_COLOR) {
      glClientActiveTextureARB_p(GL_TEXTURE0);
      glInterleavedArrays (GL_T2F_C4F_N3F_V3F,sizeof(GFXColorVertex),0);
      if (gl_options.Multitexture) {
          glClientActiveTextureARB_p(GL_TEXTURE1);
          glTexCoordPointer(2,GL_FLOAT,sizeof(GFXColorVertex),&data.colors[0].s);
          glClientActiveTextureARB_p(GL_TEXTURE0);
      }
    } else {
      glClientActiveTextureARB_p(GL_TEXTURE0);
      glInterleavedArrays (GL_T2F_N3F_V3F,sizeof(GFXVertex),0);
      if (gl_options.Multitexture) {
          glClientActiveTextureARB_p(GL_TEXTURE1);
          glTexCoordPointer(2,GL_FLOAT,sizeof(GFXVertex),&data.vertices[0].s);
          glClientActiveTextureARB_p(GL_TEXTURE0);
      }
    }
  }else if(display_list!=0) {
    
  } else 
    {      
      if (changed&HAS_COLOR) {
          glClientActiveTextureARB_p(GL_TEXTURE0);
          glInterleavedArrays (GL_T2F_C4F_N3F_V3F,sizeof(GFXColorVertex),&data.colors[0]);
          if (gl_options.Multitexture) {
              glClientActiveTextureARB_p(GL_TEXTURE1);
              glTexCoordPointer(2,GL_FLOAT,sizeof(GFXColorVertex),&data.colors[0].s);
              glClientActiveTextureARB_p(GL_TEXTURE0);
          }
      } else {
          glClientActiveTextureARB_p(GL_TEXTURE0);
          glInterleavedArrays (GL_T2F_N3F_V3F,sizeof(GFXVertex),&data.vertices[0]);
          if (gl_options.Multitexture) {
              glClientActiveTextureARB_p(GL_TEXTURE1);
              glTexCoordPointer(2,GL_FLOAT,sizeof(GFXVertex),&data.vertices[0].s);
              glClientActiveTextureARB_p(GL_TEXTURE0);
          }
          if (gl_error=glGetError()) printf ("VBO19 Error %d\n",gl_error);
      }
      if (lock&&glLockArraysEXT_p)
	(*glLockArraysEXT_p) (0,numVertices);
  }
}
extern void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a);

void GFXVertexList::EndDrawState(GFXBOOL lock) {
  if (vbo_data) {

  }else if(display_list!=0) {
    
  } else {
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
  static bool smooth_lines = XMLSupport::parse_bool( vs_config->getVariable("graphics","smooth_lines","true") );
  static bool smooth_points= XMLSupport::parse_bool( vs_config->getVariable("graphics","smooth_points","false") );  //Hardware support for this seems... sketchy

  if(vbo_data==0&&display_list!=0) {
      //Big issue: display lists cannot discriminate between lines/points/triangles,
      //    so, for now, we'll limit smoothing to single-mode GFXVertexLists, which, by the way,
      //    are the only ones being used, AFAIK.
      bool blendchange=false;
      if (unique_mode&&(numlists>0)) switch (*mode) {
      case GFXLINE:
      case GFXLINESTRIP:
      case GFXPOLY:
      case GFXPOINT:
          if (((*mode==GFXPOINT)&&smooth_points)||((*mode!=GFXPOINT)&&smooth_lines)) {
              BLENDFUNC src,dst;
              GFXGetBlendMode(src,dst);
              if ((dst!=ZERO)&&((src==ONE)||(src==SRCALPHA))) {
                  GFXPushBlendMode();
                  GFXBlendMode(SRCALPHA,dst);
                  GFXEnable(SMOOTH);
                  blendchange=true;
              }
          }
          break;
      }
      
      GFXCallList(display_list);

      if (blendchange) {
          GFXPopBlendMode();
          GFXDisable(SMOOTH);
      }
  } else {
      int totoffset=0;
      if (changed&HAS_INDEX) {
	char stride = changed&HAS_INDEX;
	GLenum indextype = (changed & INDEX_BYTE)
	  ?GL_UNSIGNED_BYTE
	  : ((changed & INDEX_SHORT) 
	     ? GL_UNSIGNED_SHORT 
	     : GL_UNSIGNED_INT);
        if (vbo_data&&memcmp(&index,&this->index,sizeof(INDEX))==0) {
          BindInd(display_list);            
          for (int i=0;i<numlists;i++) {
            glDrawElements (PolyLookup(mode[i]),offsets[i], indextype, (void*)(stride*totoffset));//changed&INDEX_BYTE == stride!
            totoffset +=offsets[i];
          }
        }else{
          if (vbo_data)
            BindInd(0);
          for (int i=0;i<numlists;i++) {
            glDrawElements (PolyLookup(mode[i]),offsets[i], indextype, &index.b[stride*totoffset]);//changed&INDEX_BYTE == stride!
            totoffset +=offsets[i];
          }
        }
      } else {
	    for (int i=0;i<numlists;i++) {
          bool blendchange=false;
          switch (mode[i]) {
          case GFXLINE:
          case GFXLINESTRIP:
          case GFXPOLY:
          case GFXPOINT:
              if (((mode[i]==GFXPOINT)&&smooth_points)||((mode[i]!=GFXPOINT)&&smooth_lines)) {
                  BLENDFUNC src,dst;
                  GFXGetBlendMode(src,dst);
                  if ((dst!=ZERO)&&((src==ONE)||(src==SRCALPHA))) {
                      GFXPushBlendMode();
                      GFXBlendMode(SRCALPHA,dst);
                      GFXEnable(SMOOTH);
                      blendchange=true;
                  }
              }
              break;
          }

	      glDrawArrays(PolyLookup(mode[i]), totoffset, offsets[i]);
	      totoffset += offsets[i];

          if (blendchange) {
              GFXPopBlendMode();
              GFXDisable(SMOOTH);
          }
	    }
      }
  }
}


GFXVertexList::~GFXVertexList() {
    if (vbo_data) {
      (*glDeleteBuffersARB_p)(1,(GLuint*)&vbo_data);
       if (display_list) {
         (*glDeleteBuffersARB_p)(1,(GLuint*)&display_list);
       }     
    }else if (display_list)
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

union GFXVertexList::VDAT * GFXVertexList::Map(bool read, bool write) {
  if (GFX_BUFFER_MAP_UNMAP) {
  if (vbo_data) {
    if (display_list) {
      BindInd(display_list);
      index.b=(unsigned char*)(*glMapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,read?(write?GL_READ_WRITE_ARB:GL_READ_ONLY_ARB):GL_WRITE_ONLY_ARB);
    }
    BindBuf(vbo_data);
    void * ret=(*glMapBufferARB_p)(GL_ARRAY_BUFFER_ARB,read?(write?GL_READ_WRITE_ARB:GL_READ_ONLY_ARB):GL_WRITE_ONLY_ARB);    
    if (changed&HAS_COLOR) {
      data.colors=(GFXColorVertex*)ret;
    }else{
      data.vertices=(GFXVertex*)ret;
    }
  }
  }

  return &data;
}
void GFXVertexList::UnMap() {
  if (GFX_BUFFER_MAP_UNMAP) {
  if (vbo_data) {
    if (display_list) {
      BindInd(display_list);
      (*glUnmapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB);        
    }
    BindBuf(vbo_data);
    (*glUnmapBufferARB_p)(GL_ARRAY_BUFFER_ARB);      
    data.colors=NULL;
    data.vertices=NULL;  
  }
  }
}
  ///Returns the array of vertices to be mutated
union GFXVertexList::VDAT * GFXVertexList::BeginMutate (int offset) {
  this->Map(false,true);
  return &data;
}

///Ends mutation and refreshes display list
void GFXVertexList::EndMutate (int newvertexsize) {
  this->UnMap();
  if (!(changed&CHANGE_MUTABLE)) {
    changed |= CHANGE_CHANGE;
  }
  if (!vbo_data){
    RenormalizeNormals ();
    RefreshDisplayList();
  }else {
    RefreshDisplayList();
  }
  if (changed&CHANGE_CHANGE) {
    changed&=(~CHANGE_CHANGE);
  }
  if (newvertexsize) {
    numVertices = newvertexsize;
  }


}


GFXVertexList::GFXVertexList() {
  vbo_data=0;
  display_list=0;
  memset(this,0,sizeof(GFXVertexList));
}//private, only for inheriters
