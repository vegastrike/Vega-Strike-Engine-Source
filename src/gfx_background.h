#ifndef BACKGROUND_H
#define BACKGROUND_H

//#include <gl/gl.h>
#include "gfx_aux.h"

class Background{
	Texture *up;
	Texture *left;
	Texture *front;
	Texture *right;
	Texture *back;
	Texture *down;

public:
	Background(char *file);
	~Background();

	void Draw();
};

#endif
