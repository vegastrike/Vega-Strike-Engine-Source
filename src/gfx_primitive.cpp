#include "gfx_primitive.h"
#include "gfx_aux.h"

//extern PFNGLCOLORTABLEEXTPROC glColorTable;
int texcnt = 1;

void Primitive::InitPrimitive()
{
	forcelogos = NULL;
	squadlogos = NULL;

	Identity(orientation);
	Identity(translation);
	Identity(transformation);
	Identity(stackstate);
}

Primitive:: Primitive ()
{
	InitPrimitive();
}

Primitive::~Primitive()
{
}

void Primitive::Draw()
{
  //FIXME VEGASTRIKE	static float rot = 0;
}

Vector &Primitive::Position()
{
	return pos;
}

