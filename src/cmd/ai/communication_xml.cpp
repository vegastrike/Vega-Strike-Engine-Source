#include <vector>
#include <string>
#include <expat.h>
#include "vegastrike.h"
#include "communication.h"
#include <assert.h>
#include "vsfilesystem.h"
static int unitlevel;
using namespace XMLSupport;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
namespace CommXML {
  enum Names {
	UNKNOWN,
	NODE,
	EDGE,
	NAME,
	INDEX,
	VALUE,
	SOUND,
	SEXE,
	FILENAME
  };

  const EnumMap::Pair element_names[4] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("Node", NODE),
	EnumMap::Pair ("Edge", EDGE),
	EnumMap::Pair ("Sound", SOUND)
  };
  const EnumMap::Pair attribute_names[6] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("Text", NAME), 
	EnumMap::Pair ("Index", INDEX), 
	EnumMap::Pair ("Relationship", VALUE), 
	EnumMap::Pair ("file", FILENAME),
	EnumMap::Pair ("sex", SEXE)
};


  const EnumMap element_map(element_names, 4);
  const EnumMap attribute_map(attribute_names, 6);

}

static vector <std::string> contrabandlists;
void FSM::beginElement(void *userData, const XML_Char *names, const XML_Char **atts) {
	((FSM*)userData)->beginElement(names, AttributeList(atts));
}

extern int createSound( string file, bool val);
void FSM::beginElement(const string &name, const AttributeList attributes) {
using namespace CommXML;
	AttributeList::const_iterator iter;
	Names elem = (Names)element_map.lookup(name);
	string nam;
	float val;
	int sound=-1;
	unsigned char sexe;
	switch(elem) {
	case SOUND:
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			switch(attribute_map.lookup((*iter).name)) {
			case SEXE:
			  sexe =XMLSupport::parse_int ((*iter).value);
			  break;
			case FILENAME:
			  sound=createSound((*iter).value,false);
			  break;
			}
		}
		if (sound!=-1) {
		  nodes.back().AddSound(sound,sexe);
		}
                break;
	case UNKNOWN:
		unitlevel++;
//		cerr << "Unknown element start tag '" << name << "' detected " << endl;
		return;
	case NODE:
		unitlevel++;
                {vector<string>messages;
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
                  if (strtoupper((*iter).name)=="RELATIONSHIP") {
                    val=parse_float((*iter).value);
                  }else {
                    
                    string tmp = strtoupper((*iter).name);
                    int num=0;
                    if (1==sscanf(tmp.c_str(),"TEXT%d",&num)||tmp=="TEXT") {
                      while (!(num<messages.size())) {
                        messages.push_back(string());
                      }
                      nam=(*iter).value;
                      {for (string::iterator i=nam.begin();i!=nam.end();i++) {
                        if (*i=='\\') {
                          *i='\n';
                        }
                      }}
                      messages[num]=nam;
                    }
                  }
		}             
		nodes.push_back(Node(messages,val));
                }
       		break;
	case EDGE:
		unitlevel++;
		for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
			if ((attribute_map.lookup((*iter).name))==INDEX) {
				nodes.back().edges.push_back(parse_int((*iter).value));
			}
		}
		break;
        default :
		break;
	}
}
void FSM::endElement(void *userData, const XML_Char *name) {
using namespace CommXML;
//	((Universe::Faction*)userData)->endElement(name);

//	void Universe::Faction::endElement(const string &name) {
	Names elem = (Names)element_map.lookup(name);
	switch(elem) {
	case UNKNOWN:
		unitlevel--;
//		cerr << "Unknown element end tag '" << name << "' detected " << endl;
		break;
	default:
		unitlevel--;
		break;
	}
}

void FSM::LoadXML(const char * filename) {
 using namespace CommXML;
 using namespace VSFileSystem;
 unitlevel=0;
  //const int chunk_size = 16384; 
  //FILE * inFile = VSFileSystem::vs_open (filename, "r");
  VSFile f;
  VSError err = f.OpenReadOnly( filename, CommFile);
  if(err>Ok) {
	assert(0);
	return;
  }
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &FSM::beginElement, &FSM::endElement);
 
  XML_Parse(parser, (f.ReadFull()).c_str(),f.Size(), 1);
  /*
  do {
	char buf[chunk_size];
	int length;
	length = VSFileSystem::vs_read (buf,1, chunk_size,inFile);
	//length = inFile.gcount();
	XML_Parse(parser, buf,length, VSFileSystem::vs_feof(inFile));
  } while(!VSFileSystem::vs_feof(inFile));
  */
  f.Close();
  XML_ParserFree (parser);
}
