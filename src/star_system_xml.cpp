#include <expat.h>
#include "xml_support.h"
#include "star_system.h"
#include "gfx_background.h"
#include "gfx_env_map_gent.h"
#include "gfx_aux_texture.h"
#include "planet.h"
void StarSystem::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((StarSystem*)userData)->beginElement(name, AttributeList(atts));
}

void StarSystem::endElement(void *userData, const XML_Char *name) {
  ((StarSystem*)userData)->endElement(name);
}


namespace StarXML {
    enum Names {
      UNKNOWN,
      XFILE,
      X,
      Y,
      Z,
      RI,
      RJ,
      RK,
      SI,
      SJ,
      SK,
      NAME,
      RADIUS,
      GRAVITY,
      VELOCITY,
      PPOSITION,
      SYSTEM,
      PLANET,
      UNIT,
      BACKGROUND
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Planet", PLANET),
    EnumMap::Pair ("System", SYSTEM),
	EnumMap::Pair ("Unit", UNIT)
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("background", BACKGROUND), 
    EnumMap::Pair ("file", XFILE), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z), 
    EnumMap::Pair ("ri", RI), 
    EnumMap::Pair ("rj", RJ), 
    EnumMap::Pair ("rk", RK), 
    EnumMap::Pair ("si", SI),     
    EnumMap::Pair ("sj", SJ),     
    EnumMap::Pair ("sk", SK),
    EnumMap::Pair ("name", NAME),
    EnumMap::Pair ("radius", RADIUS),
    EnumMap::Pair ("gravity", GRAVITY),
    EnumMap::Pair ("velocity", VELOCITY),
    EnumMap::Pair ("position", PPOSITION)

};

  const EnumMap element_map(element_names, 4);
  const EnumMap attribute_map(attribute_names, 17);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace StarXML;

void StarSystem::beginElement(const string &name, const AttributeList &attributes) {
  xml->cursun.i=0;
  xml->cursun.j=0;
  xml->cursun.k=0;
  char * filename =NULL;
  float gravity=0;
  float velocity=0;
  float position=0;
  Vector S(0,0,0), pos(0,0,0), R(0,0,0);
  Names elem = (Names)element_map.lookup(name);
  float radius=1;
  AttributeList::const_iterator iter;
  switch(elem) {
  case UNKNOWN:
	xml->unitlevel++;

//    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    return;

  case SYSTEM:
	assert (xml->unitlevel==0);
	xml->unitlevel++;
    pos = Vector (0,0,0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
  case BACKGROUND:
	xml->backgroundname=(*iter).value;
	break;
   case NAME:
	this->name = new char [strlen((*iter).value.c_str())+1];
	strcpy(this->name,(*iter).value.c_str());
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
      }

    }

    break;

  case PLANET:
	assert (xml->unitlevel>0);
	xml->unitlevel++;
    S = Vector (0,1,0);
    R = Vector (0,0,1);
	filename = new char [1];
	filename[0]='\0';
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	delete []filename;
    filename = new char [strlen((*iter).value.c_str())+1];
	strcpy(filename,(*iter).value.c_str());
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
      case SI:
	S.i=parse_float((*iter).value);
	break;
      case SJ:
	S.j=parse_float((*iter).value);
	break;
      case SK:
	S.k=parse_float((*iter).value);
	break;
      case X:
 	assert(xml->unitlevel==2);
 	xml->cursun.i=parse_float((*iter).value);
 	break;
      case Y:
 	assert(xml->unitlevel==2);
 	xml->cursun.j=parse_float((*iter).value);
 	break;
      case Z:
 	assert(xml->unitlevel==2);
 	xml->cursun.k=parse_float((*iter).value);
 	break;
      case RADIUS:
	radius=parse_float((*iter).value);
	break;
      case PPOSITION:
	position=parse_float((*iter).value);
	break;
      case VELOCITY:
	velocity=parse_float((*iter).value);
	break;
      case GRAVITY:
	gravity=parse_float((*iter).value);
	break;
      }

    }  
    if (xml->unitlevel>2) {
      assert(xml->moons.size()!=0);
      xml->moons[xml->moons.size()-1]->beginElement(R,S,velocity,position,gravity,radius,filename,xml->unitlevel-1);
    } else {
      xml->moons.push_back(new Planet(R,S,velocity,position,gravity,radius,filename));
      xml->moons[xml->moons.size()-1]->SetOrigin(xml->cursun);
	}
    delete []filename;
    break;
  case UNIT:
    assert (xml->unitlevel>0);
    xml->unitlevel++;
    S = Vector (0,1,0);
    R = Vector (0,0,1);
    filename = new char [1];
    filename[0]='\0';
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	delete []filename;
	filename = new char [strlen((*iter).value.c_str())+1];
	strcpy(filename,(*iter).value.c_str());
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
      case SI:
	S.i=parse_float((*iter).value);
	break;
      case SJ:
	S.j=parse_float((*iter).value);
	break;
      case SK:
	S.k=parse_float((*iter).value);
	break;
      case X:
 	assert(xml->unitlevel==2);
 	xml->cursun.i=parse_float((*iter).value);
 	break;
      case Y:
 	assert(xml->unitlevel==2);
 	xml->cursun.j=parse_float((*iter).value);
 	break;
      case Z:
 	assert(xml->unitlevel==2);
 	xml->cursun.k=parse_float((*iter).value);
 	break;
      case PPOSITION:
	position=parse_float((*iter).value);
	break;
      case VELOCITY:
	velocity=parse_float((*iter).value);
	break;
      }

    }  
    if (xml->unitlevel>2) {
      assert(xml->moons.size()!=0);
      xml->moons[xml->moons.size()-1]->Planet::beginElement(R,S,velocity,position,gravity,radius,filename,xml->unitlevel-1,true);
    } else {
      xml->moons.push_back((Planet *)new Unit(filename,true));
      xml->moons[xml->moons.size()-1]->SetAI(new PlanetaryOrbit(xml->moons[xml->moons.size()-1],velocity,position,R,S));
      xml->moons[xml->moons.size()-1]->SetOrigin(xml->cursun);      
    }
    delete []filename;
    break;
	
  default:
	
    break;
  }
}

void StarSystem::endElement(const string &name) {
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
	if (xml->unitlevel==0) {
		numprimaries = xml->moons.size();
		this->primaries=new Unit * [xml->moons.size()];
		for(int i=0;i<xml->moons.size();i++) {
			
			primaries[i]=xml->moons[i];
		}
	}
}


void StarSystem::LoadXML(const char *filename) {
//  shield.number=0;
  const int chunk_size = 16384;
 // rrestricted=yrestricted=prestricted=false;
  FILE * inFile = fopen (filename, "r+b");
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new StarXML;
  xml->backgroundname = string("cube");
  xml->unitlevel=0;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &StarSystem::beginElement, &StarSystem::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
#ifdef NV_CUBE_MAP
	LightMap[0]=new Texture ((xml->backgroundname+"_right_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_X);
	LightMap[1]=new Texture ((xml->backgroundname+"_left_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_X);
	LightMap[2]=new Texture ((xml->backgroundname+"_up_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_Y);
	LightMap[3]=new Texture ((xml->backgroundname+"_down_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_Y);
	LightMap[4]=new Texture ((xml->backgroundname+"_front_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_Z);
	LightMap[5]=new Texture ((xml->backgroundname+"_back_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_Z);
#else
	FILE * tempo = fopen ((xml->backgroundname+"_light.bmp").c_str(),"r+b");
	if (!tempo) {
		EnvironmentMapGeneratorMain (xml->backgroundname.c_str(),(xml->backgroundname+"_light").c_str());
	}else {
		fclose (tempo);
	}
	LightMap[0] = new Texture((xml->backgroundname+"_light.bmp").c_str(), 1);
#endif
  bg = new Background(xml->backgroundname.c_str());

  delete xml;
}
