#include "quadtree.h"
#include "xml_support.h"
#include "gfxlib.h"
#include "aux_texture.h"
#include <assert.h>
#include "png_texture.h"

extern enum BLENDFUNC parse_alpha (char * tmp );


struct TerrainData {
	int scale;
	float OriginX;
	float OriginY;
	std::string file;
};
struct TerraXML {
	float detail;
  	std::vector <GFXMaterial> mat;
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
		STATICDETAIL,
		REFLECT,
		COLOR,
		SCALE,
		ORIGINX,
		SCALES,
		ORIGINY,
		SCALET,
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
		EnumMap::Pair ("StaticDetail", STATICDETAIL),
		EnumMap::Pair ("Level", LEVEL),
		EnumMap::Pair ("Blend", BLEND),
		EnumMap::Pair ("File", FFILE),
		EnumMap::Pair ("Reflect", REFLECT),
		EnumMap::Pair ("Color", COLOR),
		EnumMap::Pair ("Scale", SCALE),
		EnumMap::Pair ("ScaleS", SCALES),
		EnumMap::Pair ("ScaleT", SCALET),
		EnumMap::Pair ("OriginX", ORIGINX),
		EnumMap::Pair ("OriginY", ORIGINY),
		EnumMap::Pair ("red", RED),
		EnumMap::Pair ("green", GREEN),
		EnumMap::Pair ("blue", BLUE),
		EnumMap::Pair ("alpha", ALPHA),
		EnumMap::Pair ("power", POWER)
	};
	const EnumMap element_map(element_names,9);
	const EnumMap attribute_map(attribute_names,18);
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
				detail=parse_float((*iter).value);
				break;
			case STATICDETAIL:
				xml->detail=parse_float((*iter).value);
				break;
			case LEVEL:
				RootCornerData.Level=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TEXTURE:
		textures.push_back(TerrainTexture());
		xml->mat.push_back(GFXMaterial());
		GFXGetMaterial (0,xml->mat.back());
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case FFILE:
				textures.back().tex.filename=strdup ((*iter).value.c_str());
				break;
			case BLEND:
				sscanf (((*iter).value).c_str(),"%s %s",csrc,cdst);
				textures.back().blendSrc = parse_alpha (csrc);
				textures.back().blendDst = parse_alpha (cdst);
				break;
			case COLOR:
				textures.back().color=parse_int (((*iter).value));
				break;
			case SCALES:
			  textures.back().scales = parse_float ((*iter).value);
			  break;
			case SCALET:
			  textures.back().scalet = parse_float ((*iter).value);
			  break;
			}
		}
		break;
	case MATERIAL:
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case REFLECT:
				textures.back().reflect=parse_bool((*iter).value);
				break;
			case POWER:
				xml->mat.back().power=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINDIFFUSE:

		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->mat.back().dr=parse_float((*iter).value);
				break;
			case BLUE:
				xml->mat.back().db=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->mat.back().da=parse_float((*iter).value);
				break;
			case GREEN:
				xml->mat.back().dg=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINEMISSIVE:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->mat.back().er=parse_float((*iter).value);
				break;
			case BLUE:
				xml->mat.back().eb=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->mat.back().ea=parse_float((*iter).value);
				break;
			case GREEN:
				xml->mat.back().eg=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINSPECULAR:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->mat.back().sr=parse_float((*iter).value);
				break;
			case BLUE:
				xml->mat.back().sb=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->mat.back().sa=parse_float((*iter).value);
				break;
			case GREEN:
				xml->mat.back().sg=parse_float((*iter).value);
				break;
			}
		}
		break;
	case TERRAINAMBIENT:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case RED:
				xml->mat.back().ar=parse_float((*iter).value);
				break;
			case BLUE:
				xml->mat.back().ab=parse_float((*iter).value);
				break;
			case ALPHA:
				xml->mat.back().aa=parse_float((*iter).value);
				break;
			case GREEN:
				xml->mat.back().ag=parse_float((*iter).value);
				break;
			}
		}
		break;
	case DATA:
		xml->data.push_back(TerrainData());
		for (iter = attributes.begin();iter!=attributes.end();iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case SCALE:
				xml->data.back().scale=parse_int((*iter).value);
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
	case UNKNOWN:
	default:
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
  unsigned int i;
  for (i=0;i<textures.size();i++) {
    if (textures[i].tex.filename) {
      Texture * tex = new Texture (textures[i].tex.filename);
      free (textures[i].tex.filename);
      textures[i].tex.t = tex;
      GFXSetMaterial (textures[i].material,xml->mat[i]);
    } else {
      textures[i].tex.t = NULL;
    }

  } 
  root = new quadsquare (&RootCornerData);
  for (i=0;i<xml->data.size();i++) {
    HeightMapInfo hm;
    hm.XOrigin =xml->data[i].OriginX;
    hm.ZOrigin=xml->data[i].OriginY;
    hm.Scale = xml->data[i].scale;
    int format;int bpp; unsigned char * palette;
    hm.Data = (short *) readImage (xml->data[i].file.c_str(),bpp, format, hm.XSize,hm.ZSize, palette, &heightmapTransform);
	  //LoadData();

    if (hm.Data) {
      hm.RowWidth = hm.XSize;
      root->AddHeightMap (RootCornerData,hm);
      free (hm.Data);
    }
  }
  
 // root->StaticCullData (RootCornerData,xml->detail);
  delete xml;
}
