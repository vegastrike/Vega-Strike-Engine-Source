/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdio.h>
//#include <gl/gl.h>
//#include <gl/glu.h>
#include <fcntl.h>
//#include <io.h>
#include "gfxlib.h"
#include "wrapgfx.h"
#include "lin_time.h"
#include "planet.h"

#include "profile.h"

//#include "cmd.h"
//#include "glob.h"

//FIXME VEGASTRIKE#include <delayimp.h>



//Object *novas[40];
//Object *aeons[10];
//Object *scouts[10];

//static Animation *explosion;

//delayload:gldrv.dll 
//Delayimp.lib 
Unit *unit;
//Force **forces[NUM_FORCES];




WrapGFX::WrapGFX(int argc, char** argv)
{
  //currentcamera = 0;
	numlights = NUM_LIGHT; 
	topobject = NULL;
	active = FALSE;

	//Select drivers
	

	GFXInit(argc,argv);
	//if(TRUE)
	ForceLogo = new Texture ("TechPriRGB.bmp","TechPriA.bmp");
	SquadLogo = new Texture ("TechSecRGB.bmp","TechSecA.bmp");
#ifdef NV_CUBE_MAP
	LightMap[0]=new Texture ("cube_right_light.bmp",1,CUBEMAP,CUBEMAP_POSITIVE_X);
	LightMap[1]=new Texture ("cube_left_light.bmp",1,CUBEMAP,CUBEMAP_NEGATIVE_X);
	LightMap[2]=new Texture ("cube_up_light.bmp",1,CUBEMAP,CUBEMAP_POSITIVE_Y);
	LightMap[3]=new Texture ("cube_down_light.bmp",1,CUBEMAP,CUBEMAP_NEGATIVE_Y);
	LightMap[4]=new Texture ("cube_front_light.bmp",1,CUBEMAP,CUBEMAP_POSITIVE_Z);
	LightMap[5]=new Texture ("cube_back_light.bmp",1,CUBEMAP,CUBEMAP_NEGATIVE_Z);
#else
	LightMap[0] = new Texture("light.bmp", 1);
#endif
	//else
	//	LightMap = new Texture("light.bmp", 0);

	StartGFX();
	InitInput();

	hud_camera = Camera();
	star_system = new StarSystem(new Planet("test_system.dat"));
	//	delete star_system;
	
}

WrapGFX::~WrapGFX()
{
	//if(topobject != NULL)
	//	delete topobject;
#ifdef NV_CUBE_MAP

  delete LightMap[0];
  delete LightMap[1];
  delete LightMap[2];
  delete LightMap[3];
  delete LightMap[4];
  delete LightMap[5];
#else
  delete LightMap[0];
#endif
	delete ForceLogo;
	delete SquadLogo;
	GFXShutdown();
	//delete mouse;
}
//sets up all the stuff... in this case the ships to be rendered

BOOL WrapGFX::activateLightMap() {
#ifdef NV_CUBE_MAP
  LightMap[0]->MakeActive();
  LightMap[1]->MakeActive();
  LightMap[2]->MakeActive();
  LightMap[3]->MakeActive();
  LightMap[4]->MakeActive();
  LightMap[5]->MakeActive();
#else
    LightMap[0]->MakeActive();
#endif
    return TRUE;
}

BOOL WrapGFX::StartGFX()
{
  //	SetViewport();
	GFXBeginScene();
	GFXMaterial mat;
	mat.ar = 1.00F;
	mat.ag = 1.00F;
	mat.ab = 1.00F;
	mat.aa = 1.00F;

	mat.dr = 1.00F;
	mat.dg = 1.00F;
	mat.db = 1.00F;
	mat.da = 1.00F;

	mat.sr = 1.00F;
	mat.sg = 1.00F;
	mat.sb = 1.00F;
	mat.sa = 1.00F;

	mat.er = 0.0F;
	mat.eg = 0.0F;
	mat.eb = 0.0F;
	mat.ea = 1.0F;
	int tmp;
	GFXSetMaterial(tmp, mat);
	//GFXSelectMaterial(tmp);
	int ligh;
	GFXCreateLightContext(ligh);
	GFXSetLightContext (ligh);
	GFXCreateLight (ligh);
	GFXSetLight (ligh, POSITION, GFXColor (2,2,2,0));
	//	GFXSetLight (ligh, ATTENUATE, GFXColor (.3,0,1));
	GFXSetLight (ligh, DIFFUSE, GFXColor (1,1,1,1));
	GFXEnableLight (ligh);
	//GFXLoadIdentity(VIEW);
	//GFXLookAt(Vector(0,0,0), Vector(0,0,1), Vector(0,-1,0)); // optimization: cache this friggin' matrix
      	GFXEndScene();
	return TRUE;
}

void WrapGFX::Loop(void main_loop()) {
  GFXLoop(main_loop);
}

void WrapGFX::StartDraw()
{
  RESETTIME();
	GFXBeginScene();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	SetViewport();


	//HandleMouse(); //do what you will with the data. we might wanna place the DX stuff elsewhere so it can be used in a more well rounded way?
	//HandleKeyboard();

	SetViewport();
}

void WrapGFX::EndDraw()
{
	GFXEndScene();
	//REPORTTIME("End of frame");
	//glFinish();//finish all drawing commands
	//SwapBuffers(_hDC); //swap the buffers
}

void WrapGFX::Switch(){}


/************************************************************************
extern char *viddrv;

FARPROC WINAPI DliNotify(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch(dliNotify)
	{
	case dliNotePreLoadLibrary:
		pdli->szDll = viddrv;
		return 0;
		break;
	default:
		return 0;
	}
}

FARPROC WINAPI DliFailure(unsigned dliNotify, PDelayLoadInfo pdli)
{
	switch(dliNotify)
	{
	case dliFailLoadLib:
		//load a library and then return the module #;
		return 0;
		break;
	default:
		return 0;
	}
}

PfnDliHook   __pfnDliNotifyHook = DliNotify;
PfnDliHook   __pfnDliFailureHook = DliFailure;
****************************************************************/
