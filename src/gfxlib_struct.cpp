#include "gfxlib_struct.h"
#include "gfxlib.h"
#include "gldrv/gl_globals.h"
#include <stdio.h>
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"

#include "options.h"

#include <vector>

extern vs_options game_options;


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
#ifndef NO_VBO_SUPPORT
static void BindBuf(unsigned int vbo_data) {
    (*glBindBufferARB_p)(GL_ARRAY_BUFFER_ARB,vbo_data);  
}
static void BindInd(unsigned int element_data) {
    (*glBindBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,element_data);
}
#endif
void GFXVertexList::RefreshDisplayList () 
{

#ifndef NO_VBO_SUPPORT
  if (game_options.vbo&&!vbo_data) {
    if (glGenBuffersARB_p==0||glBindBufferARB_p==0||glBufferDataARB_p==0||glMapBufferARB_p==0||glUnmapBufferARB_p==0) {
      game_options.vbo=0;
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
#endif
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
          GFXTexCoord224f(vtx.s,vtx.t,vtx.s,vtx.t,vtx.tx,vtx.ty,vtx.tz,vtx.tw);
	      glColor4fv (&vtx.r);
	      glNormal3fv(&vtx.i);
	      glVertex3fv(&vtx.x);	
	    }
      }else {
	    for(a=0; a<offsets[i]; a++) {
          const GFXColorVertex& vtx=data.colors[offset+a];
          GFXTexCoord224f(vtx.s,vtx.t,vtx.s,vtx.t,vtx.tx,vtx.ty,vtx.tz,vtx.tw);
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
          GFXTexCoord224f(vtx.s,vtx.t,vtx.s,vtx.t,vtx.tx,vtx.ty,vtx.tz,vtx.tw);
	      glVertex3fv(&vtx.x);
	    }
      }else {
	    for(a=0; a<offsets[i]; a++) {
          const GFXVertex& vtx=data.vertices[offset+a];
	      glNormal3fv(&vtx.i);
          GFXTexCoord224f(vtx.s,vtx.t,vtx.s,vtx.t,vtx.tx,vtx.ty,vtx.tz,vtx.tw);
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
  if (!numVertices) 
    return;//don't do anything if there are no vertices
#ifndef NO_VBO_SUPPORT
  if (vbo_data) {
    /*if (gl_error=glGetError()) {
      printf ("VBO18.5 Error %d\n",gl_error);
    }*/

    BindBuf(vbo_data);
    if (changed&HAS_INDEX) {
      /*if (gl_error=glGetError()) {
        printf ("VBO18.5a Error %d\n",gl_error);
      }*/
      
      BindInd(display_list);
      /*if (gl_error=glGetError()) {
        printf ("VBO18.5b Error %d\n",gl_error);
      }*/

    }
    if (changed&HAS_COLOR) {
      if (gl_options.Multitexture)
          glClientActiveTextureARB_p(GL_TEXTURE0);
      glInterleavedArrays (GL_T2F_C4F_N3F_V3F,sizeof(GFXColorVertex),0);
      if (gl_options.Multitexture) {
          glClientActiveTextureARB_p(GL_TEXTURE1);
          glTexCoordPointer(2,GL_FLOAT,sizeof(GFXColorVertex),(void*)((char*)&data.colors[0].s - (char*)data.vertices));
          glEnableClientState(GL_TEXTURE_COORD_ARRAY);
          glClientActiveTextureARB_p(GL_TEXTURE2);
          glTexCoordPointer(4,GL_FLOAT,sizeof(GFXColorVertex),(void*)((char*)&data.colors[0].tx - (char*)data.vertices));
          glEnableClientState(GL_TEXTURE_COORD_ARRAY);
          glClientActiveTextureARB_p(GL_TEXTURE0);
      }
    } else {
      if (gl_options.Multitexture)
          glClientActiveTextureARB_p(GL_TEXTURE0);
      glInterleavedArrays (GL_T2F_N3F_V3F,sizeof(GFXVertex),0);
      if (gl_options.Multitexture) {
          glClientActiveTextureARB_p(GL_TEXTURE1);
          glTexCoordPointer(2,GL_FLOAT,sizeof(GFXVertex),(void*)((char*)&data.vertices[0].s - (char*)data.vertices));
          glEnableClientState(GL_TEXTURE_COORD_ARRAY);
          glClientActiveTextureARB_p(GL_TEXTURE2);
          glTexCoordPointer(4,GL_FLOAT,sizeof(GFXVertex),(void*)((char*)&data.vertices[0].tx - (char*)data.vertices));
          glEnableClientState(GL_TEXTURE_COORD_ARRAY);
          glClientActiveTextureARB_p(GL_TEXTURE0);
      }
    }
  } else 
#endif
      if(display_list!=0) {
    
  } else 
    {      
      if (changed&HAS_COLOR) {
          if (gl_options.Multitexture)
              glClientActiveTextureARB_p(GL_TEXTURE0);
          glInterleavedArrays (GL_T2F_C4F_N3F_V3F,sizeof(GFXColorVertex),&data.colors[0]);
          if (gl_options.Multitexture) {
              glClientActiveTextureARB_p(GL_TEXTURE1);
              glTexCoordPointer(2,GL_FLOAT,sizeof(GFXColorVertex),&data.colors[0].s);
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
              glClientActiveTextureARB_p(GL_TEXTURE2);
              glTexCoordPointer(4,GL_FLOAT,sizeof(GFXColorVertex),&data.colors[0].tx);
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
              glClientActiveTextureARB_p(GL_TEXTURE0);
          }
      } else {
          if (gl_options.Multitexture) 
              glClientActiveTextureARB_p(GL_TEXTURE0);
          glInterleavedArrays (GL_T2F_N3F_V3F,sizeof(GFXVertex),&data.vertices[0]);
          if (gl_options.Multitexture) {
              glClientActiveTextureARB_p(GL_TEXTURE1);
              glTexCoordPointer(2,GL_FLOAT,sizeof(GFXVertex),&data.vertices[0].s);
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
              glClientActiveTextureARB_p(GL_TEXTURE2);
              glTexCoordPointer(4,GL_FLOAT,sizeof(GFXVertex),&data.vertices[0].tx);
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
              glClientActiveTextureARB_p(GL_TEXTURE0);
          }
      }
#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
      if (lock&&glLockArraysEXT_p)
          (*glLockArraysEXT_p) (0,numVertices);
#endif
  }
}
extern void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a);

void GFXVertexList::EndDrawState(GFXBOOL lock) {
  if (vbo_data) {

  }else if(display_list!=0) {
    
  } else {
#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
    if (lock&&glUnlockArraysEXT_p&&numVertices)
        (*glUnlockArraysEXT_p) ();
#endif
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
 //Hardware support for this seems... sketchy

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
          if (((*mode==GFXPOINT)&&gl_options.smooth_points)||((*mode!=GFXPOINT)&&gl_options.smooth_lines)) {
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
        bool use_vbo;
        if (vbo_data&&memcmp(&index,&this->index,sizeof(INDEX))==0) {
            #ifndef NO_VBO_SUPPORT
            BindInd(display_list);
            #endif
            use_vbo = true;
        } else {
            #ifndef NO_VBO_SUPPORT
            if (vbo_data)
                BindInd(0);
            #endif
            use_vbo = false;
        }
        if (glMultiDrawElements_p) {
            static std::vector<bool> drawn;
            static std::vector<const GLvoid*> glindices;
            static std::vector<GLsizei> glcounts;
            
            drawn.clear();
            drawn.resize(numlists,false);
            for (int i=0; i<numlists; totoffset += offsets[i++]) if (!drawn[i]) {
                glindices.clear();
                glcounts.clear();
                
                for (int j=i, offs=totoffset; j<numlists; offs += offsets[j++]) if(!drawn[j] && (mode[j]==mode[i])) {
                    glindices.push_back(use_vbo ? (GLvoid*)(stride*offs)
                                                : (GLvoid*)&index.b[stride*offs]);
                    glcounts.push_back(offsets[j]);
                    drawn[j] = true;
                }
                
                if (glindices.size() == 1)
                    glDrawElements(PolyLookup(mode[i]), glcounts[0], indextype, glindices[0]); else
                    glMultiDrawElements_p(PolyLookup(mode[i]), &glcounts[0], indextype, &glindices[0], glindices.size());
            }
        } else {
            for (int i=0;i<numlists;i++) {
                glDrawElements (PolyLookup(mode[i]),offsets[i], indextype, 
                    use_vbo ? (void*)(stride*totoffset)
                            : &index.b[stride*totoffset] );//changed&INDEX_BYTE == stride!
                totoffset += offsets[i];
            }
        }
      } else {
        if (glMultiDrawArrays_p) {
            static std::vector<bool> drawn;
            static std::vector<GLint> gloffsets;
            static std::vector<GLsizei> glcounts;
            
            drawn.clear();
            drawn.resize(numlists,false);
            for (int i=0; i<numlists; totoffset += offsets[i++]) if (!drawn[i]) {
                gloffsets.clear();
                glcounts.clear();
                
                for (int j=i, offs=totoffset; j<numlists; offs += offsets[j++]) if(!drawn[j] && (mode[j]==mode[i])) {
                    gloffsets.push_back(offs);
                    glcounts.push_back(offsets[j]);
                    drawn[j] = true;
                }
                
                bool blendchange=false;
                switch (mode[i]) {
                case GFXLINE:
                case GFXLINESTRIP:
                case GFXPOLY:
                case GFXPOINT:
                    if (((mode[i]==GFXPOINT)&&gl_options.smooth_points)||((mode[i]!=GFXPOINT)&&gl_options.smooth_lines)) {
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
                
                if (gloffsets.size() == 1)
                    glDrawArrays(PolyLookup(mode[i]), gloffsets[0], glcounts[0]); else
                    glMultiDrawArrays_p(PolyLookup(mode[i]), &gloffsets[0], &glcounts[0], gloffsets.size());
        
                if (blendchange) {
                    GFXPopBlendMode();
                    GFXDisable(SMOOTH);
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
                    if (((mode[i]==GFXPOINT)&&gl_options.smooth_points)||((mode[i]!=GFXPOINT)&&gl_options.smooth_lines)) {
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
}


GFXVertexList::~GFXVertexList() {
#ifndef NO_VBO_SUPPORT
    if (vbo_data) {
      (*glDeleteBuffersARB_p)(1,(GLuint*)&vbo_data);
       if (display_list) {
         (*glDeleteBuffersARB_p)(1,(GLuint*)&display_list);
       }     
    } else 
#endif
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

union GFXVertexList::VDAT * GFXVertexList::Map(bool read, bool write) {
#ifndef NO_VBO_SUPPORT
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
#endif

  return &data;
}
void GFXVertexList::UnMap() {
#ifndef NO_VBO_SUPPORT
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
#endif
}
  ///Returns the array of vertices to be mutated
union GFXVertexList::VDAT * GFXVertexList::BeginMutate (int offset) {
  return this->Map(false,true);
}

///Ends mutation and refreshes display list
void GFXVertexList::EndMutate (int newvertexsize) {
  this->UnMap();
  if (!(changed&CHANGE_MUTABLE)) {
    changed |= CHANGE_CHANGE;
  }
  if (newvertexsize) {
    numVertices = newvertexsize;
    //Must keep synchronized - we'll only permit changing vertex count on single-list objects
    if (numlists==1) *offsets = numVertices; 
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

}


GFXVertexList::GFXVertexList() {
  vbo_data=0;
  display_list=0;
  memset(this,0,sizeof(GFXVertexList));
}//private, only for inheriters
