//#include "unit.h"
//#include "unit_template.h"
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
#include "unit_const_cache.h"
#ifdef _WIN32
#define strcasecmp stricmp
#endif
extern int GetModeFromName (const char *);

extern Unit * CreateGameTurret (std::string tur,int faction);

template <class UnitType>
bool GameUnit<UnitType>::UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  bool bl = this->UpgradeSubUnitsWithFactory( up, subunitoffset, touchme, downgrade, numave, percentage,&CreateGameTurret);
  if (!up->SubUnits.empty())
    this->DisableTurretAI();
  return bl;
}
extern char * GetUnitDir (const char *);
template <class UnitType>
double GameUnit<UnitType>::Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts) {
#if 0
  if (shield.number==2) {
    printf ("shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
  }else {
    printf ("shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);    

  }
#endif
  int upgradefac=FactionUtil::GetUpgradeFaction();
  const Unit * up = UnitConstCache::getCachedConst (StringIntKey(file,upgradefac));
  if (!up) {
    up = UnitConstCache::setCachedConst (StringIntKey (file,upgradefac),
			     UnitFactory::createUnit (file.c_str(),true,upgradefac));
  }
  char * unitdir  = GetUnitDir(this->name.c_str());
  string templnam = string(unitdir)+".template";	  
  const Unit * templ = UnitConstCache::getCachedConst (StringIntKey(templnam,this->faction));
	if (templ==NULL) {
	  templ = UnitConstCache::setCachedConst (StringIntKey(templnam,this->faction),UnitFactory::createUnit (templnam.c_str(),true,this->faction));
	}
	free (unitdir);
	double percentage=0;
	if (up->name!="LOAD_FAILED") {
	  
	  for  (int i=0;percentage==0;i++ ) {
		  if (!this->Unit::Upgrade(up,mountoffset+i, subunitoffset+i, GetModeFromName(file.c_str()),force, percentage,(templ->name=="LOAD_FAILED")?NULL:templ),false,false) {
	      percentage=0;
	    }
	    if (!loop_through_mounts||(i+1>=this->GetNumMounts ())||percentage>0) {
	      break;
	    }
	  }
	}
#if 0
  if (shield.number==2) {
    printf ("shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
  }else {
    printf ("shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);    

  }
#endif

	return percentage;
}
