#include "gfxlib_struct.h"
#include <stdlib.h>
GFXLight::GFXLight (const bool enabled, const GFXColor &vect, const GFXColor &diffuse, const GFXColor &specular, const GFXColor &ambient, const GFXColor&attenuate, const GFXColor &direction, float exp, float cutoff) {}
void GFXVertexList::RefreshDisplayList () {}

void GFXVertexList::BeginDrawState(GFXBOOL lock) {
}
void GFXVertexList::EndDrawState(GFXBOOL lock) {
}

void GFXVertexList::Draw (enum POLYTYPE poly, int numV) {
}
void GFXVertexList::Draw (enum POLYTYPE poly, int numV, unsigned char *index) {
}
void GFXVertexList::Draw (enum POLYTYPE poly, int numV, unsigned short *index) {
}
void GFXVertexList::Draw (enum POLYTYPE poly, int numV, unsigned int *index) {
}
GFXVertexList::VDAT * GFXVertexList::Map(bool read, bool write) {
  return &data;
}
void GFXVertexList::UnMap() {

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

void GFXVertexList::DrawOnce (){}

void GFXVertexList::Draw()
{
}
void GFXVertexList::Draw (enum POLYTYPE *mode,const INDEX index, const int numlists, const int *offsets) {
}

extern GFXBOOL /*GFXDRVAPI*/ GFXGetMaterial(const unsigned int number, GFXMaterial &material) { return GFXFALSE;}
extern void /*GFXDRVAPI*/ GFXSetMaterial(unsigned int &number, const GFXMaterial &material) {}
///Creates a Display list. 0 is returned if no memory is avail for a display list
extern int /*GFXDRVAPI*/ GFXCreateList() { return 0;}
///Ends the display list call.  Returns false if unsuccessful
extern GFXBOOL /*GFXDRVAPI*/ GFXEndList() { return GFXFALSE;}
///Removes a display list from application memory
extern void /*GFXDRVAPI*/ GFXDeleteList (int list) {}
GFXVertexList::~GFXVertexList() {
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
