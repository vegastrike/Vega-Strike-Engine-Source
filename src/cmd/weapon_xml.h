
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
    PROJECTILE
  } type;
  enum MOUNT_SIZE {NOWEAP=0x0,LIGHT=0x1,MEDIUM=0x2,HEAVY=0x4,CAPSHIPLIGHT=0x8,CAPSHIPHEAVY=0x10,SPECIAL=0x20, LIGHTMISSILE=0x40,MEDIUMMISSILE=0x80,HEAVYMISSILE=0x100,CAPSHIPLIGHTMISSILE=0x200, CAPSHIPHEAVYMISSILE=0x400,SPECIALMISSILE=0x800} size;
  string file;
  int sound;
  float r,g,b,a;
  float Speed,PulseSpeed,RadialSpeed,Range,Radius, Length;
  float Damage,Stability,Longrange;
  float EnergyRate,Refire;
  weapon_info(enum WEAPON_TYPE typ) {init();Type(typ);}
  weapon_info(const weapon_info&);
  //  weapon_info& operator = (const weapon_info &tmp);
  void init(); 
  void Type (enum WEAPON_TYPE typ); 
  void MntSize(enum MOUNT_SIZE size) {this->size = size;}
};
enum weapon_info::MOUNT_SIZE lookupMountSize (const char * str);
void LoadWeapons(const char *filename);
weapon_info * getTemplate(const string &key);

#endif
