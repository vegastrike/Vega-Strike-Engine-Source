
#ifndef _CMD_WEAPON_XML_H_
#define _CMD_WEAPON_XML_H_
#include <string>




using std::string;
struct weapon_info {
  enum WEAPON_TYPE {
    UNKNOWN,
    BEAM,
    BALL,
    BOLT,
    MISSILE
  } type;
  string file;
  float r,g,b,a;
  float Speed,PulseSpeed,RadialSpeed,Range,Radius, Length;
  float Damage,Stability,Longrange;
  float EnergyRate, EnergyConsumption,Refire;
  weapon_info(enum WEAPON_TYPE typ) {init();Type(typ);}
  weapon_info(const weapon_info&);
  weapon_info& operator = (const weapon_info &tmp);
  void init(); 
  void Type (enum WEAPON_TYPE typ); 
};

void LoadWeapons(const char *filename);
weapon_info * getTemplate(const string &key);

#endif
