#include "unit.h"
#include "unit_factory.h"
#include "images.h"
#include "universe.h"
#include "vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "gfx/cockpit.h"
#include "savegame.h"
#include "config_xml.h"
#include "xml_serializer.h"
#include "audiolib.h"
#include "vs_globals.h"
#ifdef _WIN32
#define strcasecmp stricmp
#endif
extern int GetModeFromName (const char *);

void GameUnit::GameMount::ReplaceMounts (const Unit::Mount *other) {
  Mount::ReplaceMounts( other);
  sound = AUDCreateSound (sound,type->type!=weapon_info::PROJECTILE);//copy constructor basically
}


bool GameUnit::UpgradeSubUnits (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  bool bl = Unit::UpgradeSubUnits( up, subunitoffset, touchme, downgrade, numave, percentage);
  up->DisableTurretAI();
  DisableTurretAI();
  return bl;
}
extern char * GetUnitDir (const char *);
double GameUnit::Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts) {
#if 0
  if (shield.number==2) {
    printf ("shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
  }else {
    printf ("shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);    

  }
#endif
	GameUnit * up = GameUnitFactory::createUnit (file.c_str(),true,FactionUtil::GetFaction("upgrades"));
	static GameUnit * last_template=NULL;
	char * unitdir  = GetUnitDir(name.c_str());
	
	GameUnit * templ = NULL;
	if (last_template!=NULL) {
	  if (last_template->name==(string (unitdir)+".template")) {
	    templ = last_template;
#if 0
	    printf ("cache hit");
#endif
	  }else {
	    last_template->Kill();
	    last_template=NULL;
	  }
	}
	if (templ==NULL) {
	  templ = GameUnitFactory::createUnit ((string (unitdir)+".template").c_str(),true,this->faction);
	  last_template=templ;
	}
	free (unitdir);
	double percentage=0;
	if (up->name!="LOAD_FAILED") {
	  
	  for  (int i=0;percentage==0;i++ ) {
		  if (!this->Unit::Upgrade(up,mountoffset+i, subunitoffset+i, GetModeFromName(file.c_str()),force, percentage,(templ->name=="LOAD_FAILED")?NULL:templ)) {
	      percentage=0;
	    }
	    if (!loop_through_mounts||(i+1>=this->GetNumMounts ())) {
	      break;
	    }
	  }
	}
	up->Kill();
#if 0
  if (shield.number==2) {
    printf ("shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
  }else {
    printf ("shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);    

  }
#endif

	return percentage;
}
