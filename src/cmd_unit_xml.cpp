#include "cmd_unit.h"
#include "xml_support.h"

//#include <iostream.h>
#include <fstream>
#include <expat.h>
//#include <values.h>
#include <float.h>
#include "gfx_mesh.h"

void Unit::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((Unit*)userData)->beginElement(name, AttributeList(atts));
}

void Unit::endElement(void *userData, const XML_Char *name) {
  ((Unit*)userData)->endElement(name);
}


namespace UnitXML {
    enum Names {
      UNKNOWN,
      UNIT,
      SUBUNIT,
      MESHFILE,
      XFILE,
      X,
      Y,
      Z,
      RI,
      RJ,
      RK,
      QI,
      QJ,
      QK
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Unit", UNIT),
    EnumMap::Pair ("SubUnit", SUBUNIT),
    EnumMap::Pair ("MeshFile", MESHFILE)
  };

  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("file", XFILE), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z), 
    EnumMap::Pair ("ri", RI), 
    EnumMap::Pair ("rj", RJ), 
    EnumMap::Pair ("rk", RK), 
    EnumMap::Pair ("qi", QI),     
    EnumMap::Pair ("qj", QJ),     
    EnumMap::Pair ("qk", QK)
};

  const EnumMap element_map(element_names, 4);
  const EnumMap attribute_map(attribute_names, 11);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace UnitXML;

void Unit::beginElement(const string &name, const AttributeList &attributes) {
    string filename;
    Vector Q;
    Vector R;
    Vector pos;
  Names elem = (Names)element_map.lookup(name);

	AttributeList::const_iterator iter;
  switch(elem) {
  case UNKNOWN:
//    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    break;
  case MESHFILE:
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	xml->meshes.push_back(new Mesh((*iter).value.c_str(), true));
	break;
      }
    }
    break;
  case SUBUNIT:

    Q = Vector (0,1,0);
    R = Vector (0,0,1);
    pos = Vector (0,0,0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case XFILE:
	filename = (*iter).value;
	break;
      case X:
	pos.i=parse_float((*iter).value);
	break;
      case Y:
	pos.j=parse_float((*iter).value);
	break;
      case Z:
	pos.k=parse_float((*iter).value);
	break;
      case RI:
	R.i=parse_float((*iter).value);
	break;
      case RJ:
	R.j=parse_float((*iter).value);
	break;
      case RK:
	R.k=parse_float((*iter).value);
	break;
      case QI:
	Q.i=parse_float((*iter).value);
	break;
      case QJ:
	Q.j=parse_float((*iter).value);
	break;
      case QK:
	Q.k=parse_float((*iter).value);
	break;
      }

      Q.Normalize();
      R.Normalize();
      Vector P;
      CrossProduct (Q,R,P);
      int indx = xml->units.size();
      xml->units.push_back(new Unit (filename.c_str(), true));
      xml->units[indx]->prev_physical_state= Transformation(Quaternion::from_vectors(P,Q,R),pos);
      xml->units[indx]->curr_physical_state=xml->units[indx]->prev_physical_state;
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
//    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  default:
    ;
  }
}

void Unit::LoadXML(const char *filename) {
  const int chunk_size = 16384;
  
  FILE * inFile = fopen (filename, "r+b");
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new XML;
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &Unit::beginElement, &Unit::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
fclose (inFile);
  // Load meshes into subunit
  nummesh = xml->meshes.size();
  meshdata = new Mesh*[nummesh];
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  int a;
  for( a=0; a<nummesh; a++) {
    meshdata[a] = xml->meshes[a];
  }

  numsubunit = xml->units.size();
  subunits = new Unit*[numsubunit];
  for( a=0; a<numsubunit; a++) {
    subunits[a] = xml->units[a];
  }
  
  calculate_extent();
  delete xml;
}

