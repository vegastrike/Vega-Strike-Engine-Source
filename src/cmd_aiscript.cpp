#include "cmd_aiscript.h"
#include "xml_support.h"
#include "cmd_flybywire.h"
void AIScript::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((AIScript*)userData)->beginElement(name, AttributeList(atts));
}

void AIScript::endElement(void *userData, const XML_Char *name) {
  ((AIScript*)userData)->endElement(name);
}

namespace AiXml {
    enum Names {
      SCRIPT,
      MOVETO,
      VECTOR,
      TARGET,
      X,
      Y,
      Z,
      ACCURACY,
	  UNKNOWN,
	  EXECUTEFOR,
	  TIME,
	  AFTERBURN,
	  CHANGEHEAD,
	  MATCHLIN,
	  MATCHANG,
	  MATCHVEL,
	  ANGULAR,
	  LINEAR,
	  LOCAL,
	  TERMINATE
    };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Script", SCRIPT),
    EnumMap::Pair ("Vector", VECTOR),
	EnumMap::Pair ("Moveto", MOVETO),
	EnumMap::Pair ("Target", TARGET),
	EnumMap::Pair ("ExecuteFor", EXECUTEFOR),
	EnumMap::Pair ("ChangeHead", CHANGEHEAD),
    EnumMap::Pair ("MatchLin", MATCHLIN), 
    EnumMap::Pair ("MatchAng", MATCHANG), 
    EnumMap::Pair ("MatchVel", MATCHVEL),
    EnumMap::Pair ("Angular", ANGULAR), 
    EnumMap::Pair ("Linear", LINEAR)
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("accuracy", ACCURACY), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z),
	EnumMap::Pair ("Time", TIME),
    EnumMap::Pair ("Terminate", TERMINATE), 
    EnumMap::Pair ("Local", LOCAL) 

};

  const EnumMap element_map(element_names, 12);
  const EnumMap attribute_map(attribute_names, 8);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace AiXml;

void AIScript::beginElement(const string &name, const AttributeList &attributes) {
  Names elem = (Names)element_map.lookup(name);
  AttributeList::const_iterator iter;
  switch(elem) {
  case UNKNOWN:
	xml->unitlevel++;

//    cerr << "Unknown element start tag '" << name << "' detected " << endl;
    return;

  case SCRIPT:
	assert (xml->unitlevel==0);
	xml->unitlevel++;
    break;

  case LINEAR:
  case VECTOR:
	assert (xml->unitlevel>=2);
	xml->unitlevel++;
	xml->vec=Vector(0,0,0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case X:
 	xml->vec.i=parse_float((*iter).value);
 	break;
      case Y:
 	xml->vec.j=parse_float((*iter).value);
 	break;
      case Z:
 	xml->vec.k=parse_float((*iter).value);
 	break;
      }
	}
	break;
  case ANGULAR:
	assert (xml->unitlevel>=2);
	xml->unitlevel++;
	xml->ang=Vector(0,0,0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case X:
 	xml->ang.i=parse_float((*iter).value);
 	break;
      case Y:
 	xml->ang.j=parse_float((*iter).value);
 	break;
      case Z:
 	xml->ang.k=parse_float((*iter).value);
 	break;
      }

    }
    break;
  case MOVETO:
    assert (xml->unitlevel>=1);
    xml->unitlevel++;
	xml->acc = 2;
	xml->afterburn = true;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
	  case AFTERBURN:
	xml->afterburn=parse_bool((*iter).value);
      case ACCURACY:
	xml->acc=parse_int((*iter).value);
	break;
	  }
	}
	break;

  case CHANGEHEAD:
    assert (xml->unitlevel>=1);
    xml->unitlevel++;
	xml->acc = 2;
	xml->afterburn = true;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case ACCURACY:
	xml->acc=parse_int((*iter).value);
	break;
	  }
	}
	break;

  case MATCHLIN:
  case MATCHANG:
  case MATCHVEL:
    assert (xml->unitlevel>=1);
    xml->unitlevel++;
	xml->acc = 2;
	xml->afterburn = true;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case TERMINATE:
	xml->afterburn=parse_bool((*iter).value);
	break;
      case LOCAL:
	xml->acc=parse_bool((*iter).value);
	break;
	  }
	}
	break;

  case EXECUTEFOR:
    assert (xml->unitlevel==1);
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case TIME:
	xml->executefor=parse_int((*iter).value);
	break;
	  }
	}
	break;
	
  default:
	
    break;
  }
}

void AIScript::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);

  switch(elem) {
  case UNKNOWN:
	  xml->unitlevel--;
//    cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;
  case MOVETO:
	  xml->unitlevel--;
	  xml->orders.push_back(new Orders::MoveTo(xml->vec,xml->afterburn,xml->acc));
	  break;
  case CHANGEHEAD:
	  xml->unitlevel--;
	  xml->orders.push_back(new Orders::ChangeHeading(xml->vec,xml->acc));
	  break;
  case MATCHANG:
	  xml->unitlevel--;
	  xml->orders.push_back(new MatchAngularVelocity(xml->ang,((bool)xml->acc),xml->afterburn));
	  break;
  case MATCHLIN:
	  xml->unitlevel--;
	  xml->orders.push_back(new MatchLinearVelocity(xml->vec,((bool)xml->acc),xml->afterburn));
	  break;
  case MATCHVEL:
	  xml->unitlevel--;
	  xml->orders.push_back(new MatchVelocity(xml->vec,xml->ang,((bool)xml->acc),xml->afterburn));
	  break;
  case EXECUTEFOR:
	  xml->unitlevel--;
	  if (xml->executefor>0) {
	  xml->orders[xml->orders.size()-1]=new ExecuteFor(xml->orders[xml->orders.size()-1],xml->executefor);
	  }
	  break;
  default:
	  xml->unitlevel--;
    break;
  }
}


void AIScript::LoadXML() {
  const int chunk_size = 16384;
  FILE * inFile = fopen (filename, "r+b");
  if(!inFile) {
    assert(0);
    return;
  }

  xml = new AIXML;
  xml->unitlevel=0;
  xml->executefor=0;
  xml->acc=2;
  xml->vec=Vector(0,0,0);
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &AIScript::beginElement, &AIScript::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));
  fclose (inFile);
  for (int i=0;i<xml->orders.size();i++) {
	xml->orders[i]->SetParent(parent);
	EnqueueOrder (xml->orders[i]);
  }
  delete xml;
}
AIScript::AIScript (const char * scriptname):Order () {
	filename = new char [strlen (scriptname)+1];
	strcpy(filename,scriptname);

}

AIScript::~AIScript () {

	if (filename) {
			delete [] filename;
	}
}

AI * AIScript::Execute () {
	if (filename) {
		LoadXML ();
		delete [] filename;
		filename = NULL;
	
	}
	return Order::Execute();

}