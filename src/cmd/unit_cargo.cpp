//#include "unit_template.h"
#include "images.h"
#include "xml_serializer.h"
#include <algorithm>
#include "vs_globals.h"
#include "config_xml.h"
#include "unit_factory.h"
#include <assert.h>
#include "cmd/ai/aggressive.h"
#include "unit_const_cache.h"
#include <set>
static const GFXColor disable (1,0,0,1);

extern int GetModeFromName (const char *);
extern std::set <std::string> GetListOfDowngrades();
extern void ClearDowngradeMap();
extern double ComputeMinDowngradePercent();
template<class UnitType>
vector <CargoColor>& GameUnit<UnitType>::FilterDowngradeList (vector <CargoColor> & mylist, bool downgrade)
{
  const Unit * templ=NULL;
  const Unit * downgradelimit=NULL;
  static bool staticrem =XMLSupport::parse_bool (vs_config->getVariable ("general","remove_impossible_downgrades","true"));
  static float MyPercentMin= ComputeMinDowngradePercent();

  for (unsigned int i=0;i<mylist.size();i++) {
    bool removethis=staticrem;
    int mode=GetModeFromName(mylist[i].cargo.content.c_str());
    if (mode!=2 || (!downgrade)) {
      const Unit * NewPart =  UnitConstCache::getCachedConst (StringIntKey (mylist[i].cargo.content.c_str(),FactionUtil::GetFaction("upgrades")));
      if (!NewPart){
	NewPart= UnitConstCache::setCachedConst (StringIntKey (mylist[i].cargo.content,FactionUtil::GetFaction("upgrades")),UnitFactory::createUnit(mylist[i].cargo.content.c_str(),false,FactionUtil::GetFaction("upgrades")));
      }
      if (NewPart->name==string("LOAD_FAILED")) {
	const Unit * NewPart = UnitConstCache::getCachedConst (StringIntKey (mylist[i].cargo.content.c_str(),faction));
	if (!NewPart){
	  NewPart= UnitConstCache::setCachedConst (StringIntKey (mylist[i].cargo.content, faction),
				       UnitFactory::createUnit(mylist[i].cargo.content.c_str(),false,faction));
	}
      }
      if (NewPart->name!=string("LOAD_FAILED")) {
	int maxmountcheck = NewPart->GetNumMounts()?GetNumMounts():1;
	char * unitdir  = GetUnitDir(name.c_str());
	string templnam = string(unitdir)+".template";
        string limiternam = string(unitdir)+".blank";
        
	if (!downgrade) {
	  templ = UnitConstCache::getCachedConst (StringIntKey(templnam,faction));
	  if (templ==NULL) {
	    templ = UnitConstCache::setCachedConst (StringIntKey(templnam,faction),UnitFactory::createUnit (templnam.c_str(),true,this->faction));
	  }
          if (templ->name == std::string("LOAD_FAILED")) templ = NULL;

        }else {
            downgradelimit = UnitConstCache::getCachedConst (StringIntKey(limiternam,faction));
            if (downgradelimit==NULL) {
                downgradelimit = UnitConstCache::setCachedConst (StringIntKey (limiternam,faction),UnitFactory::createUnit(limiternam.c_str(),true,this->faction));
            }
            if (downgradelimit->name == std::string("LOAD_FAILED")) downgradelimit = NULL;
        }
	free (unitdir);
	for (int m=0;m<maxmountcheck;m++) {
	  int s =0;
	  for (un_iter ui=getSubUnits();s==0||((*ui)!=NULL);++ui,++s) {
	    double percent=1;
	    if (downgrade) {
	      if (canDowngrade (NewPart,m,s,percent,downgradelimit)) {
		if (percent>MyPercentMin) {
		  removethis=false;
		  break;
		}
	      }  
	    }else {

	      if (canUpgrade (NewPart,m,s,mode,false/*force*/, percent,templ)) {
		removethis=false;
		break;
	      }
	    }
	    
	    if (*ui==NULL) {
	    break;
	    }
	  }
	}
      }
  
    } else {
      removethis=true;
    }
    if (removethis) {
      if (downgrade) {
        mylist.erase (mylist.begin()+i);	  
        i--;
      } else {
        mylist[i].color=disable;
      }
    }
  }
  return mylist;
}

template<class UnitType>
vector <CargoColor>& GameUnit<UnitType>::FilterUpgradeList (vector <CargoColor> & mylist) {
	static bool filtercargoprice = XMLSupport::parse_bool (vs_config->getVariable ("cargo","filter_expensive_cargo","false"));
	if (filtercargoprice) {
	Cockpit * cp = _Universe->isPlayerStarship (this);
	if (cp) {
	  for (unsigned int i=0;i<mylist.size();i++) {
	    if (mylist[i].cargo.price>cp->credits) {
//	      mylist.erase (mylist.begin()+i);
//	      i--;
          mylist[i].color=disable;
	    }
	  }
	}
	}
	return FilterDowngradeList(mylist,false);
}
inline float uniformrand (float min, float max) {
	return ((float)(rand ())/RAND_MAX)*(max-min)+min;
}
inline QVector randVector (float min, float max) {
	return QVector (uniformrand(min,max),
					uniformrand(min,max),
					uniformrand(min,max));
}
extern void SwitchUnits (Unit *,Unit*);
template<class UnitType>
void GameUnit<UnitType>::EjectCargo (unsigned int index) {
  Cargo * tmp=NULL;
  Cargo ejectedPilot;
  string name;

  Cockpit * cp = NULL;
  if (index==((unsigned int)-1)) {
    int pilotnum = _Universe->CurrentCockpit();
    name = "Pilot";
    if (NULL!=(cp = _Universe->isPlayerStarship (this))) {
      string playernum =string("player")+((pilotnum==0)?string(""):XMLSupport::tostring(pilotnum));
      //name = vs_config->getVariable(playernum,"callsign","TigerShark");
    }
    ejectedPilot.content="eject";
    ejectedPilot.mass=.1;
    ejectedPilot.volume=1;
    tmp = &ejectedPilot;
  }
  if (index<numCargo()) {
    tmp = &GetCargo (index);
  }
  if (tmp) {
    string tmpcontent=tmp->content;
    if (tmp->mission)
      tmpcontent="Mission_Cargo";

    if (tmp->quantity>0) {
      const int sslen=strlen("starships");
      Unit * cargo = NULL;
      if (tmp->category.length()>=(unsigned int)sslen) {
	if ((!tmp->mission)&&memcmp (tmp->category.c_str(),"starships",sslen)==0) {
	  string ans = tmpcontent;
	  unsigned int blank = ans.find (".blank");
	  if (blank != string::npos) {
	    ans = ans.substr (0,blank);
	  }
	  Flightgroup * fg = this->getFlightgroup();
	  int fgsnumber=0;
	  if (fg!=NULL) {
	    fgsnumber=fg->nr_ships;
	    fg->nr_ships++;
	    fg->nr_ships_left++;
	  }
	  cargo = UnitFactory::createUnit (ans.c_str(),false,faction,"",fg,fgsnumber);
	  cargo->PrimeOrders();
	  cargo->SetAI (new Orders::AggressiveAI ("default.agg.xml","default.int.xml"));
	  cargo->SetTurretAI();	  
	  //he's alive!!!!!
	}
      }
      if (!cargo) {
		  if (tmpcontent=="eject") {
			  cargo = UnitFactory::createUnit ("eject",false,faction);
		  }else {
			  string tmpnam = tmpcontent+".cargo";
			  cargo = UnitFactory::createUnit (tmpnam.c_str(),false,FactionUtil::GetFaction("upgrades"));
		  }
      }
      if (cargo->name=="LOAD_FAILED") {
	cargo->Kill();
	cargo = UnitFactory::createUnit ("generic_cargo",false,FactionUtil::GetFaction("upgrades"));
      }
      if (cargo->rSize()>=rSize()) {
	cargo->Kill();
      }else {
	cargo->SetPosAndCumPos (Position()+randVector(-rSize(), rSize()));
	cargo->SetOwner (this);
	cargo->SetVelocity(Velocity+randVector(-.25,.25).Cast());
	cargo->mass = tmp->mass;
	if (name.length()>0) {
	  cargo->name=name;
	} else {
	  if (tmp) {
	    cargo->name=tmpcontent;
	  }
	}
	if (cp&&_Universe->numPlayers()==1) {
	  cargo->SetOwner(NULL);
	  PrimeOrders();
	  cargo->SetTurretAI();
	  cargo->faction=faction;
	  cp->SetParent (cargo,"","",Position());
	  SwitchUnits (NULL,cargo);
	}
	_Universe->activeStarSystem()->AddUnit(cargo);
	if ((unsigned int) index!=((unsigned int)-1)) {
	  if (index<image->cargo.size()) {
	    RemoveCargo (index,1,true);
	  }
	}
      }
    }

  }
}
