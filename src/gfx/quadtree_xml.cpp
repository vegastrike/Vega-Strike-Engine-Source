#include "quadtree.h"
#include "xml_support.h"
#include "gfxlib.h"


extern enum BLENDFUNC parse_alpha (char * tmp );


struct TerrainData {
	float scale;
	float OriginX;
	float OriginY;
	std::string file;
};
struct TerraXML {
	float detail;
	float level;
	std::vector <TerrainTexture> tex;
	std::vector <TerrainData> data;
};

void QuadTree::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
	((QuadTree*)userData)->beginElement(name, XMLSupport::AttributeList(atts));
}

void QuadTree::endElement(void *userData, const XML_Char *name) {
	((QuadTree*)userData)->endElement(name);
}

using	XMLSupport::EnumMap;

namespace TerrainXML {
	enum Names {
		UNKNOWN,
		TEXTURE,
		TERRAIN,
		MATERIAL,
		DATA,
		LEVEL,
		BLEND,
		FFILE,
		DETAIL,
		REFLECT,
		COLOR,
		SCALE,
		ORIGINX,
		ORIGINY,
		TERRAINAMBIENT,
		TERRAINDIFFUSE,
		TERRAINSPECULAR,
		TERRAINEMISSIVE,
		RED,
		GREEN,
		BLUE,
		ALPHA,
		POWER
	};
	const EnumMap::Pair element_names[] = {
		EnumMap::Pair ("UNKNOWN", UNKNOWN),
		EnumMap::Pair ("Terrain", TERRAIN),
		EnumMap::Pair ("Texture", TEXTURE),
		EnumMap::Pair ("Material", MATERIAL),
		EnumMap::Pair ("Data", DATA),
		EnumMap::Pair ("Ambient", TERRAINAMBIENT),
		EnumMap::Pair ("Diffuse", TERRAINDIFFUSE),
		EnumMap::Pair ("Specular",TERRAINSPECULAR),
		EnumMap::Pair ("Emissive",TERRAINEMISSIVE)
	};
	const EnumMap::Pair attribute_names[] = {
		EnumMap::Pair ("UNKNOWN", UNKNOWN),
		EnumMap::Pair ("Detail", DETAIL),
		EnumMap::Pair ("Level", LEVEL),
		EnumMap::Pair ("Blend", BLEND),
		EnumMap::Pair ("File", FFILE),
		EnumMap::Pair ("Reflect", REFLECT),
		EnumMap::Pair ("Color", COLOR),
		EnumMap::Pair ("Scale", SCALE),
		EnumMap::Pair ("OriginX", ORIGINX),
		EnumMap::Pair ("OriginY", ORIGINY),
		EnumMap::Pair ("red", RED),
		EnumMap::Pair ("green", GREEN),
		EnumMap::Pair ("blue", BLUE),
		EnumMap::Pair ("alpha", ALPHA),
		EnumMap::Pair ("power", POWER),
	};
	const EnumMap element_map(element_names,9);
	const EnumMap attribute_map(attribute_names,15);
}
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;

using XMLSupport::parse_float;

using namespace TerrainXML;

void QuadTree::beginElement(const string &name, const AttributeList &attributes) {
	Names elem = (Names)element_map.lookup(name);
	AttributeList::const_iterator iter;
	char csrc[128];
		char cdst[128];
	switch(elem) {
	case TERRAIN:
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case DETAIL:
				xml->detail=parse_float((*iter).value);
				break;
			case LEVEL:
				xml->level=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TEXTURE:
		xml->tex.push_back(TerrainTexture());
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case FFILE:
				xml->tex.back().file=strdup ((*iter).value.c_str());
				break;
			case BLEND:
				sscanf (((*iter).value).c_str(),"%s %s",csrc,cdst);
				xml->tex.back().blendSrc = parse_alpha (csrc);
				xml->tex.back().blendDst = parse_alpha (cdst);
				break;
			case COLOR:
				xml->tex.back().color=parse_int (((*iter).value));
				break;
			}
		}
		break;
	case MATERIAL:
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case REFLECT:
				xml->tex.back().reflect=parse_bool((*iter).value);
				break;
			case POWER:
				xml->tex.back().material.power=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINDIFFUSE:

		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->tex.back().material.dr=parse_float((*iter).value);
				break;
			case BLUE:
				xml->tex.back().material.db=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->tex.back().material.da=parse_float((*iter).value);
				break;
			case GREEN:
				xml->tex.back().material.dg=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINEMISSIVE:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->tex.back().material.er=parse_float((*iter).value);
				break;
			case BLUE:
				xml->tex.back().material.eb=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->tex.back().material.ea=parse_float((*iter).value);
				break;
			case GREEN:
				xml->tex.back().material.eg=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINSPECULAR:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->tex.back().material.sr=parse_float((*iter).value);
				break;
			case BLUE:
				xml->tex.back().material.sb=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->tex.back().material.sa=parse_float((*iter).value);
				break;
			case GREEN:
				xml->tex.back().material.sg=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINAMBIENT:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->tex.back().material.ar=parse_float((*iter).value);
				break;
			case BLUE:
				xml->tex.back().material.ab=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->tex.back().material.aa=parse_float((*iter).value);
				break;
			case GREEN:
				xml->tex.back().material.ag=parse_float((*iter).value);
				break;
			}
		}
		break;
	case DATA:
		xml->data.push_back(TerrainData());
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case SCALE:
				xml->data.back().scale=parse_float((*iter).value);
				break;
			case ORIGINX:
				xml->data.back().OriginX=parse_float((*iter).value);
				break;
			case ORIGINY:
				xml->data.back().OriginY=parse_float((*iter).value);
				break;
			case FFILE:
				xml->data.back().file=(*iter).value;
				break;
			}
		}
		break;
	}
}

void QuadTree::endElement(const string &name) {
}

void QuadTree::LoadXML (const char *filename) {
  const int chunk_size = 16384;
  std::vector <unsigned int> ind;  
  FILE* inFile = fopen (filename, "r");
  if(!inFile) {
    assert(0);
    return;
  }
  xml = new TerraXML;
  xml->level = 15;
  xml->detail=20;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &QuadTree::beginElement, &QuadTree::endElement);
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread(buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);



  delete xml;
}
