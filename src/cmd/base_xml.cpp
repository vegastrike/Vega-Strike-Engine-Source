#include <vector>
#include <string>
#include <expat.h>
#include <assert.h>
#include "base.h"
#include "xml_support.h"
#include "vs_path.h"
static int unitlevel;
using namespace XMLSupport;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
namespace BaseXML {
	enum Names {
		UNKNOWN=UpgradingInfo::MAXMODE,
		BASE,
		ROOM,
		SHIP,
		LINK,
		COMP,
		LAUNCH,
		TEXTURE,
		TEXT,
		INDEX,
		SPRITEFILE,
		MODES,
		X,
		Y,
		Z,
		RI,
		RJ,
		RK,
		QI,
		QJ,
		QK,
		WID,
		HEI
	};
	const int NUM_ELEMENTS=8;
	const int NUM_ATTRIBUTES=24;
	const EnumMap::Pair element_names[NUM_ELEMENTS] = {
		EnumMap::Pair ("UNKNOWN", UNKNOWN),
		EnumMap::Pair ("Base", BASE),
		EnumMap::Pair ("Room", ROOM),
		EnumMap::Pair ("Link", LINK),
		EnumMap::Pair ("Comp", COMP),
		EnumMap::Pair ("Launch", LAUNCH),
		EnumMap::Pair ("Texture", TEXTURE),
		EnumMap::Pair ("Ship", SHIP)
	};
	const EnumMap::Pair attribute_names[NUM_ATTRIBUTES] = {
		EnumMap::Pair ("UNKNOWN", UNKNOWN),
		EnumMap::Pair ("NewsMode", UpgradingInfo::NEWSMODE), 
		EnumMap::Pair ("ShipMode", UpgradingInfo::SHIPDEALERMODE), 
		EnumMap::Pair ("UpgradeMode", UpgradingInfo::UPGRADEMODE), 
		EnumMap::Pair ("DowngradeMode", UpgradingInfo::DOWNGRADEMODE),
		EnumMap::Pair ("BriefingMode", UpgradingInfo::BRIEFINGMODE), 
		EnumMap::Pair ("MissionMode", UpgradingInfo::MISSIONMODE), 
		EnumMap::Pair ("SellMode", UpgradingInfo::SELLMODE), 
		EnumMap::Pair ("BuyMode", UpgradingInfo::BUYMODE),
		EnumMap::Pair ("Text", TEXT), 
		EnumMap::Pair ("Index", INDEX), 
		EnumMap::Pair ("File", SPRITEFILE), 
		EnumMap::Pair ("Modes", MODES),
		EnumMap::Pair ("x", X),
		EnumMap::Pair ("y", Y),
		EnumMap::Pair ("z", Z),
		EnumMap::Pair ("ri", RI),
		EnumMap::Pair ("rj", RJ),
		EnumMap::Pair ("rk", RK),
		EnumMap::Pair ("qi", QI),
		EnumMap::Pair ("qj", QJ),
		EnumMap::Pair ("qk", QK),
		EnumMap::Pair ("wid", WID),
		EnumMap::Pair ("hei", HEI)
	};


	const EnumMap element_map(element_names, NUM_ELEMENTS);
	const EnumMap attribute_map(attribute_names, NUM_ATTRIBUTES);

}


using namespace BaseXML;

void Base::beginElement(void *userData, const XML_Char *names, const XML_Char **atts) {
	((Base*)userData)->beginElement(names, AttributeList(atts));
}

void Base::CallCommonLinks (std::string name, std::string value) {
	switch(attribute_map.lookup(name)) {
	case TEXT:
		rooms.back()->links.back()->text=value;
		break;
	case X:
		rooms.back()->links.back()->x=parse_float(value);
		break;
	case Y:
		rooms.back()->links.back()->y=parse_float(value);
		break;
	case WID:
		rooms.back()->links.back()->wid=parse_float(value);
		break;
	case HEI:
		rooms.back()->links.back()->hei=parse_float(value);
		break;
	}
}

void Base::beginElement(const string &name, const AttributeList attributes) {
	AttributeList::const_iterator iter;
	Names elem = (Names)element_map.lookup(name);
	int lookup;
	float x,y,z;
	Vector P,Q,R;
	switch(elem) {
	case LINK:
		if (unitlevel==2) {
			rooms.back()->links.push_back(new Room::Goto ());
			for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
				CallCommonLinks ((*iter).name,(*iter).value);
				switch(attribute_map.lookup((*iter).name)) {
				case INDEX:
					((Room::Goto*)rooms.back()->links.back())->index=parse_int((*iter).value);
					break;
				}
			}
		}
		break;
	case LAUNCH:
		if (unitlevel==2) {
			rooms.back()->links.push_back(new Room::Launch ());
			for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
				CallCommonLinks ((*iter).name,(*iter).value);
			}
		}
		break;
	case COMP:
		if (unitlevel==2) {
			rooms.back()->links.push_back(new Room::Comp ());
			for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
				CallCommonLinks ((*iter).name,(*iter).value);
				lookup=attribute_map.lookup((*iter).name);
				if (lookup<UpgradingInfo::MAXMODE) {
					((Room::Comp*)rooms.back()->links.back())->modes.push_back((UpgradingInfo::BaseMode)(lookup));
				}
			}
		}
		break;
	case UNKNOWN:
		return;
	case TEXTURE:
		if (unitlevel==2) {
			x=y=0;
			for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
				switch(attribute_map.lookup((*iter).name)) {
				case SPRITEFILE:
#ifdef BASE_MAKER
					rooms.back()->texfiles.push_back((*iter).value);
#endif
					rooms.back()->texes.push_back(new Sprite(((*iter).value).c_str()));
					break;
				case X:
					x=parse_float((*iter).value);
					break;
				case Y:
					y=parse_float((*iter).value);
					break;
				}
			}
			rooms.back()->texes.back()->SetPosition(x,y);
		}
		break;
	case SHIP:
		if (unitlevel==2) {
			for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
				switch(attribute_map.lookup((*iter).name)) {
				case X:
					x=parse_float((*iter).value);
					break;
				case Y:
					y=parse_float((*iter).value);
					break;
				case Z:
					z=parse_float((*iter).value);
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
			P=Q.Cross(R);
			P.Normalize();
			rooms.back()->ships.push_back(new Matrix(P.i,P.j,P.k,Q.i,Q.j,Q.k,R.i,R.j,R.k,QVector(x,y,z)));
		}
		break;
	case ROOM:
		if (unitlevel==1) {
			rooms.push_back(new Room);
			for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
				switch(attribute_map.lookup((*iter).name)) {
				case TEXT:
					rooms.back()->deftext=(*iter).value;
					break;
				}
			}
		}
		break;
	case BASE:
		break;
	default :
		break;
	}
	unitlevel++;
}
void Base::endElement(void *userData, const XML_Char *name) {
	Names elem = (Names)element_map.lookup(name);
	(((Base*)userData)->unitlevel)--;
	switch(elem) {
	case UNKNOWN:
		break;
	default:
		break;
	}
}

void Base::LoadXML(const char * filename) {
  unitlevel=0;
  const int chunk_size = 16384;
  cout << "Base::LoadXML " << filename << endl;
  vschdir("bases");
  FILE * inFile = fopen (filename, "r");
  vscdup();
  if(!inFile) {
	assert(0);
	return;
  }
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Base::beginElement, &Base::endElement);
 
  do {
	char buf[chunk_size];
	int length;
	length = fread (buf,1, chunk_size,inFile);
	XML_Parse(parser, buf,length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
}
