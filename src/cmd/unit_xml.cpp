#include "unit.h"

#include "xml_support.h"
#include "gfx/halo.h"
//#include <iostream.h>
#include <fstream>
#include <expat.h>
//#include <values.h>
#include <float.h>
#include "gfx/mesh.h"
#include "gfx/sphere.h"
#include "gfx/bsp.h"
#include "gfx/sprite.h"
#include "audiolib.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include <assert.h>
#include "images.h"
#include "collide/rapcol.h"
#define VS_PI 3.1415926536
void Unit::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((Unit*)userData)->beginElement(name, AttributeList(atts));
}

void Unit::endElement(void *userData, const XML_Char *name) {
  ((Unit*)userData)->endElement(name);
}


namespace UnitXML {
    enum Names {
      UNKNOWN,
      UNIT,
      SUBUNIT,
      MESHFILE,
      SHIELDMESH,
      BSPMESH,
      MOUNT,
      MESHLIGHT,
      DOCK,
      XFILE,
      X,
      Y,
      Z,
      RI,
      RJ,
      RK,
      QI,
      QJ,
      QK,
      RED,
      GREEN,
      BLUE,
      ALPHA,
      MOUNTSIZE,
      WEAPON,
      DEFENSE,
      ARMOR,
      FORWARD,
      RETRO,
      FRONT,
      BACK,
      LEFT,
      RIGHT,
      TOP,
      BOTTOM,
      SHIELDS,
      RECHARGE,
      LEAK,
      HULL,
      STRENGTH,
      STATS,
      MASS,
      MOMENTOFINERTIA,
      FUEL,
      THRUST,
      MANEUVER,
      YAW,
      ROLL,
      PITCH,
      ENGINE,
      COMPUTER,
      AACCEL,
      ENERGY,
      REACTOR,
      LIMIT,
      RESTRICTED,
      MAX,
      MIN,
      MAXSPEED,
      AFTERBURNER,
      SHIELDTIGHT,
      ITTS,
      AMMO,
      HUDIMAGE,
      SOUND,
      EEERROR,
      MAXCONE,
      RANGE,
      ISCOLOR,
      RADAR,
      CLOAK,
      CLOAKRATE,
      CLOAKMIN,
      CLOAKENERGY,
      CLOAKGLASS,
      CLOAKWAV,
      CLOAKMP3,
      ENGINEWAV,
      ENGINEMP3,
      HULLWAV,
      HULLMP3,
      ARMORWAV,
      ARMORMP3,
      SHIELDWAV,
      SHIELDMP3,
      EXPLODEWAV,
      EXPLODEMP3,
      COCKPIT,
      JUMP,
      DELAY,
      JUMPENERGY,
      JUMPWAV,
      DOCKINTERNAL
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Unit", UNIT),
    EnumMap::Pair ("SubUnit", SUBUNIT),
    EnumMap::Pair ("Sound", SOUND),
    EnumMap::Pair ("MeshFile", MESHFILE),
    EnumMap::Pair ("ShieldMesh",SHIELDMESH),
    EnumMap::Pair ("BspMesh",BSPMESH),
    EnumMap::Pair ("Light",MESHLIGHT),
    EnumMap::Pair ("Defense", DEFENSE),
    EnumMap::Pair ("Armor", ARMOR),
    EnumMap::Pair ("Shields", SHIELDS),
    EnumMap::Pair ("Hull", HULL),
    EnumMap::Pair ("Stats", STATS),
    EnumMap::Pair ("Thrust", THRUST),
    EnumMap::Pair ("Maneuver", MANEUVER),
    EnumMap::Pair ("Engine", ENGINE),
    EnumMap::Pair ("Computer",COMPUTER),
    EnumMap::Pair ("Cloak", CLOAK),
    EnumMap::Pair ("Energy", ENERGY),
    EnumMap::Pair ("Reactor", REACTOR),
    EnumMap::Pair ("Restricted", RESTRICTED),
    EnumMap::Pair ("Yaw", YAW),
    EnumMap::Pair ("Pitch", PITCH),
    EnumMap::Pair ("Roll", ROLL),
    EnumMap::Pair ("Mount", MOUNT),
    EnumMap::Pair ("Radar", RADAR),
    EnumMap::Pair ("Cockpit", COCKPIT),
    EnumMap::Pair ("Jump", JUMP),
    EnumMap::Pair ("Dock", DOCK)
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("file", XFILE), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z), 
    EnumMap::Pair ("ri", RI), 
    EnumMap::Pair ("rj", RJ), 
    EnumMap::Pair ("rk", RK), 
    EnumMap::Pair ("qi", QI),     
    EnumMap::Pair ("qj", QJ),     
    EnumMap::Pair ("qk", QK),
    EnumMap::Pair ("red",RED),
    EnumMap::Pair ("green",GREEN),
    EnumMap::Pair ("blue",BLUE),    
    EnumMap::Pair ("alpha",ALPHA),
    EnumMap::Pair ("size", MOUNTSIZE),
    EnumMap::Pair ("forward",FORWARD),
    EnumMap::Pair ("retro", RETRO),    
    EnumMap::Pair ("front", FRONT),
    EnumMap::Pair ("back", BACK),
    EnumMap::Pair ("left", LEFT),
    EnumMap::Pair ("right", RIGHT),
    EnumMap::Pair ("top", TOP),
    EnumMap::Pair ("bottom", BOTTOM),
    EnumMap::Pair ("recharge", RECHARGE),
    EnumMap::Pair ("leak", LEAK),
    EnumMap::Pair ("strength", STRENGTH),
    EnumMap::Pair ("mass", MASS),
    EnumMap::Pair ("momentofinertia", MOMENTOFINERTIA),
    EnumMap::Pair ("fuel", FUEL),
    EnumMap::Pair ("yaw", YAW),
    EnumMap::Pair ("pitch", PITCH),
    EnumMap::Pair ("roll", ROLL),
    EnumMap::Pair ("accel", AACCEL),
    EnumMap::Pair ("limit", LIMIT),
    EnumMap::Pair ("max", MAX),
    EnumMap::Pair ("min", MIN),
    EnumMap::Pair ("weapon", WEAPON),
    EnumMap::Pair ("maxspeed", MAXSPEED),
    EnumMap::Pair ("afterburner", AFTERBURNER),
    EnumMap::Pair ("tightness",SHIELDTIGHT),
    EnumMap::Pair ("itts",ITTS),
    EnumMap::Pair ("ammo", AMMO),
    EnumMap::Pair ("HudImage",HUDIMAGE),
    EnumMap::Pair ("Engine",MAXCONE),
    EnumMap::Pair ("Error",EEERROR),
    EnumMap::Pair ("Range",RANGE),
    EnumMap::Pair ("EngineMp3",ENGINEMP3),
    EnumMap::Pair ("EngineWav",ENGINEWAV),
    EnumMap::Pair ("HullMp3",HULLMP3),
    EnumMap::Pair ("HullWav",HULLWAV),
    EnumMap::Pair ("ArmorMp3",ARMORMP3),
    EnumMap::Pair ("ArmorWav",ARMORWAV),
    EnumMap::Pair ("ShieldMp3",SHIELDMP3),
    EnumMap::Pair ("ShieldWav",SHIELDWAV),
    EnumMap::Pair ("ExplodeMp3",EXPLODEMP3),
    EnumMap::Pair ("ExplodeWav",EXPLODEWAV),
    EnumMap::Pair ("CloakRate",CLOAKRATE),
    EnumMap::Pair ("CloakGlass",CLOAKGLASS),
    EnumMap::Pair ("CloakEnergy",CLOAKENERGY),
    EnumMap::Pair ("CloakMin",CLOAKMIN),
    EnumMap::Pair ("CloakMp3",CLOAKMP3),
    EnumMap::Pair ("CloakWav",CLOAKWAV),
    EnumMap::Pair ("Color",ISCOLOR),
    EnumMap::Pair ("Restricted", RESTRICTED),
    EnumMap::Pair ("Delay", DELAY),
    EnumMap::Pair ("JumpEnergy", JUMPENERGY),
    EnumMap::Pair ("JumpWav", JUMPWAV),
    EnumMap::Pair ("DockInternal", DOCKINTERNAL)
    

};

  const EnumMap element_map(element_names, 29);
  const EnumMap attribute_map(attribute_names, 69);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace UnitXML;

int parseMountSizes (const char * str) {
  char tmp[13][50];
  int ans = weapon_info::NOWEAP;
  int num= sscanf (str,"%s %s %s %s %s %s %s %s %s %s %s %s %s",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],tmp[9],tmp[10],tmp[11],tmp[12]);
  for (int i=0;i<num;i++) {
    ans |= lookupMountSize (tmp[i]);
  }
  return ans;
}
static short CLAMP_SHORT(float x) {return (short)(((x)>65536)?65536:((x)<0?0:(x)));}  
void Unit::beginElement(const string &name, const AttributeList &attributes) {
    string filename;
    Vector P;
    int indx;
    short ammo=-1;
    Vector Q;
    Vector R;
    Vector pos;
    bool tempbool;
    float fbrltb[6];
    Names elem = (Names)element_map.lookup(name);
    int mntsiz=weapon_info::NOWEAP;
    AttributeList::const_iterator iter;
    float halocolor[4];

  switch(elem) {
  case UNKNOWN:
	xml->unitlevel++;

//    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    break;
 case SHIELDMESH:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->shieldmesh =(new Mesh((*iter).value.c_str(), true, faction));
	break;
      case SHIELDTIGHT: 
	shieldtight = parse_float ((*iter).value);
	break;
      }
    }
    break;
  case BSPMESH:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    xml->hasBSP = false;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->bspmesh =(new Mesh((*iter).value.c_str(), true, faction));
	xml->hasBSP = true;	
	break;
      }
    }
    break;

  case MESHFILE:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->meshes.push_back(new Mesh((*iter).value.c_str(), true, faction));
	break;
      }
    }
    break;
  case DOCK:
    tempbool=true;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    pos=Vector(0,0,0);
    P=Vector (1,1,1);
    Q=Vector (FLT_MAX,FLT_MAX,FLT_MAX);
    R=Vector (FLT_MAX,FLT_MAX,FLT_MAX);
    
    for (iter = attributes.begin();iter!=attributes.end();iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case DOCKINTERNAL:
	tempbool=parse_bool ((*iter).value);
	break;
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case TOP:
	R.j=parse_float((*iter).value);
	break;
      case BOTTOM:
	Q.j=parse_float((*iter).value);
	break;
      case LEFT:
	Q.i=parse_float((*iter).value);
	break;
      case RIGHT:
	R.i=parse_float((*iter).value);
	break;
      case BACK:
	Q.k=parse_float((*iter).value);
	break;
      case FRONT:
	R.k=parse_float((*iter).value);
	break;
      case MOUNTSIZE:
	P.i=parse_float((*iter).value);
	P.j=parse_float((*iter).value);
	break;
      }
    }
    if (Q.i==FLT_MAX||Q.j==FLT_MAX||Q.k==FLT_MAX||R.i==FLT_MAX||R.j==FLT_MAX||R.k==FLT_MAX) {
      image->dockingports.push_back (DockingPorts(pos,P.i,tempbool));
    }else {
      Vector tQ = Q.Min (R);
      Vector tR = R.Max (Q);
      image->dockingports.push_back (DockingPorts (tQ,tR,tempbool));
    }
    break;
  case MESHLIGHT:
    vs_config->gethColor ("unit","engine",halocolor,0xffffffff);
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    P=Vector (1,1,1);
    Q=Vector (1,1,1);
    pos=Vector(0,0,0);
    for (iter = attributes.begin();iter!=attributes.end();iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RED:
	halocolor[0]=parse_float((*iter).value);
	break;
      case GREEN:
       	halocolor[1]=parse_float((*iter).value);
	break;
      case BLUE:
	halocolor[2]=parse_float((*iter).value);
	break;
      case ALPHA:
	halocolor[3]=parse_float((*iter).value);
	break;
      case XFILE:
	filename = (*iter).value;
	break;
      case MOUNTSIZE:
	P.i=parse_float((*iter).value);
	P.j=parse_float((*iter).value);
	break;
      }
    }
   xml->halos.push_back(new Halo(filename.c_str(),GFXColor(halocolor[0],halocolor[1],halocolor[2],halocolor[3]),pos,P.i,P.j));
    break;
  case MOUNT:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    Q = Vector (0,1,0);
    R = Vector (0,0,1);
    pos = Vector (0,0,0);
    tempbool=false;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case WEAPON:
	filename = (*iter).value;
	break;
      case AMMO:
	ammo = parse_int ((*iter).value);
	break;
      case MOUNTSIZE:
	tempbool=true;
	mntsiz=parseMountSizes((*iter).value.c_str());
	break;
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RI:
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	R.j=parse_float((*iter).value);
	break;
      case RK:
	R.k=parse_float((*iter).value);
	break;
      case QI:
	Q.i=parse_float((*iter).value);
	break;
      case QJ:
	Q.j=parse_float((*iter).value);
	break;
      case QK:
	Q.k=parse_float((*iter).value);
	break;
      }

    }
    Q.Normalize();
    if (fabs(Q.i)==fabs(R.i)&&fabs(Q.j)==fabs(R.j)&&fabs(Q.k)==fabs(R.k)){
      Q.i=-1;
      Q.j=0;
      Q.k=0;
    }
    R.Normalize();
    
    CrossProduct (Q,R,P);
    CrossProduct (R,P,Q);
    Q.Normalize();
    //Transformation(Quaternion (from_vectors (P,Q,R),pos);
    indx = xml->mountz.size();
    xml->mountz.push_back(new Mount (filename.c_str(), ammo));
    xml->mountz[indx]->SetMountPosition(Transformation(Quaternion::from_vectors(P,Q,R),pos));
    //xml->mountz[indx]->Activate();
    if (tempbool)
      xml->mountz[indx]->size=mntsiz;
    else
      xml->mountz[indx]->size = xml->mountz[indx]->type.size;
    //->curr_physical_state=xml->units[indx]->prev_physical_state;

    break;

  case SUBUNIT:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    Q = Vector (0,1,0);
    R = Vector (0,0,1);
    pos = Vector (0,0,0);
    fbrltb[0] =-1;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	filename = (*iter).value;
	break;
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RI:
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	R.j=parse_float((*iter).value);
	break;
      case RK:
	R.k=parse_float((*iter).value);
	break;
      case QI:
	Q.i=parse_float((*iter).value);
	break;
      case QJ:
	Q.j=parse_float((*iter).value);
	break;
      case QK:
	Q.k=parse_float((*iter).value);
	break;
      case RESTRICTED:
	fbrltb[0]=parse_float ((*iter).value);//minimum dot turret can have with "fore" vector 
	break;
      }

    }
    Q.Normalize();
    R.Normalize();
    
    CrossProduct (Q,R,P);
    indx = xml->units.size();
    xml->units.push_back(new Unit (filename.c_str(), true,true,faction,NULL)); // I set here the fg arg to NULL
    xml->units.back()->SetOwner (this);
    xml->units[indx]->prev_physical_state= Transformation(Quaternion::from_vectors(P,Q,R),pos);
    xml->units[indx]->curr_physical_state=xml->units[indx]->prev_physical_state;
    xml->units[indx]->limits.structurelimits=R;
    xml->units[indx]->limits.limitmin=fbrltb[0];
    
    break;
  case JUMP:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    jump.drive = -1;//activate the jump unit
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case JUMPENERGY:
	jump.energy = parse_int ((*iter).value);
	break;
      case DELAY:
	jump.delay = parse_int ((*iter).value);
	break;
      case FUEL:
	jump.energy = -parse_int ((*iter).value);
	break;
      }
    }
    break;
  case SOUND:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case CLOAKWAV:
	sound->cloak = AUDCreateSoundWAV ((*iter).value,false);
	break;
      case JUMPWAV:
	sound->jump = AUDCreateSoundWAV ((*iter).value,false);
	break;
      case CLOAKMP3:
	sound->cloak = AUDCreateSoundMP3 ((*iter).value,false);
	break;
      case ENGINEWAV:
	sound->engine = AUDCreateSoundWAV ((*iter).value,true);
	break;
      case ENGINEMP3:
	sound->engine = AUDCreateSoundMP3((*iter).value,true); 
	break;
      case SHIELDMP3:
	sound->shield = AUDCreateSoundMP3((*iter).value,false); 
	break;
      case SHIELDWAV:
	sound->shield = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case EXPLODEMP3:
	sound->explode = AUDCreateSoundMP3((*iter).value,false); 
	break;
      case EXPLODEWAV:
	sound->explode = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case ARMORMP3:
	sound->armor = AUDCreateSoundMP3((*iter).value,false); 
	break;
      case ARMORWAV:
	sound->armor = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case HULLWAV:
	sound->hull = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case HULLMP3:
	sound->hull = AUDCreateSoundMP3((*iter).value,false); 
	break;
      }
    }
    if (sound->cloak==-1) {
      sound->cloak=AUDCreateSound(vs_config->getVariable ("unitaudio","cloak", "sfx43.wav"),false);
    }
    if (sound->engine==-1) {
      sound->engine=AUDCreateSound (vs_config->getVariable ("unitaudio","afterburner","sfx10.wav"),true);
    }
    if (sound->shield==-1) {
      sound->shield=AUDCreateSound (vs_config->getVariable ("unitaudio","shield","sfx09.wav"),false);
    }
    if (sound->armor==-1) {
      sound->armor=AUDCreateSound (vs_config->getVariable ("unitaudio","armor","sfx08.wav"),false);
    }
    if (sound->hull==-1) {
      sound->hull=AUDCreateSound (vs_config->getVariable ("unitaudio","armor","sfx08.wav"),false);
    }
    if (sound->explode==-1) {
      sound->explode=AUDCreateSound (vs_config->getVariable ("unitaudio","explode","sfx03.wav"),false);
    }
      
    break;    
  case CLOAK:
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    image->cloakrate=.2*32767*SIMULATION_ATOM;
    cloakmin=1;
    image->cloakenergy=0;
    cloaking = (short) 32768;//lowest negative number
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case CLOAKMIN:
	cloakmin = 32767*parse_float ((*iter).value);
	break;
      case CLOAKGLASS:
	image->cloakglass=parse_bool ((*iter).value);
	break;
      case CLOAKRATE:
	image->cloakrate = 32767*parse_float ((*iter).value)*SIMULATION_ATOM;
	break;
      case CLOAKENERGY:
	image->cloakenergy = parse_float ((*iter).value);
	break;
      }
    }
    if ((cloakmin&0x1)&&!image->cloakglass) {
      cloakmin-=1;
    }
    if ((cloakmin&0x1)==0&&image->cloakglass) {
      cloakmin+=1;
    }
    break;
  case ARMOR:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case FRONT:
	armor.front = CLAMP_SHORT(parse_float((*iter).value));
	break;
      case BACK:
	armor.back= CLAMP_SHORT(parse_float((*iter).value));
	break;
      case LEFT:
	armor.left= CLAMP_SHORT(parse_float((*iter).value));
	break;
      case RIGHT:
	armor.right= CLAMP_SHORT(parse_float((*iter).value));
	break;
      }
    }

 
    break;
  case SHIELDS:
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case FRONT:
	fbrltb[0] = parse_float((*iter).value);
	shield.number++;
	break;
      case BACK:
	fbrltb[1]=parse_float((*iter).value);
	shield.number++;
	break;
      case LEFT:
	fbrltb[3]=parse_float((*iter).value);
	shield.number++;
	break;
      case RIGHT:
	fbrltb[2]=parse_float((*iter).value);
	shield.number++;
	break;
      case TOP:
	fbrltb[4]=parse_float((*iter).value);
	shield.number++;
	break;
      case BOTTOM:
	fbrltb[5]=parse_float((*iter).value);
	shield.number++;
	break;
      case RECHARGE:
	shield.recharge=parse_float((*iter).value);
	break;
      case LEAK:
	shield.leak = parse_int ((*iter).value);
	break;
      }
    }
    if (fbrltb[0]>65535||fbrltb[1]>65535)
      shield.number=2;

    switch (shield.number) {
    case 2:
      shield.fb[2]=shield.fb[0]=fbrltb[0];
      shield.fb[3]=shield.fb[1]=fbrltb[1];
      break;
    case 6:
      shield.fbrltb.v[0]=CLAMP_SHORT(fbrltb[0]);
      shield.fbrltb.v[1]=CLAMP_SHORT(fbrltb[1]);
      shield.fbrltb.v[2]=CLAMP_SHORT(fbrltb[2]);
      shield.fbrltb.v[3]=CLAMP_SHORT(fbrltb[3]);
      shield.fbrltb.v[4]=CLAMP_SHORT(fbrltb[4]);
      shield.fbrltb.v[5]=CLAMP_SHORT(fbrltb[5]);
      shield.fbrltb.fbmax= CLAMP_SHORT((fbrltb[0]+fbrltb[1])*.5);
      shield.fbrltb.rltbmax= CLAMP_SHORT((fbrltb[2]+fbrltb[3]+fbrltb[4]+fbrltb[5])*.25);
      
      break;
    case 4:
    default:
      shield.fbrl.frontmax = shield.fbrl.front = CLAMP_SHORT(fbrltb[0]);
      shield.fbrl.backmax = shield.fbrl.back = CLAMP_SHORT(fbrltb[1]);
      shield.fbrl.rightmax = shield.fbrl.right = CLAMP_SHORT(fbrltb[2]);
      shield.fbrl.leftmax = shield.fbrl.left = CLAMP_SHORT(fbrltb[3]);
    }

    break;
  case HULL:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case STRENGTH:
	hull = parse_float((*iter).value);
	break;
      }
    }
   
    break;
  case STATS:
	assert (xml->unitlevel==1);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MASS:
	mass = parse_float((*iter).value);
	break;
      case MOMENTOFINERTIA:
	MomentOfInertia=parse_float((*iter).value);
	break;
      case FUEL:
	fuel=parse_float((*iter).value);
	break;
      }
    }
	break;
  case MANEUVER:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case YAW:
	limits.yaw = parse_float((*iter).value)*(VS_PI/180);
	break;
      case PITCH:
	limits.pitch=parse_float((*iter).value)*(VS_PI/180);
	break;
      case ROLL:
	limits.roll=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }

    
    break;

  case ENGINE:
	  
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case AACCEL:
	//accel=parse_float((*iter).value);
	break;
      case FORWARD:
	limits.forward=parse_float((*iter).value);
	break;
      case RETRO:
	limits.retro=parse_float((*iter).value);
	break;
      case AFTERBURNER:
	limits.afterburn=parse_float ((*iter).value);
	break;
      case LEFT:
	limits.lateral=parse_float((*iter).value);
	break;
      case RIGHT:
	limits.lateral=parse_float((*iter).value);
	break;
      case TOP:
	limits.vertical=parse_float((*iter).value);
	break;
      case BOTTOM:
	limits.vertical=parse_float((*iter).value);
	break;
    }
    }

    break;

  case COMPUTER:  
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAXSPEED:
	computer.max_speed=parse_float((*iter).value);
	break;
      case AFTERBURNER:
	computer.max_ab_speed=parse_float((*iter).value);
	break;
      case YAW:
	computer.max_yaw=parse_float((*iter).value)*(VS_PI/180);
	break;
      case PITCH:
	computer.max_pitch=parse_float((*iter).value)*(VS_PI/180);
	break;
      case ROLL:
	computer.max_roll=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;
  case RADAR:
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case ITTS:
	computer.itts=parse_bool ((*iter).value);
	break;
      case EEERROR:
	computer.radar.error=parse_float ((*iter).value);
	break;
      case MAXCONE:
	computer.radar.maxcone = parse_float ((*iter).value);
	break;
      case RANGE:
	computer.radar.maxrange = parse_float ((*iter).value);
	break;
      case ISCOLOR:
	computer.radar.color=parse_bool ((*iter).value);
	break;
      }
    }
    break;
  case REACTOR:
	assert (xml->unitlevel==2);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case RECHARGE:
	recharge=parse_float((*iter).value);
	break;
      case LIMIT:
	maxenergy=energy=CLAMP_SHORT(parse_float((*iter).value));
	break;
    }
    }
    break;

  case YAW:
    xml->yprrestricted+=Unit::XML::YRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	xml->ymax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	xml->ymin=parse_float((*iter).value)*(VS_PI/180);
	break;
    }
    }
    break;

  case PITCH:
    xml->yprrestricted+=Unit::XML::PRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	xml->pmax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	xml->pmin=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;

  case ROLL:
    xml->yprrestricted+=Unit::XML::RRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	xml->rmax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	xml->rmin=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;

  case UNIT:
	assert (xml->unitlevel==0);
	xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      default:
	break;
      case COCKPIT:
	fprintf (stderr,"Cockpit attrib deprecated use tag");
	break;
      }
    }
    break;
  case COCKPIT:
    assert (xml->unitlevel==1);
    xml->unitlevel++;

    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	image->cockpitImage = (*iter).value;
	break;
      case X:
	image->CockpitCenter.i =parse_float ((*iter).value);
	break;
      case Y:
	image->CockpitCenter.j =parse_float ((*iter).value);
	break;
      case Z:
	image->CockpitCenter.k =parse_float ((*iter).value);
	break;
      }
    }
    break;
  case DEFENSE:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case HUDIMAGE:
	image->hudImage = new Sprite ((*iter).value.c_str());
	break;
      default:
	break;
      }
    }

    break;

  case THRUST:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;

  case ENERGY:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;

  case RESTRICTED:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;
	
  default:
	
    break;
  }
}

void Unit::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
	  xml->unitlevel--;
//    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  default:
	  xml->unitlevel--;
    break;
  }
}
void Unit::LoadXML(const char *filename)
{
  shield.number=0;
  const int chunk_size = 16384;
 // rrestricted=yrestricted=prestricted=false;
  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
    cout << "Unit file " << filename << " not found" << endl;
    assert(0);
    return;
  }
  image->CockpitCenter.Set (0,0,0);
  xml = new XML;
  xml->shieldmesh = NULL;
  xml->bspmesh = NULL;
  xml->hasBSP = true;
  xml->unitlevel=0;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Unit::beginElement, &Unit::endElement);
  
  do {
#ifdef BIDBG
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
#else
    char buf[chunk_size];
#endif
    int length;
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
#ifdef BIDBG
    XML_ParseBuffer(parser, length, feof(inFile));
#else
    XML_Parse (parser,buf,length,feof(inFile));
#endif
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
  // Load meshes into subunit
  nummesh = xml->meshes.size();
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  int a;
  meshdata = new Mesh*[nummesh+1];
  for(a=0; a < nummesh; a++) {
    meshdata[a] = xml->meshes[a];
  }
  numhalos = xml->halos.size();
  if(numhalos)
    halos = new Halo*[numhalos];
  else
    halos=NULL;
  for(a=0; a < numhalos; a++) {
    halos[a]=xml->halos[a];
  }
  nummounts = xml->mountz.size();
  if (nummounts)
    mounts = new Mount [nummounts];
  else
    mounts = NULL;
  char parity=0;
  for(a=0; a < nummounts; a++) {
    mounts[a]=*xml->mountz[a];
    delete xml->mountz[a];			//do it stealthily... no cons/destructor
  }
  for (a=0;a<nummounts;a++) {
#define HALF_MOUNT_SOUNDS 
#ifdef HALF_MOUNT_SOUNDS
	if (a%2==parity) {
		int b=a;
		if(a % 4 == 2 && a < (nummounts-1)) 
			if (mounts[a].type.type != weapon_info::PROJECTILE&&mounts[a+1].type.type != weapon_info::PROJECTILE)
			  b=a+1;
		mounts[b].sound = AUDCreateSound (mounts[b].type.sound,mounts[b].type.type!=weapon_info::PROJECTILE);
    }
	else if (mounts[a].type.type == weapon_info::PROJECTILE) {
#endif
      mounts[a].sound = AUDCreateSound (mounts[a].type.sound,false);      
#ifdef HALF_MOUNT_SOUNDS
    }
#endif
  }
  numsubunit = xml->units.size();
  if (numsubunit)
    subunits = new Unit*[numsubunit];
  else
    subunits=NULL;
  for( a=0; a<numsubunit; a++) {
    subunits[a] = xml->units[a];
  }
  if (!SubUnit) {
    calculate_extent();
    UpdateCollideQueue();
  }
  string tmpname (filename);
  vector <bsp_polygon> polies;
  if (xml->shieldmesh) {
    meshdata[nummesh] = xml->shieldmesh;
    
    if (!CheckBSP ((tmpname+"_shield.bsp").c_str())) {
      BuildBSPTree ((tmpname+"_shield.bsp").c_str(), false, meshdata[nummesh]);
    }
    if (CheckBSP ((tmpname+"_shield.bsp").c_str())) {
      bspShield = new BSPTree ((tmpname+"_shield.bsp").c_str());
    }
    if (meshdata[nummesh]) {
      meshdata[nummesh]->GetPolys(polies);
      colShield = new csRapidCollider (polies);
    }
  }
  else {
    SphereMesh * tmp = new SphereMesh (rSize(),8,8,"shield.bmp", NULL, false,ONE, ONE);
    tmp->GetPolys (polies);
    colShield = new csRapidCollider (polies);
    static int shieldstacks = XMLSupport::parse_int (vs_config->getVariable ("graphics","shield_detail","16"));
    if (shieldstacks!=8) {
      delete tmp;
      tmp = new SphereMesh (rSize(),shieldstacks,shieldstacks,"shield.bmp", NULL, false,ONE, ONE);
    }
    meshdata[nummesh] = tmp;
    bspShield=NULL;
    colShield=NULL;
  }
  meshdata[nummesh]->EnableSpecialFX();
  if (xml->hasBSP) {
    tmpname += ".bsp";
    if (!CheckBSP (tmpname.c_str())) {
      BuildBSPTree (tmpname.c_str(), false, xml->bspmesh);
    }
    if (CheckBSP (tmpname.c_str())) {
      bspTree = new BSPTree (tmpname.c_str());
    }	
  } else {
    bspTree = NULL;
    colTree = NULL;
  }
  polies.clear();
  if (!xml->bspmesh) {
    for (int j=0;j<nummesh;j++) {
      meshdata[j]->GetPolys(polies);
    }
  }else {
    xml->bspmesh->GetPolys (polies);
  }
  colTree = new csRapidCollider (polies);
  
  if (xml->bspmesh) {
    delete xml->bspmesh;
  }
  delete xml;
}
