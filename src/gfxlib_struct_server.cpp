#include "gfxlib_struct.h"

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
