struct MeshXML {
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
      STARTFRAME,
      ALPHATEST
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
    class Flightgroup * fg;
    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;
    ///All logos on this unit
    vector <ZeLogo> logos;
    vector<Names> state_stack;
    bool sharevert;
    bool usenormals;
    bool reverse;
    bool force_texture;
    int load_stage;
    int point_state;
    int vertex_state;
	Vector scale;
    Vector lodscale;
    vector <ZeTexture> decals;
    bool recalc_norm;
    int num_vertices;
    vector<GFXVertex> vertices;
    ///keep count to make averaging easy 
    vector<int>vertexcount;
    vector<GFXVertex> lines;
    vector<GFXVertex> tris;
    vector<GFXVertex> quads;
    vector <vector<GFXVertex> > linestrips;
    vector <vector<GFXVertex> > tristrips;
    vector <vector<GFXVertex> > trifans;
    vector <vector<GFXVertex> > quadstrips;
    int tstrcnt;
    int tfancnt;
    int qstrcnt;
    int lstrcnt;
    vector<int> lineind;
    vector<int> nrmllinstrip;
    vector<int> linestripind;
    ///for possible normal computation
    vector<int> triind;
    vector<int> nrmltristrip;
    vector<int> tristripind;
    vector<int> nrmltrifan;
    vector<int> trifanind;
    vector<int> nrmlquadstrip;
    vector<int> quadstripind;
    vector<int> quadind;
    vector<int> trishade;
    vector<int> quadshade;
    vector<int> *active_shade;
    vector<GFXVertex> *active_list;
    vector<int> *active_ind;
    vector <Mesh *> lod;
    vector <float> lodsize;
    GFXVertex vertex;
    GFXMaterial material;
    int faction;
    Mesh * mesh;
};

