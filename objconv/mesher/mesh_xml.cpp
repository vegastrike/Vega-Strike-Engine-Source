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

struct GFXMaterial
{  /// ambient rgba, if you don't like these things, ask me to rename them
	float ar;float ag;float ab;float aa;
  /// diffuse rgba
	float dr;float dg;float db;float da;
  /// specular rgba
	float sr;float sg;float sb;float sa;
  /// emissive rgba
	float er;float eg;float eb;float ea;
  /// specular power
	float power; 
};

struct line{
	bool flatshade;
	int indexref[2];
	float s[2];
	float t[2];

};

struct triangle{
	bool flatshade;
	int indexref[3];
	float s[3];
	float t[3];
	
};

struct quad{
	bool flatshade;
	int indexref[4];
	float s[4];
	float t[4];
};

enum polytype{ //FIXME
	LINE,
	TRIANGLE,
	QUAD
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
	CULLFACE,
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
	USENORMALS,
    REVERSE,
	POLYGONOFFSET,
	DETAILTEXTURE,
	DETAILPLANE,
	FRAMESPERSECOND,
	STARTFRAME
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
	struct ZeTexture {
        string decal_name;
        string alpha_name;
        string animated_name;
    };
	struct vec3f{
		float x;
		float y;
		float z;
	};

    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;
    vector<Names> state_stack;
    vector<GFXVertex> vertices;

	vector<line> lines;
    vector<triangle> tris;
    vector<quad> quads;
	//FIXME - strips not currently supported
    vector <vector<GFXVertex> > linestrips;
    vector <vector<GFXVertex> > tristrips;
    vector <vector<GFXVertex> > trifans;
    vector <vector<GFXVertex> > quadstrips;
	//END FIXME

	vector <ZeLogo> logos;
	bool sharevert;
    bool usenormals;
    bool reverse;
    bool force_texture;
	bool recalc_norm;
	bool shouldreflect;
    
	
	vec3f detailplane;
	bool reflect;
	bool lighting;
	bool cullface;
	float polygon_offset;
	int blend_src;
	int blend_dst;
	int point_state;

    GFXVertex vertex;

	int curpolytype;
	int curpolyindex;
	
	line linetemp;
	triangle triangletemp;
	quad quadtemp;
	GFXMaterial material;
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
  EnumMap::Pair("Ref",XML::REF),
  EnumMap::Pair("DetailPlane",XML::DETAILPLANE)
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
  EnumMap::Pair ("CullFace",XML::CULLFACE),
  EnumMap::Pair ("ForceTexture",XML::FORCETEXTURE),
  EnumMap::Pair ("UseNormals",XML::USENORMALS),
  EnumMap::Pair ("PolygonOffset",XML::POLYGONOFFSET),
  EnumMap::Pair ("DetailTexture",XML::DETAILTEXTURE),
  EnumMap::Pair ("FramesPerSecond",XML::FRAMESPERSECOND)
};


const EnumMap XML::element_map(XML::element_names, 24);
const EnumMap XML::attribute_map(XML::attribute_names, 37);



enum BLENDFUNC{
    ZERO            = 1,
	ONE             = 2, 
    SRCCOLOR        = 3,
	INVSRCCOLOR     = 4, 
    SRCALPHA        = 5,
	INVSRCALPHA     = 6, 
    DESTALPHA       = 7,
	INVDESTALPHA    = 8, 
    DESTCOLOR       = 9,
	INVDESTCOLOR    = 10, 
    SRCALPHASAT     = 11,
    CONSTALPHA    = 12, 
    INVCONSTALPHA = 13,
    CONSTCOLOR = 14,
    INVCONSTCOLOR = 15
};



enum BLENDFUNC parse_alpha (const char * tmp ) {
  if (strcmp (tmp,"ZERO")==0) {
    return ZERO;
  }
  if (strcmp (tmp,"ONE")==0) {
    return ONE;
  }
  if (strcmp (tmp,"SRCCOLOR")==0) {
    return SRCCOLOR;
  }
  if (strcmp (tmp,"INVSRCCOLOR")==0) {
    return INVSRCCOLOR;
  }
  if (strcmp (tmp,"SRCALPHA")==0) {
    return SRCALPHA;
  }
  if (strcmp (tmp,"INVSRCALPHA")==0) {
    return INVSRCALPHA;
  }
  if (strcmp (tmp,"DESTALPHA")==0) {
    return DESTALPHA;
  }
  if (strcmp (tmp,"INVDESTALPHA")==0) {
    return INVDESTALPHA;
  }
  if (strcmp (tmp,"DESTCOLOR")==0) {
    return DESTCOLOR;
  }
  if (strcmp (tmp,"INVDESTCOLOR")==0) {
    return INVDESTCOLOR;
  }
  if (strcmp (tmp,"SRCALPHASAT")==0) {
    return SRCALPHASAT;
  }
  if (strcmp (tmp,"CONSTALPHA")==0) {
    return CONSTALPHA;
  }
  if (strcmp (tmp,"INVCONSTALPHA")==0) {
    return INVCONSTALPHA;
  }
  if (strcmp (tmp,"CONSTCOLOR")==0) {
    return CONSTCOLOR;
  }
  if (strcmp (tmp,"INVCONSTCOLOR")==0) {
    return INVCONSTCOLOR;
  }
  return ZERO;
}

bool shouldreflect (string r) {
    if (strtoupper(r)=="FALSE")
		return false;
	int i;
	for (i=0;i<(int)r.length();++i) {
		if (r[i]!='0'&&r[i]!='.'&&r[i]!='+'&&r[i]!='e')
			return true;
	}
	return false;
}



void beginElement(const string &name, const AttributeList &attributes, XML * xml) {
  
  AttributeList::const_iterator iter;
  XML::Names elem = (XML::Names)XML::element_map.lookup(name);
  XML::Names top;
  if(xml->state_stack.size()>0) top = *xml->state_stack.rbegin();
  xml->state_stack.push_back(elem);
  switch(elem) {
  case XML::DETAILPLANE:
	 for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
	    switch(XML::attribute_map.lookup((*iter).name)) {
		case XML::X:
			xml->detailplane.x=XMLSupport::parse_float(iter->value);
			break;
		case XML::Y:
			xml->detailplane.y=XMLSupport::parse_float(iter->value);
			break;
			
		case XML::Z:
			xml->detailplane.z=XMLSupport::parse_float(iter->value);
			break;
			}
	  }
	  break;
  case XML::MATERIAL:
  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		    switch(XML::attribute_map.lookup((*iter).name)) {
			case XML::USENORMALS:
			  xml->usenormals = XMLSupport::parse_bool (iter->value);
			  break;
		    case XML::POWER:
		      xml->material.power=XMLSupport::parse_float((*iter).value);
		      break;
		    case XML::REFLECT:
		      xml->reflect= ( shouldreflect((*iter).value));
		      break;
		    case XML::LIGHTINGON:
		      xml->lighting= (XMLSupport::parse_bool((*iter).value)); 
		      break;
		    case XML::CULLFACE:
		      xml->cullface =(XMLSupport::parse_bool((*iter).value)); 
		      break;
		    }
		  }
		  break;
  case XML::DIFFUSE:
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.dr=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.db=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.da=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.dg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::EMISSIVE:
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.er=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.eb=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.ea=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.eg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::SPECULAR:
	   for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.sr=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.sb=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.sa=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.sg=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::AMBIENT:
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		  switch(XML::attribute_map.lookup((*iter).name)) {
		  case XML::RED:
			  xml->material.ar=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::BLUE:
			  xml->material.ab=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::ALPHA:
			  xml->material.aa=XMLSupport::parse_float((*iter).value);
			  break;
		  case XML::GREEN:
			  xml->material.ag=XMLSupport::parse_float((*iter).value);
			  break;
		  }
	  }
	  break;
  case XML::UNKNOWN:
   fprintf (stderr, "Unknown element start tag '%s' detected\n",name.c_str());
    break;
  case XML::MESH:
	memset(&xml->material, 0, sizeof(xml->material));
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	  case XML::REVERSE:
		xml->reverse = XMLSupport::parse_bool((*iter).value);
		break;
      case XML::FORCETEXTURE:
		xml->force_texture=XMLSupport::parse_bool ((*iter).value);
		break;
	  case XML::SCALE:
		xml->scale =  XMLSupport::parse_float ((*iter).value);
		break;
	  case XML::SHAREVERT:
		xml->sharevert = XMLSupport::parse_bool ((*iter).value);
		break;
	  case XML::POLYGONOFFSET:
	    xml->polygon_offset = XMLSupport::parse_float ((*iter).value);
	    break;
	  case XML::BLENDMODE:
		{
		char *csrc=strdup ((*iter).value.c_str());
		char *cdst=strdup((*iter).value.c_str());
		sscanf (((*iter).value).c_str(),"%s %s",csrc,cdst);
		xml->blend_src=parse_alpha (csrc);
		xml->blend_dst=parse_alpha (cdst);
		free (csrc);
		free (cdst);
		}
		break;
	  case XML::DETAILTEXTURE: //FIXME
		break;
      case XML::TEXTURE: //FIXME
		break;
      case XML::ALPHAMAP: //FIXME
		break;
      case XML::ANIMATEDTEXTURE: //FIXME
		break;
	  case XML::UNKNOWN: //FIXME
		break;
      }
    }
	break;
  case XML::POINTS:
    break;
  case XML::POINT:
	memset(&xml->vertex, 0, sizeof(xml->vertex));
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
	memset(&xml->linetemp, 0, sizeof(xml->linetemp));
	xml->curpolytype=LINE;
	xml->curpolyindex=0;
	xml->linetemp.flatshade=0;
    break;
  case XML::TRI:
	memset(&xml->triangletemp, 0, sizeof(xml->triangletemp));
	xml->curpolytype=TRIANGLE;
	xml->curpolyindex=0;
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::FLATSHADE:
		if ((*iter).value=="Flat") {
			xml->triangletemp.flatshade=1;
		}else {
			xml->triangletemp.flatshade=0;
		}break;
	  }
	}
    break;
  case XML::QUAD:
	memset(&xml->quadtemp, 0, sizeof(xml->quadtemp));
	xml->curpolytype=QUAD;
	xml->curpolyindex=0;
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::FLATSHADE:
		if ((*iter).value=="Flat") {
			xml->quadtemp.flatshade=1;
		}else {
			xml->quadtemp.flatshade=0;
		}break;
	  }
	}
    break;
  case XML::LINESTRIP: //FIXME
    break;
  case XML::TRISTRIP: //FIXME
    break;
  case XML::TRIFAN: //FIXME
    break;
  case XML::QUADSTRIP: //FIXME
    break;
  case XML::VERTEX:
	unsigned int index;
    float s, t;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::POINT:
		index = XMLSupport::parse_int((*iter).value);
		break;
      case XML::S:
		s = XMLSupport::parse_float((*iter).value);
		break;
      case XML::T:
		t = XMLSupport::parse_float((*iter).value);
		break;
     }
    }
	switch(xml->curpolytype){
	case LINE:
		xml->linetemp.indexref[xml->curpolyindex]=index;
		xml->linetemp.s[xml->curpolyindex]=s;
		xml->linetemp.t[xml->curpolyindex]=t;
		break;
	case TRIANGLE:
		xml->triangletemp.indexref[xml->curpolyindex]=index;
		xml->triangletemp.s[xml->curpolyindex]=s;
		xml->triangletemp.t[xml->curpolyindex]=t;
		break;
	case QUAD:
		xml->triangletemp.indexref[xml->curpolyindex]=index;
		xml->triangletemp.s[xml->curpolyindex]=s;
		xml->triangletemp.t[xml->curpolyindex]=t;
		break;
	}
	xml->curpolyindex+=1;	
    break;
  case XML::LOD: //FIXME
    break;
  case XML::LOGO: //FIXME
    break;
  case XML::REF: //FIXME
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
    xml->vertices.push_back (xml->vertex);
    break;
  case XML::VERTEX:
	break;
  case XML::POINTS:
    break;
  case XML::LINE:
	xml->lines.push_back(xml->linetemp);
    break;
  case XML::TRI:
	xml->tris.push_back (xml->triangletemp);
    break;
  case XML::QUAD:
	xml->quads.push_back (xml->quadtemp);
    break;
  case XML::LINESTRIP://FIXME
    break;
  case XML::TRISTRIP://FIXME
    break;
  case XML::TRIFAN://FIXME
    break;
  case XML::QUADSTRIP://FIXME
    break;
  case XML::POLYGONS:
    break;
  case XML::REF://FIXME
    break;
  case XML::LOGO: //FIXME
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



int main (int argc, char** argv) {
	if (argc!=3){
		fprintf(stderr,"wrong number of arguments, aborting\n");
		for(int i = 0; i<argc;i++){
			fprintf(stderr,"%d : %s\n",i,argv[i]);
		}
		exit(-1);
	}

  XML memfile =(LoadXML(argv[1],1));
  fprintf(stderr,"number of vertices: %d\nnumber of lines: %d\nnumber of triangles: %d\nnumber of quads: %d\n",memfile.vertices.size(),memfile.lines.size(),memfile.tris.size(),memfile.quads.size());
  FILE * Outputfile=fopen(argv[2],"w"); 
  int intbuf;
  float floatbuf;
  float versionnumber=(1.0)/(16.0);
  fwrite(&versionnumber,sizeof(float),1,Outputfile);// VERSION number for BinaryFormattedXMesh
  floatbuf = memfile.scale;
  fwrite(&floatbuf,sizeof(float),1,Outputfile);// Mesh Scale
  intbuf= memfile.vertices.size();
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of vertices
  return 0;
}


