#include "config.h"
#include "cockpit.h"
#include "xml_support.h"
#include "gauge.h"
#include <float.h>
#include "hud.h"
#include "vdu.h"
#include "mesh.h"
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
namespace CockpitXML {
    enum Names {
      UNKNOWN=UnitImages::NUMGAUGES,
      COCKPIT,
      MESH,
      CROSSHAIRS,
      RADAR,
	  REARRADAR,
      LVDU,
      RVDU,
      AVDU,
      VDUTYPE,
      PANEL,
      ROWS,
      COLS,
      XFILE,
	  SOUNDFILE,
      XCENT,
      YCENT,
      TOPY,
      BOTY,
      XSIZE,
      YSIZE,
      MYFONT,
      RED,
      GREEN,
      BLUE,
      COCKPITOFFSET,
      VIEWOFFSET,
      FRONT,
      BACK,
      LEFT,
      RIGHT,
//    use the UnitImages enum for the gauge values instead!
/*    KARMORF,
      KARMORB,
      KARMORR,
      KARMORL,
      KFUEL, 
      KSHIELDF,
      KSHIELDR,
      KSHIELDL,
      KSHIELDB, 
      KENERGY,
      KAUTO,
      KEJECT,
      KLOCK,
      KHULL,
      KWARPENERGY,
      KKPS,
      KSETKPS,
      KFPS,
	  COCKPIT_LAG,
*/
      G_UP,
      G_DOWN,
      G_LEFT,
      G_RIGHT
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Cockpit", COCKPIT),
    EnumMap::Pair ("Radar", RADAR),
    EnumMap::Pair ("RearRadar", REARRADAR),
    EnumMap::Pair ("LeftVDU", LVDU),
    EnumMap::Pair ("VDU", AVDU),
    EnumMap::Pair ("RightVDU", RVDU),
    EnumMap::Pair ("Panel", PANEL),
    EnumMap::Pair ("Crosshairs", CROSSHAIRS),
    EnumMap::Pair ("ArmorF", UnitImages::ARMORF),
    EnumMap::Pair ("ArmorR", UnitImages::ARMORR),
    EnumMap::Pair ("ArmorL", UnitImages::ARMORL),
    EnumMap::Pair ("ArmorB", UnitImages::ARMORB),
    EnumMap::Pair ("ShieldF", UnitImages::SHIELDF),
    EnumMap::Pair ("ShieldR", UnitImages::SHIELDR),
    EnumMap::Pair ("ShieldL", UnitImages::SHIELDL),
    EnumMap::Pair ("ShieldB", UnitImages::SHIELDB),
    EnumMap::Pair ("Fuel", UnitImages::FUEL),
    EnumMap::Pair ("Energy", UnitImages::ENERGY),
    EnumMap::Pair ("Eject", UnitImages::EJECT),
    EnumMap::Pair ("Lock", UnitImages::LOCK),
	EnumMap::Pair ("MissileLock",UnitImages::MISSILELOCK),
	EnumMap::Pair ("Jump",UnitImages::JUMP),
	EnumMap::Pair ("ECM",UnitImages::ECM),	
    EnumMap::Pair ("Hull", UnitImages::HULL),
	EnumMap::Pair ("WarpEnergy", UnitImages::WARPENERGY),
    EnumMap::Pair ("Speed", UnitImages::KPS),
    EnumMap::Pair ("SetSpeed", UnitImages::SETKPS),
    EnumMap::Pair ("Auto", UnitImages::AUTOPILOT),
	EnumMap::Pair ("Collision", UnitImages::COLLISION),
    EnumMap::Pair ("FPS", UnitImages::COCKPIT_FPS),
    EnumMap::Pair ("WarpFieldStrength", UnitImages::WARPFIELDSTRENGTH)
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("mesh", MESH),
    EnumMap::Pair ("file", XFILE),
    EnumMap::Pair ("soundfile", SOUNDFILE),
    EnumMap::Pair ("font", MYFONT),
    EnumMap::Pair ("front", FRONT),
    EnumMap::Pair ("left", LEFT),
    EnumMap::Pair ("right", RIGHT),
    EnumMap::Pair ("back", BACK),
    EnumMap::Pair ("xcent", XCENT),
    EnumMap::Pair ("ycent", YCENT),
    EnumMap::Pair ("width", XSIZE),
    EnumMap::Pair ("height", YSIZE),
    EnumMap::Pair ("Top", TOPY),
    EnumMap::Pair ("Bottom", BOTY),
    EnumMap::Pair ("ViewOffset", VIEWOFFSET),
    EnumMap::Pair ("CockpitOffset", COCKPITOFFSET),
    EnumMap::Pair ("GaugeUp",G_UP),
    EnumMap::Pair ("GaugeDown",G_DOWN),
    EnumMap::Pair ("GaugeLeft",G_LEFT),
    EnumMap::Pair ("GaugeRight",G_RIGHT),
    EnumMap::Pair ("TextRows", ROWS),
    EnumMap::Pair ("TextCols", COLS),
    EnumMap::Pair ("r", RED),
    EnumMap::Pair ("g", GREEN),
    EnumMap::Pair ("b", BLUE),
    EnumMap::Pair ("type", VDUTYPE)
  };

  const EnumMap element_map(element_names, 32);
  const EnumMap attribute_map(attribute_names, 27);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace CockpitXML;


string getRes(string inp) {
  string::size_type where=inp.rfind(".");
  int x = g_game.x_resolution;
  if (x<700) x=640;
  else if (x<840) x=800;
  else if (x<1100) x=1024;
  else if (x<1400) x=1280;
  else if (x<1700) x=1600;
  else x=1600;
  string rez =XMLSupport::tostring(x);
  
  if (where==string::npos) {
    return inp+"_"+rez+".spr";
  }else {
    return inp.substr(0,where)+"_"+rez+".spr";
  } 
}
void GameCockpit::beginElement(const string &name, const AttributeList &attributes) {
  static bool cockpit_smooth=XMLSupport::parse_bool(vs_config->getVariable("graphics","cockpit_smooth_texture","false"));
  static bool crosshair_smooth=XMLSupport::parse_bool(vs_config->getVariable("graphics","crosshair_smooth_texture","true"));
  AttributeList::const_iterator iter;
  Gauge::DIRECTION tmpdir=Gauge::GAUGE_UP;
  VSSprite ** newsprite=NULL;
  VDU **newvdu=NULL;
  VSSprite * adjsprite=NULL;
  std::string gaugename ("shieldstat.spr");
  std::string myfont ("9x12.font");
  Names elem = (Names)element_map.lookup(name);
  Names attr;
  unsigned int mymodes=0;
  float xsize=-1,ysize=-1,xcent=FLT_MAX,ycent=FLT_MAX;
  float leftx=-10;  float rightx=-10;
  float topy=-10; float boty = -10;
  short rows=13;
  short cols=15;
  int default_mode=VDU::TARGET;
  VSSprite * oldpit=NULL;
  bool replaced[4]={false,false,false,false};
  int counter=0;
  switch (elem) {
  case COCKPIT:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      attr = (Names)attribute_map.lookup((*iter).name);
      switch (attr) {
      case MYFONT:
	myfont = (*iter).value;
	break;
      case RED:
	textcol.r = XMLSupport::parse_float ((*iter).value);
	break;
      case GREEN:
	textcol.g = XMLSupport::parse_float ((*iter).value);
	break;
      case BLUE:
	textcol.b = XMLSupport::parse_float ((*iter).value);
	break;
      case VIEWOFFSET:
		  viewport_offset = XMLSupport::parse_float ((*iter).value);
	break;
      case COCKPITOFFSET:
		  cockpit_offset = XMLSupport::parse_float ((*iter).value);
	break;
      case XFILE:
        {
          std::string tmp=getRes((*iter).value);
          oldpit=Pit[0];
          Pit[0]= new VSSprite (tmp.c_str(),cockpit_smooth?BILINEAR:NEAREST);
          if (!Pit[0]->LoadSuccess()){
            delete Pit[0];
            Pit[0]= new VSSprite ((*iter).value.c_str(),cockpit_smooth?BILINEAR:NEAREST);
          }
          replaced[0]=true;
          if (oldpit) {

            delete oldpit;
          }
        }
	break;
      case SOUNDFILE:
	SetSoundFile((*iter).value);
	break;
      case MESH:
		  mesh = Mesh::LoadMeshes ((*iter).value.c_str(),Vector(1,1,1),0,NULL);
	break;
      case FRONT:
      case BACK:
      case LEFT:
      case RIGHT:
        {
          std::string tmp=getRes((*iter).value);
          oldpit=Pit[attr-FRONT];
          Pit[attr-FRONT] = new VSSprite (tmp.c_str(),cockpit_smooth?BILINEAR:NEAREST);
          if (!Pit[attr-FRONT]->LoadSuccess()) {
            delete Pit[attr-FRONT];
            Pit[attr-FRONT] = new VSSprite ((*iter).value.c_str(),cockpit_smooth?BILINEAR:NEAREST);
          }
          replaced[attr-FRONT]=true;
          if (oldpit){

            delete oldpit;
          }
        }
	break;
	  default:
		  break;
      } 
    }
    text = new TextPlane ();
    for (counter=0;counter<4;++counter) {
      if (!replaced[counter]) {
        delete Pit[counter];
        Pit[counter]=false;
      }
    }
    break;
  case UnitImages::JUMP:
  case UnitImages::MISSILELOCK:
  case UnitImages::ECM:
  case UnitImages::ARMORF:
  case UnitImages::ARMORR:
  case UnitImages::ARMORL:
  case UnitImages::ARMORB:
  case UnitImages::FUEL:
  case UnitImages::SHIELDF:
  case UnitImages::SHIELDL:
  case UnitImages::SHIELDR:
  case UnitImages::SHIELDB:
  case UnitImages::ENERGY:
  case UnitImages::WARPENERGY:
  case UnitImages::EJECT:
  case UnitImages::LOCK:
  case UnitImages::HULL:
  case UnitImages::KPS:
  case UnitImages::SETKPS:
  case UnitImages::AUTOPILOT:
  case UnitImages::COLLISION:
  case UnitImages::COCKPIT_FPS:
  case UnitImages::WARPFIELDSTRENGTH:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      switch (attribute_map.lookup((*iter).name)) {
      case XFILE:
	gaugename = (*iter).value;
	break;
      case TOPY:
	topy = XMLSupport::parse_float ((*iter).value);
	break;
      case BOTY:
	boty = XMLSupport::parse_float ((*iter).value);
	break;
      case LEFT:
	leftx = XMLSupport::parse_float ((*iter).value);
	break;
      case RIGHT:
	rightx = XMLSupport::parse_float ((*iter).value);
	break;
      case XSIZE:
	xsize = XMLSupport::parse_float ((*iter).value);
	break;
      case YSIZE:
	ysize = XMLSupport::parse_float ((*iter).value);
	break;
      case XCENT:
	xcent = XMLSupport::parse_float ((*iter).value);
	break;
      case YCENT:
	ycent = XMLSupport::parse_float ((*iter).value);
	break;
      case G_UP:
	tmpdir = Gauge::GAUGE_UP;
	break;
      case G_DOWN:
	tmpdir = Gauge::GAUGE_DOWN;
	break;
      case G_LEFT:
	tmpdir = Gauge::GAUGE_LEFT;
	break;
      case G_RIGHT:
	tmpdir = Gauge::GAUGE_RIGHT;
	break;
      }
    }
    gauges[elem] = new Gauge (gaugename.c_str(), tmpdir);
    if (xsize!=-1) {
      gauges[elem]->SetSize (xsize,ysize);
    }
    if (xcent!=FLT_MAX) {
      gauges[elem]->SetPosition (xcent,ycent);
    }
    if (leftx!=-10&&rightx!=-10&&topy!=-10&&boty!=-10) {
      gauges[elem]->SetPosition (.5*(leftx+rightx),.5*(topy+boty));
      gauges[elem]->SetSize (fabs(leftx-rightx),fabs(topy-boty));
    }
    break;
  case CROSSHAIRS:
  case PANEL: 
    Panel.push_back (NULL);
    newsprite = &Panel.back ();
    if (elem==CROSSHAIRS) {
      Panel.back() = Panel.front();
      Panel.front()=NULL;//make sure null at the beginning
    }
    goto loadsprite;
  case RADAR: newsprite = &Radar[0];goto loadsprite;
  case REARRADAR: newsprite = &Radar[1];goto loadsprite;
  case LVDU: vdu.push_back(NULL);newvdu = &vdu.back();mymodes=VDU::MANIFEST|VDU::WEAPON|VDU::DAMAGE|VDU::OBJECTIVES|VDU::SHIELD;default_mode=VDU::OBJECTIVES;
#ifdef NETCOMM_WEBCAM
		mymodes = mymodes | VDU::WEBCAM;
#endif
  	if( Network!=NULL)
		mymodes = mymodes | VDU::NETWORK;
	goto loadsprite;
  case RVDU: vdu.push_back(NULL);newvdu = &vdu.back();mymodes=VDU::TARGETMANIFEST|VDU::NAV|VDU::TARGET;default_mode=VDU::TARGET;
  goto loadsprite;
  case AVDU:vdu.push_back(NULL);newvdu = &vdu.back();mymodes=VDU::MSG;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      switch (attribute_map.lookup((*iter).name)) {
      case VDUTYPE:
        {
          mymodes=parse_vdu_type ((*iter).value.c_str());
          std::string firstmode=(*iter).value.substr(0,(*iter).value.find(" "));
          default_mode=parse_vdu_type(firstmode.c_str());
        }
        break;
      default:
	break;
      }
    }
    goto loadsprite;
  loadsprite:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      switch (attribute_map.lookup((*iter).name)) {
      case XFILE:
	if (newsprite) {
          std::string tmp=getRes((*iter).value);
          bool bil=elem==PANEL?cockpit_smooth:crosshair_smooth;
          (*newsprite) = new VSSprite (tmp.c_str(),bil?BILINEAR:NEAREST);
          if (!(*newsprite)->LoadSuccess()) {
            delete (*newsprite);
            (*newsprite) = new VSSprite ((*iter).value.c_str(),bil?BILINEAR:NEAREST);
          }
	  adjsprite = *newsprite;
	} else if (newvdu) {
          VDU * tmp=new VDU ((*iter).value.c_str(),text,mymodes,rows,cols,&StartArmor[0],&maxhull);         
	  (*newvdu) =  tmp;
	  adjsprite = *newvdu;
          if (tmp->getMode()!=default_mode)
            for (int i=0;i<32;++i) {
              tmp->SwitchMode(NULL);
              if (tmp->getMode()==default_mode)
                break;
            }
	}
	break;
      case TOPY:
	topy = XMLSupport::parse_float ((*iter).value);
	break;
      case BOTY:
	boty = XMLSupport::parse_float ((*iter).value);
	break;
      case LEFT:
	leftx = XMLSupport::parse_float ((*iter).value);
	break;
      case RIGHT:
	rightx = XMLSupport::parse_float ((*iter).value);
	break;
      case XSIZE:
	xsize = XMLSupport::parse_float ((*iter).value);
	break;
      case YSIZE:
	ysize = XMLSupport::parse_float ((*iter).value);
	break;
      case XCENT:
	xcent = XMLSupport::parse_float ((*iter).value);
	break;
      case YCENT:
	ycent = XMLSupport::parse_float ((*iter).value);
	break;
      case ROWS:
	rows =  XMLSupport::parse_int ((*iter).value);
	break;
      case COLS:
	cols = XMLSupport::parse_int ((*iter).value);
	break;
	break;

      }
    }
    if (adjsprite) {
      if (xsize!=-1) {
	adjsprite->SetSize (xsize,ysize);
      }
      if (xcent!=FLT_MAX) {
	adjsprite->SetPosition (xcent,ycent);
      }
      if (leftx!=-10&&rightx!=-10&&topy!=-10&&boty!=-10) {
	adjsprite->SetPosition (.5*(leftx+rightx),.5*(topy+boty));
	adjsprite->SetSize (fabs(leftx-rightx),fabs(topy-boty));
      }
    } else {
      if (newsprite==&Panel.back()) {
	Panel.erase (Panel.end()-1);//don't want null panels
      }
    }
    break;
  default:
	  break;
  }
  
}
void GameCockpit::endElement(const string &name) {

}


using namespace VSFileSystem;
void GameCockpit::LoadXML (const char * filename) {
  const int chunk_size = 16384;

  VSFile f;
  VSError err = Unspecified;

  if (filename[0]!='\0') {
  	VSError err = f.OpenReadOnly( filename, CockpitFile);
  }
  LoadXML( f);
}

void GameCockpit::LoadXML ( VSFileSystem::VSFile & f) {

  if(!f.Valid()) {
	 cockpit_offset=0;
	 viewport_offset=0;
	 Panel.push_back(new VSSprite ("crosshairs.spr"));
	 return;
  }
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Cockpit::beginElement, &Cockpit::endElement);
  
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
    XML_Parse (parser,buf,length,VSFileSystem::vs_feof(inFile));
#endif
  } while(!VSFileSystem::vs_feof(inFile));
  VSFileSystem::vs_close (inFile);
  */
  XML_ParserFree (parser);
}
