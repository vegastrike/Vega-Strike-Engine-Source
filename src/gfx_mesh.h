/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _MESH_H_
#define _MESH_H_

#include <string>
#include <vector>
#include "xml_support.h"
#include "quaternion.h"
#include "gfx_aux.h"
#include "gfx_transform.h"
#include "gfxlib_struct.h"
using std::vector;
using std::string;
class Planet;
class Unit;

struct GFXVertex;
class GFXVertexList;
class GFXQuadstrip;
struct GFXMaterial;
class BoundingBox;
struct bsp_vector {
        float x,y,z;
};
struct bsp_polygon {
    vector <bsp_vector> v;
};

struct MeshFX {
  GFXLight effect;
  float TTL;
};
struct MeshFX;
struct MeshDrawContext {
  float mat[16];
  vector <MeshFX> *SpecialFX;
  MeshDrawContext(const float m[16]) { memcpy(mat, m, sizeof(float[16])); }
};
using XMLSupport::EnumMap;
using XMLSupport::AttributeList;

#define NUM_MESH_SEQUENCE 4
#define MESH_SPECIAL_FX_ONLY 3
class Mesh
{
private:
  // Display list hack
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
      TRI, 
      QUAD,
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
      FLATSHADE,
      TEXTURE,
      ALPHAMAP,
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
      OFFSET

    };
    enum PointState {
      P_X = 0x1,
      P_Y = 0x2,
      P_Z = 0x4,
      P_I = 0x8,
      P_J = 0x10,
      P_K = 0x20
    };
    enum VertexState {
      V_POINT = 0x1,
      V_S = 0x2,
      V_T = 0x4
    };
    enum LogoState {
      V_TYPE = 0x1,
      V_ROTATE = 0x2,
      V_SIZE=0x4,
      V_OFFSET=0x8,
      V_REF=0x10
    };
    struct ZeLogo {
      int type;
      float rotate;
      float size;
      float offset;
      vector <int> refpnt;
      vector <float> refweight;
    };

    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;
    vector <ZeLogo> logos;
    vector<Names> state_stack;
    int load_stage;
    int point_state;
    int vertex_state;
    float scale;
    string decal_name;
    string alpha_name;
    bool recalc_norm;
    int num_vertices;
    vector<GFXVertex> vertices;
    vector<int>vertexcount;//keep count to make averaging easy 
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
    vector<int> triind;//for possible normal computation
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
    GFXVertex vertex;
	GFXMaterial material;
  } *xml;

  void LoadXML(const char *filename, Mesh *oldmesh);
  void LoadBinary (const char * filename, Mesh * oldmesh);
  void CreateLogos();
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

protected:
 
  Vector local_pos; 

  static Hashtable<string, Mesh,char[127]> meshHashTable;
  int refcount;
  Vector mx;//bounding box
  Vector mn;
  float radialSize;
  Mesh *orig;

  Logo *forcelogos;
  int numforcelogo;

  Logo *squadlogos;
  int numsquadlogo;
  
  GFXVertexList *vlist;//tri,quad,line
  
  unsigned int myMatNum;
  Texture *Decal;  
  GFXBOOL envMap;
  GFXBOOL will_be_drawn;  
  enum BLENDFUNC blendSrc;
  enum BLENDFUNC blendDst;
  // Support for reorganized rendering
  vector<MeshDrawContext> *draw_queue;
  int draw_sequence;
  string hash_name;
  void InitUnit();
  friend class OrigMeshContainer;
  vector <MeshFX> LocalFX;
public:
  Mesh();
  Mesh(const char *filename, bool xml=false);
  ~Mesh();
  void EnableSpecialFX();
  void GetPolys(vector <bsp_polygon> &);
  //  void SetPosition (const Vector&);
  Vector &Position() {return local_pos;}
  //  const char *get_name(){return name}
  void Draw(const Transformation &quat = identity_transformation, const Matrix = identity_matrix);
  virtual void ProcessDrawQueue();
  static void ProcessUndrawnMeshes();
  void setEnvMap(GFXBOOL newValue) {envMap = newValue;}
  void UpdateHudMatrix();//puts an object on the hud with the matrix
  Vector corner_min() { return mn; }
  Vector corner_max() { return mx; }
  BoundingBox * getBoundingBox();
  bool queryBoundingBox (const Vector &start,const float err);
  bool queryBoundingBox (const Vector &start, const Vector & end, const float err); 
  float rSize () {return radialSize;}
  void UpdateFX();//based on TTL, etc
  void AddDamageFX (const Vector &LocalPos, const Vector &LocalNorm, const float percentage, const GFXColor &color=GFXColor (1,1,1,1)); 

};
#endif
