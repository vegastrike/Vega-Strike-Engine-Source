#ifndef LOGO_H_
#define LOGO_H_

#include "gfxlib.h"

//struct glVertex;
struct Texture;
class Vector;
class Logo{
	int numlogos;
	//glVertex **LogoCorner;

	GFXVertexList **vlists;
	Texture* Decal;
public:
	Logo(int numberlogos,Vector* center, Vector* normal, float* sizes, float* rotations, float offset, Texture * Dec, Vector *Ref);
	Logo(const Logo &rval){*this = rval;}
	~Logo ();

	void SetDecal(Texture *decal)
	{
		Decal = decal;
	}
	void Draw();

};
#endif