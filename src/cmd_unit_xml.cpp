#include "cmd_unit.h"
#include "xml_support.h"

#include <iostream.h>
#include <fstream.h>
#include <expat.h>

namespace UnitXML {
    enum Names {
      UNKNOWN,
      UNIT,
      SUBUNIT,
      MESHFILE,
      XFILE
    };

  const EnumMap::Pair element_names[] = {
    {"UNKNOWN", UNKNOWN},
    {"Unit", UNIT},
    {"SubUnit", SUBUNIT},
    {"MeshFile", MESHFILE} };

  const EnumMap::Pair attribute_names[] = {
    {"UNKNOWN", UNKNOWN},
    {"file", XFILE} };

  const EnumMap element_map(element_names, 4);
  const EnumMap attribute_map(attribute_names, 2);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace UnitXML;

void Unit::beginElement(const string &name, const AttributeList &attributes) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    break;
  case MESHFILE:
    for(AttributeList::const_iterator iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->meshes.push_back(new Mesh((*iter).value.c_str(), true));
	break;
      }
    }
    break;
  case SUBUNIT:
    for(AttributeList::const_iterator iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->units.push_back(new Unit((*iter).value.c_str(), true));
	break;
      }
    }
    break;    break;
  default:
    ;
  }
}

void Unit::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  default:
    ;
  }
}

void Unit::LoadXML(const char *filename) {
  const int chunk_size = 16384;
  
  ifstream inFile(filename, ios::in | ios::binary);
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new XML;

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
 
  // Load meshes into subunit
  nummesh = xml->meshes.size();
  meshdata = new Mesh*[nummesh];
  for(int a=0; a<xml->meshes.size(); a++) {
    meshdata[a] = xml->meshes[a];
  }

  numsubunit = xml->units.size();
  subunits = new Unit*[numsubunit];
  for(int a=0; a<xml->units.size(); a++) {
    subunits[a] = xml->units[a];
  }
  delete xml;
}

