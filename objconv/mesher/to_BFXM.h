#ifndef _TO_BFXM_H_
#define _TO_BFXM_H_
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
    
    ///holds a logo
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

    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;
    vector<Names> state_stack;
    vector<GFXVertex> vertices;
    vector<int> num_vertex_references;
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

	Mesh_vec3f detailplane;
	vector <Mesh_vec3f> detailplanes;
	

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

XML LoadXML(const char *filename, float unitscale);
void xmeshToBFXM(XML memfile,FILE* Outputfile,char mode); //converts input file to BFXM creates new, or appends record based on mode
int writesuperheader(XML memfile, FILE* Outputfile); //Writes superheader to file Outputfile
int appendrecordfromxml(XML memfile, FILE* Outputfile); // Append a record specified in memfile to the output file and return number of bytes written. Assumes Outputfile is appropriately positioned at the end of the file.
int appendmeshfromxml(XML memfile, FILE* Outputfile); // Append a mesh specified in memfile to the output file and return number of bytes written. Assumes Outputfile is appropriately positioned at the end of the file.
void AddNormal (GFXVertex &outp,
                const GFXVertex &inp);
void SetNormal (GFXVertex &outp,
                const GFXVertex &a,
                const GFXVertex &b,
                const GFXVertex &c);



#endif
