
#ifndef _CMD_WEAPON_XML_H_
#define _CMD_WEAPON_XML_H_
#include <string>
#include "gfx/vec.h"



using std::string;
struct weapon_info {
  enum WEAPON_TYPE {
    UNKNOWN,
    BEAM,
    BALL,
    BOLT,
    PROJECTILE
  } type;
  enum MOUNT_SIZE {NOWEAP=0x0,LIGHT=0x1,MEDIUM=0x2,HEAVY=0x4,CAPSHIPLIGHT=0x8,CAPSHIPHEAVY=0x10,SPECIAL=0x20, LIGHTMISSILE=0x40,MEDIUMMISSILE=0x80,HEAVYMISSILE=0x100,CAPSHIPLIGHTMISSILE=0x200, CAPSHIPHEAVYMISSILE=0x400,SPECIALMISSILE=0x800, AUTOTRACKING=0x1000} size;
  string file;
  string weapon_name;
  Vector offset;
  int role_bits;
  int sound;
  float r,g,b,a;
  float Speed,PulseSpeed,RadialSpeed,Range,Radius, Length;
  float Damage,PhaseDamage,Stability,Longrange,LockTime;
  float EnergyRate,Refire,volume;
  float TextureStretch;
  void init() {TextureStretch=1;role_bits=0;offset=Vector(0,0,0);size=NOWEAP;r=g=b=a=127;Length=5;Speed=10;PulseSpeed=15;RadialSpeed=1;Range=100;Radius=.5;Damage=1.8;PhaseDamage=0;Stability=60;Longrange=.5;LockTime=0;EnergyRate=18;Refire=.2;sound=-1;volume=0;} 
  void Type (enum WEAPON_TYPE typ) {type=typ;switch(typ) {case BOLT:file=string("");break;case BEAM:file=string("beamtexture.bmp");break;case BALL:file=string("ball.ani");break;case PROJECTILE:file=string("missile.xmesh");break;default:break;}} 
  void MntSize(enum MOUNT_SIZE size) {this->size = size;}
  weapon_info(enum WEAPON_TYPE typ) {init();Type(typ);}
  weapon_info::weapon_info(const weapon_info &tmp) {*this = tmp;}
  //  weapon_info& operator = (const weapon_info &tmp);
};
enum weapon_info::MOUNT_SIZE lookupMountSize (const char * str);
std::string lookupMountSize (int size);
void LoadWeapons(const char *filename);
weapon_info * getTemplate(const string &key);

#endif
