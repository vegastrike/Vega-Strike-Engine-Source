#include <string>
#include <vector>
///Stores all the load-time vertex info in the XML struct FIXME light calculations
#include "expat.h"
#include "endianness.h"
using std::vector;
using std::string;
#include "xml_support.h"
#include "hashtable.h"
using namespace XMLSupport;

struct GFXVertex {
  float x,y,z; // Location
  float i,j,k; // Normal
  float s,t; // U,V coords
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
	int indexref[2]; //Index into Points
	float s[2]; //U
	float t[2]; //V

};

struct triangle{
	bool flatshade;
	int indexref[3]; //Index into Points
	float s[3]; //U
	float t[3]; //V
	
};

struct quad{
	bool flatshade;
	int indexref[4]; //Index into Points
	float s[4]; //U
	float t[4]; //V
};

struct stripelement{
	int indexref;
	float s;
	float t;
};

struct strip{
	bool flatshade;
	vector <stripelement> points;
};


struct LODholder{
	float FPS;
	float size;
	vector<unsigned char> name;
};

enum polytype{ 
	LINE,
	TRIANGLE,
	QUAD,
	LINESTRIP,
    TRISTRIP,
    TRIFAN,
    QUADSTRIP
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
    ///Saves which attributes of vertex have been set in XML file (debug)
    enum PointState {
      P_X = 0x1,
      P_Y = 0x2,
      P_Z = 0x4,
      P_I = 0x8,
      P_J = 0x10,
      P_K = 0x20
    };
    ///Saves which attributes of vertex have been set in Polygon for XML file (debug)
    enum VertexState {
      V_POINT = 0x1,
      V_S = 0x2,
      V_T = 0x4
    };
    ///Save if various logo values have been set (debug)
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
	struct vec3f{ // a 3 dimensional vector
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

	//FIXME - strips have yet to be verified to work
    vector <strip> linestrips;
    vector <strip> tristrips;
    vector <strip> trifans;
    vector <strip> quadstrips;
	//END FIXME

	//FIXME - Logos not currently supported
	vector <ZeLogo> logos;
	//End FIXME

	vec3f detailplane;
	vector <vec3f> detailplanes;
	

	bool sharevert;
    bool usenormals;
    bool reverse;
    bool force_texture;
	bool recalc_norm;
	bool shouldreflect;
	
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
	strip striptemp;
	stripelement stripelementtemp;
	LODholder lodtemp;
	vector<LODholder> LODs;

	GFXMaterial material;
    float scale;
	XML(){ //FIXME make defaults appear here.
		scale=1.0;
		sharevert=0;
		usenormals=0;
		reverse=0;
		force_texture=0;
		recalc_norm=0;
		shouldreflect=0;
		detailplane.x=0;
		detailplane.y=0;
		detailplane.z=0;
		reflect=0;
		lighting=0;
		cullface=0;
		polygon_offset=0;
		blend_src=0;
		blend_dst=0;
		point_state=0;
	}
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
			return true; // Just about anything other than "FALSE" or 0.00 etc.
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
	 memset(&xml->detailplane, 0, sizeof(xml->detailplane));
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
	  xml->detailplanes.push_back(xml->detailplane);
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
  case XML::LINESTRIP: //FIXME?
	memset(&xml->striptemp, 0, sizeof(xml->striptemp));
	xml->curpolytype=LINESTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::TRISTRIP: //FIXME?
    memset(&xml->striptemp, 0, sizeof(xml->striptemp));
	xml->curpolytype=TRISTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::TRIFAN: //FIXME?
    memset(&xml->striptemp, 0, sizeof(xml->striptemp));
	xml->curpolytype=TRIFAN;
	xml->striptemp.flatshade=0;
	break;
  case XML::QUADSTRIP: //FIXME?
    memset(&xml->striptemp, 0, sizeof(xml->striptemp));
	xml->curpolytype=QUADSTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::VERTEX:
	unsigned int index;
    float s,t;
	s=0;
	t=0;
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
	case LINESTRIP:
	case TRISTRIP:
	case TRIFAN:
	case QUADSTRIP:
		xml->stripelementtemp.indexref=index;
		xml->stripelementtemp.s=s;
		xml->stripelementtemp.t=t;
		xml->striptemp.points.push_back(xml->stripelementtemp);
		break;
	}
	xml->curpolyindex+=1;	
    break;
  case XML::LOD: //FIXME?
	  xml->lodtemp.size=0;
	  xml->lodtemp.FPS=0;
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		switch(XML::attribute_map.lookup((*iter).name)) {
		case XML::UNKNOWN:
		break;
		case XML::FRAMESPERSECOND:
			xml->lodtemp.FPS=XMLSupport::parse_float((*iter).value);
		  break;
		case XML::SIZE:
		  xml->lodtemp.size = XMLSupport::parse_float ((*iter).value);
		  break;
		case XML::LODFILE:
		  string lodname = (*iter).value.c_str();
		  for(int index=0;index<lodname.size();index++){
		  xml->lodtemp.name.push_back(lodname[index]);
		  }
		  break;
		}
	  }
	  xml->LODs.push_back(xml->lodtemp);
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
  case XML::LINESTRIP://FIXE?
	xml->linestrips.push_back(xml->striptemp);
    break;
  case XML::TRISTRIP://FIXME?
	xml->tristrips.push_back(xml->striptemp);
    break;
  case XML::TRIFAN://FIXME?
	xml->trifans.push_back(xml->striptemp);
    break;
  case XML::QUADSTRIP://FIXME?
	xml->quadstrips.push_back(xml->striptemp);
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

  XML memfile=(LoadXML(argv[1],1));
  fprintf(stderr,"number of vertices: %d\nnumber of lines: %d\nnumber of triangles: %d\nnumber of quads: %d\n",memfile.vertices.size(),memfile.lines.size(),memfile.tris.size(),memfile.quads.size());
  FILE * Outputfile=fopen(argv[2],"wb"); 
  unsigned int intbuf;
  float floatbuf;
  int versionnumber=3;
  unsigned char bytebuf;

  //HEADER

  bytebuf='B'; // "Magic Word"
  fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='F';
  fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='X';
  fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='M';
  fwrite(&bytebuf,1,1,Outputfile);

  fwrite(&versionnumber,sizeof(int),1,Outputfile);// VERSION number for BinaryFormattedXMesh
  floatbuf = VSSwapHostFloatToLittle(memfile.scale);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);// Mesh Scale
  intbuf= VSSwapHostIntToLittle(memfile.reverse);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//reverse flag
  intbuf= VSSwapHostIntToLittle(memfile.force_texture);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Force texture flag
  intbuf= VSSwapHostIntToLittle(memfile.sharevert);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Share vertex flag
  floatbuf= VSSwapHostFloatToLittle(memfile.polygon_offset);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Polygon offset
  intbuf= VSSwapHostIntToLittle(memfile.blend_src);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Blend Source
  intbuf= VSSwapHostIntToLittle(memfile.blend_dst);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Blend Destination
  floatbuf= VSSwapHostFloatToLittle(memfile.material.power);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Power
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ar);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ag);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ab);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.aa);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.dr);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.dg);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.db);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.da);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.er);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.eg);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.eb);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ea);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sr);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sg);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sb);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sa);
  fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Alpha
  //END HEADER
  //Begin Variable sized Attributes
  
  intbuf= VSSwapHostIntToLittle(memfile.detailplanes.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of detail planes
  for(int plane=0;plane<memfile.detailplanes.size();plane++){
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].x);
	fwrite(&floatbuf,sizeof(float),1,Outputfile);//Detail Plane:X
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].y);
	fwrite(&floatbuf,sizeof(float),1,Outputfile);//Detail Plane:Y
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].z);
	fwrite(&floatbuf,sizeof(float),1,Outputfile);//Detail Plane:Z
  }
  //End Variable sized Attributes

  //GEOMETRY
  intbuf= VSSwapHostIntToLittle(memfile.vertices.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of vertices
  for(int verts=0;verts<memfile.vertices.size();verts++){
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].x);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:x
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].y);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:y
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].z);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:z
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].i);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:i
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].j);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:j
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].k);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:k
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].s);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:s
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].t);
	  fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:t
  }
  intbuf= VSSwapHostIntToLittle(memfile.lines.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of lines
  for(int lines=0;lines<memfile.lines.size();lines++){
	intbuf= VSSwapHostIntToLittle(memfile.lines[lines].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<2;tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.lines[lines].indexref[tmpcounter]);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.lines[lines].s[tmpcounter]);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.lines[lines].t[tmpcounter]);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.tris.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of triangles
  for(int tris=0;tris<memfile.tris.size();tris++){
	intbuf= VSSwapHostIntToLittle(memfile.tris[tris].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<3;tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.tris[tris].indexref[tmpcounter]);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.tris[tris].s[tmpcounter]);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.tris[tris].t[tmpcounter]);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.quads.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of Quads
  for(int quads=0;quads<memfile.quads.size();quads++){
	intbuf= VSSwapHostIntToLittle(memfile.quads[quads].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<4;tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.quads[quads].indexref[tmpcounter]);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.quads[quads].s[tmpcounter]);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.quads[quads].t[tmpcounter]);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of linestrips
  for(int ls=0;ls<memfile.linestrips.size();ls++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].points.size());
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current linestrip
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[ls].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].points[tmpcounter].indexref);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].s);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].t);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of tristrips
  for(int ts=0;ts<memfile.linestrips.size();ts++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ts].points.size());
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current tristrip
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ts].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[ts].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[ts].points[tmpcounter].indexref);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ts].points[tmpcounter].s);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ts].points[tmpcounter].t);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of trifans
  for(int tf=0;tf<memfile.linestrips.size();tf++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[tf].points.size());
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current trifan
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[tf].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[tf].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[tf].points[tmpcounter].indexref);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[tf].points[tmpcounter].s);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[tf].points[tmpcounter].t);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of quadstrips
  for(int qs=0;qs<memfile.linestrips.size();qs++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[qs].points.size());
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current quadstrip
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[qs].flatshade);
	fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[qs].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[qs].points[tmpcounter].indexref);
		fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[qs].points[tmpcounter].s);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[qs].points[tmpcounter].t);
		fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  //END GEOMETRY
  return 0;
}


