#ifndef ANIMATION_H
#define ANIMATION_H
#include "lin_time.h"
#include "gfx_primitive.h"

class Animation:public Primitive
{
	Texture **Decal;
	short numframes;
	float timeperframe;
	scalar_t cumtime;
	int texturename[2];
	float height; //half the height so you can do fancy vector translatons to campspace
	float width;

	void InitAnimation();

public:
	Animation();
	Animation(char *);
	~Animation();

	void Draw();
};

#endif
