#ifndef SPRITE_H
#define SPRITE_H

#include "gfxlib.h"
#include "gfx_aux_texture.h"
#include "gfx_mesh.h"

class Sprite:public Mesh {
	float xcenter;
	float ycenter;
	float width;
	float height;
	
	float left, right, top, bottom;

	float rotation;

	Texture *surface;
	//GFXVertexList *vlist;

	//BOOL changed;

public:
	Sprite(char *file);
	~Sprite();

	void Draw();

	void Rotate(const float &rad){Yaw(rad);};

	void SetPosition(const float &x1, const float &y1);
	void GetPosition(float &x1, float &y1);

	void SetRotation(const float &rot);
	void GetRotation(float &rot);
	//float &Rotation(){return rotation;};
};

#endif