#include "cmd/unit_generic.h"
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
extern bool validateHardCodedScript(std::string s);
//serves to run through a XML file that nests things for "and". 



using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
namespace AIEvents {
  AIEvresult::AIEvresult (int type, float const min, const float max, float timetofinish,float timeuntilinterrupts, float priority , const std::string &aiscript) {
	 this->timetointerrupt=timeuntilinterrupts;
	 this->timetofinish=timetofinish;
	 this->type = type; 
         this->priority=priority;
	 this->max = max; 

	 this->min = min; 

	 this->script = aiscript;

         if (!validateHardCodedScript(this->script)) {
           static int aidebug = XMLSupport::parse_int(vs_config->getVariable("AI","debug_level","0"));
           if (aidebug) {
             for (int i=0;i<10;++i) {               
               printf ("SERIOUS WARNING %s\n",this->script.c_str());
               fprintf (stderr,"SERIOUS WARNING: %s\n",this->script.c_str());
             }
           }
           printf ("SERIOUS WARNING in AI script: no fast method to perform %s\nwhen type %d is at least %f and at most %f with priority %f for %f time\n",this->script.c_str(),type,min,max,priority,timetofinish);
         }
  }

  const int  AIUNKNOWN=0;
  const int AIMIN =1;	
  const int AIMAX =2;
  const int AISCRIPT =3;
  const int AINOT=4;
  const int TIMEIT=5;
  const int OBEDIENCE=6;
  const int TIMETOINTERRUPT=7;
  const int PRIORITY=8;
  const XMLSupport::EnumMap::Pair AIattribute_names[] = {
    EnumMap::Pair ("UNKNOWN", AIUNKNOWN),
    EnumMap::Pair ("min", AIMIN), 
    EnumMap::Pair ("max", AIMAX),
    EnumMap::Pair ("not", AINOT),
    EnumMap::Pair ("Script", AISCRIPT),
    EnumMap::Pair ("time", TIMEIT),
    EnumMap::Pair ("obedience", OBEDIENCE),
    EnumMap::Pair ("timetointerrupt", TIMETOINTERRUPT),
    EnumMap::Pair ("priority", PRIORITY)	
  };
  const XMLSupport::EnumMap attr_map(AIattribute_names, 9);

  void GeneralAIEventBegin (void *userData, const XML_Char *name, const XML_Char **atts) {
    AttributeList attributes (atts);
    string aiscriptname ("");
    float min= -FLT_MAX; float max=FLT_MAX;
    ElemAttrMap * eam = ((ElemAttrMap *)userData);
	float timetofinish = eam->maxtime;
	float timetointerrupt = 0;
    int elem = eam->element_map.lookup(name);
    AttributeList::const_iterator iter;
    float priority=4;
    eam->level++;
    if (elem==0) {
      eam->result.push_back(std::list<AIEvresult>());
      eam->result.push_back(std::list<AIEvresult>());
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
      assert (eam->level!=1&&eam->result.size()>=2);//might not have a back on result();
      if (eam->result.back().size()!=eam->result[eam->result.size()-2].size())
	eam->result.push_back(eam->result.back());

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
        case PRIORITY:
          priority = XMLSupport::parse_float((*iter).value);
          break;
	default: 
	  break;
	}
      }
      AIEvresult newelem(elem, min,max,timetofinish,timetointerrupt,priority,aiscriptname);
      eam->result.back().push_back (newelem);
      eam->result[eam->result.size()-2].push_back (newelem);
    }
  }  

  void GeneralAIEventEnd (void *userData, const XML_Char *name) {
    ElemAttrMap * eam = ((ElemAttrMap *)userData);
    eam->level--;    
    if (eam->result.back().size()==0) {
      eam->result.pop_back();     
      assert(eam->level==0);
    }else {
      eam->result.back().pop_back();
    }        
  }  
  void LoadAI(const char * filename, ElemAttrMap &result, const string &faction) {//returns obedience
	using namespace VSFileSystem;
    const int chunk_size = 16384;

    static float cfg_obedience=XMLSupport::parse_float (vs_config->getVariable ("AI",
								      "Targetting",
								      "obedience",
								      ".99"));
    result.obedience=cfg_obedience;

    result.maxtime=10;
    //full_filename = string("ai/events/") + faction+string("/")+filename;
    //FILE * inFile = VSFileSystem::vs_open (full_filename.c_str(), "r");
    VSFile f;
    VSError err;
    err = f.OpenReadOnly (filename, AiFile);
    if(err>Ok) {
      printf("ai file %s not found\n",filename);
      string full_filename=filename;
      full_filename=full_filename.substr(0,strlen(filename)-4);
      string::size_type where = full_filename.find_last_of(".");
      string type=".agg.xml";
      if (where!=string::npos) {
        type = full_filename.substr(where);
        full_filename=full_filename.substr(0,where)+".agg.xml";
        err = f.OpenReadOnly (full_filename, AiFile);              
      }
      if (err>Ok) {
        printf("ai file %s again not found\n",full_filename.c_str());
        full_filename="default";
        full_filename+=type;
        err = f.OpenReadOnly (full_filename, AiFile);
      }
      if (err>Ok) {
        printf("ai file again %s again not found\n",full_filename.c_str());
        err = f.OpenReadOnly ("default.agg.xml", AiFile);
        if (err>Ok) {
          fprintf(stderr,"ai file again default.agg.xml again not found\n");
          return; // Who knows what will happen now? Crash?
        }
      }
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
