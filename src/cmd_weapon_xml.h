
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
  weapon_info() {init();}
  void init() {r=g=b=a=.5;Speed=10;PulseSpeed=15;RadialSpeed=1;Range=100;Radius=.5;Damage=1.8;Stability=.5;Longrange=.5;EnergyRate=18;EnergyConsumption=18;}
};

void LoadWeapons(const char *filename);
weapon_info * getTemplate(const string &key);

#endif
