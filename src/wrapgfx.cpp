#include <stdio.h>
//#include <gl/gl.h>
//#include <gl/glu.h>
#include <fcntl.h>
//#include <io.h>
#include "gfxlib.h"
#include "wrapgfx.h"
#include "lin_time.h"

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
	currentcamera = 0;
	numlights = NUM_LIGHT; 
	topobject = NULL;
	active = FALSE;

	//Select drivers
	

	GFXInit(argc,argv);
	//if(TRUE)
	ForceLog = new Texture ("TechPriRGB.bmp","TechPriA.bmp");
	SquadLog = new Texture ("TechSecRGB.bmp","TechSecA.bmp");
	LightMap = new Texture("light.bmp", 1);
	//else
	//	LightMap = new Texture("light.bmp", 0);
	StartGFX();
}

WrapGFX::~WrapGFX()
{
	//if(topobject != NULL)
	//	delete topobject;
	delete LightMap;
	delete ForceLog;
	delete SquadLog;
	GFXShutdown();
	//delete mouse;
}
//sets up all the stuff... in this case the ships to be rendered

BOOL WrapGFX::StartGFX()
{
	SetViewport();
	GFXBeginScene();
	GFXMaterial mat;
	GFXGetMaterial(0, mat);
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

	mat.er = 0.10F;
	mat.eg = 0.10F;
	mat.eb = 0.10F;
	mat.ea = 0.10F;

	GFXSetMaterial(0, mat);
	GFXSelectMaterial(0);
	GFXEndScene();
	return TRUE;
}

void WrapGFX::StartDraw()
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GFXBeginScene();
//	SetViewport();


	//HandleMouse(); //do what you will with the data. we might wanna place the DX stuff elsewhere so it can be used in a more well rounded way?
	//HandleKeyboard();
	UpdateTime();

	SetViewport();
}

void WrapGFX::EndDraw()
{
	GFXEndScene();
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
