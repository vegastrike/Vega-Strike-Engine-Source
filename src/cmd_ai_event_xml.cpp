#include "xml_support.h"
#include <string>
#include <vector>
#include <list>
#include <float.h>

//serves to run through a XML file that nests things for "and". 


namespace AIEvent {
#define AIUNKNOWN 0
#define AIMIN 1
#define AIMAX 2
  const EnumMap::Pair AIattribute_names[] = {
    EnumMap::Pair ("UNKNOWN", AIUNKNOWN),
    EnumMap::Pair ("min", AIMIN), 
    EnumMap::Pair ("max", AIMAX),
    EnumMap::Pair ("Script", AISCRIPT)
  };
  const EnumMap attr_map(AIattribute_names, 4);

  void GeneralAIEventBegin (void *userData, const XML_Char *name, const XML_Char **atts) {
    string aiscriptname ("default_behavior.xml");
    float min= -FLT_MAX; float max=FLT_MAX;
    ElemAttrMap * eam = ((ElemAttrMap *)userData);
    int elem = eam->element_map.lookup(name);
    AttributeList::const_iterator iter;
    eam->level++;
    if (elem==eam->result.size()) {

    }else {
      assert (eam->level!=1);//might not have a back on result();
      if (eam->level==2) 
	result.push_back(std::list<AIEvresult>());      

      for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
	switch(attr_map.lookup((*iter).name)) {
	case AIMIN:
	  min=parse_float((*iter).value);
	  break;
	case AIMAX:
	  max = parse_float ((*iter).value);
	  break;
	case AISCRIPT:
	  aiscriptname = (*iter).value;
	default: 
	  break;
	}
      }
      eam->result.back().push_back (AIEvresult(elem, min,max,aiscriptname));
    }
  }  

  void GeneralAIEventEnd (void *userData, const XML_Char *name, const XML_Char **atts) {
      eam->level--;    
    
  }  
  void LoadAI(const char * filename, ElemAttrMap &result) {
    const int chunk_size = 16384;
    FILE * inFile = fopen (filename, "r+b");
    if(!inFile) {
      assert(0);
      return;
    }	
    assert (result.size()==numelem);
    XML_Parser parser = XML_ParserCreate (NULL);
    XML_SetUserData (parser, &result);
    XML_SetElementHandler (parser, &GeneralAIEventBegin, &GeneralAIEventEnd);
    do {
      char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
      int length;    
      length = fread (buf,1, chunk_size,inFile);
      XML_ParseBuffer(parser, length, feof(inFile));
    } while(!feof(inFile));
    fclose (inFile);
    assert (result.level==0);
    result.level =0;
      
  }


}
