#ifndef QUADTREE_H_
#define QUADTREE_H_
#include "quadsquare.h"
#include "xml_support.h"
#include "matrix.h"
class Texture;
/** 
 * This class is a wrapper class for quadsquare
 * It takes care of activating textures, having possession of the vlist
 * and having posession of the static vars that get set
 */
struct TerraXML;
class QuadTree {
  int minX;
  int minZ;
  unsigned int  maxX;
  unsigned int  maxZ;
  void SetXSizes (int mX, unsigned int maxX);
  void SetZSizes (int mZ, unsigned int maxZ);
  Vector Scales;
  float detail;
  IdentityTransform *nonlinear_transform;
  quadcornerdata RootCornerData;
  quadsquare *root;
  quadsquare *neighbors[4];
  std::vector <TerrainTexture> textures;
  std::vector <unsigned int> unusedvertices;
  GFXVertexList vertices;
  unsigned int VertexAllocated;
  unsigned int VertexCount;
  void LoadData();
  void LoadXML (const char * filename, const Vector & scales, const float radius);
  TerraXML * xml;
  void SetNeighbors (quadsquare * east, quadsquare * north, quadsquare *west, quadsquare * south);
 protected:
  Matrix transformation;
 public:
   
  QuadTree (const char * filename, const Vector & scales, const float Radius);
  ~QuadTree();
  void Render();
  void SetNeighbors (QuadTree * east, QuadTree * north, QuadTree *west, QuadTree * south);
  void Update(unsigned short numstages, unsigned short whichstage, updateparity *updateorder=identityparity);
  void SetTransformation (const Matrix &transformation);
  float GetHeight (Vector Location, Vector & normal, const Matrix & transform,   float TotalTerrainSizeX=0, float TotalTerrainSizeZ=0);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  void beginElement(const std::string &name, const XMLSupport::AttributeList &attributes);
  void endElement(const std::string &name);
  float getminX() {return 0;}  float getminZ() {return 0;}
  float getmaxX() {return minX+(float)maxX;}  float getmaxZ() {return minZ+(float)maxZ;}
  float getSizeX() {return maxX;} float getSizeZ () {return maxZ;}
  void StaticCullData(const float detail);
  bool GetGroundPos (QVector &Location, Vector & norm, float TTSX=0,float TTSZ=0) {return GetGroundPos (Location,norm,transformation,TTSX,TTSZ);}
  bool GetGroundPos (QVector &Location, Vector & norm, const Matrix& trans, float TotalTerrainSizeX=0, float TotalTerrainSizeZ=0);
  Vector GetNormal (const Vector & position, const Vector & requestednorm);
};

#endif
