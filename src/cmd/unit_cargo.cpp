#include "unit.h"
#include "images.h"
#include "xml_serializer.h"
#include <algorithm>
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>
using XMLSupport::tostring;
using namespace std;
std::string CargoToString (const Cargo& cargo) {
  return tostring(cargo.mass)+string("\" price=\"") +tostring(cargo.price)+ string("\" volume=\"")+tostring(cargo.volume)+string("\" quantity=\"")+tostring(cargo.quantity)+string("\" file=\"")+cargo.content;
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
    }

  }
  string retval= CargoToString(un->image->cargo[0]);

  for (unsigned int kk=1;kk<un->image->cargo.size();kk++) {
    retval+= string ("\"/><Cargo mass=\"")+CargoToString(un->image->cargo[kk]);
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
void Unit::RemoveCargo (unsigned int i, int quantity) {
  assert (i<image->cargo.size());
  if (quantity>image->cargo[i].quantity)
    quantity=image->cargo[i].quantity;
  mass-=quantity*image->cargo[i].mass;
  image->cargo[i].quantity-=quantity;
  if (image->cargo[i].quantity<=0)
    image->cargo.erase (image->cargo.begin()+i);
}

float Unit::PriceCargo (const std::string &s) {
  Cargo tmp;
  tmp.content=s;
  vector <Cargo>::iterator mycargo = std::find (image->cargo.begin(),image->cargo.end(),tmp);
  float price;
  if (mycargo==image->cargo.end()) {
    static float spacejunk=parse_float (vs_config->getVariable ("cargo","space_junk_price","10"));
    price = spacejunk;
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
Cargo* Unit::GetCargo (const std::string &s) {
  Cargo searchfor;
  searchfor.content=s;
  vector<Cargo>::iterator tmp =(std::find(image->cargo.begin(),image->cargo.end(),searchfor));
  if (tmp==image->cargo.end())
    return NULL;
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
