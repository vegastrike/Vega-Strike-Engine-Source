#include <expat.h>
#include "xml_support.h"
#include "vegaconfig.h"

#include "vs_globals.h"
#include "vegastrike.h"

void VegaConfig::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((VegaConfig*)userData)->beginElement(name, AttributeList(atts));
}

void VegaConfig::endElement(void *userData, const XML_Char *name) {
  ((VegaConfig*)userData)->endElement(name);
}

namespace VegaConfigXML {
    enum Names {
    };

  const EnumMap::Pair element_names[] = {
  };
  const EnumMap::Pair attribute_names[] = {
  };
};

  const EnumMap element_map(element_names, 5);
  const EnumMap attribute_map(attribute_names, 28);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace VegaConnfigXML;

void VegaConfig::beginElement(const string &name, const AttributeList &attributes) {

  Names elem = (Names)element_map.lookup(name);


  AttributeList::const_iterator iter;
  switch(elem) {
  case UNKNOWN:
	xml->unitlevel++;

//    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    return;


}

void VegaConfig::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
	  xml->unitlevel--;
//    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  default:
	  xml->unitlevel--;
    break;
  }
}


void VegaConfig::LoadXML(const char *filename) {

  const int chunk_size = 16384;

  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new VegaConfigXML;

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &VegaConfig::beginElement, &VegaConfig::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);

}

