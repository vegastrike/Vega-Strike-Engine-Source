#include "cockpit.h"
#include "xml_support.h"
#include "gauge.h"
#include <float.h>
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
namespace CockpitXML {
    enum Names {
      UNKNOWN,
      COCKPIT,
      CROSSHAIRS,
      RADAR,
      LVDU,
      RVDU,
      PANEL,
      XFILE,
      XCENT,
      YCENT,
      XSIZE,
      YSIZE,
      COCKPITOFFSET,
      VIEWOFFSET,
      FRONT,
      LEFT,
      RIGHT,
      BACK,
      KARMORF,
      KARMORR,
      KARMORL,
      KARMORB,
      KFUEL, 
      KSHIELDF,
      KSHIELDR,
      KSHIELDL,
      KSHIELDB, 
      KENERGY,
      KEJECT,
      KLOCK,
      KHULL,
      G_UP,
      G_DOWN,
      G_LEFT,
      G_RIGHT
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Cockpit", COCKPIT),
    EnumMap::Pair ("Radar", RADAR),
    EnumMap::Pair ("LeftVDU", LVDU),
    EnumMap::Pair ("RightVDU", RVDU),
    EnumMap::Pair ("Panel", PANEL),
    EnumMap::Pair ("Crosshairs", CROSSHAIRS),
    EnumMap::Pair ("ArmorF", KARMORF),
    EnumMap::Pair ("ArmorR", KARMORR),
    EnumMap::Pair ("ArmorL", KARMORL),
    EnumMap::Pair ("ArmorB", KARMORB),
    EnumMap::Pair ("ShieldF", KSHIELDF),
    EnumMap::Pair ("ShieldR", KSHIELDR),
    EnumMap::Pair ("ShieldL", KSHIELDL),
    EnumMap::Pair ("ShieldB", KSHIELDB),
    EnumMap::Pair ("Fuel", KFUEL),
    EnumMap::Pair ("Energy", KENERGY),
    EnumMap::Pair ("Eject", KEJECT),
    EnumMap::Pair ("Lock", KLOCK),
    EnumMap::Pair ("Hull", KHULL)
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("file", XFILE),
    EnumMap::Pair ("front", FRONT),
    EnumMap::Pair ("left", LEFT),
    EnumMap::Pair ("right", RIGHT),
    EnumMap::Pair ("back", BACK),
    EnumMap::Pair ("xcent", XCENT),
    EnumMap::Pair ("ycent", YCENT),
    EnumMap::Pair ("width", XSIZE),
    EnumMap::Pair ("height", YSIZE),
    EnumMap::Pair ("ViewOffset", VIEWOFFSET),
    EnumMap::Pair ("CockpitOffset", COCKPITOFFSET),
    EnumMap::Pair ("GaugeUp",G_UP),
    EnumMap::Pair ("GaugeDown",G_DOWN),
    EnumMap::Pair ("GaugeLeft",G_LEFT),
    EnumMap::Pair ("GaugeRight",G_RIGHT),

  };

  const EnumMap element_map(element_names, 20);
  const EnumMap attribute_map(attribute_names, 16);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace CockpitXML;

void Cockpit::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((Cockpit*)userData)->beginElement(name, AttributeList(atts));
}

void Cockpit::endElement(void *userData, const XML_Char *name) {
  ((Cockpit*)userData)->endElement(name);
}

void Cockpit::beginElement(const string &name, const AttributeList &attributes) {
  AttributeList::const_iterator iter;
  Gauge::DIRECTION tmpdir=Gauge::GAUGE_UP;
  Sprite ** newsprite;
  std::string gaugename ("shieldstat.spr");
  Names elem = (Names)element_map.lookup(name);
  Names attr;
  float xsize=-1,ysize=-1,xcent=FLT_MAX,ycent=FLT_MAX;
  switch (elem) {
  case COCKPIT:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      attr = (Names)attribute_map.lookup((*iter).name);
      switch (attr) {
      case VIEWOFFSET:
	viewport_offset = parse_float ((*iter).value);
	break;
      case COCKPITOFFSET:
	cockpit_offset = parse_float ((*iter).value);
	break;
      case XFILE:
	Pit[0]= new Sprite ((*iter).value.c_str());
	break;
      case FRONT:
      case BACK:
      case LEFT:
      case RIGHT:
	Pit[attr-FRONT] = new Sprite ((*iter).value.c_str());
	break;
      } 
    }
    break;
  case KARMORF:
  case KARMORR:
  case KARMORL:
  case KARMORB:
  case KFUEL:
  case KSHIELDF:
  case KSHIELDL:
  case KSHIELDR:
  case KSHIELDB:
  case KENERGY:
  case KEJECT:
  case KLOCK:
  case KHULL:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      switch (attribute_map.lookup((*iter).name)) {
      case XFILE:
	gaugename = (*iter).value;
	break;
      case XSIZE:
	xsize = parse_float ((*iter).value);
	break;
      case YSIZE:
	ysize = parse_float ((*iter).value);
	break;
      case XCENT:
	xcent = parse_float ((*iter).value);
	break;
      case YCENT:
	ycent = parse_float ((*iter).value);
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
    gauges[elem-KARMORF] = new Gauge (gaugename.c_str(), tmpdir);
    if (xsize!=-1) {
      gauges[elem-KARMORF]->SetSize (xsize,ysize);
    }
    if (xcent!=FLT_MAX) {
      gauges[elem-KARMORF]->SetPosition (xcent,ycent);
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
  case RADAR: newsprite = &Radar;goto loadsprite;
  case LVDU: newsprite = &VDU[0]; goto loadsprite;
  case RVDU: newsprite = &VDU[1]; goto loadsprite;
  loadsprite:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) { 
      switch (attribute_map.lookup((*iter).name)) {
      case XFILE:
	(*newsprite) = new Sprite ((*iter).value.c_str());
	break;
      case XSIZE:
	xsize = parse_float ((*iter).value);
	break;
      case YSIZE:
	ysize = parse_float ((*iter).value);
	break;
      case XCENT:
	xcent = parse_float ((*iter).value);
	break;
      case YCENT:
	ycent = parse_float ((*iter).value);
	break;
      }
    }
    if (*newsprite) {
      if (xsize!=-1) {
	(*newsprite)->SetSize (xsize,ysize);
      }
      if (xcent!=FLT_MAX) {
	(*newsprite)->SetPosition (xcent,ycent);
      }
    } else {
      if (newsprite==&Panel.back()) {
	Panel.erase (Panel.end()-1);//don't want null panels
      }
    }
    break;
  }
  
}
void Cockpit::endElement(const string &name) {

}



void Cockpit::LoadXML (const char * filename) {
  const int chunk_size = 16384;
  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
    cockpit_offset=0;
    viewport_offset=0;
    Panel.push_back(new Sprite ("crosshairs.spr"));
    return;
  }
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Cockpit::beginElement, &Cockpit::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
}
