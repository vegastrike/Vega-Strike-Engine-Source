#include "wrapgfx.h"
#include "gfxlib.h"

void WrapGFX::SetLight(int num)
{
	//float position[4];
	//position[0] = lights[num].direction.i;
	//position[1] = lights[num].direction.j;
	//position[2] = lights[num].direction.k;
	//position[3] = 0;
	//glLightfv(GL_LIGHT0+num, GL_POSITION, position);
	
	//someday put in support for local lights
	GFXSetLightPosition(num, lights[num].direction);


	float args[4];
	args[0] = args[1] = args[2] = args[3] = 0.0;
	
	//glLightfv(GL_LIGHT0+num, GL_DIFFUSE, args);
	//glLightfv(GL_LIGHT0+num, GL_SPECULAR, args);
	//glLightfv(GL_LIGHT0+num, GL_AMBIENT, args);

	args[0] = lights[num].r;
	args[1] = lights[num].g;
	args[2] = lights[num].b;
	args[3] = 1.0;

	GFXSetLightColor(num, args);

	//glLightfv(GL_LIGHT0+num, GL_DIFFUSE, args);
	//glLightfv(GL_LIGHT0+num, GL_AMBIENT, args);
}

void WrapGFX::SetLight(int num, Light &newparams)
{
	lights[num] = newparams;
	SetLight(num);
}

void WrapGFX::EnableLight(int num)
{
	//glEnable(GL_LIGHT0+num);
	GFXEnableLight(num);
}

void WrapGFX::DisableLight(int num)
{
	//glDisable(GL_LIGHT0+num);
	GFXDisableLight(num);
}
