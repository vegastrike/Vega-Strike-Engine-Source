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
#include "galaxy_xml.h"
#include <algorithm>
#include "config_xml.h"
#include "vs_globals.h"
#include "xml_support.h"


#include "cmd/script/mission.h"
#include "cmd/unit.h"
#if defined(WITH_MACOSX_BUNDLE)
#import <sys/param.h>
#endif
#include "savegame.h"
using namespace std;
///Decides whether to toast the jump star from the cache
extern void CacheJumpStar (bool);
Universe::Universe(int argc, char** argv, const char * galaxy, const vector<std::string> &playerNames)
{
	current_cockpit=0;
#if defined(WITH_MACOSX_BUNDLE)
    // get the current working directory so when glut trashes it we can restore.
    char pwd[MAXPATHLEN];
    getcwd (pwd,MAXPATHLEN);
#endif
	//Select drivers
	GFXInit(argc,argv);
#if defined(WITH_MACOSX_BUNDLE)
    // Restore it
    chdir(pwd);
#endif
	StartGFX();
	InitInput();

	hud_camera = Camera();
	LoadWeapons("weapon_list.xml");
	LoadFactionXML("factions.xml");	
	this->galaxy = new GalaxyXML::Galaxy (galaxy);

	script_system=NULL;
}
void Universe::SetupCockpits(vector  <string> playerNames) {
	for (unsigned int i=0;i<playerNames.size();i++) {
	  cockpit.push_back( new Cockpit ("",NULL,playerNames[i]));
	}
}
void Universe::LoadStarSystem(StarSystem * s) {
  star_system.push_back (s);
  
}
bool Universe::StillExists (StarSystem * s) {
  return std::find (star_system.begin(),star_system.end(),s)!=star_system.end();
}

void Universe::UnloadStarSystem (StarSystem * s) {
  //not sure what to do here? serialize?
}
void Universe::Init (string systemfile, const Vector & centr,const string planetname) {
  CacheJumpStar(false);
  string fullname=systemfile+".system";
  StarSystem * ss;
  ss=GenerateStarSystem((char *)fullname.c_str(),"",centr);
}
Universe::~Universe()
{
  CacheJumpStar(true);
  DeInitInput();
  unsigned int i;
  for (i=0;i<this->factions.size();i++) {
    delete factions[i];
  }
  for (i=0;i<cockpit.size();i++) {
    delete cockpit[i];
  }
  cockpit.clear();
  GFXShutdown();
	//delete mouse;
}
//sets up all the stuff... in this case the ships to be rendered

void Universe::activateLightMap() {
	getActiveStarSystem(0)->activateLightMap();
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
void Universe::SetActiveCockpit (int i) {
  current_cockpit=i;
}
void Universe::SetActiveCockpit (Cockpit * cp) {
  for (unsigned int i=0;i<cockpit.size();i++) {
    if (cockpit[i]==cp) {
      SetActiveCockpit (i);
      return;
    }
  }
}
Cockpit * Universe::isPlayerStarship(Unit * un) {
  if (!un)
    return NULL;
  for (unsigned int i=0;i<cockpit.size();i++) {
    if (un==cockpit[i]->GetParent())
      return cockpit[i];
  }
  return NULL;
}
void Universe::Loop(void main_loop()) {
  GFXLoop(main_loop);
}
extern void micro_sleep (unsigned int howmuch);
extern int getmicrosleep ();
void SortStarSystems (std::vector <StarSystem *> &ss, StarSystem * drawn) {
  if ((*ss.begin())==drawn) {
    return;
  }
  vector<StarSystem*>::iterator drw = std::find (ss.begin(),ss.end(),drawn);
  if (drw!=ss.end()) {
    StarSystem * tmp=drawn;
    vector<StarSystem*>::iterator i=ss.begin();
    while (i<=drw) {
      StarSystem * t=*i;
      *i=tmp;
      tmp = t;
      i++;
    }
  }
}

void CalculateCoords (unsigned int i,unsigned int size, float &x,float &y,float &w,float &h){
  if (size<=1) {
    x=y=0;
    w=h=1;
    return;
  }
  if (size<=3||i<(size/2)) {
    y=0;
    h=1;
    w=1./((float)size);
    x= ((float)i)/size;
    if (size>3) {
      h=.5;
    }
  }
  if (size>3) {
    if (i>size/2) {
      y=.5;
      h=.5;
      x=((float)i-(size/2))/(size-size/2);
      w=1/(size-size/2);
    }
  }
}

extern float rand01();
void Universe::StartDraw()
{
#ifndef WIN32
  RESETTIME();
#endif
  GFXBeginScene();
  unsigned int i;
  StarSystem * lastStarSystem = NULL;
  for (i=0;i<cockpit.size();i++) {
    SetActiveCockpit (i);
    float x,y,w,h;
    CalculateCoords (i,cockpit.size(),x,y,w,h);
    AccessCamera()->SetSubwindow (x,y,w,h);
    if (cockpit.size()>1&&AccessCockpit(i)->activeStarSystem!=lastStarSystem) {
      active_star_system[0]->SwapOut();
      lastStarSystem=AccessCockpit()->activeStarSystem;
      active_star_system[0]=lastStarSystem;
      lastStarSystem->SwapIn();
    }
    AccessCockpit()->SelectProperCamera();
    if (cockpit.size()>0)
      AccessCamera()->UpdateGFX();
    activeStarSystem()->Draw();
    AccessCamera()->SetSubwindow (0,0,1,1);      
  }
  UpdateTime();
  static float nonactivesystemtime = XMLSupport::parse_float (vs_config->getVariable ("physics","InactiveSystemTime",".3"));
  static unsigned int numrunningsystems = XMLSupport::parse_int (vs_config->getVariable ("physics","NumRunningSystems","4"));
  float systime=nonactivesystemtime;
  _Universe->SetActiveCockpit (((int)(rand01()*cockpit.size()))%cockpit.size());
  
  for (i=0;i<star_system.size()&&i<numrunningsystems;i++) {
    star_system[i]->Update((i==0)?1:systime/i,true);
  }
  StarSystem::ProcessPendingJumps();
  for (i=0;i<cockpit.size();i++) {
    SetActiveCockpit(i);
    cockpit[i]->Update();
    ProcessInput(i);
  }
  //  micro_sleep (getmicrosleep());//so we don't starve the audio thread  
  GFXEndScene();
  //remove systems not recently visited?
  static int sorttime=0;
  static int howoften = XMLSupport::parse_int(vs_config->getVariable ("general","garbagecollectfrequency","20"));
  if (howoften!=0) {
    if ((sorttime++)%howoften==1) {
      SortStarSystems(star_system,active_star_system.back());
      static unsigned int numrunningsystems = XMLSupport::parse_int(vs_config->getVariable ("general","numoldsystems","6"));
      static bool deleteoldsystems = XMLSupport::parse_bool (vs_config->getVariable ("general","deleteoldsystems","true"));
      if (star_system.size()>numrunningsystems&&deleteoldsystems) {
	if (std::find (active_star_system.begin(),active_star_system.end(),star_system.back())==active_star_system.end()) {
	  delete star_system.back();
	  star_system.pop_back();
	} else {
	  fprintf (stderr,"error with active star system list\n");
	}
      }
    }
  }
  

}
void Universe::WriteSaveGame () 
{
  for (unsigned int i=0;i<cockpit.size();i++) {
    if (AccessCockpit(i)) {
      if (AccessCockpit(i)->GetParent()) {
	if(AccessCockpit(i)->GetParent()->GetHull()>0) {  
	  AccessCockpit(i)->savegame->WriteSaveGame (AccessCockpit(i)->activeStarSystem->getFileName().c_str(),AccessCockpit(i)->GetParent()->Position(),AccessCockpit(i)->credits,AccessCockpit(i)->GetUnitFileName());
	  AccessCockpit(i)->GetParent()->WriteUnit(AccessCockpit(i)->GetUnitModifications().c_str());
	} 
      }
    }
  }
}


StarSystem *Universe::getStarSystem(string name){

  vector<StarSystem*>::iterator iter;

  for(iter = star_system.begin();iter!=star_system.end();iter++){
    StarSystem *ss=*iter;
    if(ss->getName()==name){
      return ss;
    }
  }

  return NULL;
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
