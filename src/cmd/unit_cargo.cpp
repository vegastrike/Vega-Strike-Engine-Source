#include "unit.h"
#include "images.h"
#include "xml_serializer.h"
#include <algorithm>
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>

Cargo * GetMasterPartList(const char *input_buffer){
  static Unit MasterPartList ("master_part_list",false,_Universe->GetFaction("upgrades"));
  unsigned int i;
  return MasterPartList.GetCargo (input_buffer,i);
}


using XMLSupport::tostring;
using namespace std;
std::string CargoToString (const Cargo& cargo) {
  return string ("\t\t\t<Cargo mass=\"")+XMLSupport::tostring((float)cargo.mass)+string("\" price=\"") +XMLSupport::tostring((float)cargo.price)+ string("\" volume=\"")+XMLSupport::tostring((float)cargo.volume)+string("\" quantity=\"")+XMLSupport::tostring((int)cargo.quantity)+string("\" file=\"")+cargo.content+string("/>\n");
}

std::string Unit::cargoSerializer (const XMLType &input, void * mythis) {
  Unit * un= (Unit *)mythis;
  if (un->image->cargo.size()==0) {
    return string("0");
  }
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

void Unit::AddCargo (const Cargo &carg) {
  mass+=carg.quantity*carg.mass;
  image->cargo.push_back (carg);
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
