#ifndef QUADTREE_H_
#define QUADTREE_H_
#include "quadsquare.h"
#include "xml_support.h"
struct Texture;
/** 
 * This class is a wrapper class for quadsquare
 * It takes care of activating textures, having possession of the vlist
 * and having posession of the static vars that get set
 */
struct TerraXML;
class QuadTree {
  float detail;
  IdentityTransform * nonlinear_transform;
  Matrix transformation;
  quadcornerdata RootCornerData;
  quadsquare *root;
  std::vector <TerrainTexture> textures;
  std::vector <unsigned int> unusedvertices;
  GFXVertexList vertices;
  unsigned int VertexAllocated;
  unsigned int VertexCount;
  void LoadData();
  void LoadXML (const char * filename);
  TerraXML * xml;
 public:
  QuadTree (const char * filename, const Vector & scales);
  ~QuadTree();
  void Render();
  void Update(unsigned short numstages, unsigned short whichstage);
  void SetTransformation (const Matrix transformation);
  float GetHeight (Vector Location, Vector & normal);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  void beginElement(const std::string &name, const XMLSupport::AttributeList &attributes);
  void endElement(const std::string &name);


};

#endif
