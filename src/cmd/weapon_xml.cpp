
#include "star_system.h"


#include "weapon_xml.h"
#include <assert.h>
#include "audiolib.h"
#include "unit_generic.h"
#include "beam.h"
#include "unit_const_cache.h"
#include "vsfilesystem.h"
#include "role_bitmask.h"
#include "endianness.h"
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

weapon_info	getWeaponInfoFromBuffer( char * netbuf, int & size)
{
		weapon_info wi(weapon_info::UNKNOWN);
		unsigned short file_len = 0, weap_len = 0;
		int offset=0;

		// Get the weapon_info structure
		memcpy( &wi, netbuf+offset, sizeof( wi));
		offset += sizeof( wi);
		memcpy( &file_len, netbuf+offset, sizeof( file_len));
		offset += sizeof( file_len);
		file_len = VSSwapHostShortToLittle( file_len);
		char * filename = new char[file_len];
		memcpy( filename, netbuf, file_len);
		offset += file_len;
		memcpy( &weap_len, netbuf+offset, sizeof( weap_len));
		offset += sizeof( weap_len);
		weap_len = VSSwapHostShortToLittle( weap_len);
		char * weapname = new char[weap_len];
		memcpy( weapname, netbuf, weap_len);
		wi.file = string( filename);
		wi.weapon_name = string( weapname);
		delete filename;
		delete weapname;

		return wi;
}

void		setWeaponInfoToBuffer( weapon_info wi, char * netbuf, int & bufsize)
{
	bufsize = sizeof( wi)+sizeof( wi.file)+sizeof( wi.weapon_name);
	netbuf = new char [bufsize+1];
	netbuf[bufsize]=0;
	int offset = 0;

	unsigned short file_len = wi.file.length();
	unsigned short weap_len = wi.weapon_name.length();
	char * file = new char[file_len+1];
	char * weapon_name = new char[weap_len+1];
	memcpy( file, wi.file.c_str(), file_len);
	file[file_len] = 0;
	memcpy( weapon_name, wi.weapon_name.c_str(), weap_len);
	weapon_name[weap_len] = 0;

	// Copy the struct weapon_info in the buffer
	memcpy( netbuf+offset, &wi, sizeof( wi));
	offset += sizeof( wi);
	// Copy the size of filename in the buffer
	memcpy( netbuf+offset, &file_len, sizeof( file_len));
	offset += sizeof( file_len);
	// Copy the filename in the buffer because in weapon_info, it is a string
	memcpy( netbuf+offset, file, file_len);
	offset += file_len;
	// Copy the size of filename in the buffer
	memcpy( netbuf+offset, &weap_len, sizeof( weap_len));
	offset += sizeof( weap_len);
	// Copy the weapon_name in the buffer because in weapon_info, it is a string
	memcpy( netbuf+offset, weapon_name, weap_len);

	delete file;
	delete weapon_name;
}

void	weapon_info::netswap()
{
	// Enum elements are the size of an int
	// byte order swap doesn't work with ENUM - MAY NEED TO FIND A WORKAROUND SOMEDAY
	//type = VSSwapHostIntToLittle( type);
	//size = VSSwapHostIntToLittle( size);
	offset.netswap();
	role_bits = VSSwapHostIntToLittle( role_bits);
	sound = VSSwapHostIntToLittle( sound);
	r = VSSwapHostFloatToLittle( r);
	g = VSSwapHostFloatToLittle( g);
	b = VSSwapHostFloatToLittle( b);
	a = VSSwapHostFloatToLittle( a);
	Speed = VSSwapHostFloatToLittle( Speed);
	PulseSpeed = VSSwapHostFloatToLittle( PulseSpeed);
	RadialSpeed = VSSwapHostFloatToLittle( RadialSpeed);
	Range = VSSwapHostFloatToLittle( Range);
	Radius = VSSwapHostFloatToLittle( Radius);
	Length = VSSwapHostFloatToLittle( Length);
	Damage = VSSwapHostFloatToLittle( Damage);
	PhaseDamage = VSSwapHostFloatToLittle( PhaseDamage);
	Stability = VSSwapHostFloatToLittle( Stability);
	Longrange = VSSwapHostFloatToLittle( Longrange);
	LockTime = VSSwapHostFloatToLittle( LockTime);
	EnergyRate = VSSwapHostFloatToLittle( EnergyRate);
	Refire = VSSwapHostFloatToLittle( Refire);
	volume = VSSwapHostFloatToLittle( volume);
	TextureStretch = VSSwapHostFloatToLittle( TextureStretch);
}

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
    ROLE,ANTIROLE,
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
    EnumMap::Pair("AntiRole",ANTIROLE),
    EnumMap::Pair ("TextureStretch",TEXTURESTRETCH)
  };
  const EnumMap element_map(element_names, 10);
  const EnumMap attribute_map(attribute_names, 32);
  Hashtable <string, weapon_info, 257> lookuptable;
  string curname;
  weapon_info tmpweapon(weapon_info::BEAM);
  int level=-1;
  void beginElement (void *userData, const XML_Char *name, const XML_Char **atts) {
    static float game_speed=XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
    static bool adj_gun_speed=XMLSupport::parse_bool (vs_config->getVariable ("physics","gun_speed_adjusted_game_speed","false"));
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
	  VSFileSystem::vs_fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case NAME:
	  curname = (*iter).value;
	  tmpweapon.weapon_name=curname;
	  break;
	case ROLE:
  	  tmpweapon.role_bits = ROLES::readBitmask(iter->value);
	  break;
	case ANTIROLE:
  	  tmpweapon.role_bits = ROLES::readBitmask(iter->value);
          tmpweapon.role_bits = ~tmpweapon.role_bits;
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
	  VSFileSystem::vs_fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
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
	  VSFileSystem::vs_fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
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
	  VSFileSystem::vs_fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
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
	  VSFileSystem::vs_fprintf (stderr,"Unknown Weapon Element %s",(*iter).name.c_str());
	  break;
	case VOLUME:
	  tmpweapon.volume = XMLSupport::parse_float ((*iter).value);
	  break;
	case SPEED:
		tmpweapon.Speed = XMLSupport::parse_float ((*iter).value);
		if(tmpweapon.Speed<1000){
			tmpweapon.Speed=tmpweapon.Speed*(adj_gun_speed?(1.0+gun_speed/1.25):gun_speed); 
		} else {
		  if(tmpweapon.Speed<2000){
			tmpweapon.Speed=tmpweapon.Speed*(adj_gun_speed?(1.0+gun_speed/2.5):(gun_speed));
		  }else{
			  if(tmpweapon.Speed<4000){
				tmpweapon.Speed=tmpweapon.Speed*(adj_gun_speed?(1.0+gun_speed/6.0):(gun_speed));
			  } else {
				  if (tmpweapon.Speed<8000){
					tmpweapon.Speed=tmpweapon.Speed*(adj_gun_speed?(1.0+gun_speed/17.0):(gun_speed));
				  }
			  }
		  }
		}
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
	  tmpweapon.Range= (adj_gun_speed?(1.0+gun_speed/16.0):(gun_speed))*XMLSupport::parse_float ((*iter).value);
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
using namespace VSFileSystem;
extern string strtolower(const string &foo);
weapon_info* getTemplate(const string &kkey) {
  string key =strtolower(kkey);
  weapon_info * wi =  lookuptable.Get(strtoupper(key));
  if (wi) {
    if (!WeaponMeshCache::getCachedMutable (wi->weapon_name)) {
		static string sharedmountdir = vs_config->getVariable("data","mountlocation","weapons");

		//string meshshell=VSFileSystem::sharedmeshes+"/"+sharedmountdir+string ("/") + key;
		string meshname=key+".bfxm";
		if (LookForFile( meshname, MeshFile)<=Ok) {
			WeaponMeshCache::setCachedMutable (wi->weapon_name,wi->gun= Mesh::LoadMesh (meshname.c_str(),Vector(1,1,1),0,NULL));
			if (LookForFile( meshname, MeshFile)<=Ok) {
				WeaponMeshCache::setCachedMutable (wi->weapon_name+"_flare",wi->gun1=Mesh::LoadMesh (meshname.c_str(),Vector(1,1,1),0,NULL));
			}
		}
    }
  }
  return wi;
}

void LoadWeapons(const char *filename) {
  using namespace VSFileSystem;
  const int chunk_size = 16384;
  VSFile f;
  VSError err = f.OpenReadOnly( filename, UnknownFile);
  if (err>Ok) {
    return;
  }
  XML_Parser parser = XML_ParserCreate (NULL);
  XML_SetElementHandler (parser, &beginElement, &endElement);
  XML_Parse (parser,(f.ReadFull()).c_str(),f.Size(),1);

  /*
 do {
#ifdef BIDBG
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
#else
    char buf[chunk_size];
#endif
    int length;
    
    length = VSFileSystem::vs_read (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
#ifdef BIDBG
    XML_ParseBuffer(parser, length, VSFileSystem::vs_feof(inFile));
#else
    XML_Parse (parser,buf,length,VSFileSystem::vs_feof (inFile));
#endif
  } while(!VSFileSystem::vs_feof(inFile));
  */
 f.Close();
 XML_ParserFree (parser);
}
extern enum weapon_info::MOUNT_SIZE lookupMountSize (const char * str);
