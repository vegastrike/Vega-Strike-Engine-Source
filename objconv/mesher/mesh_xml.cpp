#include <string>
#include <vector>
///Stores all the load-time vertex info in the XML struct FIXME light calculations
#include <expat.h>

using std::vector;
using std::string;
#include "xml_support.h"
#include "hashtable.h"
using namespace XMLSupport;
struct GFXVertex {
  float x,y,z;
  float i,j,k;
  float s,t;
  GFXVertex operator * (float s) {
    GFXVertex ret (*this);
    ret.x*=s;
    ret.y*=s;
    ret.z*=s;
	return ret;
  }
};
struct XML {
  enum Names {
    //elements
    UNKNOWN, 
    MATERIAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    EMISSIVE,
    MESH, 
    POINTS, 
    POINT, 
    LOCATION, 
    NORMAL, 
    POLYGONS,
    LINE,
    LOD,
    TRI, 
    QUAD,
    LODFILE,
    LINESTRIP,
    TRISTRIP,
    TRIFAN,
    QUADSTRIP,
    VERTEX,
    LOGO,
    REF,
      //attributes
    POWER,
    REFLECT,
    LIGHTINGON,
    FLATSHADE,
    TEXTURE,
    FORCETEXTURE,
    ALPHAMAP,
    SHAREVERT,
    ALPHA,
    RED,
    GREEN,
    BLUE,
    X,
    Y,
    Z,
    I,
    J,
    K,
    S,
    T,
    SCALE,
    BLENDMODE,
    TYPE,
    ROTATE,
    WEIGHT,
    SIZE,
    OFFSET,
    ANIMATEDTEXTURE,
    REVERSE
  };
    ///Saves which attributes of vertex have been set in XML file
    enum PointState {
      P_X = 0x1,
      P_Y = 0x2,
      P_Z = 0x4,
      P_I = 0x8,
      P_J = 0x10,
      P_K = 0x20
    };
    ///Saves which attributes of vertex have been set in Polygon for XML file
    enum VertexState {
      V_POINT = 0x1,
      V_S = 0x2,
      V_T = 0x4
    };
    ///Save if various logo values have been set
    enum LogoState {
      V_TYPE = 0x1,
      V_ROTATE = 0x2,
      V_SIZE=0x4,
      V_OFFSET=0x8,
      V_REF=0x10
    };
    ///To save the constructing of a logo
    struct ZeLogo {
      ///Which type the logo is (0 = faction 1 = squad >2 = internal use
      unsigned int type;
      ///how many degrees logo is rotated
      float rotate;
      ///Size of the logo
      float size;
      ///offset of polygon of logo
      float offset;
      ///the reference points that the logo is weighted against
      vector <int> refpnt;
      ///the weight of the points in weighted average of refpnts
      vector <float> refweight;
    };
    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;
    vector<Names> state_stack;
    vector<GFXVertex> vertices;
    GFXVertex vertex;
    float scale;
  };

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;

const EnumMap::Pair XML::element_names[] = {
  EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
  EnumMap::Pair("Material", XML::MATERIAL),
  EnumMap::Pair("LOD", XML::LOD),
  EnumMap::Pair("Ambient", XML::AMBIENT),
  EnumMap::Pair("Diffuse", XML::DIFFUSE),
  EnumMap::Pair("Specular", XML::SPECULAR),
  EnumMap::Pair("Emissive", XML::EMISSIVE),
  EnumMap::Pair("Mesh", XML::MESH),
  EnumMap::Pair("Points", XML::POINTS),
  EnumMap::Pair("Point", XML::POINT),
  EnumMap::Pair("Location", XML::LOCATION),
  EnumMap::Pair("Normal", XML::NORMAL),
  EnumMap::Pair("Polygons", XML::POLYGONS),
  EnumMap::Pair("Line", XML::LINE),
  EnumMap::Pair("Tri", XML::TRI),
  EnumMap::Pair("Quad", XML::QUAD),
  EnumMap::Pair("Linestrip",XML::LINESTRIP),
  EnumMap::Pair("Tristrip", XML::TRISTRIP),
  EnumMap::Pair("Trifan", XML::TRIFAN),
  EnumMap::Pair("Quadstrip", XML::QUADSTRIP),
  EnumMap::Pair("Vertex", XML::VERTEX),
  EnumMap::Pair("Logo", XML::LOGO),
  EnumMap::Pair("Ref",XML::REF)
};

const EnumMap::Pair XML::attribute_names[] = {
  EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
  EnumMap::Pair("Scale",XML::SCALE),
  EnumMap::Pair("Blend",XML::BLENDMODE),
  EnumMap::Pair("texture", XML::TEXTURE),
  EnumMap::Pair("alphamap", XML::ALPHAMAP),
  EnumMap::Pair("sharevertex", XML::SHAREVERT),
  EnumMap::Pair("red", XML::RED),
  EnumMap::Pair("green", XML::GREEN),
  EnumMap::Pair("blue", XML::BLUE),
  EnumMap::Pair("alpha", XML::ALPHA),
  EnumMap::Pair("power", XML::POWER),
  EnumMap::Pair("reflect", XML::REFLECT),
  EnumMap::Pair("x", XML::X),
  EnumMap::Pair("y", XML::Y),
  EnumMap::Pair("z", XML::Z),
  EnumMap::Pair("i", XML::I),
  EnumMap::Pair("j", XML::J),
  EnumMap::Pair("k", XML::K),
  EnumMap::Pair("Shade", XML::FLATSHADE),
  EnumMap::Pair("point", XML::POINT),
  EnumMap::Pair("s", XML::S),
  EnumMap::Pair("t", XML::T),
  //Logo stuffs
  EnumMap::Pair("Type",XML::TYPE),
  EnumMap::Pair("Rotate", XML::ROTATE),
  EnumMap::Pair("Weight", XML::WEIGHT),
  EnumMap::Pair("Size", XML::SIZE),
  EnumMap::Pair("Offset",XML::OFFSET),
  EnumMap::Pair("meshfile",XML::LODFILE),
  EnumMap::Pair ("Animation",XML::ANIMATEDTEXTURE),
  EnumMap::Pair ("Reverse",XML::REVERSE),
  EnumMap::Pair ("LightingOn",XML::LIGHTINGON),
  EnumMap::Pair ("ForceTexture",XML::FORCETEXTURE)
};



const EnumMap XML::element_map(XML::element_names, 23);
const EnumMap XML::attribute_map(XML::attribute_names, 32);







void beginElement(const string &name, const AttributeList &attributes, XML * xml) {
  
  AttributeList::const_iterator iter;
  XML::Names elem = (XML::Names)XML::element_map.lookup(name);
  XML::Names top;
  if(xml->state_stack.size()>0) top = *xml->state_stack.rbegin();
  xml->state_stack.push_back(elem);
  switch(elem) {
	  case XML::MATERIAL:
  case XML::DIFFUSE:
	  break;
  case XML::EMISSIVE:
	  break;
  case XML::SPECULAR:
	  break;
  case XML::AMBIENT:
	  break;
  case XML::UNKNOWN:
   fprintf (stderr, "Unknown element start tag '%s' detected\n",name.c_str());
    break;
  case XML::MESH:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::ANIMATEDTEXTURE:
	break;
      case XML::REVERSE:
	break;
      case XML::FORCETEXTURE:
	break;
      case XML::TEXTURE:
	break;
      case XML::ALPHAMAP:
	break;
      case XML::SCALE:
	break;
      case XML::SHAREVERT:
	break;
      case XML::BLENDMODE:
	break;
      }
    }break;
  case XML::POINTS:
    break;
  case XML::POINT:
 //   assert(top==XML::POINTS);
    break;
  case XML::LOCATION:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::X:
	xml->vertex.x = XMLSupport::parse_float((*iter).value);
	break;
      case XML::Y:
	xml->vertex.y = XMLSupport::parse_float((*iter).value);
	break;
     case XML::Z:
	xml->vertex.z = XMLSupport::parse_float((*iter).value);
	break;
      case XML::S:
	xml->vertex.s = XMLSupport::parse_float ((*iter).value);
	break;
      case XML::T:
	xml->vertex.t = XMLSupport::parse_float ((*iter).value);
	break;
      }
    }
    break;
  case XML::NORMAL:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
	fprintf (stderr, "Unknown attribute '%s' encountered in Normal tag\n",(*iter).name.c_str());
	break;
      case XML::I:
	xml->vertex.i = XMLSupport::parse_float((*iter).value);
	break;
      case XML::J:
	xml->vertex.j = XMLSupport::parse_float((*iter).value);
	break;
      case XML::K:
	xml->vertex.k = XMLSupport::parse_float((*iter).value);
	break;
      }
    }
    break;
  case XML::POLYGONS:
    break;
  case XML::LINE:
    break;
  case XML::TRI:
    break;
  case XML::LINESTRIP:
    break;

  case XML::TRISTRIP:
    break;

  case XML::TRIFAN:
    break;

  case XML::QUADSTRIP:
    break;
   
  case XML::QUAD:
    break;
  case XML::LOD: 
    break;
  case XML::VERTEX:
    break;
  case XML::LOGO: 
    break;
  case XML::REF:
    break;
      }
    
}

void endElement(const string &name, XML * xml) {
  xml->state_stack.pop_back();
  XML::Names elem = (XML::Names)XML::element_map.lookup(name);

  unsigned int i;
  switch(elem) {
  case XML::UNKNOWN:
    fprintf (stderr,"Unknown element end tag '%s' detected\n",name.c_str());
    break;
  case XML::POINT:
    xml->vertices.push_back (xml->vertex*xml->scale);
    break;
  case XML::POINTS:
    break;
  case XML::LINE:
    break;
  case XML::TRI:
    break;
  case XML::QUAD:
    break;
  case XML::LINESTRIP:
    break;
  case XML::TRISTRIP:
    break;
  case XML::TRIFAN:
    break;
  case XML::QUADSTRIP://have to fix up nrmlquadstrip so that it 'looks' like a quad list for smooth shading
    break;
  case XML::POLYGONS:
    break;
  case XML::REF:
    break;
  case XML::LOGO:
    break;
  case XML::MATERIAL:
	  break;
  case XML::DIFFUSE:
	  break;
  case XML::EMISSIVE:
	  break;
  case XML::SPECULAR:
	  break;
  case XML::AMBIENT:
	  break;
  case XML::MESH:
    break;
  default:
    ;
  }
}

void beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  beginElement(name, AttributeList(atts),(XML *)userData);
}



void endElement(void *userData, const XML_Char *name) {
  endElement(name, (XML *) userData);
}


XML LoadXML(const char *filename, float unitscale) {
  const int chunk_size = 16384;
  FILE* inFile = fopen (filename, "r");
  if(!inFile) {
    fprintf (stderr,"Cannot Open Mesh File %s\n",filename);
    exit(0);
    return XML();
  }

  XML xml;
  xml.scale = unitscale;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, &xml);
  XML_SetElementHandler(parser, &beginElement, &endElement);
  
  do {
    char buf[chunk_size];
    int length;
    
    length = fread(buf,1, chunk_size,inFile);
    XML_Parse (parser,buf,length,feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
  // Now, copy everything into the mesh data structures
  return xml;
}


int main () {
  XML blah (LoadXML("test.xmesh",1));
  printf ("size %d",blah.vertices.size());
  return 0;
}
