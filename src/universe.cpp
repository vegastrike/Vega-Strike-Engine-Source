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
#include <fcntl.h>
#include "gfxlib.h"
#include "universe.h"
#include "lin_time.h"
#include "in.h"
#include "gfx/aux_texture.h"
#include "profile.h"
#include "gfx/cockpit.h"
#include "cmd/weapon_xml.h"
//#include "mission.h"
//#include "vs_globals.h"

Universe::Universe(int argc, char** argv)
{
	//Select drivers
	

	GFXInit(argc,argv);

	StartGFX();
	InitInput();

	hud_camera = Camera();
	cockpit = new Cockpit ("",NULL);
	LoadWeapons("weapon_list.xml");
	LoadFactionXML("factions.xml");	
}


void Universe::LoadStarSystem(StarSystem * s) {
  star_system.push_back (s);
}
void Universe::UnloadStarSystem (StarSystem * s) {
  //not sure what to do here? serialize?
}
void Universe::Init (string systemfile, const Vector & centr,const string planetname) {

  string fullname=systemfile+".system";
  StarSystem * ss;
  LoadStarSystem(ss=new StarSystem((char *)fullname.c_str(),centr,planetname));
  pushActiveStarSystem (ss);
}
Universe::~Universe()
{
  DeInitInput();
  unsigned int i;
  for (i=0;i<this->factions.size();i++) {
    delete factions[i];
  }
  delete cockpit;
  GFXShutdown();
	//delete mouse;
}
//sets up all the stuff... in this case the ships to be rendered

void Universe::activateLightMap() {
	activeStarSystem()->activateLightMap();
}

void Universe::StartGFX()
{

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
	mat.power=60.0F;
	unsigned int tmp;
	GFXSetMaterial(tmp, mat);
	GFXSelectMaterial(tmp);
	int ligh;
	//	GFXSetSeparateSpecularColor (GFXTRUE);
	GFXCreateLightContext(ligh);
	GFXSetLightContext (ligh);
	GFXLightContextAmbient (GFXColor (0,0,0,1));
	/*
	  ///now planets make light
	GFXCreateLight (ligh, GFXLight(true,GFXColor (0.001,0.001,.001),GFXColor (01,1,1,1),GFXColor(0,0,0,1), GFXColor (.2,.2,.2,1), GFXColor (1,0,0)),true);
	//GFXEnableLight (ligh);
	//	GFXCreateLight (ligh, GFXLight(true,GFXColor (0.001,0.001,.001),GFXColor (1,1,.6,1),GFXColor(1,1,1,1), GFXColor (0,0,0,1), GFXColor (1,.0000,.000000004)),false);
	GFXEnableLight (ligh);
	*/
      	GFXEndScene();
}

void Universe::Loop(void main_loop()) {
  GFXLoop(main_loop);
}
extern void micro_sleep (unsigned int howmuch);
extern int getmicrosleep ();
void Universe::StartDraw()
{
#ifndef WIN32
	RESETTIME();
#endif
	GFXBeginScene();

  _Universe->activeStarSystem()->Draw();
  StarSystem::DrawJumpStars();
  UpdateTime();
  for (int i=0;i<star_system.size();i++) {
    star_system[i]->Update();
  }
  StarSystem::ProcessPendingJumps();
  //  micro_sleep (getmicrosleep());//so we don't starve the audio thread  
  GFXEndScene();





}


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
