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
   GFXMaterial () {
      dr=dg=db=da=sr=sg=sb=sa=ea=aa=1.0f;
      er=eg=eb=ar=ag=ab=0.0f;
      power=60.0f;
      //defaults for material struct    
   }
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
	strip(){
	  flatshade=false;
	  points=vector<stripelement>();
	}
};


struct LODholder{ // Holds 1 LOD entry
	float size;
	vector<unsigned char> name;
	LODholder(){
	  name= vector<unsigned char>();
	  size=0;
	}
};

struct animframe{ // Holds one animation frame
	vector<unsigned char> name;
	animframe(){
	  name= vector<unsigned char>();
	}
};

struct animdef{ // Holds animation definition
	vector<unsigned char> name;
	float FPS;
	vector<int> meshoffsets;
	animdef(){
	  name= vector<unsigned char>();
	  FPS=0;
	  meshoffsets=vector<int>();
	}
};


struct textureholder{ // Holds 1 texture entry
	int type;
	int index;
	vector<unsigned char> name;
	textureholder(){
	  name= vector<unsigned char>();
	}
};

enum textype{
	ALPHAMAP,
	ANIMATION,
	TEXTURE
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
	//new
	ANIMDEF,
	ANIMFRAME,
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
	STARTFRAME,
	FRAMEMESHNAME,
	ANIMATIONNAME,
	ANIMATIONFRAMEINDEX,
	ANIMATIONMESHINDEX
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
	  ZeLogo(){
		  refpnt=vector<int>();
		  refweight=vector<float>();
		  size=0;
		  offset=0;
		  rotate=0;
		  type=0;
	  }
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

	vector <ZeLogo> logos;

	vec3f detailplane;
	vector <vec3f> detailplanes;
	

	bool sharevert;
    bool usenormals;
    bool reverse;
    bool force_texture;
	
	bool reflect;
	bool lighting;
	bool cullface;
	float polygon_offset;
	int blend_src;
	int blend_dst;

    GFXVertex vertex;
	textureholder texturetemp;
	vector<textureholder> textures;
	textureholder detailtexture;

	int curpolytype;
	int curpolyindex;
	
	line linetemp;
	triangle triangletemp;
	quad quadtemp;
	strip striptemp;
	stripelement stripelementtemp;
	LODholder lodtemp;
	animframe animframetemp;
	animdef animdeftemp;
	vector<LODholder> LODs;
	vector<animframe> animframes;
	vector<animdef> animdefs;

	GFXMaterial material;
    float scale;
	XML(){ //FIXME make defaults appear here.
		scale=1.0;
		sharevert=0;
		usenormals=0;
		reverse=0;
		force_texture=0;
		reflect=1;
		lighting=1;
		cullface=1;
		polygon_offset=0;
		blend_src=ONE;
		blend_dst=ZERO;
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
  EnumMap::Pair("DetailPlane",XML::DETAILPLANE),
  EnumMap::Pair("AnimationDefinition",XML::ANIMDEF),
  EnumMap::Pair("Frame",XML::ANIMFRAME),
  EnumMap::Pair ("AnimationFrameIndex",XML::ANIMATIONFRAMEINDEX)
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
  EnumMap::Pair ("FramesPerSecond",XML::FRAMESPERSECOND),
  EnumMap::Pair ("FrameMeshName",XML::FRAMEMESHNAME),
  EnumMap::Pair ("AnimationName",XML::ANIMATIONNAME),
  EnumMap::Pair ("AnimationMeshIndex",XML::ANIMATIONMESHINDEX)
};


const EnumMap XML::element_map(XML::element_names, 27);
const EnumMap XML::attribute_map(XML::attribute_names, 40);






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
	//memset(&xml->material, 0, sizeof(xml->material));
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
	  case XML::DETAILTEXTURE:
		  {
		string detnametmp=(*iter).value.c_str();
		xml->detailtexture.type=TEXTURE;
		xml->detailtexture.index=0;
		xml->detailtexture.name=vector<unsigned char>();
		for(int detnamelen=0;detnamelen<detnametmp.size();detnamelen++){
			xml->detailtexture.name.push_back(detnametmp[detnamelen]);
		}
		  }
		break;
      case XML::TEXTURE: 
	  case XML::ALPHAMAP: 
	  case XML::ANIMATEDTEXTURE:
	  case XML::UNKNOWN: //FIXME?
		 {
          XML::Names whichtype = XML::UNKNOWN;
          int strsize=0;
          if (strtoupper(iter->name).find("ANIMATION")==0) {
              xml->texturetemp.type=ANIMATION;
			  whichtype = XML::ANIMATEDTEXTURE;
              strsize = strlen ("ANIMATION");
          }
          if (strtoupper(iter->name).find("TEXTURE")==0){
              xml->texturetemp.type=TEXTURE;
			  whichtype= XML::TEXTURE;
              strsize = strlen ("TEXTURE");
          }
          if (strtoupper(iter->name).find("ALPHAMAP")==0){
              xml->texturetemp.type=ALPHAMAP;
			  whichtype=XML::ALPHAMAP;
              strsize= strlen ("ALPHAMAP");
          }
          if (whichtype!=XML::UNKNOWN) {
              unsigned int texindex =0;
              string ind(iter->name.substr (strsize));
              if (!ind.empty()){
				texindex=atoi(ind.c_str());
			  }
			  xml->texturetemp.index=texindex;
			  xml->texturetemp.name=vector<unsigned char>();
			  string nomdujour=iter->value.c_str();
			  for(int tni=0;tni<nomdujour.size();tni++){
				xml->texturetemp.name.push_back(nomdujour[tni]);
			  }
			xml->textures.push_back(xml->texturetemp);
		  }
		 }
		break;
	  }
	}
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
	xml->striptemp=strip();
	xml->curpolytype=LINESTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::TRISTRIP: //FIXME?
    xml->striptemp=strip();
	xml->curpolytype=TRISTRIP;
	xml->striptemp.flatshade=0;
	break;
  case XML::TRIFAN: //FIXME?
    xml->striptemp=strip();
	xml->curpolytype=TRIFAN;
	xml->striptemp.flatshade=0;
	break;
  case XML::QUADSTRIP: //FIXME?
    xml->striptemp=strip();
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
		xml->quadtemp.indexref[xml->curpolyindex]=index;
		xml->quadtemp.s[xml->curpolyindex]=s;
		xml->quadtemp.t[xml->curpolyindex]=t;
		break;
	case LINESTRIP:
	case TRISTRIP:
	case TRIFAN:
	case QUADSTRIP:
		xml->stripelementtemp=stripelement();
		xml->stripelementtemp.indexref=index;
		xml->stripelementtemp.s=s;
		xml->stripelementtemp.t=t;
		xml->striptemp.points.push_back(xml->stripelementtemp);
		break;
	}
	xml->curpolyindex+=1;	
    break;
  case XML::LOD: //FIXME?
	  xml->lodtemp=LODholder();
	  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
		switch(XML::attribute_map.lookup((*iter).name)) {
		case XML::UNKNOWN:
		break;
		case XML::FRAMESPERSECOND:
		  continue;
		  break;
		case XML::SIZE:
		  xml->lodtemp.size = XMLSupport::parse_float ((*iter).value);
		  break;
		case XML::LODFILE:
		  string lodname = (*iter).value.c_str();
		  xml->lodtemp.name=vector<unsigned char>();
		  for(int index=0;index<lodname.size();index++){
			xml->lodtemp.name.push_back(lodname[index]);
		  }
		  break;
		}
	  }
	  xml->LODs.push_back(xml->lodtemp);
	  break;
  case XML::LOGO: 
	unsigned int typ;
    float rot, siz,offset;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
		fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
		break;
      case XML::TYPE:
		typ = XMLSupport::parse_int((*iter).value);
		break;
      case XML::ROTATE:
		rot = XMLSupport::parse_float((*iter).value);
		break;
      case XML::SIZE:
		siz = XMLSupport::parse_float((*iter).value);
		break;
      case XML::OFFSET:
		offset = XMLSupport::parse_float ((*iter).value);
		break;
      }
    }
    xml->logos.push_back(XML::ZeLogo());
    xml->logos[xml->logos.size()-1].type = typ;
    xml->logos[xml->logos.size()-1].rotate = rot;
    xml->logos[xml->logos.size()-1].size = siz;
    xml->logos[xml->logos.size()-1].offset = offset;
    break;
  case XML::REF: //FIXME
	{
	unsigned int ind=0;
    float indweight=1;
    bool foundindex=false;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
      case XML::UNKNOWN:
		fprintf (stderr,"Unknown attribute '%s' encountered in Vertex tag\n",(*iter).name.c_str() );
		break;
      case XML::POINT:
		ind = XMLSupport::parse_int((*iter).value);
		foundindex=true;
		break;
      case XML::WEIGHT:
		indweight = XMLSupport::parse_float((*iter).value);
		break;
      }
    }
    
    xml->logos[xml->logos.size()-1].refpnt.push_back(ind);
    xml->logos[xml->logos.size()-1].refweight.push_back(indweight);
	}
    break;
  case XML::ANIMDEF:
	xml->animdeftemp=animdef();
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	    case XML::ANIMATIONNAME:
		{
		  string animname = (*iter).value.c_str();
		  xml->animdeftemp.name=vector<unsigned char>();
		  for(int index=0;index<animname.size();index++){
			xml->animdeftemp.name.push_back(animname[index]);
		  }
		}
		  break;
		case XML::FRAMESPERSECOND:
		  xml->animdeftemp.FPS=XMLSupport::parse_float((*iter).value);
		  break;
	  }
	}
	break;
  case XML::ANIMATIONFRAMEINDEX:
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	  case XML::ANIMATIONMESHINDEX:
        xml->animdeftemp.meshoffsets.push_back(XMLSupport::parse_int((*iter).value));
		break;
	  }
	}
    break;
  case XML::ANIMFRAME:
    xml->animframetemp=animframe();
	for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(XML::attribute_map.lookup((*iter).name)) {
	    case XML::FRAMEMESHNAME:
		  string framename = (*iter).value.c_str();
		  xml->animframetemp.name=vector<unsigned char>();
		  for(int index=0;index<framename.size();index++){
			xml->animframetemp.name.push_back(framename[index]);
		  }
		  break;
	  }
	}
	xml->animframes.push_back(xml->animframetemp);
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
  case XML::ANIMDEF:
	xml->animdefs.push_back(xml->animdeftemp);
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

int writesuperheader(XML memfile, FILE* Outputfile); //Writes superheader to file Outputfile
int appendrecordfromxml(XML memfile, FILE* Outputfile); // Append a record specified in memfile to the output file and return number of bytes written. Assumes Outputfile is appropriately positioned at the end of the file.
int appendmeshfromxml(XML memfile, FILE* Outputfile); // Append a mesh specified in memfile to the output file and return number of bytes written. Assumes Outputfile is appropriately positioned at the end of the file.
void ReverseToFile(FILE* Inputfile,FILE* Outputfile); //Translate BFXM file Inputfile to text file Outputfile


int main (int argc, char** argv) {
	if (argc!=4){
		fprintf(stderr,"wrong number of arguments, aborting\n");
		for(int i = 0; i<argc;i++){
			fprintf(stderr,"%d : %s\n",i,argv[i]);
		}
		exit(-1);
	}

  bool append=(argv[3][0]=='a');
  bool create=(argv[3][0]=='c');
  bool reverse=(argv[3][0]=='r');
//  fprintf(stderr,"number of vertices: %d\nnumber of lines: %d\nnumber of triangles: %d\nnumber of quads: %d\n",memfile.vertices.size(),memfile.lines.size(),memfile.tris.size(),memfile.quads.size());
  FILE * Outputfile;
  if(append){
	Outputfile=fopen(argv[2],"rb+"); //append to end, but not append, which doesn't do what you want it to.
	fseek(Outputfile, 0, SEEK_END);
  }else if(create){
	Outputfile=fopen(argv[2],"wb+"); //create file for BFXM output
  } else if(reverse){
	FILE* Inputfile=fopen(argv[1],"rb");
	Outputfile=fopen(argv[2],"w+"); //create file for text output
	ReverseToFile(Inputfile,Outputfile);
	exit(0);
  } else {
	  fprintf(stderr,"Invalid flag: %c - aborting",argv[3][0]);
	  exit(-1);
  }
  

  XML memfile=(LoadXML(argv[1],1));
  unsigned int intbuf;
  float floatbuf;
  unsigned char bytebuf;

  int runningbytenum=0;
  if(!append){
	runningbytenum+=writesuperheader(memfile,Outputfile); // Write superheader
  }
  runningbytenum+=appendrecordfromxml(memfile,Outputfile); //Append one record

  rewind(Outputfile);
  fseek(Outputfile,4+7*sizeof(int),SEEK_SET);
  fread(&intbuf,sizeof(int),1,Outputfile);//Current number of records
  intbuf=VSSwapHostIntToLittle(intbuf);
  ++intbuf;
  intbuf=VSSwapHostIntToLittle(intbuf);
  fseek(Outputfile,4+7*sizeof(int),SEEK_SET);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//number of records++

  fseek(Outputfile,4+sizeof(int),SEEK_SET);
  fread(&intbuf,sizeof(int),1,Outputfile);//Current length of file
  intbuf=VSSwapHostIntToLittle(intbuf);
  intbuf+=runningbytenum;
  intbuf=VSSwapHostIntToLittle(intbuf);
  fseek(Outputfile,4+sizeof(int),SEEK_SET);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Correct number of bytes for total file

  return 0;
}
 
int writesuperheader(XML memfile, FILE* Outputfile){
  unsigned int intbuf;
  float floatbuf;
  int versionnumber=VSSwapHostIntToLittle(10);
  unsigned char bytebuf;
  int runningbytenum=0;
  //SUPER HEADER

  bytebuf='B'; // "Magic Word"
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='F';
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='X';
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  bytebuf='M';
  runningbytenum+=fwrite(&bytebuf,1,1,Outputfile);
  runningbytenum+=sizeof(int)*fwrite(&versionnumber,sizeof(int),1,Outputfile);// VERSION number for BinaryFormattedXMesh
  intbuf=VSSwapHostIntToLittle(0);//Length of File Placeholder
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// Number of bytes in file
  //Super-Header Meaty part
  intbuf=VSSwapHostIntToLittle(4+(8*sizeof(int)));//Super-Header length in Bytes for version 0.10
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// Number of bytes in Superheader
  intbuf=VSSwapHostIntToLittle(8);//Number of fields per vertex
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//
  intbuf=VSSwapHostIntToLittle(3);//Number of fields per polygon structure
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// 
  intbuf=VSSwapHostIntToLittle(3);//Number of fields per referenced vertex
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// 
  intbuf=VSSwapHostIntToLittle(1);//Number of fields per referenced animation
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// 
  intbuf=VSSwapHostIntToLittle(0);//Number of records - initially 0
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// Number of records

  return runningbytenum;
}

int appendrecordfromxml(XML memfile, FILE* Outputfile){
  unsigned int intbuf;
  int runningbytenum=0;
  //Record Header
  intbuf=VSSwapHostIntToLittle(12);// Size of Record Header in bytes
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// Number of bytes in record header
  intbuf=VSSwapHostIntToLittle(0);//Size of Record in bytes
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// Number of bytes in record
  intbuf=VSSwapHostIntToLittle(1+memfile.LODs.size()+memfile.animframes.size());//Number of meshes = 1 + numLODs + numAnims. 
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);// Number of meshes
  
  runningbytenum+=appendmeshfromxml(memfile,Outputfile); // write top level mesh
  int mesh;
  for(mesh=0;mesh<memfile.LODs.size();mesh++){ //write all LOD meshes
	string LODname="";
	for(int i=0;i<memfile.LODs[mesh].name.size();i++){
         LODname+=memfile.LODs[mesh].name[i];
	}
	XML submesh=LoadXML(LODname.c_str(),1);
	runningbytenum+=appendmeshfromxml(submesh,Outputfile);
  }
  for(mesh=0;mesh<memfile.animframes.size();mesh++){ //write all Animation Frames
	string animname="";
	for(int i=0;i<memfile.animframes[mesh].name.size();i++){
         animname+=memfile.animframes[mesh].name[i];
	}
	XML submesh=LoadXML(animname.c_str(),1);
	runningbytenum+=appendmeshfromxml(submesh,Outputfile);
  }
  
  fseek(Outputfile,(-1*(runningbytenum))+4,SEEK_CUR);
  intbuf=runningbytenum;
  intbuf= VSSwapHostIntToLittle(intbuf);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Correct number of bytes for total record
  fseek(Outputfile,0,SEEK_END);
  return runningbytenum;

}
int appendmeshfromxml(XML memfile, FILE* Outputfile){
  unsigned int intbuf;
  float floatbuf;
  unsigned char bytebuf;
  int runningbytenum=0;
  
  //Mesh Header
  intbuf= VSSwapHostIntToLittle(11*sizeof(int)+19*sizeof(float));
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Size of Mesh header in Bytes
  intbuf= VSSwapHostIntToLittle(0);// Temp - rewind and fix.
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Size of this Mesh in Bytes
  floatbuf = VSSwapHostFloatToLittle(memfile.scale);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);// Mesh Scale
  intbuf= VSSwapHostIntToLittle(memfile.reverse);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//reverse flag
  intbuf= VSSwapHostIntToLittle(memfile.force_texture);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Force texture flag
  intbuf= VSSwapHostIntToLittle(memfile.sharevert);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Share vertex flag
  floatbuf= VSSwapHostFloatToLittle(memfile.polygon_offset);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Polygon offset
  intbuf= VSSwapHostIntToLittle(memfile.blend_src);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Blend Source
  intbuf= VSSwapHostIntToLittle(memfile.blend_dst);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Blend Destination
  floatbuf= VSSwapHostFloatToLittle(memfile.material.power);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Power
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ar);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ag);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ab);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.aa);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Ambient:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.dr);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.dg);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.db);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.da);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Diffuse:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.er);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.eg);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.eb);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.ea);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Emissive:Alpha
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sr);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Red
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sg);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Green
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sb);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Blue
  floatbuf= VSSwapHostFloatToLittle(memfile.material.sa);
  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Material:Specular:Alpha
  intbuf= VSSwapHostIntToLittle(memfile.cullface);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Cullface
  intbuf= VSSwapHostIntToLittle(memfile.lighting);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//lighting
  intbuf= VSSwapHostIntToLittle(memfile.reflect);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//reflect
  intbuf= VSSwapHostIntToLittle(memfile.usenormals);
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//usenormals
  //END HEADER
  //Begin Variable sized Attributes
  int VSAstart=runningbytenum;
  intbuf= VSSwapHostIntToLittle(0); // Temp value will overwrite later
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Length of Variable sized attribute section in bytes
  //Detail texture
  { 
	int namelen=memfile.detailtexture.name.size();
	intbuf= VSSwapHostIntToLittle(namelen);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Length of name of detail texture
        int nametmp;
	for(nametmp=0;nametmp<namelen;nametmp++){
		bytebuf= memfile.detailtexture.name[nametmp];
		runningbytenum+=fwrite(&bytebuf,sizeof(char),1,Outputfile);//char by char name of detail texture
	}
	int padlength=(sizeof(int)-(namelen%sizeof(int)))%sizeof(int);
	for(nametmp=0;nametmp<padlength;nametmp++){
		bytebuf=0;
		runningbytenum+=fwrite(&bytebuf,sizeof(char),1,Outputfile);//Padded so that next field is word aligned
	}
  }
  //Detail Planes
  intbuf= VSSwapHostIntToLittle(memfile.detailplanes.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of detail planes
  for(int plane=0;plane<memfile.detailplanes.size();plane++){
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].x);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Detail Plane:X
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].y);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Detail Plane:Y
	floatbuf= VSSwapHostFloatToLittle(memfile.detailplanes[plane].z);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Detail Plane:Z
  }
  //Textures
  {
  intbuf= VSSwapHostIntToLittle(memfile.textures.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of textures
  for(int texnum=0;texnum<memfile.textures.size();texnum++){
	intbuf= VSSwapHostIntToLittle(memfile.textures[texnum].type);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//texture # texnum: type
	intbuf= VSSwapHostIntToLittle(memfile.textures[texnum].index);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//texture # texnum: index
	int namelen=memfile.textures[texnum].name.size();
	intbuf= VSSwapHostIntToLittle(namelen);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Length of name of texture # texnum
    int nametmp;
	for(nametmp=0;nametmp<namelen;nametmp++){
		bytebuf= memfile.textures[texnum].name[nametmp];
		runningbytenum+=fwrite(&bytebuf,sizeof(char),1,Outputfile);//Name of texture # texnum
	}
	int padlength=(sizeof(int)-(namelen%sizeof(int)))%sizeof(int);
	for(nametmp=0;nametmp<padlength;nametmp++){
		bytebuf=0;
		runningbytenum+=fwrite(&bytebuf,sizeof(char),1,Outputfile);//Padded so that next field is word aligned
	}
  }
  }

  //Logos
  //FIXME?
  intbuf= VSSwapHostIntToLittle(memfile.logos.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of logos
  for(int logonum=0;logonum<memfile.logos.size();logonum++){
	floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].size);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//logo # logonum: size
	floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].offset);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//logo # logonum: offset
	floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].rotate);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//logo # logonum: rotation
	intbuf= VSSwapHostIntToLittle(memfile.logos[logonum].type);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//logo # logonum: type
	int numrefs=memfile.logos[logonum].refpnt.size();
	intbuf=VSSwapHostIntToLittle(numrefs);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//logo # logonum: number of references
	for(int ref=0;ref<numrefs;ref++){
	  intbuf=VSSwapHostIntToLittle(memfile.logos[logonum].refpnt[ref]);
	  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//logo # logonum: reference # ref
	  floatbuf= VSSwapHostFloatToLittle(memfile.logos[logonum].refweight[ref]);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//logo # logonum: reference # ref weight
	}
  }

  //LODs + Animations
  //LODs
  int submeshref=1;
  intbuf=VSSwapHostIntToLittle(memfile.LODs.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of LODs
  for(int lod=0;lod<memfile.LODs.size();lod++){
	floatbuf= VSSwapHostFloatToLittle(memfile.LODs[lod].size);
	runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//LOD # lod: size
	intbuf=submeshref;
	intbuf=VSSwapHostIntToLittle(intbuf);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//LOD mesh offset
	submeshref++;
  }

  //Current VS File format is not compatible with new animation specification - can't test until I fix old files (only 1 at present uses animations)
  
  	intbuf=VSSwapHostIntToLittle(memfile.animdefs.size());
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of animdefs
	for(int anim=0;anim<memfile.animdefs.size();anim++){
	  int namelen=memfile.animdefs[anim].name.size();
	  intbuf= VSSwapHostIntToLittle(namelen);
	  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Length of name animation
          int nametmp;
	  for(nametmp=0;nametmp<namelen;nametmp++){
		bytebuf= memfile.animdefs[anim].name[nametmp];
		runningbytenum+=fwrite(&bytebuf,sizeof(char),1,Outputfile);//char by char of above
	  }
	  int padlength=(sizeof(int)-(namelen%sizeof(int)))%sizeof(int);
	  for(nametmp=0;nametmp<padlength;nametmp++){
		bytebuf=0;
		runningbytenum+=fwrite(&bytebuf,sizeof(char),1,Outputfile);//Padded so that next field is word aligned
	  }
	  floatbuf=VSSwapHostFloatToLittle(memfile.animdefs[anim].FPS);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//Animdef # anim: FPS
	  for(int offset=0;offset<memfile.animdefs[anim].meshoffsets.size();offset++){
		intbuf=submeshref+memfile.animdefs[anim].meshoffsets[offset];
		intbuf=VSSwapHostIntToLittle(intbuf);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//animation mesh offset
	  }
	}

  //End Variable sized Attributes
  int VSAend=runningbytenum;
  //GEOMETRY
  intbuf= VSSwapHostIntToLittle(memfile.vertices.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of vertices
  for(int verts=0;verts<memfile.vertices.size();verts++){
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].x);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:x
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].y);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:y
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].z);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:z
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].i);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:i
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].j);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:j
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].k);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:k
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].s);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:s
	  floatbuf=VSSwapHostFloatToLittle(memfile.vertices[verts].t);
	  runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//vertex #vert:t
  }
  intbuf= VSSwapHostIntToLittle(memfile.lines.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of lines
  for(int lines=0;lines<memfile.lines.size();lines++){
	intbuf= VSSwapHostIntToLittle(memfile.lines[lines].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<2;tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.lines[lines].indexref[tmpcounter]);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.lines[lines].s[tmpcounter]);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.lines[lines].t[tmpcounter]);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.tris.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of triangles
  for(int tris=0;tris<memfile.tris.size();tris++){
	intbuf= VSSwapHostIntToLittle(memfile.tris[tris].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<3;tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.tris[tris].indexref[tmpcounter]);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.tris[tris].s[tmpcounter]);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.tris[tris].t[tmpcounter]);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.quads.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of Quads
  for(int quads=0;quads<memfile.quads.size();quads++){
	intbuf= VSSwapHostIntToLittle(memfile.quads[quads].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<4;tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.quads[quads].indexref[tmpcounter]);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.quads[quads].s[tmpcounter]);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.quads[quads].t[tmpcounter]);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of linestrips
  for(int ls=0;ls<memfile.linestrips.size();ls++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].points.size());
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current linestrip
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[ls].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[ls].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].s);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ls].points[tmpcounter].t);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of tristrips
  for(int ts=0;ts<memfile.linestrips.size();ts++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ts].points.size());
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current tristrip
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[ts].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[ts].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[ts].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ts].points[tmpcounter].s);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[ts].points[tmpcounter].t);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of trifans
  for(int tf=0;tf<memfile.linestrips.size();tf++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[tf].points.size());
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current trifan
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[tf].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[tf].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[tf].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[tf].points[tmpcounter].s);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[tf].points[tmpcounter].t);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  intbuf= VSSwapHostIntToLittle(memfile.linestrips.size());
  runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of quadstrips
  for(int qs=0;qs<memfile.linestrips.size();qs++){
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[qs].points.size());
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Number of elements in current quadstrip
	intbuf= VSSwapHostIntToLittle(memfile.linestrips[qs].flatshade);
	runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//Flatshade flag
	for(int tmpcounter=0;tmpcounter<memfile.linestrips[qs].points.size();tmpcounter++){
		intbuf= VSSwapHostIntToLittle(memfile.linestrips[qs].points[tmpcounter].indexref);
		runningbytenum+=sizeof(int)*fwrite(&intbuf,sizeof(int),1,Outputfile);//point index
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[qs].points[tmpcounter].s);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//s coord
		floatbuf= VSSwapHostFloatToLittle(memfile.linestrips[qs].points[tmpcounter].t);
		runningbytenum+=sizeof(float)*fwrite(&floatbuf,sizeof(float),1,Outputfile);//t coord
	}
  }
  //END GEOMETRY


  fseek(Outputfile,(-1*(runningbytenum))+4,SEEK_CUR);
  intbuf=runningbytenum;
  intbuf= VSSwapHostIntToLittle(intbuf);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Correct number of bytes for total mesh
  fseek(Outputfile,(-1*(runningbytenum-VSAstart)),SEEK_END);
  intbuf=VSAend-VSAstart;
  intbuf= VSSwapHostIntToLittle(intbuf);
  fwrite(&intbuf,sizeof(int),1,Outputfile);//Correct number of bytes for Variable Sized Attribute section
  fseek(Outputfile,0,SEEK_END);
  return runningbytenum;
}


void ReverseToFile(FILE* Inputfile, FILE* Outputfile){
  unsigned int intbuf;
  float floatbuf;
  unsigned char bytebuf;
  int word32index=0;
  union chunk32{
	  int i32val;
	  float f32val;
	  unsigned char c8val[4];
  } * inmemfile;
  fseek(Inputfile,4+sizeof(int),SEEK_SET);
  fread(&intbuf,sizeof(int),1,Inputfile);//Length of Inputfile
  int Inputlength=VSSwapHostIntToLittle(intbuf);
  inmemfile=(chunk32*)malloc(Inputlength);
  if(!inmemfile) {fprintf(stderr,"Buffer allocation failed, Aborting"); exit(-1);}
  rewind(Inputfile);
  fread(inmemfile,1,Inputlength,Inputfile);
  fclose(Inputfile);
  int Inputlength32=Inputlength/4;
  //Extract superheader fields
  word32index+=1;
  int version = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
  word32index+=2;
  int Superheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
  word32index+=1;
  int NUMFIELDSPERVERTEX = VSSwapHostIntToLittle(inmemfile[word32index].i32val); //Number of fields per vertex:integer (8)
  word32index+=1;
  int NUMFIELDSPERPOLYGONSTRUCTURE = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//  Number of fields per polygon structure: integer (3)
  word32index+=1;
  int NUMFIELDSPERREFERENCEDVERTEX = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of fields per referenced vertex: integer (3)
  word32index+=1;
  int NUMFIELDSPERREFERENCEDANIMATION = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of fields per referenced animation: integer (1)
  word32index+=1;
  int numrecords = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of records: integer
  word32index=(Superheaderlength/4); // Go to first record
  //For each record
  for(int recordindex=0;recordindex<numrecords;recordindex++){
	  int recordbeginword=word32index;
	  //Extract Record Header
	  int recordheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	  word32index+=1;
	  int recordlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
      word32index+=1;
	  int nummeshes = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of meshes in the current record
	  word32index=recordbeginword+(recordheaderlength/4);
	  //For each mesh
	  for(int meshindex=0;meshindex<nummeshes;meshindex++){
		  if(recordindex>0||meshindex>0){
			string filename="";
			filename+=(char)(recordindex+48);
			filename+="_";
			filename+=(char)(meshindex+48);
			filename+=".xmesh";
			Outputfile=fopen(filename.c_str(),"w+");
		  }
		  //Extract Mesh Header
		  int meshbeginword=word32index;
		  int meshheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	      word32index+=1;
	      int meshlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
		  float scale=VSSwapHostFloatToLittle(inmemfile[meshbeginword+2].f32val);//scale
		  int reverse=VSSwapHostIntToLittle(inmemfile[meshbeginword+3].i32val);//reverse flag
		  int forcetexture=VSSwapHostIntToLittle(inmemfile[meshbeginword+4].i32val);//force texture flag
		  int sharevert=VSSwapHostIntToLittle(inmemfile[meshbeginword+5].i32val);//share vertex flag
		  float polygonoffset=VSSwapHostFloatToLittle(inmemfile[meshbeginword+6].f32val);//polygonoffset
		  int bsrc=VSSwapHostIntToLittle(inmemfile[meshbeginword+7].i32val);//Blendmode source
		  int bdst=VSSwapHostIntToLittle(inmemfile[meshbeginword+8].i32val);//Blendmode destination
		  float	power=VSSwapHostFloatToLittle(inmemfile[meshbeginword+9].f32val);//Specular: power
		  float	ar=VSSwapHostFloatToLittle(inmemfile[meshbeginword+10].f32val);//Ambient: red
		  float	ag=VSSwapHostFloatToLittle(inmemfile[meshbeginword+11].f32val);//Ambient: green
		  float	ab=VSSwapHostFloatToLittle(inmemfile[meshbeginword+12].f32val);//Ambient: blue
		  float	aa=VSSwapHostFloatToLittle(inmemfile[meshbeginword+13].f32val);//Ambient: Alpha
		  float	dr=VSSwapHostFloatToLittle(inmemfile[meshbeginword+14].f32val);//Diffuse: red
		  float	dg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+15].f32val);//Diffuse: green
		  float	db=VSSwapHostFloatToLittle(inmemfile[meshbeginword+16].f32val);//Diffuse: blue
		  float	da=VSSwapHostFloatToLittle(inmemfile[meshbeginword+17].f32val);//Diffuse: Alpha
		  float	er=VSSwapHostFloatToLittle(inmemfile[meshbeginword+18].f32val);//Emmissive: red
		  float	eg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+19].f32val);//Emmissive: green
		  float	eb=VSSwapHostFloatToLittle(inmemfile[meshbeginword+20].f32val);//Emmissive: blue
		  float	ea=VSSwapHostFloatToLittle(inmemfile[meshbeginword+21].f32val);//Emmissive: Alpha
		  float	sr=VSSwapHostFloatToLittle(inmemfile[meshbeginword+22].f32val);//Specular: red
		  float	sg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+23].f32val);//Specular: green
		  float	sb=VSSwapHostFloatToLittle(inmemfile[meshbeginword+24].f32val);//Specular: blue
		  float	sa=VSSwapHostFloatToLittle(inmemfile[meshbeginword+25].f32val);//Specular: Alpha
		  int cullface=VSSwapHostIntToLittle(inmemfile[meshbeginword+26].i32val);//CullFace
		  int lighting=VSSwapHostIntToLittle(inmemfile[meshbeginword+27].i32val);//lighting
		  int reflect=VSSwapHostIntToLittle(inmemfile[meshbeginword+28].i32val);//reflect
		  int usenormals=VSSwapHostIntToLittle(inmemfile[meshbeginword+29].i32val);//usenormals
		  //End Header
		  // Go to Arbitrary Length Attributes section
		  word32index=meshbeginword+(meshheaderlength/4);
		  int VSAbeginword=word32index;
		  int LengthOfArbitraryLengthAttributes=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Length of Arbitrary length attributes section in bytes
		  word32index+=1;
		  fprintf(Outputfile,"<Mesh scale=\"%f\" reverse=\"%d\" forcetexture=\"%d\" sharevert=\"%d\" polygonoffset=\"%f\" blendmode=\"%d %d\" ",scale,reverse,forcetexture,sharevert,polygonoffset,bsrc,bdst);
		  
		  string detailtexturename="";
		  int detailtexturenamelen=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//detailtexture name length
		  word32index+=1;
		  int stringindex=0;
		  int namebound=((detailtexturenamelen+(detailtexturenamelen%4))/4);
		  for(stringindex=0;stringindex<namebound;stringindex++){
			for(int bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			    detailtexturename+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
			  }
			}
			word32index+=1;
		  }
		  fprintf(Outputfile," detailtexture=\"%s\" ",detailtexturename.c_str());

		  vector <XML::vec3f> Detailplanes; //store detail planes until finish printing mesh attributes
		  int numdetailplanes=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of detailplanes
		  word32index+=1;
		  for(int detailplane=0;detailplane<numdetailplanes;detailplane++){
			float x=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//x-coord
			float y=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//y-coord
			float z=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//z-coord
			word32index+=3;
			XML::vec3f temp;
			temp.x=x;
			temp.y=y;
			temp.z=z;
			Detailplanes.push_back(temp);
		  } //End detail planes
		  //Textures
		  int numtextures=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of textures
		  word32index+=1;
		  for(int tex=0;tex<numtextures;tex++){
			int textype=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//texture type
			int texindex=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//texture index
			int texnamelen=VSSwapHostIntToLittle(inmemfile[word32index+2].i32val);//texture name length
			word32index+=3;
			string texname="";
			int namebound=((texnamelen+(texnamelen%4))/4);
			for(stringindex=0;stringindex<namebound;stringindex++){
			  for(int bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			      texname+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
				}
			  }
			  word32index+=1;
			}
			switch(textype){
			case ALPHAMAP:
				fprintf(Outputfile," alphamap");
				break;
			case ANIMATION:
				fprintf(Outputfile," animation");
				break;
			case TEXTURE:
				fprintf(Outputfile," texture");
				break;
			}
			if(texindex){
				fprintf(Outputfile,"%d",texindex);
			}
			fprintf(Outputfile,"=\"%s\" ",texname.c_str());
		  }
		  fprintf(Outputfile,">\n");
		  //End Textures
		  fprintf(Outputfile,"<Material power=\"%f\" cullface=\"%d\" reflect=\"%d\" lighting=\"%d\" usenormals=\"%d\">\n",power,cullface,lighting,reflect,usenormals);
		  fprintf(Outputfile,"\t<Ambient Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",ar,ag,ab,aa);
		  fprintf(Outputfile,"\t<Diffuse Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",dr,dg,db,da);
		  fprintf(Outputfile,"\t<Emissive Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",er,eg,eb,ea);
		  fprintf(Outputfile,"\t<Specular Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",sr,sg,sb,sa);
		  fprintf(Outputfile,"</Material>\n");

		  for(int detplane=0;detplane<Detailplanes.size();detplane++){
			  fprintf(Outputfile,"<DetailPlane x=\"%f\" y=\"%f\" z=\"%f\" />\n",Detailplanes[detplane].x,Detailplanes[detplane].y,Detailplanes[detplane].z);
		  }
		  //Logos
		  int numlogos=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of logos
		  word32index+=1;
		  for(int logo=0;logo<numlogos;logo++){
		    float size=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//size
	        float offset=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//offset
	        float rotation=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//rotation
	        int type=VSSwapHostIntToLittle(inmemfile[word32index+3].i32val);//type
	        int numrefs=VSSwapHostIntToLittle(inmemfile[word32index+4].i32val);//number of reference points
			fprintf(Outputfile,"<Logo type=\"%d\" rotate=\"%f\" size=\"%f\" offset=\"%f\">\n",type,rotation,size,offset);
			word32index+=5;
			for(int ref=0;ref<numrefs;ref++){
		      int refnum=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Logo ref
		      float weight=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//reference weight
			  fprintf(Outputfile,"\t<Ref point=\"%d\" weight=\"%f\"/>\n",refnum,weight);
			  word32index+=2;
			}
			fprintf(Outputfile,"</Logo>\n");
		  }
		  //End logos
		  //LODs
		  int numLODs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of LODs
		  word32index+=1;
		  for(int LOD=0;LOD<numLODs;LOD++){
			float size=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//Size
			int index=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//Mesh index
			fprintf(Outputfile,"<LOD size=\"%f\" meshfile=\"%d_%d.xmesh\"/>\n",size,recordindex,index);
			word32index+=2;
		  }
		  //End LODs
		  //AnimationDefinitions
		  int numanimdefs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation definitions
		  word32index+=1;
		  for(int anim=0;anim<numanimdefs;anim++){
			int animnamelen=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of name
			word32index+=1;
			string animname="";
			int namebound=((animnamelen+(animnamelen%4))/4);
			for(stringindex=0;stringindex<namebound;stringindex++){
			  for(int bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			      animname+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
				}
			  }
			  word32index+=1;
			}
			float FPS=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//FPS
			fprintf(Outputfile,"<AnimationDefinition AnimationName=\"%s\" FPS=\"%f\">\n",animname.c_str(),FPS);
			int numframerefs=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//number of animation frame references
		    word32index+=2;
			for(int fref=0;fref<numframerefs;fref++){
			  int ref=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation frame references
		      word32index+=NUMFIELDSPERREFERENCEDANIMATION;
			  fprintf(Outputfile,"<AnimationFrameIndex AnimationMeshIndex=\"%d\"/>\n",ref);
			}
			fprintf(Outputfile,"</AnimationDefinition>\n");
		  }
		  //End AnimationDefinitions
		  //End VSA
		  //go to geometry
		  word32index=VSAbeginword+(LengthOfArbitraryLengthAttributes/4);
		  //Vertices
		  fprintf(Outputfile,"<Points>\n");
		  int numvertices=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int vert=0;vert<numvertices;vert++){
			float x=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//x
			float y=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//y
			float z=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//z
			float i=VSSwapHostFloatToLittle(inmemfile[word32index+3].f32val);//i
			float j=VSSwapHostFloatToLittle(inmemfile[word32index+4].f32val);//j
			float k=VSSwapHostFloatToLittle(inmemfile[word32index+5].f32val);//k
			float s=VSSwapHostFloatToLittle(inmemfile[word32index+6].f32val);//s
			float t=VSSwapHostFloatToLittle(inmemfile[word32index+7].f32val);//t
		    word32index+=NUMFIELDSPERVERTEX;
			fprintf(Outputfile,"<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n\t<Normal i=\"%f\" j=\"%f\" k=\"%f\"/>\n</Point>\n",x,y,z,s,t,i,j,k);
		  }
		  fprintf(Outputfile,"</Points>\n");
		  //End Vertices
		  //Lines
		  fprintf(Outputfile,"<Polygons>\n");
		  int numlines=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int rvert=0;rvert<numlines;rvert++){
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=1;
			int ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Line flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Line>\n",flatshade,ind1,s1,t1,ind2,s2,t2);
		  }
		  //End Lines
		  //Triangles
		  int numtris=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int rtvert=0;rtvert<numtris;rtvert++){
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=1;
			int ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind3=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float s3=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t3=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Tri flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Tri>\n",flatshade,ind1,s1,t1,ind2,s2,t2,ind3,s3,t3);
		  }
		  //End Triangles
		  //Quads
		  int numquads=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int rqvert=0;rqvert<numquads;rqvert++){
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=1;
			int ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind3=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float s3=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t3=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind4=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float s4=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t4=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Quad flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Quad>\n",flatshade,ind1,s1,t1,ind2,s2,t2,ind3,s3,t3,ind4,s4,t4);
		  }
		  //End Quads
		  //Linestrips
		  int numlinestrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int lstrip=0;lstrip<numlinestrips;lstrip++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Linestrip flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Linestrip>");
		  }
		  //End Linestrips
		  //Tristrips
		  int numtristrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int tstrip=0;tstrip<numtristrips;tstrip++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Tristrip flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Tristrip>");
		  }
		  //End Tristrips
		  //Trifans
		  int numtrifans=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int tfan=0;tfan<numtrifans;tfan++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Trifan flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Trifan>");
		  }
		  //End Trifans
		  //Quadstrips
		  int numquadstrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int qstrip=0;qstrip<numquadstrips;qstrip++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Quadstrip flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Quadstrip>");
		  }
		  //End Quadstrips
		  fprintf(Outputfile,"</Polygons>\n");
		  //End Geometry
		  //go to next mesh
		  fprintf(Outputfile,"</Mesh>\n");
		  word32index=meshbeginword+(meshlength/4);
	  }	
	  //go to next record
	  word32index=recordbeginword+(recordlength/4);
  }
}
