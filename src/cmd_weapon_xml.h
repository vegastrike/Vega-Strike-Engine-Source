
#ifndef _CMD_WEAPON_XML_H_
#define _CMD_WEAPON_XML_H_
#include <string>

using std::string;

struct weapon_info {
  int type;
  string file;
  float r,g,b,a;
  float Speed,PulseSpeed,RadialSpeed,Range,Radius, Length;
  float Damage,Stability,Longrange;
  float EnergyRate, EnergyConsumption,Refire;
  weapon_info(int typ) {init();Type(typ);}
  void init(); 
  void Type (int typ); 
};

void LoadWeapons(const char *filename);
weapon_info * getTemplate(const string &key);

#endif
