#include "unit.h"
#include "images.h"
#include "xml_serializer.h"
#include <algorithm>
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>
Unit& GetUnitMasterPartList () {
  static Unit MasterPartList ("master_part_list",true,_Universe->GetFaction("upgrades"));
  return MasterPartList;
}

Cargo * GetMasterPartList(const char *input_buffer){
  unsigned int i;
  return GetUnitMasterPartList().GetCargo (input_buffer,i);
}

void Unit::ImportPartList (const std::string& category, float price, float pricedev,  float quantity, float quantdev) {
  unsigned int numcarg = GetUnitMasterPartList().numCargo();
  for (unsigned int i=0;i<numcarg;i++) {
    Cargo c= GetUnitMasterPartList().GetCargo (i);
    if (c.category==category) {
      c.quantity=quantity-quantdev;
      c.price*=price-pricedev;
      //stupid way
      c.quantity+=quantdev*2*((float)rand())/RAND_MAX;
      c.price+=pricedev*2*((float)rand())/RAND_MAX;
      c.price=fabs(c.price);
      if (c.price <.01)
	c.price+=.01;
      c.quantity=abs (c.quantity);
      AddCargo(c,false);
    }
  }

}
extern int GetModeFromName (const char *);
vector <Cargo>& Unit::FilterDowngradeList (vector <Cargo> & mylist) {
  static bool staticrem =XMLSupport::parse_bool (vs_config->getVariable ("general","remove_impossible_downgrades","true"));
  for (unsigned int i=0;i<mylist.size();i++) {
    bool removethis=staticrem;
    if (GetModeFromName(mylist[i].content.c_str())!=2) {
      Unit * NewPart = new Unit (mylist[i].content.c_str(),false,_Universe->GetFaction("upgrades"));
      NewPart->SetFaction(faction);
      if (NewPart->name==string("LOAD_FAILED")) {
	NewPart->Kill();
	NewPart = new Unit (mylist[i].content.c_str(),false,faction);
      }
      if (NewPart->name!=string("LOAD_FAILED")) {
	int maxmountcheck = NewPart->nummounts?nummounts:1;
	for (int m=0;m<maxmountcheck;m++) {
	  int s =0;
	  for (un_iter ui=getSubUnits();s==0||((*ui)!=NULL);++ui,++s) {
	    double percent=1;
	    if (canDowngrade (NewPart,m,s,percent)) {
	      if (percent>.9) {
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

UnitImages &Unit::GetImageInformation() {
  return *image;
}
using XMLSupport::tostring;
using namespace std;
std::string CargoToString (const Cargo& cargo) {
  return string ("\t\t\t<Cargo mass=\"")+XMLSupport::tostring((float)cargo.mass)+string("\" price=\"") +XMLSupport::tostring((float)cargo.price)+ string("\" volume=\"")+XMLSupport::tostring((float)cargo.volume)+string("\" quantity=\"")+XMLSupport::tostring((int)cargo.quantity)+string("\" file=\"")+cargo.content+string("\"/>\n");
}
std::string Unit::massSerializer (const XMLType &input, void *mythis) {
  Unit * un = (Unit *)mythis;
  float mass = un->mass;
  for (unsigned int i=0;i<un->image->cargo.size();i++) {
    mass-=un->image->cargo[i].mass*un->image->cargo[i].quantity;
  }
  return XMLSupport::tostring((float)mass);
}
void Unit::SortCargo() {
  Unit *un=this;
  std::sort (un->image->cargo.begin(),un->image->cargo.end());

  for (unsigned int i=0;i+1<un->image->cargo.size();i++) {
    if (un->image->cargo[i].content==un->image->cargo[i+1].content) {
      float tmpmass = un->image->cargo[i].quantity*un->image->cargo[i].mass+un->image->cargo[i+1].quantity*un->image->cargo[i+1].mass;
      float tmpvolume = un->image->cargo[i].quantity*un->image->cargo[i].volume+un->image->cargo[i+1].quantity*un->image->cargo[i+1].volume;
      un->image->cargo[i].quantity+=un->image->cargo[i+1].quantity;
      tmpmass/=un->image->cargo[i].quantity;
      tmpvolume/=un->image->cargo[i].quantity;
      un->image->cargo[i].volume=tmpvolume;
      un->image->cargo[i].mass=tmpmass;
      un->image->cargo.erase(un->image->cargo.begin()+(i+1));//group up similar ones
      i--;
    }

  }
}
std::string Unit::cargoSerializer (const XMLType &input, void * mythis) {
  Unit * un= (Unit *)mythis;
  if (un->image->cargo.size()==0) {
    return string("0");
  }
  un->SortCargo();
  string retval("");
  if (!(un->image->cargo.empty())) {
    retval= un->image->cargo[0].category+string ("\">\n")+CargoToString(un->image->cargo[0]);
    
    for (unsigned int kk=1;kk<un->image->cargo.size();kk++) {
      if (un->image->cargo[kk].category!=un->image->cargo[kk-1].category) {
	retval+=string("\t\t</Category>\n\t\t<Category file=\"")+un->image->cargo[kk].category+string ("\">\n");
      }
      retval+=CargoToString(un->image->cargo[kk]); 
    }
    retval+=string("\t\t</Category>\n\t\t<Category file=\"nothing");
  }else {
    retval= string ("nothing");//nothing
  }
  return retval;
}



bool Unit::CanAddCargo (const Cargo &carg)const {
  float total_volume=carg.quantity*carg.volume;
  for (unsigned int i=0;i<image->cargo.size();i++) {
    total_volume+=image->cargo[i].quantity*image->cargo[i].volume;
  }
  return (total_volume<=image->cargo_volume);
}

void Unit::AddCargo (const Cargo &carg, bool sort) {
  mass+=carg.quantity*carg.mass;
  image->cargo.push_back (carg);
  if (sort)
    SortCargo();
}
int Unit::RemoveCargo (unsigned int i, int quantity,bool eraseZero) {
  assert (i<image->cargo.size());
  if (quantity>image->cargo[i].quantity)
    quantity=image->cargo[i].quantity;
  mass-=quantity*image->cargo[i].mass;
  image->cargo[i].quantity-=quantity;
  if (image->cargo[i].quantity<=0&&eraseZero)
    image->cargo.erase (image->cargo.begin()+i);
  return quantity;
}

float Unit::PriceCargo (const std::string &s) {
  Cargo tmp;
  tmp.content=s;
  vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
  float price;
  if (mycargo==image->cargo.end()) {
    Cargo * masterlist;
    if ((masterlist=GetMasterPartList (s.c_str()))!=NULL) {
      price =masterlist->price;
    } else {
      static float spacejunk=parse_float (vs_config->getVariable ("cargo","space_junk_price","10"));
      price = spacejunk;
    }
  } else {
    price = (*mycargo).price;
  }
  return price;
}
bool Unit::SellCargo (unsigned int i, int quantity, float &creds, Cargo & carg, Unit *buyer){
  if (i<0||i>=image->cargo.size()||!buyer->CanAddCargo(image->cargo[i])||mass<image->cargo[i].mass)
    return false;
  if (quantity>image->cargo[i].quantity)
    quantity=image->cargo[i].quantity;
  carg = image->cargo[i];
  creds+=quantity*buyer->PriceCargo (image->cargo[i].content);
  carg =Cargo (image->cargo[i]);
  carg.quantity=quantity;
  buyer->AddCargo (carg);
  
  RemoveCargo (i,quantity);
  return true;
}
extern void SwitchUnits (Unit *,Unit*);
void Unit::EjectCargo (unsigned int index) {
  Cargo * tmp=NULL;
  Cargo ejectedPilot;
  string name;

  Cockpit * cp = NULL;
  if (index==((unsigned int)-1)) {
    int pilotnum = _Universe->CurrentCockpit();
    name = "eject";
    if (NULL!=(cp = _Universe->isPlayerStarship (this))) {
      string playernum =string("player")+((pilotnum==0)?string(""):XMLSupport::tostring(pilotnum));
      name = vs_config->getVariable(playernum,"callsign","TigerShark");
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
    if (tmp->quantity>0) {
      Unit * cargo = new Unit (tmp->content.c_str(),false,_Universe->GetFaction("upgrades"));
      if (cargo->name=="LOAD_FAILED") {
	cargo->Kill();
	cargo = new Unit ("generic_cargo",false,_Universe->GetFaction("upgrades"));
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
	    cargo->name=tmp->content;
	  }
	}
	if (cp) {
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

bool Unit::SellCargo (const std::string &s, int quantity, float & creds, Cargo &carg, Unit *buyer){
  Cargo tmp;
  tmp.content=s;
  vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
  if (mycargo==image->cargo.end())
    return false;

  return SellCargo (mycargo-image->cargo.begin(),quantity,creds,carg,buyer);
}
unsigned int Unit::numCargo ()const {
  return image->cargo.size();
}
Cargo& Unit::GetCargo (unsigned int i) {
  return image->cargo[i];
}

float Unit::CourseDeviation (const Vector &OriginalCourse, const Vector &FinalCourse) const{
  if (ViewComputerData().max_ab_speed>.001)
    return ((OriginalCourse-(FinalCourse)).Magnitude()/ViewComputerData().max_ab_speed);
  else
    return (FinalCourse-OriginalCourse).Magnitude();
}

std::string Unit::GetManifest (unsigned int i, Unit * scanningUnit, const Vector &oldspd) const{
///FIXME somehow mangle string
  string mangled = image->cargo[i].content;
  static float scramblingmanifest=XMLSupport::parse_float (vs_config->getVariable ("general","PercentageSpeedChangeToFaultSearch",".5"));
  if (CourseDeviation (oldspd,GetVelocity())>scramblingmanifest) {
    for (string::iterator i=mangled.begin();i!=mangled.end();i++) {
      (*i)+=(rand()%3-1);
    }
  }

  return mangled;
}


Cargo* Unit::GetCargo (const std::string &s, unsigned int &i) {
  Cargo searchfor;
  searchfor.content=s;
  vector<Cargo>::iterator tmp =(std::find(image->cargo.begin(),image->cargo.end(),searchfor));
  if (tmp==image->cargo.end())
    return NULL;
  i= (tmp-image->cargo.begin());
  return &(*tmp);
}
bool Unit::BuyCargo (const Cargo &carg, float & creds){
  if (!CanAddCargo(carg)||creds<carg.quantity*carg.price) {
    return false;    
  }
  AddCargo (carg);
  creds-=carg.quantity*carg.price;
  mass+=carg.quantity*carg.mass;
  return true;
}
bool Unit::BuyCargo (unsigned int i, unsigned int quantity, Unit * seller, float&creds) {
  Cargo soldcargo= seller->image->cargo[i];
  if (quantity>(unsigned int)soldcargo.quantity)
    quantity=soldcargo.quantity;
  if (quantity==0)
    return false;
  soldcargo.quantity=quantity;
  if (BuyCargo (soldcargo,creds)) {
    seller->RemoveCargo (i,quantity,false);
    return true;
  }
  return false;
}
bool Unit::BuyCargo (const std::string &cargo,unsigned int quantity, Unit * seller, float & creds) {
  unsigned int i;
  if (seller->GetCargo(cargo,i)) {
    return BuyCargo (i,quantity,seller,creds);
  }
  return false;
}
