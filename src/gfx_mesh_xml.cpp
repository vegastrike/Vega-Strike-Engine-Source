#include "gfx_mesh.h"

#include <iostream.h>
#include <fstream.h>
#include <expat.h>
#include "xml_support.h"

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

const EnumMap::Pair Mesh::element_names[] = {
  {"UNKNOWN", UNKNOWN},
  {"Mesh", MESH},
  {"Points", POINTS},
  {"Point", POINT},
  {"Location", LOCATION},
  {"Normal", NORMAL},
  {"Polygons", POLYGONS},
  {"Tri", TRI},
  {"Quad", QUAD},
  {"Vertex", VERTEX} };

const EnumMap Mesh::element_map(element_names, 10);

void Mesh::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((Mesh*)userData)->beginElement(name, AttributeList(atts));
}

void Mesh::endElement(void *userData, const XML_Char *name) {
  ((Mesh*)userData)->endElement(name);
}

void Mesh::beginElement(const string &name, const AttributeList &attributes) {
  cerr << "Start tag: " << name << endl;

  ElementNames elem = (ElementNames)element_map.lookup(name);
  ElementNames top;
  if(xml_state.size()>0) top = *xml_state.rbegin();
  xml_state.push_back(elem);

  switch(elem) {
  case UNKNOWN:
    cerr << "Unknown element start tag " << name << " detected " << endl;
    break;
  case MESH:
    assert(xml_state.size()==1);
    break;
  case POINTS:
    assert(top==MESH);
    break;
  case POINT:
    assert(top==POINTS);
    break;
  case LOCATION:
    assert(top==POINT);
    break;
  case NORMAL:
    assert(top==POINT);
    break;
  case POLYGONS:
    assert(top==MESH);
    break;
  case TRI:
    assert(top==POLYGONS);
    break;
  case QUAD:
    assert(top==POLYGONS);
    break;
  case VERTEX:
    assert(top==TRI || top==QUAD);
    break;
  }
}

void Mesh::endElement(const string &name) {
  cerr << "End tag: " << name << endl;

  ElementNames elem = (ElementNames)element_map.lookup(name);
  assert(*xml_state.rbegin() == elem);
  xml_state.pop_back();

  switch(elem) {
  case UNKNOWN:
    cerr << "Unknown element end tag " << name << " detected " << endl;
    break;
  default:
    ;
  }
}

void Mesh::LoadXML(char *filename) {
  const int chunk_size = 16384;
  
  ifstream inFile(filename, ios::in | ios::binary);
  if(!inFile) {
    assert(0);
    return;
  }

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Mesh::beginElement, &Mesh::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    inFile.read(buf, chunk_size);
    length = inFile.gcount();
    XML_ParseBuffer(parser, length, inFile.eof());
  } while(!inFile.eof());
}

