#include "unit.h"
#include "images.h"
#include "xml_serializer.h"
#include <algorithm>
#include "vs_globals.h"
#include "config_xml.h"
#include "unit_factory.h"
#include <assert.h>
#include "cmd/ai/aggressive.h"

extern int GetModeFromName (const char *);
vector <Cargo>& GameUnit::FilterDowngradeList (vector <Cargo> & mylist)
{
  static bool staticrem =XMLSupport::parse_bool (vs_config->getVariable ("general","remove_impossible_downgrades","true"));
  static float MyPercentMin = XMLSupport::parse_float (vs_config->getVariable("general","remove_downgrades_less_than_percent",".9"));
  for (unsigned int i=0;i<mylist.size();i++) {
    bool removethis=staticrem;
    if (GetModeFromName(mylist[i].content.c_str())!=2) {
      Unit * NewPart = UnitFactory::createUnit(mylist[i].content.c_str(),false,FactionUtil::GetFaction("upgrades"));
      NewPart->SetFaction(faction);
      if (NewPart->name==string("LOAD_FAILED")) {
	NewPart->Kill();
	NewPart = UnitFactory::createUnit (mylist[i].content.c_str(),false,faction);
      }
      if (NewPart->name!=string("LOAD_FAILED")) {
	int maxmountcheck = NewPart->GetNumMounts()?GetNumMounts():1;
	for (int m=0;m<maxmountcheck;m++) {
	  int s =0;
	  for (un_iter ui=getSubUnits();s==0||((*ui)!=NULL);++ui,++s) {
	    double percent=1;
	    if (canDowngrade (NewPart,m,s,percent)) {
	      if (percent>MyPercentMin) {
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
      NewPart->Kill();
    } else {
      removethis=true;
    }
    if (removethis) {
      mylist.erase (mylist.begin()+i);
      i--;
    }
  }
  return mylist;
}

vector <Cargo>& GameUnit::FilterUpgradeList (vector <Cargo> & mylist) {
	static bool filtercargoprice = XMLSupport::parse_bool (vs_config->getVariable ("cargo","filter_expensive_cargo","false"));
	if (filtercargoprice) {
	Cockpit * cp = _Universe->isPlayerStarship (this);
  if (cp) {
    for (unsigned int i=0;i<mylist.size();i++) {
      if (mylist[i].price>cp->credits) {
	mylist.erase (mylist.begin()+i);
	i--;
      }
    }
  }
	}
  return mylist;
}






extern void SwitchUnits (Unit *,Unit*);
void GameUnit::EjectCargo (unsigned int index) {
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
      if (tmp->category.length()>=sslen) {
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
		  cargo = UnitFactory::createUnit (tmpcontent.c_str(),false,FactionUtil::GetFaction("upgrades"));
      }
      if (cargo->name=="LOAD_FAILED") {
	cargo->Kill();
	cargo = UnitFactory::createUnit ("generic_cargo",false,FactionUtil::GetFaction("upgrades"));
      }
      if (cargo->rSize()>=rSize()) {
	cargo->Kill();
      }else {
	cargo->SetPosAndCumPos (Position());
	cargo->SetOwner (this);
	cargo->SetVelocity(Velocity);
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


