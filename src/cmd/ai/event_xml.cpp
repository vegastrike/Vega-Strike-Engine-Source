#include "xml_support.h"
#include "event_xml.h"
#include <string>
#include <vector>
#include <list>
#include <float.h>
#include <assert.h>
#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "configxml.h"
//serves to run through a XML file that nests things for "and". 



using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
namespace AIEvents {
  AIEvresult::AIEvresult (int type, float const min, const float max, float timetofinish,float timeuntilinterrupts, const std::string &aiscript) {
	 this->timetointerrupt=timeuntilinterrupts;
	 this->timetofinish=timetofinish;
	 this->type = type; 

	 this->max = max; 

	 this->min = min; 

	 this->script = aiscript;

  }

  const int  AIUNKNOWN=0;
  const int AIMIN =1;	
  const int AIMAX =2;
  const int AISCRIPT =3;
  const int AINOT=4;
  const int TIMEIT=5;
  const int OBEDIENCE=6;
  const int TIMETOINTERRUPT=7;
  const XMLSupport::EnumMap::Pair AIattribute_names[] = {
    EnumMap::Pair ("UNKNOWN", AIUNKNOWN),
    EnumMap::Pair ("min", AIMIN), 
    EnumMap::Pair ("max", AIMAX),
    EnumMap::Pair ("not", AINOT),
    EnumMap::Pair ("Script", AISCRIPT),
    EnumMap::Pair ("time", TIMEIT),
    EnumMap::Pair ("obedience", OBEDIENCE),
    EnumMap::Pair ("timetointerrupt", TIMETOINTERRUPT)	
  };
  const XMLSupport::EnumMap attr_map(AIattribute_names, 8);

  void GeneralAIEventBegin (void *userData, const XML_Char *name, const XML_Char **atts) {
    AttributeList attributes (atts);
    string aiscriptname ("");
    float min= -FLT_MAX; float max=FLT_MAX;
    ElemAttrMap * eam = ((ElemAttrMap *)userData);
	float timetofinish = eam->maxtime;
	float timetointerrupt = 0;
    int elem = eam->element_map.lookup(name);
    AttributeList::const_iterator iter;
    eam->level++;
    if (elem==0) {
      for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
	switch(attr_map.lookup((*iter).name)) {
	case TIMEIT:
	  eam->maxtime=(short)XMLSupport::parse_float((*iter).value);
	  break;
	case OBEDIENCE:
	  eam->obedience=(float)(XMLSupport::parse_float((*iter).value));
	}
      }
    }else {
      assert (eam->level!=1);//might not have a back on result();
      if (eam->level==2) 
	eam->result.push_back(std::list<AIEvresult>());

      for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
	switch(attr_map.lookup((*iter).name)) {
	case AINOT:
	  elem = -elem;//since elem can't be 0 (see above) this will save the "not" info
	  break;
	case AIMIN:
	  min=XMLSupport::parse_float((*iter).value);
	  break;
	case AIMAX:
	  max = XMLSupport::parse_float ((*iter).value);
	  break;
	case AISCRIPT:
	  aiscriptname = (*iter).value;
	  break;
	case TIMEIT:
		timetofinish=XMLSupport::parse_float ((*iter).value);
		break;
	case TIMETOINTERRUPT:
		timetointerrupt=XMLSupport::parse_float((*iter).value);
		break;
	default: 
	  break;
	}
      }
      eam->result.back().push_back (AIEvresult(elem, min,max,timetofinish,timetointerrupt,aiscriptname));
    }
  }  

  void GeneralAIEventEnd (void *userData, const XML_Char *name) {
    ((ElemAttrMap *)userData)->level--;    
    
  }  
  void LoadAI(const char * filename, ElemAttrMap &result, const string &faction) {//returns obedience
	using namespace VSFileSystem;
    const int chunk_size = 16384;
    string full_filename;
    result.obedience=XMLSupport::parse_float (vs_config->getVariable ("AI",
								      "Targetting",
								      "obedience",
								      ".99"));
    result.maxtime=10;
    //full_filename = string("ai/events/") + faction+string("/")+filename;
    full_filename = faction+string("/")+filename;
    //FILE * inFile = VSFileSystem::vs_open (full_filename.c_str(), "r");
	VSFile f;
	VSError err = f.OpenReadOnly( full_filename);
    if (err>Ok) {
      full_filename = filename;
      err = f.OpenReadOnly (full_filename, AiFile);
    }
    if(err>Ok) {
      printf("ai file %s not found\n",filename);
      assert(0);
      return;
    }	
    XML_Parser parser = XML_ParserCreate (NULL);
    XML_SetUserData (parser, &result);
    XML_SetElementHandler (parser, &GeneralAIEventBegin, &GeneralAIEventEnd);
    XML_Parse(parser, (f.ReadFull()).c_str(), f.Size(), 1);
	/*
    do {
      char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
      int length;    
      length = VSFileSystem::vs_read (buf,1, chunk_size,inFile);
      XML_ParseBuffer(parser, length, VSFileSystem::vs_feof(inFile));
    } while(!VSFileSystem::vs_feof(inFile));
	*/
	f.Close();
	XML_ParserFree (parser);
//    assert (result.level==0);
	if (result.level!=0) {
		fprintf (stderr,"Error loading AI script %s for faction %s. Final count not zero.\n",filename,faction.c_str());
	}
    result.level =0;
      
  }


}
