//#include "unit.h"
#include "unit_xml.h"
#include "unit_factory.h"
#include "audiolib.h"
#include "xml_support.h"
//#include <iostream.h>
#include <fstream>
#include <expat.h>
//#include <values.h>
#include <float.h>
#include "configxml.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include <assert.h>
#include "images.h"
#include "xml_serializer.h"
#include "vs_path.h"
#include "gfx/cockpit_generic.h"
#include "unit_bsp.h"
#include "unit_collide.h"
#include "gfx/bsp.h"
#include "unit_generic.h"
#include "gfx/sphere.h"
#include "role_bitmask.h"
#include "cmd/collide/rapcol.h"
#define VS_PI 3.1415926536

int GetModeFromName (const char * input_buffer) {
      if (strlen (input_buffer)>3) {
	if (input_buffer[0]=='a'&&
	    input_buffer[1]=='d'&&
	    input_buffer[2]=='d') {
	  return 1;
	}
	if (input_buffer[0]=='m'&&
	    input_buffer[1]=='u'&&
	    input_buffer[2]=='l') {
	  return 2;
	}
      }
      return 0;
}

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
      RAPIDMESH,
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
      MINTARGETSIZE,
      MAXCONE,
      LOCKCONE,
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
      EXPLOSIONANI,
      COCKPIT,
      JUMP,
      DELAY,
      JUMPENERGY,
      JUMPWAV,
      DOCKINTERNAL,
      WORMHOLE,
      RAPID,
      USEBSP,
      AFTERBURNENERGY,
      MISSING,
      UNITSCALE,
      PRICE,
      VOLUME,
      QUANTITY,
      CARGO,
      HOLD,
      CATEGORY,
      IMPORT,
      PRICESTDDEV,
      QUANTITYSTDDEV,
      DAMAGE,
      COCKPITDAMAGE,
      REPAIRDROID,
      ECM,
      DESCRIPTION,
      UPGRADE,
      MOUNTOFFSET,
      SUBUNITOFFSET,
      SLIDE_START,
      SLIDE_END,
      TRACKINGCONE,
      MISSIONCARGO,
      MAXIMUM,
      LIGHTTYPE,
      COMBATROLE,
      RECURSESUBUNITCOLLISION
    };

  const EnumMap::Pair element_names[37]= {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Unit", UNIT),
    EnumMap::Pair ("SubUnit", SUBUNIT),
    EnumMap::Pair ("Sound", SOUND),
    EnumMap::Pair ("MeshFile", MESHFILE),
    EnumMap::Pair ("ShieldMesh",SHIELDMESH),
    EnumMap::Pair ("RapidMesh",RAPIDMESH),
    EnumMap::Pair ("BSPMesh",BSPMESH),
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
    EnumMap::Pair ("Dock", DOCK),
    EnumMap::Pair ("Hold",HOLD),
    EnumMap::Pair ("Cargo",CARGO),
    EnumMap::Pair ("Category",CATEGORY),
    EnumMap::Pair ("Import",IMPORT),
    EnumMap::Pair ("CockpitDamage",COCKPITDAMAGE),
    EnumMap::Pair ("Upgrade",UPGRADE      ),
    EnumMap::Pair ("Description",DESCRIPTION)
    
  };
  const EnumMap::Pair attribute_names[96] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("missing",MISSING),
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
    EnumMap::Pair ("ExplosionAni",EXPLOSIONANI),
    EnumMap::Pair ("MaxCone",MAXCONE),
    EnumMap::Pair ("TrackingCone",TRACKINGCONE),
    EnumMap::Pair ("LockCone",LOCKCONE),
    EnumMap::Pair ("MinTargetSize",MINTARGETSIZE),
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
    EnumMap::Pair ("AfterburnEnergy", AFTERBURNENERGY),
    EnumMap::Pair ("JumpEnergy", JUMPENERGY),
    EnumMap::Pair ("JumpWav", JUMPWAV),
    EnumMap::Pair ("DockInternal", DOCKINTERNAL),
    EnumMap::Pair ("RAPID", RAPID),
    EnumMap::Pair ("BSP", USEBSP),
    EnumMap::Pair ("Wormhole", WORMHOLE),
    EnumMap::Pair ("Scale", UNITSCALE),
    EnumMap::Pair ("Price",PRICE),
    EnumMap::Pair ("Volume",VOLUME),
    EnumMap::Pair ("Quantity",QUANTITY),
    EnumMap::Pair ("PriceStdDev",PRICESTDDEV),
    EnumMap::Pair ("QuantityStdDev",QUANTITYSTDDEV),
    EnumMap::Pair ("Damage",DAMAGE),
    EnumMap::Pair ("RepairDroid",REPAIRDROID),
    EnumMap::Pair ("ECM",ECM),
    EnumMap::Pair ("Description",DESCRIPTION),
    EnumMap::Pair ("MountOffset",MOUNTOFFSET),
    EnumMap::Pair ("SubunitOffset",SUBUNITOFFSET),
    EnumMap::Pair ("SlideEnd",SLIDE_START),
    EnumMap::Pair ("SlideStart",SLIDE_END),
    EnumMap::Pair ("MissionCargo",MISSIONCARGO),
    EnumMap::Pair ("Maximum",MAXIMUM),
    EnumMap::Pair ("LightType",LIGHTTYPE),
    EnumMap::Pair ("CombatRole",COMBATROLE),
    EnumMap::Pair ("RecurseSubunitCollision",RECURSESUBUNITCOLLISION)
  };

  const EnumMap element_map(element_names, 37);
  const EnumMap attribute_map(attribute_names, 96);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
extern int GetModeFromName (const char *);
extern int parseMountSizes (const char * str);

static short CLAMP_SHORT(float x) {return (short)(((x)>65536)?65536:((x)<0?0:(x)));}  

void Unit::beginElement(const string &name, const AttributeList &attributes) {
using namespace UnitXML;
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
  static float game_accel = XMLSupport::parse_float (vs_config->getVariable ("physics","game_accel","1"));
  Cargo carg;
  short volume=-1;
  string filename;
  QVector P;
  int indx;
  QVector Q;
  QVector R;
  QVector pos;
  bool tempbool;
  float fbrltb[6];
  AttributeList::const_iterator iter;
  float halocolor[4];
  short ammo=-1;
  int mntsiz=weapon_info::NOWEAP;
  string light_type;
  Names elem = (Names)element_map.lookup(name);
#define ADDTAGNAME(a) image->unitwriter->AddTag (a)
#define ADDTAG  image->unitwriter->AddTag (name)
#define ADDELEMNAME(a,b,c) image->unitwriter->AddElement(a,b,c)
#define ADDELEM(b,c) image->unitwriter->AddElement((*iter).name,b,c)
#define ADDDEFAULT image->unitwriter->AddElement((*iter).name,stringHandler,XMLType((*iter).value))
#define ADDELEMI(b) ADDELEM(intStarHandler,XMLType(&b))
#define ADDELEMF(b) ADDELEM(floatStarHandler,XMLType(&b))
  switch(elem) {
  case SHIELDMESH:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	ADDELEM(stringHandler,(*iter).value);
	addShieldMesh( xml, (*iter).value.c_str(), xml->unitscale, faction,flightgroup);
	//xml->shieldmesh = createMesh((*iter).value.c_str(), xml->unitscale, faction,flightgroup);
	break;
      case SHIELDTIGHT: 
	ADDDEFAULT;
	shieldtight = parse_float ((*iter).value);
	break;
      }
    }
    break;
  case RAPIDMESH:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;

    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	ADDDEFAULT;
	addRapidMesh( xml, (*iter).value.c_str(), xml->unitscale, faction,NULL);
	xml->hasColTree = true;	
	break;
      case RAPID:
	ADDDEFAULT;
	xml->hasColTree=parse_bool ((*iter).value);
	break;
      }
    }
    break;
  case BSPMESH:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    xml->hasBSP = false;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	ADDDEFAULT;
	addBSPMesh( xml, (*iter).value.c_str(), xml->unitscale, faction,NULL);
	xml->hasBSP = true;	
	break;
      case RAPID:
	ADDDEFAULT;
	xml->hasColTree=parse_bool ((*iter).value);
	break;
      case USEBSP:
	ADDDEFAULT;
	xml->hasBSP=parse_bool ((*iter).value);
	break;
      }
    }
    break;
  case HOLD:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case VOLUME:
	ADDELEM(floatStarHandler,XMLType(&image->cargo_volume));
	image->cargo_volume=parse_float ((*iter).value);
	break;
      }
    }
    image->unitwriter->AddTag ("Category");
    image->unitwriter->AddElement ("file",Unit::cargoSerializer,XMLType ((int)0));
    image->unitwriter->EndTag ("Category");
    break;
  case IMPORT:
    Q.i=Q.k=0;
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case QUANTITY:
	carg.quantity=parse_int ((*iter).value);
	//import cargo from ze maztah liztz
	break;
      case PRICE:
	carg.price = parse_float ((*iter).value);
	break;
      case PRICESTDDEV:
	Q.i = parse_float ((*iter).value);
	break;
      case QUANTITYSTDDEV:
	Q.k = parse_float ((*iter).value);
	break;

      }
    }
    ImportPartList (xml->cargo_category, carg.price, Q.i,carg.quantity,Q.k);
    break;
  case CATEGORY:
    //this is autogenerated by the handler
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->cargo_category = XMLSupport::replace_space((*iter).value);
	break;
      }
    }
    break;
  case CARGO:
    ///handling taken care of above;
    assert (xml->unitlevel>=2);
    xml->unitlevel++;
    carg.category = xml->cargo_category;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case QUANTITY:
	carg.quantity=parse_int ((*iter).value);
	break;
      case MASS:
	carg.mass=parse_float ((*iter).value);
	break;
      case VOLUME:
	carg.volume=parse_float ((*iter).value);
	break;
      case PRICE:
	carg.price=parse_float ((*iter).value);
	break;
      case MISSIONCARGO:
	carg.mission=parse_bool((*iter).value);
	break;
      case XFILE:
	carg.content = XMLSupport::replace_space((*iter).value);
	break;
      case DESCRIPTION:
	carg.description= strdup ((*iter).value.c_str());//mem leak...but hey--only for mpl
	break;
      }
    }
    if (carg.mass!=0)
      AddCargo (carg,false);
    break;
  case MESHFILE:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	ADDDEFAULT;
	pushMesh( xml, (*iter).value.c_str(), xml->unitscale, faction,flightgroup);
	//xml->meshes.push_back(createMesh((*iter).value.c_str(), xml->unitscale, faction,flightgroup));
	break;
      }
    }
    break;
  case UPGRADE:
    {
      assert (xml->unitlevel>=1);
      xml->unitlevel++;
 
      double percent;
      int moffset=0;
      int soffset=0;
    //don't serialize    
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {

      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	filename = (*iter).value.c_str();
	break;
      case SUBUNITOFFSET:
	soffset = parse_int ((*iter).value);
	break;
      case MOUNTOFFSET:
	moffset = parse_int ((*iter).value);
	break;
      }
    }    
    Unit *upgradee =UnitFactory::createUnit(filename.c_str(),true,FactionUtil::GetFaction("upgrades"));
	Unit::Upgrade (upgradee,soffset,moffset,GetModeFromName (filename.c_str()),true,percent,NULL);
    upgradee->Kill();
    }
    break;
  case DOCK:
    ADDTAG;
    tempbool=false;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    pos=QVector(0,0,0);
    P=QVector (1,1,1);
    Q=QVector (FLT_MAX,FLT_MAX,FLT_MAX);
    R=QVector (FLT_MAX,FLT_MAX,FLT_MAX);

    for (iter = attributes.begin();iter!=attributes.end();iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case DOCKINTERNAL:
	ADDDEFAULT;
	tempbool=parse_bool ((*iter).value);
	break;
      case X:
	ADDDEFAULT;
	pos.i=xml->unitscale*parse_float((*iter).value);
	break;
      case Y:
	ADDDEFAULT;
	pos.j=xml->unitscale*parse_float((*iter).value);
	break;
      case Z:
	ADDDEFAULT;
	pos.k=xml->unitscale*parse_float((*iter).value);
	break;
      case TOP:
	ADDDEFAULT;
	R.j=xml->unitscale*parse_float((*iter).value);
	break;
      case BOTTOM:
	ADDDEFAULT;
	Q.j=xml->unitscale*parse_float((*iter).value);
	break;
      case LEFT:
	ADDDEFAULT;
	Q.i=xml->unitscale*parse_float((*iter).value);
	break;
      case RIGHT:
	ADDDEFAULT;
	R.i=parse_float((*iter).value);
	break;
      case BACK:
	ADDDEFAULT;
	Q.k=xml->unitscale*parse_float((*iter).value);
	break;
      case FRONT:
	ADDDEFAULT;
	R.k=xml->unitscale*parse_float((*iter).value);
	break;
      case MOUNTSIZE:
	ADDDEFAULT;
	P.i=xml->unitscale*parse_float((*iter).value);
	P.j=xml->unitscale*parse_float((*iter).value);
	break;
      }
    }
    if (Q.i==FLT_MAX||Q.j==FLT_MAX||Q.k==FLT_MAX||R.i==FLT_MAX||R.j==FLT_MAX||R.k==FLT_MAX) {
      image->dockingports.push_back (DockingPorts(pos.Cast(),P.i,tempbool));
    }else {
      QVector tQ = Q.Min (R);
      QVector tR = R.Max (Q);
      image->dockingports.push_back (DockingPorts (tQ.Cast(),tR.Cast(),tempbool));
    }
    break;
  case MESHLIGHT:
    ADDTAG;
    vs_config->gethColor ("unit","engine",halocolor,0xffffffff);
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    P=QVector (1,1,1);
    Q=QVector (1,1,1);
    pos=QVector(0,0,0);
    for (iter = attributes.begin();iter!=attributes.end();iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case LIGHTTYPE:
	ADDDEFAULT;
	light_type = (*iter).value;
	break;
      case X:
	ADDDEFAULT;
	pos.i=xml->unitscale*parse_float((*iter).value);
	break;
      case Y:
	ADDDEFAULT;
	pos.j=xml->unitscale*parse_float((*iter).value);
	break;
      case Z:
	ADDDEFAULT;
	pos.k=xml->unitscale*parse_float((*iter).value);
	break;
      case RED:
	ADDDEFAULT;
	halocolor[0]=parse_float((*iter).value);
	break;
      case GREEN:
	ADDDEFAULT;
       	halocolor[1]=parse_float((*iter).value);
	break;
      case BLUE:
	ADDDEFAULT;
	halocolor[2]=parse_float((*iter).value);
	break;
      case ALPHA:
	ADDDEFAULT;
	halocolor[3]=parse_float((*iter).value);
	break;
      case XFILE:
	ADDDEFAULT;
	filename = (*iter).value;
	break;
      case MOUNTSIZE:
	ADDDEFAULT;
	P.i=xml->unitscale*parse_float((*iter).value);
	P.j=xml->unitscale*parse_float((*iter).value);
	P.k=xml->unitscale*parse_float((*iter).value);
	break;
      }
    }
	addHalo( filename.c_str(),pos,P.Cast(),GFXColor(halocolor[0],halocolor[1],halocolor[2],halocolor[3]),light_type);
    break;
  case MOUNT:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    Q = QVector (0,1,0);
    R = QVector (0,0,1);
    pos = QVector (0,0,0);
    tempbool=false;
    ADDELEMNAME("size",Unit::mountSerializer,XMLType(XMLSupport::tostring(xml->unitscale),(int)xml->mountz.size()));
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case VOLUME:
	volume=XMLSupport::parse_int ((*iter).value);
	break;
      case WEAPON:
	filename = (*iter).value;
	break;
      case AMMO:
	ammo = XMLSupport::parse_int ((*iter).value);
	break;
      case MOUNTSIZE:
	tempbool=true;
	mntsiz=parseMountSizes((*iter).value.c_str());
	break;
      case X:
	pos.i=xml->unitscale*parse_float((*iter).value);
	break;
      case Y:
	pos.j=xml->unitscale*parse_float((*iter).value);
	break;
      case Z:
	pos.k=xml->unitscale*parse_float((*iter).value);
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
    xml->mountz.push_back(createMount (filename.c_str(), ammo,volume));
    xml->mountz[indx]->SetMountPosition(Transformation(Quaternion::from_vectors(P.Cast(),Q.Cast(),R.Cast()),pos));
    //xml->mountz[indx]->Activate();
    if (tempbool)
      xml->mountz[indx]->size=mntsiz;
    else
      xml->mountz[indx]->size = xml->mountz[indx]->type->size;
    //->curr_physical_state=xml->units[indx]->prev_physical_state;

    break;

  case SUBUNIT:
    ADDTAG;
    assert (xml->unitlevel==1);
    ADDELEMNAME("file",Unit::subunitSerializer,XMLType((int)xml->units.size()));
    xml->unitlevel++;
    Q = QVector (0,1,0);
    R = QVector (0,0,1);
    pos = QVector (0,0,0);
    fbrltb[0] =-1;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	filename = (*iter).value;
	break;
      case X:
	ADDDEFAULT;
	pos.i=xml->unitscale*parse_float((*iter).value);
	break;
      case Y:
	ADDDEFAULT;
	pos.j=xml->unitscale*parse_float((*iter).value);
	break;
      case Z:
	ADDDEFAULT;
	pos.k=xml->unitscale*parse_float((*iter).value);
	break;
      case RI:
	ADDDEFAULT;
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	ADDDEFAULT;
	R.j=parse_float((*iter).value);
	break;
      case RK:
	ADDDEFAULT;
	R.k=parse_float((*iter).value);
	break;
      case QI:
	ADDDEFAULT;
	Q.i=parse_float((*iter).value);
	break;
      case QJ:
	ADDDEFAULT;
	Q.j=parse_float((*iter).value);
	break;
      case QK:
	ADDDEFAULT;
	Q.k=parse_float((*iter).value);
	break;
      case RESTRICTED:
	ADDDEFAULT;
	fbrltb[0]=parse_float ((*iter).value);//minimum dot turret can have with "fore" vector 
	break;
      }

    }
    //    Q.Normalize();
    //    R.Normalize();
    
    //    CrossProduct (Q,R,P);
    indx = xml->units.size();
    xml->units.push_back(UnitFactory::createUnit (filename.c_str(),true,faction,xml->unitModifications,NULL)); // I set here the fg arg to NULL
    if (xml->units.back()->name=="LOAD_FAILED") {
	  xml->units.back()->limits.yaw=0;
	  xml->units.back()->limits.pitch=0;
	  xml->units.back()->limits.roll=0;
	  xml->units.back()->limits.lateral = xml->units.back()->limits.retro = xml->units.back()->limits.forward = xml->units.back()->limits.afterburn=0.0;

    }

    xml->units.back()->SetRecursiveOwner (this);
    xml->units[indx]->SetOrientation (Q,R);
    R.Normalize();
    xml->units[indx]->prev_physical_state = xml->units[indx]->curr_physical_state;
    xml->units[indx]->SetPosition(pos);
    //    xml->units[indx]->prev_physical_state= Transformation(Quaternion::from_vectors(P,Q,R),pos);
    //    xml->units[indx]->curr_physical_state=xml->units[indx]->prev_physical_state;
    xml->units[indx]->limits.structurelimits=R.Cast();
    xml->units[indx]->limits.limitmin=fbrltb[0];
    xml->units[indx]->name = filename;
    if (xml->units[indx]->image->unitwriter!=NULL) {
      xml->units[indx]->image->unitwriter->setName (filename);
    }
    
    break;
  case COCKPITDAMAGE:
	  xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      	  switch (attribute_map.lookup((*iter).name)) {
		  case DAMAGE:
			image->cockpit_damage[xml->damageiterator++]=parse_float((*iter).value);
			break;
		  }
	  }
	  break;
  case JUMP:
    //serialization covered in LoadXML
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    jump.drive = -1;//activate the jump unit
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MISSING:
	//serialization covered in LoadXML
	if (parse_bool((*iter).value))
	  jump.drive=-2;
	break;
      case JUMPENERGY:
	//serialization covered in LoadXML
	jump.energy = CLAMP_SHORT(parse_float((*iter).value));
	break;
  case DAMAGE:
	  jump.damage=CLAMP_SHORT (parse_float((*iter).value));
	  break;
  case DELAY:
	//serialization covered in LoadXML
	jump.delay = parse_int ((*iter).value);
	break;
      case FUEL:
	//serialization covered in LoadXML
	jump.energy = -CLAMP_SHORT (parse_float((*iter).value));
	break;
      case WORMHOLE:
	//serialization covered in LoadXML
	image->forcejump=parse_bool ((*iter).value);
	if (image->forcejump)
	  jump.drive=-2;
	break;
      }
    }
    break;
  case SOUND:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case CLOAKWAV:
	ADDDEFAULT;
	sound->cloak = AUDCreateSoundWAV ((*iter).value,false);
	break;
      case JUMPWAV:
	ADDDEFAULT;
	sound->jump = AUDCreateSoundWAV ((*iter).value,false);
	break;
      case CLOAKMP3:
	ADDDEFAULT;
	sound->cloak = AUDCreateSoundMP3 ((*iter).value,false);
	break;
      case ENGINEWAV:
	ADDDEFAULT;
	sound->engine = AUDCreateSoundWAV ((*iter).value,true);
	cout<<"engine sound wav : "<<(*iter).value<<endl;
	break;
      case ENGINEMP3:
	ADDDEFAULT;
	sound->engine = AUDCreateSoundMP3((*iter).value,true); 
	cout<<"engine sound mp3 : "<<(*iter).value<<endl;
	break;
      case SHIELDMP3:
	ADDDEFAULT;
	sound->shield = AUDCreateSoundMP3((*iter).value,false); 
	break;
      case SHIELDWAV:
	ADDDEFAULT;
	sound->shield = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case EXPLODEMP3:
	ADDDEFAULT;
	sound->explode = AUDCreateSoundMP3((*iter).value,false); 
	break;
      case EXPLODEWAV:
	ADDDEFAULT;
	sound->explode = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case ARMORMP3:
	ADDDEFAULT;
	sound->armor = AUDCreateSoundMP3((*iter).value,false); 
	break;
      case ARMORWAV:
	ADDDEFAULT;
	sound->armor = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case HULLWAV:
	ADDDEFAULT;
	sound->hull = AUDCreateSoundWAV((*iter).value,false); 
	break;
      case HULLMP3:
	ADDDEFAULT;
	sound->hull = AUDCreateSoundMP3((*iter).value,false); 
	break;
      }
    }
    if (sound->cloak==-1) {
      sound->cloak=AUDCreateSound(vs_config->getVariable ("unitaudio","cloak", "sfx43.wav"),false);
    }
    if (sound->engine==-1) {
      sound->engine=AUDCreateSound (vs_config->getVariable ("unitaudio","afterburner","sfx10.wav"),true);
	  cout<<"default engine sound : sfx10.wav - sound->engine : "<<sound->engine<<endl;
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
    //serialization covered elsewhere
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    image->cloakrate=(short int)(.2*32767);
    cloakmin=1;
    image->cloakenergy=0;
    cloaking = (short) 32768;//lowest negative number
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MISSING:
	//serialization covered in LoadXML
	if (parse_bool((*iter).value))
	  cloaking=(short)-1;
	break;
      case CLOAKMIN:
	//serialization covered in LoadXML
	cloakmin = (short int)(32767*parse_float ((*iter).value));
	break;
      case CLOAKGLASS:
	//serialization covered in LoadXML
	image->cloakglass=parse_bool ((*iter).value);
	break;
      case CLOAKRATE:
	//serialization covered in LoadXML
	image->cloakrate = (short int)(32767*parse_float ((*iter).value));
	break;
      case CLOAKENERGY:
	//serialization covered in LoadXML
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
	//serialization covered in LoadXML
	armor.front = CLAMP_SHORT(parse_float((*iter).value));
	break;
      case BACK:
	//serialization covered in LoadXML
	armor.back= CLAMP_SHORT(parse_float((*iter).value));
	break;
      case LEFT:
	//serialization covered in LoadXML
	armor.left= CLAMP_SHORT(parse_float((*iter).value));
	break;
      case RIGHT:
	//serialization covered in LoadXML
	armor.right= CLAMP_SHORT(parse_float((*iter).value));
	break;
      }
    }

 
    break;
  case SHIELDS:
	//serialization covered in LoadXML
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case FRONT:
	//serialization covered in LoadXML
	fbrltb[0] = parse_float((*iter).value);
	shield.number++;
	break;
      case BACK:
	//serialization covered in LoadXML
	fbrltb[1]=parse_float((*iter).value);
	shield.number++;
	break;
      case LEFT:
	//serialization covered in LoadXML
	fbrltb[3]=parse_float((*iter).value);
	shield.number++;
	break;
      case RIGHT:
	//serialization covered in LoadXML
	fbrltb[2]=parse_float((*iter).value);
	shield.number++;
	break;
      case TOP:
	//serialization covered in LoadXML
	fbrltb[4]=parse_float((*iter).value);
	shield.number++;
	break;
      case BOTTOM:
	//serialization covered in LoadXML
	fbrltb[5]=parse_float((*iter).value);
	shield.number++;
	break;
      case RECHARGE:
	//serialization covered in LoadXML
	shield.recharge=parse_float((*iter).value);
	break;
      case LEAK:
	//serialization covered in LoadXML
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
	maxhull=0;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case STRENGTH:
	hull = parse_float((*iter).value);
	break;
      case MAXIMUM:
	maxhull=parse_float ((*iter).value);
	break;
      }
    }
    if (maxhull==0) {
      maxhull = hull;
      if (maxhull==0) {
	maxhull = 1;
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
	limits.forward=game_speed*game_accel*parse_float((*iter).value);
	break;
      case RETRO:
	limits.retro=game_speed*game_accel*parse_float((*iter).value);
	break;
      case AFTERBURNER:
	limits.afterburn=game_speed*game_accel*parse_float ((*iter).value);
	break;
      case LEFT:
	limits.lateral=game_speed*game_accel*parse_float((*iter).value);
	break;
      case RIGHT:
	limits.lateral=game_speed*game_accel*parse_float((*iter).value);
	break;
      case TOP:
	limits.vertical=game_speed*game_accel*parse_float((*iter).value);
	break;
      case BOTTOM:
	limits.vertical=game_speed*game_accel*parse_float((*iter).value);
	break;
    }
    }

    break;

  case COMPUTER:  
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAXSPEED:
	computer.max_combat_speed=game_speed*parse_float((*iter).value);
	ADDELEM(speedStarHandler,XMLType(&computer.max_combat_speed));
	break;
      case AFTERBURNER:
	computer.max_combat_ab_speed=game_speed*parse_float((*iter).value);
	ADDELEM(speedStarHandler,XMLType(&computer.max_combat_ab_speed));
	break;
      case YAW:
	computer.max_yaw=parse_float((*iter).value)*(VS_PI/180);
	ADDELEM (angleStarHandler, XMLType(&computer.max_yaw));
	break;
      case PITCH:
	computer.max_pitch=parse_float((*iter).value)*(VS_PI/180);
	ADDELEM (angleStarHandler,XMLType(&computer.max_pitch));
	break;
      case ROLL:
	computer.max_roll=parse_float((*iter).value)*(VS_PI/180);
	ADDELEM (angleStarHandler,XMLType(&computer.max_roll));
	break;
      case SLIDE_START:
	computer.slide_start = parse_int ((*iter).value);
	ADDELEM (ucharStarHandler,XMLType(&computer.slide_start));
	break;
      case SLIDE_END:
	computer.slide_end = parse_int ((*iter).value);
	ADDELEM (ucharStarHandler,XMLType(&computer.slide_end));
	break;
      }
    }
    image->unitwriter->AddTag ("Radar");    
    ADDELEMNAME("itts",boolStarHandler,XMLType(&computer.itts));    
   ADDELEMNAME("color",boolStarHandler,XMLType(&computer.radar.color));    
    ADDELEMNAME("mintargetsize",charStarHandler,XMLType(&computer.radar.mintargetsize));    
    ADDELEMNAME("range",floatStarHandler,XMLType(&computer.radar.maxrange));    
    ADDELEMNAME("maxcone",floatStarHandler,XMLType(&computer.radar.maxcone));   
    ADDELEMNAME("TrackingCone",floatStarHandler,XMLType(&computer.radar.trackingcone));    
    ADDELEMNAME("lockcone",floatStarHandler,XMLType(&computer.radar.lockcone));    
    image->unitwriter->EndTag ("Radar");    
    break;
  case RADAR:
    //handled above
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case ITTS:
	computer.itts=parse_bool ((*iter).value);
	break;
      case MINTARGETSIZE:
	computer.radar.mintargetsize=parse_float ((*iter).value);
	break;
      case MAXCONE:
	computer.radar.maxcone = parse_float ((*iter).value);
	break;
      case LOCKCONE:
	computer.radar.lockcone = parse_float ((*iter).value);
	break;
      case TRACKINGCONE:
	computer.radar.trackingcone = parse_float ((*iter).value);
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
    ADDTAG;
    xml->yprrestricted+=Unit::XML::YRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	ADDDEFAULT;
	xml->ymax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	ADDDEFAULT;
	xml->ymin=parse_float((*iter).value)*(VS_PI/180);
	break;
    }
    }
    break;

  case PITCH:
    ADDTAG;
    xml->yprrestricted+=Unit::XML::PRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	ADDDEFAULT;
	xml->pmax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	ADDDEFAULT;
	xml->pmin=parse_float((*iter).value)*(VS_PI/180);
	break;
      }
    }
    break;
  case DESCRIPTION:
	ADDTAG;
      xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case RECURSESUBUNITCOLLISION:
	ADDDEFAULT;
	RecurseIntoSubUnitsOnCollision= XMLSupport::parse_bool (iter->value);
	break;
      case COMBATROLE:
		  ADDDEFAULT;
		  xml->calculated_role=true;
		  combat_role = ROLES::getRole(iter->value);
		break;
	  }
	}
	break;
  case ROLL:
    ADDTAG;
    xml->yprrestricted+=Unit::XML::RRESTR;
    assert (xml->unitlevel==2);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case MAX:
	ADDDEFAULT;
	xml->rmax=parse_float((*iter).value)*(VS_PI/180);
	break;
      case MIN:
	ADDDEFAULT;
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
      case UNITSCALE:
	xml->unitscale=parse_float((*iter).value);
	break;
      case COCKPIT:
	fprintf (stderr,"Cockpit attrib deprecated use tag");
	break;
      }
    }
    break;
  case COCKPIT:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;

    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	image->cockpitImage = (*iter).value;
	ADDELEM(stringStarHandler,XMLType (&image->cockpitImage));
	break;
      case X:
	image->CockpitCenter.i =xml->unitscale*parse_float ((*iter).value);
	ADDELEM(scaledFloatStarHandler,XMLType(tostring(xml->unitscale),&image->CockpitCenter.i));
	break;
      case Y:
	image->CockpitCenter.j =xml->unitscale*parse_float ((*iter).value);
	ADDELEM(scaledFloatStarHandler,XMLType(tostring(xml->unitscale),&image->CockpitCenter.j));
	break;
      case Z:
	image->CockpitCenter.k =xml->unitscale*parse_float ((*iter).value);
	ADDELEM(scaledFloatStarHandler,XMLType(tostring(xml->unitscale),&image->CockpitCenter.k));
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
	if ((*iter).value.length()){
	  image->hudImage = createSprite ((*iter).value.c_str());
	  xml->hudimage=(*iter).value;
	}
	break;
      case EXPLOSIONANI:
	if ((*iter).value.length()) {
	  image->explosion_type = (*iter).value;
	  {
	    cache_ani (image->explosion_type);

	  }
	}
	break;
      case REPAIRDROID:
	image->repair_droid = (unsigned char) parse_float ((*iter).value);
	break;
      case ECM:

	image->ecm = (short int)(32767*parse_float ((*iter).value));
	image->ecm = image->ecm>0?-image->ecm:image->ecm;
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
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case AFTERBURNENERGY:
	afterburnenergy =CLAMP_SHORT(parse_float((*iter).value)); 
	break;
      default:
	break;
      }
    }

    break;

  case RESTRICTED:
    ADDTAG;
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    break;
	
  case UNKNOWN:
    ADDTAG;
  default:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      ADDDEFAULT;
    }
    xml->unitlevel++;
    break;
  }
#undef ADDELEM
}

void Unit::endElement(const string &name) {
  using namespace UnitXML;
  image->unitwriter->EndTag (name);
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
unsigned char Unit::RecomputeRole() {
	//combat_role = 0;
	unsigned int mount_bitmask;
	for (unsigned int i=0;i<mounts.size();i++) {
		mount_bitmask |=mounts[i]->type->role_bits;
	}

	return combatRole();
}
void Unit::WriteUnit (const char * modifications) {
  if (image->unitwriter)
    image->unitwriter->Write(modifications);
  for (un_iter ui= getSubUnits();(*ui)!=NULL;++ui) {
    (*ui)->WriteUnit(modifications);
  } 
}
extern std::string GetReadPlayerSaveGame (int);

void Unit::LoadXML(const char *filename, const char * modifications, char * xmlbuffer, int buflength)
{
  shield.number=0;
  RecurseIntoSubUnitsOnCollision=!isSubUnit();
  const int chunk_size = 16384;
 // rrestricted=yrestricted=prestricted=false;
  FILE * inFile=NULL;
  std::string collideTreeHash = GetHashName(string(modifications)+"#"+string(filename));
  if( buflength==0 || xmlbuffer==NULL)
  {
	  if (modifications) {
		if (strlen(modifications)!=0) {
		  changehome();
		  static std::string savedunitpath=vs_config->getVariable ("data","serialized_xml","serialized_xml");
		  vschdir (savedunitpath.c_str());
		  string nonautosave=GetReadPlayerSaveGame(_Universe->CurrentCockpit());
		  // In network mode we only look in the save subdir in HOME
		  if( Network==NULL && !SERVER)
		  {
			  if (nonautosave.empty()) {
				vschdir (modifications);
			  }else {
				vschdir (nonautosave.c_str());
			  }
		  }
		  // If in network mode on client side we expect saves to be in ./save
		  else if( !SERVER)
			  vschdir( "save");
		  // With account server we expect them in the ./accounts dir
		  else if( SERVER==2)
			  vschdir( "accounts");
		  // With account server we expect them in the ./accountstmp dir
		  else if( SERVER==1)
			  vschdir( "accountstmp");


		  inFile=NULL;
		  if (filename[0])
    		    inFile = fopen (filename,"r");
		  vscdup();
		  vscdup();
		  returnfromhome();
		}
	  }
	  if (inFile==NULL)
	      if (filename[0])
		inFile = fopen (filename, "r");
	  if(!inFile) {
		cout << "Unit file " << filename << " not found" << endl;
		assert(0);
		return;
	  }
  }

  image->unitwriter=new XMLSerializer (filename,modifications,this);
  image->unitwriter->AddTag ("Unit");
  string * myhudim = new string("");
  float * myscale=&image->unitscale;
  image->unitwriter->AddElement("scale",floatStarHandler,XMLType(myscale));
  {
    image->unitwriter->AddTag ("Jump");
    image->unitwriter->AddElement("missing",lessNeg1Handler,XMLType(&jump.drive));
    image->unitwriter->AddElement("jumpenergy",shortStarHandler,XMLType(&jump.energy));
    image->unitwriter->AddElement("delay",ucharStarHandler,XMLType(&jump.delay));
    image->unitwriter->AddElement("damage",ucharStarHandler,XMLType(&jump.damage));
    image->unitwriter->AddElement("wormhole",ucharStarHandler,XMLType(&image->forcejump));
    image->unitwriter->EndTag ("Jump");
  }
  {
  unsigned int i;
  for (i=0;i<=(UnitImages::NUMGAUGES+MAXVDUS);i++) {
	image->unitwriter->AddTag("CockpitDamage");
	image->unitwriter->AddElement("damage",floatStarHandler,XMLType(&image->cockpit_damage[i]));
	image->unitwriter->EndTag("CockpitDamage");
  }}

  {
    image->unitwriter->AddTag("Defense");
    image->unitwriter->AddElement("HudImage",stringStarHandler,XMLType (myhudim));
    if (image->explosion_type.length()){
      image->unitwriter->AddElement("ExplosionAni",stringStarHandler,XMLType(&image->explosion_type));
    }
    image->unitwriter->AddElement("RepairDroid",ucharStarHandler,XMLType(&image->repair_droid));
    image->unitwriter->AddElement("ECM",shortToFloatHandler,XMLType(&image->ecm));
    {
      image->unitwriter->AddTag ("Cloak");
      image->unitwriter->AddElement("missing",cloakHandler,XMLType(&cloaking));
      image->unitwriter->AddElement("cloakmin",shortToFloatHandler,XMLType(&cloakmin));
      image->unitwriter->AddElement("cloakglass",ucharStarHandler,XMLType(&image->cloakglass));
      image->unitwriter->AddElement("cloakrate",shortToFloatHandler,XMLType(&image->cloakrate));
      image->unitwriter->AddElement("cloakenergy",floatStarHandler,XMLType(&image->cloakenergy));
      image->unitwriter->EndTag ("Cloak");
    }
    {
      image->unitwriter->AddTag ("Armor");
      image->unitwriter->AddElement("front",ushortStarHandler,XMLType(&armor.front));
      image->unitwriter->AddElement("back",ushortStarHandler,XMLType(&armor.back));
      image->unitwriter->AddElement("left",ushortStarHandler,XMLType(&armor.left));
      image->unitwriter->AddElement("right",ushortStarHandler,XMLType(&armor.right));
      image->unitwriter->EndTag ("Armor");
    }    
    {
      image->unitwriter->AddTag ("Shields");
      image->unitwriter->AddElement("front",shieldSerializer,XMLType((void *)&shield));
      image->unitwriter->AddElement("recharge",floatStarHandler,XMLType(&shield.recharge));
      image->unitwriter->AddElement("leak",charStarHandler,XMLType(&shield.leak));
      image->unitwriter->EndTag ("Shields");      
    }
    {
      image->unitwriter->AddTag ("Hull");
      image->unitwriter->AddElement("strength",floatStarHandler,XMLType(&hull));
      image->unitwriter->AddElement("maximum",floatStarHandler,XMLType(&maxhull));
      image->unitwriter->EndTag ("Hull");
    }

    image->unitwriter->EndTag("Defense");
  }
  {
    image->unitwriter->AddTag ("Energy");
    image->unitwriter->AddElement("afterburnenergy",ushortStarHandler,XMLType(&afterburnenergy));
    image->unitwriter->AddTag ("Reactor");
    image->unitwriter->AddElement ("recharge",floatStarHandler, XMLType (&recharge) );
    image->unitwriter->AddElement ("limit",ushortStarHandler, XMLType (&maxenergy) );
    image->unitwriter->EndTag ("Reactor");
    
    image->unitwriter->EndTag ("Energy");      
    
  }

  {
    image->unitwriter->AddTag ("Stats");    
    image->unitwriter->AddElement("mass",massSerializer,XMLType(&mass));
    image->unitwriter->AddElement("momentofinertia",floatStarHandler,XMLType(&MomentOfInertia));
    image->unitwriter->AddElement("fuel",floatStarHandler,XMLType(&fuel));
    image->unitwriter->EndTag ("Stats");    
    image->unitwriter->AddTag ("Thrust");    
    {
      image->unitwriter->AddTag ("Maneuver");    
      image->unitwriter->AddElement("yaw",angleStarHandler,XMLType(&limits.yaw));      
      image->unitwriter->AddElement("pitch",angleStarHandler,XMLType(&limits.pitch));      
      image->unitwriter->AddElement("roll",angleStarHandler,XMLType(&limits.roll));      
      image->unitwriter->EndTag ("Maneuver");    
    }
    {
      image->unitwriter->AddTag ("Engine");    
      image->unitwriter->AddElement("forward",accelStarHandler,XMLType(&limits.forward));      
      image->unitwriter->AddElement("retro",accelStarHandler,XMLType(&limits.retro));      
      image->unitwriter->AddElement("left",accelStarHandler,XMLType(&limits.lateral));     
      image->unitwriter->AddElement("right",accelStarHandler,XMLType(&limits.lateral));      
      image->unitwriter->AddElement("top",accelStarHandler,XMLType(&limits.vertical));      
      image->unitwriter->AddElement("bottom",accelStarHandler,XMLType(&limits.vertical));      
      image->unitwriter->AddElement("afterburner",accelStarHandler,XMLType(&limits.afterburn));      
      image->unitwriter->EndTag ("Engine");    
    }
    image->unitwriter->EndTag ("Thrust");    

  }
  image->CockpitCenter.Set (0,0,0);
  xml = new XML();
  xml->calculated_role=false;
  xml->damageiterator=0;
  xml->unitModifications = modifications;
  xml->shieldmesh = NULL;
  xml->bspmesh = NULL;
  xml->rapidmesh = NULL;
  xml->hasBSP = true;
  xml->hasColTree=true;
  xml->unitlevel=0;
  xml->unitscale=1;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Unit::beginElement, &Unit::endElement);
  
  if( buflength!=0 && xmlbuffer!=NULL)
  {
	XML_Parse (parser,xmlbuffer,buflength,1);
  }
  else
  {
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
  }
  XML_ParserFree (parser);
  // Load meshes into subunit
  image->unitwriter->EndTag ("Unit");
  meshdata= xml->meshes;
  meshdata.push_back(NULL);
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  *myhudim =xml->hudimage;
  unsigned int a;
  if (xml->mountz.size())
  {
	  // DO not destroy anymore, just affect address
	  for( a=0; a<xml->mountz.size(); a++)
		mounts.push_back( xml->mountz[a]);
    //mounts[a]=*xml->mountz[a];
    //delete xml->mountz[a];			//do it stealthily... no cons/destructor
  }
  unsigned char parity=0;
  for (a=0;a<xml->mountz.size();a++) {
    static bool half_sounds = XMLSupport::parse_bool(vs_config->getVariable ("audio","every_other_mount","false"));
    if (a%2==parity) {
      int b=a;
      if(a % 4 == 2 && (int) a < (GetNumMounts()-1)) 
	if (mounts[a]->type->type != weapon_info::PROJECTILE&&mounts[a+1]->type->type != weapon_info::PROJECTILE)
	  b=a+1;
      mounts[b]->sound = AUDCreateSound (mounts[b]->type->sound,mounts[b]->type->type!=weapon_info::PROJECTILE);
    } else if ((!half_sounds)||mounts[a]->type->type == weapon_info::PROJECTILE) {
      mounts[a]->sound = AUDCreateSound (mounts[a]->type->sound,mounts[a]->type->type!=weapon_info::PROJECTILE);    //lloping also flase in unit_customize  
    }
    if (a>0) {
      if (mounts[a]->sound==mounts[a-1]->sound&&mounts[a]->sound!=-1) {
	printf ("error");
      }
    }
  }
  for( a=0; a<xml->units.size(); a++) {
    SubUnits.prepend(xml->units[a]);
  }
  calculate_extent(false);
  if (!SubUnit) {
    //UpdateCollideQueue();
  }
  image->unitscale=xml->unitscale;
  string tmpname (filename);
#ifndef PLEASEDONTCOMMENTRANDOMTHINGSOUT
  vector <bsp_polygon> polies;

  this->colTrees = collideTrees::Get(collideTreeHash);
  if (this->colTrees) {
    this->colTrees->Inc();
  }
  BSPTree * bspTree=NULL;
  BSPTree * bspShield=NULL;
  csRapidCollider *colShield=NULL;
  csRapidCollider *colTree=NULL;
  if (xml->shieldmesh) {
    meshdata.back() = xml->shieldmesh;
    if (!this->colTrees) {
      if (!CheckBSP ((tmpname+"_shield.bsp").c_str())) {
	BuildBSPTree ((tmpname+"_shield.bsp").c_str(), false, meshdata.back());
      }
      if (CheckBSP ((tmpname+"_shield.bsp").c_str())) {
	bspShield = new BSPTree ((tmpname+"_shield.bsp").c_str());
      }
      if (meshdata.back()) {
	meshdata.back()->GetPolys(polies);
	colShield = new csRapidCollider (polies);
      }
    }
  }
  else {
    Mesh * tmp = NULL;
    if (!this->colTrees) {
#if 0
      tmp= new SphereMesh (rSize(),8,8,vs_config->getVariable("graphics","shield_texture","shield.bmp").c_str(), NULL, false,ONE, ONE);///shield not used right now for collisions
      tmp->GetPolys (polies);
      if (xml->hasColTree)
	colShield = new csRapidCollider (polies);
      else
#endif
	colShield=NULL;
    }
    static int shieldstacks = XMLSupport::parse_int (vs_config->getVariable ("graphics","shield_detail","16"));
    tmp = new SphereMesh (rSize(),shieldstacks,shieldstacks,vs_config->getVariable("graphics","shield_texture","shield.bmp").c_str(), NULL, false,ONE, ONE);
    
    meshdata.back() = tmp;
    bspShield=NULL;
    colShield=NULL;
  }
  meshdata.back()->EnableSpecialFX();
  if (!this->colTrees) {
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
    }
    polies.clear();  
    if (!xml->rapidmesh) {
      for (int j=0;j<nummesh();j++) {
	meshdata[j]->GetPolys(polies);
      }
    }else {
      xml->rapidmesh->GetPolys (polies);
    }
    if (xml->hasColTree ) {
      colTree = new csRapidCollider (polies);    
    }else {
      colTree=NULL;
    }
    this->colTrees = new collideTrees (collideTreeHash,bspTree,bspShield,colTree,colShield);
  }
  if (xml->bspmesh) {
    delete xml->bspmesh;
  }
  if (xml->rapidmesh) {
    delete xml->rapidmesh;
  }
#endif
  delete xml;
}
