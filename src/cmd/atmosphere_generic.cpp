#include "gfx/mesh.h"
#include "atmosphere.h"
#include "vegastrike.h"

#include "gfx/matrix.h"
#include "gfxlib.h"
#include "gfx/sphere.h"
int divisions = 64;

void setArray(float c0[4], const GFXColor&c1) {
	c0[0]=c1.r;
	c0[1]=c1.g;
	c0[2]=c1.b;
	c0[3]=c1.a;
}
void setArray1(float c0[3], const GFXColor&c1) {
	c0[0]=c1.r;
	c0[1]=c1.g;
	c0[2]=c1.b;
}

Atmosphere::Atmosphere(const Parameters &params) : user_params(params) {
	dome = new SphereMesh(params.radius, divisions, divisions, "white.bmp", 
					NULL,true,ONE,ZERO,false,
					0,M_PI/2);
}

Atmosphere::~Atmosphere() {
	unsigned int a;

	for(a=0; a<sunboxes.size(); a++) {
		delete sunboxes[a];
	}
}

const Atmosphere::Parameters &Atmosphere::parameters()
{
	return user_params;
}

void Atmosphere::SetParameters(const Parameters &params)
{
	user_params = params;
}

