
#include "star_system.h"


#include "weapon_xml.h"
#include <assert.h>
#include "audiolib.h"
#include "unit_generic.h"
#include "beam.h"
#include "unit_const_cache.h"
#include "vs_path.h"
#include "role_bitmask.h"
/*
weapon_info& weapon_info::operator = (const weapon_info &tmp){
  size = tmp.size;
  type = tmp.type;
  file = tmp.file;
  r = tmp.r;g=tmp.g;b=tmp.b;a=tmp.a;
  Speed=tmp.Speed;PulseSpeed=tmp.PulseSpeed;RadialSpeed=tmp.RadialSpeed;Range=tmp.Range;Radius=tmp.Radius;Length=tmp.Length;volume = tmp.volume;
  Damage=tmp.Damage;Stability=tmp.Stability;Longrange=tmp.Longrange;
  EnergyRate=tmp.EnergyRate;EnergyConsumption=tmp.EnergyConsumption;Refire=tmp.Refire;
  return *this;
}
*/

#include "xml_support.h"
#include "physics.h"
#include <vector>

#include <expat.h>


using std::vector;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

namespace BeamXML {
  enum Names {
    UNKNOWN,
    WEAPONS,
    BEAM,
    BALL,
    BOLT,
    PROJECTILE,
    APPEARANCE,
    //    MANEUVER,
    ENERGY,
    DAMAGE,
    DISTANCE,
    //attributes
    NAME,
    SOUNDMP3,
    SOUNDWAV,
    WEAPSIZE,
    XFILE,
    RED,
    GREEN,
    BLUE,
    ALPHA,
    SPEED,
    OFFSETX,OFFSETY,OFFSETZ,
    PULSESPEED,
    RADIALSPEED,
    RANGE,
    RADIUS,
    RATE,
    STABILITY,
    LONGRANGE,
    CONSUMPTION,
    REFIRE,
    LENGTH,
    PHASEDAMAGE,
    VOLUME,
    DETONATIONRADIUS,
    LOCKTIME,
	ROLE,
	TEXTURESTRETCH
	
    //YAW,
    //PITCH,
    //ROLL
  };
  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN",UNKNOWN),//don't add anything until below missile so it maps to enum WEAPON_TYPE
    EnumMap::Pair ("Beam",BEAM),
    EnumMap::Pair ("Ball",BALL),
    EnumMap::Pair ("Bolt",BOLT),
    EnumMap::Pair ("Missile", PROJECTILE),
    EnumMap::Pair ("Weapons",WEAPONS),
    EnumMap::Pair ("Appearance", APPEARANCE),
    //    EnumMap::Pair ("Maneuver",MANEUVER),
    EnumMap::Pair ("Energy",ENERGY),
    EnumMap::Pair ("Damage",DAMAGE),
    EnumMap::Pair ("Distance",DISTANCE)
  };
  const EnumMap::Pair attribute_names [] = {
    EnumMap::Pair ("UNKNOWN",UNKNOWN),
    EnumMap::Pair ("Name",NAME),
    EnumMap::Pair ("MountSize",WEAPSIZE),
    EnumMap::Pair ("file",XFILE),
    EnumMap::Pair ("soundMp3",SOUNDMP3),
    EnumMap::Pair ("soundWav",SOUNDWAV),
    EnumMap::Pair ("r",RED),
    EnumMap::Pair ("g",GREEN),
    EnumMap::Pair ("b",BLUE),
    EnumMap::Pair ("a",ALPHA),
    EnumMap::Pair ("Speed",SPEED),
    EnumMap::Pair ("Pulsespeed",PULSESPEED),
    EnumMap::Pair ("DetonationRange",DETONATIONRADIUS),
    EnumMap::Pair ("LockTime",LOCKTIME),
    EnumMap::Pair ("Radialspeed",RADIALSPEED),
    EnumMap::Pair ("Range",RANGE),
    EnumMap::Pair ("Radius",RADIUS),
    EnumMap::Pair ("Rate",RATE),
    EnumMap::Pair ("Damage",DAMAGE),
    EnumMap::Pair ("PhaseDamage",PHASEDAMAGE),
    EnumMap::Pair ("Stability",STABILITY),
    EnumMap::Pair ("Longrange",LONGRANGE),
    EnumMap::Pair ("Consumption",CONSUMPTION),
    EnumMap::Pair ("Refire",REFIRE),
    EnumMap::Pair ("Length", LENGTH),
    EnumMap::Pair ("OffsetX",OFFSETX),
    EnumMap::Pair ("OffsetY",OFFSETY),
    EnumMap::Pair ("OffsetZ",OFFSETZ),
    EnumMap::Pair ("Volume", VOLUME),
	EnumMap::Pair("Role",ROLE),
	EnumMap::Pair ("TextureStretch",TEXTURESTRETCH)
  };
  const EnumMap element_map(element_names, 10);
  const EnumMap attribute_map(attribute_names, 31);
  Hashtable <string, weapon_info,char[257]> lookuptable;
  string curname;
  weapon_info tmpweapon(weapon_info::BEAM);
  int level=-1;
  void beginElement (void *userData, const XML_Char *name, const XML_Char **atts) {
    static float game_speed=XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
    static bool adj_gun_speed=XMLSupport::parse_float (vs_config->getVariable ("physics","gun_speed_adjusted_game_speed","false"));
    static float gun_speed= XMLSupport::parse_float (vs_config->getVariable("physics","gun_speed","1"))*(adj_gun_speed?game_speed:1);
    AttributeList attributes (atts);
    //weapon_info * debugtmp = &tmpweapon;
    enum weapon_info::WEAPON_TYPE weaptyp;
    Names elem = (Names) element_map.lookup(string (name));
#ifdef TESTBEAMSONLY
    if (elem==BOLT)
      elem=BEAM;
#endif
    AttributeList::const_iterator iter;
    switch (elem) {
    case UNKNOWN:
      break;
    case WEAPONS:
      assert (level==-1);
      level++;
      break;
    case BOLT:
    case BEAM:
    case BALL:
    case PROJECTILE:
      assert (level==0);
      level++;
      switch(elem) {
      case BOLT:
	weaptyp = weapon_info::BOLT;
	break;
      case BEAM:
	weaptyp=weapon_info::BEAM;
	break;
      case BALL:
	weaptyp=weapon_info::BALL;
	break;
      case PROJECTILE:
	weaptyp=weapon_info::PROJECTILE;
	break;
      default:
	weaptyp=weapon_info::UNKNOWN;
	break;
      }
      tmpweapon.Type(weaptyp);
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case NAME:
	  curname = (*iter).value;
	  tmpweapon.weapon_name=curname;
	  break;
	case ROLE:
  	  tmpweapon.role_bits = ROLES::readBitmask(iter->value);
	  break;
	case WEAPSIZE:
	  tmpweapon.MntSize (lookupMountSize ((*iter).value.c_str()));
	  break;
	default:
	  assert (0);
	  break;
	}
      }
      break;
    case APPEARANCE:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case XFILE:
	   tmpweapon.file = (*iter).value;
	  break;
	case SOUNDMP3:
	  tmpweapon.sound = AUDCreateSoundMP3((*iter).value,tmpweapon.type!=weapon_info::PROJECTILE);
	  break;
	case SOUNDWAV:
	  tmpweapon.sound = AUDCreateSoundWAV((*iter).value,tmpweapon.type==weapon_info::PROJECTILE);
	  break;
	case OFFSETX:
	  tmpweapon.offset.i = XMLSupport::parse_float (iter->value);
	  break;
	case OFFSETY:
	  tmpweapon.offset.j = XMLSupport::parse_float (iter->value);
	  break;
	case OFFSETZ:
	  tmpweapon.offset.k = XMLSupport::parse_float (iter->value);
	  break;
	case RED:
	  tmpweapon.r = XMLSupport::parse_float ((*iter).value);
	  break;
	case GREEN:
	  tmpweapon.g = XMLSupport::parse_float ((*iter).value);
	  break;
	case BLUE:
	  tmpweapon.b = XMLSupport::parse_float ((*iter).value);
	  break;
	case ALPHA:
	  tmpweapon.a = XMLSupport::parse_float ((*iter).value);
	  break;
	case TEXTURESTRETCH:
      tmpweapon.TextureStretch=
		  XMLSupport::parse_float((*iter).value);
	  break;
 	default:
	  assert (0);
	  break;
	}
      }      
      break;
    case ENERGY:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case CONSUMPTION:
	  tmpweapon.EnergyRate = XMLSupport::parse_float ((*iter).value);
	  break;
	case RATE:
	  tmpweapon.EnergyRate = XMLSupport::parse_float ((*iter).value);
	  break;
	case STABILITY:
	  tmpweapon.Stability = XMLSupport::parse_float ((*iter).value);
	  break;
	case REFIRE:
	  tmpweapon.Refire = XMLSupport::parse_float ((*iter).value);
	  break;
	case LOCKTIME:
	  tmpweapon.LockTime = XMLSupport::parse_float ((*iter).value);
	  break;
	default:
	  assert (0);
	  break;
	}
      }      
      break;
    case DAMAGE:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case DAMAGE:
	  tmpweapon.Damage = XMLSupport::parse_float((*iter).value);
	  break;
	case RADIUS:
	  tmpweapon.Radius = XMLSupport::parse_float ((*iter).value);
	  break;
	case RADIALSPEED:
	  tmpweapon.RadialSpeed = XMLSupport::parse_float ((*iter).value);
	  break;
	case PHASEDAMAGE:
	  tmpweapon.PhaseDamage = XMLSupport::parse_float((*iter).value);
	  break;
	case RATE:
	  tmpweapon.Damage = XMLSupport::parse_float ((*iter).value);
	  break;
	case LONGRANGE:
	  tmpweapon.Longrange = XMLSupport::parse_float ((*iter).value);
	  break;
	default:
	  assert (0);
	  break;
	}
      }
      break;
    case DISTANCE:
      assert (level==1);
      level++;
      for (iter= attributes.begin(); iter!=attributes.end();iter++) {
	switch (attribute_map.lookup ((*iter).name)) {
	case UNKNOWN:
	  fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case VOLUME:
	  tmpweapon.volume = XMLSupport::parse_float ((*iter).value);
	  break;
	case SPEED:
	  tmpweapon.Speed = gun_speed*XMLSupport::parse_float ((*iter).value);
	  break;
	case PULSESPEED:
	  if (tmpweapon.type==weapon_info::BEAM) {
	    tmpweapon.PulseSpeed = XMLSupport::parse_float ((*iter).value);
	  }
	  break;
	case DETONATIONRADIUS:
	  if (tmpweapon.type!=weapon_info::BEAM) {
	    tmpweapon.PulseSpeed = XMLSupport::parse_float ((*iter).value);
	  }
	  break;
	case RADIALSPEED:
	  tmpweapon.RadialSpeed = XMLSupport::parse_float ((*iter).value);
	  break;
	case RANGE:
	  tmpweapon.Range= gun_speed*XMLSupport::parse_float ((*iter).value);
	  break;
	case RADIUS:
	  tmpweapon.Radius = XMLSupport::parse_float ((*iter).value);
	  break;
	case LENGTH:
	  tmpweapon.Length = XMLSupport::parse_float ((*iter).value);
	  break;
	default:
	  assert (0);
	  break;
	}
      }
      break;
    default:
      assert (0);
      break;
    }
  }

  void endElement (void *userData, const XML_Char *name) {
    Names elem = (Names)element_map.lookup(name);
    switch (elem) {
    case UNKNOWN:
      break;
    case WEAPONS:
      assert (level==0);
      level--;
      break;
    case BEAM:
    case BOLT:
    case BALL:
    case PROJECTILE:
      assert (level==1);
      level--;
      lookuptable.Put (strtoupper(curname),new weapon_info (tmpweapon));
      tmpweapon.init();
      break;
    case ENERGY:
    case DAMAGE:
    case DISTANCE:
    case APPEARANCE:
      assert (level==2);
      level--;
      break;
    default:
      break;
    }
  }

}

using namespace BeamXML;

weapon_info* getTemplate(const string &key) {
 
  weapon_info * wi =  lookuptable.Get(strtoupper(key));
  if (wi) {
    if (!WeaponMeshCache::getCachedMutable (wi->weapon_name)) {
      string meshname = sharedmeshes+string ("weapons/") + key+".xmesh"; 
      FILE * fp = fopen (meshname.c_str(),"rb");
      if (fp) {
	fclose (fp);
	WeaponMeshCache::setCachedMutable (wi->weapon_name,wi->gun=new Mesh (meshname.c_str(),Vector(1,1,1),0,NULL));
      }
    }
  }
  return wi;
}

void LoadWeapons(const char *filename) {
  const int chunk_size = 16384;
  FILE * inFile= fopen (filename,"r");
  if (!inFile) {
    return;
  }
  XML_Parser parser = XML_ParserCreate (NULL);
  XML_SetElementHandler (parser, &beginElement, &endElement);
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
    XML_Parse (parser,buf,length,feof (inFile));
#endif
  } while(!feof(inFile));
 fclose (inFile);
 XML_ParserFree (parser);
}
extern enum weapon_info::MOUNT_SIZE lookupMountSize (const char * str);
