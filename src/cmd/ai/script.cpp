#include "script.h"
#include "navigation.h"
#include "xml_support.h"
#include "flybywire.h"
#include <stdio.h>
#include <vector>
#include <stack>

struct AIScriptXML {
  int unitlevel;
  int acc;
  float executefor;
  bool itts;
  bool afterburn;
  bool terminate;
  char lin;
  Vector defaultvec;
  float defaultf;
  std::stack <Vector> vectors;
  std::stack <float> floats;
  std::vector <Order *> orders;
};
float& AIScript::topf(){
  if (!xml->floats.size()) {
    xml->floats.push(xml->defaultf);
    fprintf(stderr,"\nERROR: Float stack is empty... Will return %f\n",xml->defaultf);
  }
  return xml->floats.top();
}
void AIScript::popf(){
  if (xml->floats.size()<=0) {
    fprintf(stderr,"\nERROR: Float stack is empty... Will not delete\n");
    return;
  }
  xml->floats.pop();
}
Vector& AIScript::topv(){
  if (!xml->vectors.size()) {
    xml->vectors.push(xml->defaultvec);
    fprintf(stderr,"\nERROR: Vector stack is empty... Will return <%f, %f, %f>\n",xml->defaultvec.i,xml->defaultvec.j,xml->defaultvec.k);
  }
  return xml->vectors.top();
}
void AIScript::popv (){
  if (xml->vectors.size()<=0) {
    fprintf(stderr,"\nERROR: Vector stack is empty... Will not delete\n");
    return;
  }
  xml->vectors.pop();
}

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
    FFLOAT,
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
    TERMINATE,
    VALUE,
    ADD,
    SUB,
    NEG,
    NORMALIZE,
    SCALE,
    CROSS,
    DOT,
    MULTF,
    ADDF,
    FROMF,
    TOF,
    FACETARGET,
    ITTTS,
    TARGETPOS,
    THREATPOS,
    YOURPOS,
    TARGETV,
    THREATV,
    YOURV,
    TARGETWORLD,
    THREATWORLD,
    TARGETLOCAL,
    THREATLOCAL,
    YOURLOCAL,
    YOURWORLD,
    SIMATOM,
    DUPLIC,
    DEFAULT
  };

  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("Float", FFLOAT),
    EnumMap::Pair ("Script", SCRIPT),
    EnumMap::Pair ("Vector", VECTOR),
    EnumMap::Pair ("Moveto", MOVETO),
    EnumMap::Pair ("Default", DEFAULT),
    EnumMap::Pair ("Targetworld", TARGETWORLD),
    EnumMap::Pair ("Yourworld", YOURWORLD),
    EnumMap::Pair ("Targetlocal", TARGETLOCAL),
    EnumMap::Pair ("Yourlocal", YOURLOCAL),
    EnumMap::Pair ("FaceTarget", FACETARGET),
    EnumMap::Pair ("ExecuteFor", EXECUTEFOR),
    EnumMap::Pair ("ChangeHead", CHANGEHEAD),
    EnumMap::Pair ("MatchLin", MATCHLIN), 
    EnumMap::Pair ("MatchAng", MATCHANG), 
    EnumMap::Pair ("MatchVel", MATCHVEL),
    EnumMap::Pair ("Angular", ANGULAR), 
    EnumMap::Pair ("Add", ADD),
    EnumMap::Pair ("Neg", NEG),
    EnumMap::Pair ("Sub", SUB),    
    EnumMap::Pair ("Normalize", NORMALIZE),
    EnumMap::Pair ("Scale", SCALE),
    EnumMap::Pair ("Cross", CROSS),
    EnumMap::Pair ("Dot", DOT),
    EnumMap::Pair ("Multf", MULTF),
    EnumMap::Pair ("Addf", ADDF),
    EnumMap::Pair ("Fromf", FROMF),
    EnumMap::Pair ("Tof", TOF),
    EnumMap::Pair ("Linear", LINEAR),
    EnumMap::Pair ("Threatworld", THREATWORLD),
    EnumMap::Pair ("Threatlocal", THREATLOCAL)
    
  };
  const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair ("UNKNOWN", UNKNOWN),
    EnumMap::Pair ("accuracy", ACCURACY), 
    EnumMap::Pair ("x", X), 
    EnumMap::Pair ("y", Y), 
    EnumMap::Pair ("z", Z),
    EnumMap::Pair ("Time", TIME),
    EnumMap::Pair ("Terminate", TERMINATE), 
    EnumMap::Pair ("Local", LOCAL), 
    EnumMap::Pair ("Value", VALUE),
    EnumMap::Pair ("ITTS", ITTTS),
    EnumMap::Pair ("Afterburn", AFTERBURN),
    EnumMap::Pair ("Position", YOURPOS), 
    EnumMap::Pair ("TargetPos", TARGETPOS),
    EnumMap::Pair ("ThreatPos", THREATPOS),
    EnumMap::Pair ("Velocity", YOURV), 
    EnumMap::Pair ("TargetV", TARGETV),
    EnumMap::Pair ("ThreatV", THREATV),
    EnumMap::Pair ("SimlationAtom", SIMATOM),
    EnumMap::Pair ("Dup", DUPLIC)

  };

  const EnumMap element_map(element_names, 31);
  const EnumMap attribute_map(attribute_names, 19);
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace AiXml;

void AIScript::beginElement(const string &name, const AttributeList &attributes) {
  xml->itts=false;
  Unit * tmp;
  Names elem = (Names)element_map.lookup(name);
  AttributeList::const_iterator iter;
  switch(elem) {
  case DEFAULT:
    xml->unitlevel+=2;//pretend it's at a reasonable level
    break;
  case UNKNOWN:
    xml->unitlevel++;

    //	  cerr << "Unknown element start tag '" << name << "' detected " << endl;
    return;

  case SCRIPT:
    xml->unitlevel++;
    break;

  case LINEAR:
    xml->lin=1;
  case ANGULAR:
  case VECTOR:
    xml->unitlevel++;
    xml->vectors.push(Vector(0,0,0));
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case X:
	topv().i=parse_float((*iter).value);
	break;
      case Y:
	topv().j=parse_float((*iter).value);
	break;
      case Z:
	topv().k=parse_float((*iter).value);
	break;
      case DUPLIC:
	xml->vectors.pop();//get rid of dummy vector pushed on above
	xml->vectors.push (xml->vectors.top());
	break;
      case THREATPOS:
	if((tmp = this->parent->Threat())) {
	  topv() =(tmp->Position());
	} else {
	  if ((tmp = this->parent->Target())) {
	    topv()= (tmp->Position());
	  } else {
	    topv()=(xml->defaultvec);
	  }
	}
	break;
      case TARGETPOS:
	if((tmp = this->parent->Target())) {
	  topv()=(tmp->Position());
	} else {
	  topv()=(xml->defaultvec);
	}	
	break;
	
      case YOURPOS:
	topv()=(this->parent->Position());	  
	break;

      case THREATV:
	if((tmp = this->parent->Threat())) {
	  topv() =(tmp->GetVelocity());
	} else {
	  if ((tmp = this->parent->Target())) {
	    topv()= (tmp->GetVelocity());
	  } else {
	    topv()=(xml->defaultvec);
	  }
	}
	break;
      case TARGETV:
	if((tmp = this->parent->Target())) {
	  topv()=(tmp->GetVelocity());
	} else {
	  topv()=(xml->defaultvec);
	}
	break;
	
      case YOURV:
	topv()=(this->parent->GetVelocity());	  
	break;
      }
    }
    break;
  case MOVETO:
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
  case FACETARGET:
    xml->unitlevel++;
    xml->acc =3;
    xml->itts=false;
    xml->afterburn = true;
    xml->terminate=true;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case TERMINATE:
	xml->terminate=parse_bool ((*iter).value);
	break;
      case ACCURACY:
	xml->acc=parse_int((*iter).value);
	break;
      case ITTTS:
	xml->itts=parse_bool ((*iter).value);
	break;
      }
    }
    break;
    
  case CHANGEHEAD:
    xml->unitlevel++;
    xml->acc = 2;
    xml->afterburn = true;
    xml->terminate = true;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case TERMINATE:
	xml->terminate=parse_bool ((*iter).value);
	break;
      case ACCURACY:
	xml->acc=parse_int((*iter).value);
	break;
      }
    }
    break;
    /*
      case THREATPOS:
      xml->unitlevel++;
      if((tmp = this->parent->Threat())) {
      xml->vectors.push(tmp->Position());
      } else {
      if ((tmp = this->parent->Target())) {
      xml->vectors.push (tmp->Position());
      } else {
      xml->vectors.push(xml->defaultvec);
      }
      }
      break;
      case TARGETPOS:
      xml->unitlevel++;
      if((tmp = this->parent->Target())) {
      xml->vectors.push(tmp->Position());
      } else {
      xml->vectors.push(xml->defaultvec);
      }
      break;
    */
  case YOURPOS:
    xml->unitlevel++;
    xml->vectors.push(this->parent->Position());
	  
    break;
  case THREATWORLD:
    xml->unitlevel++;
    break;
  case TARGETWORLD:
    xml->unitlevel++;
    break;
  case THREATLOCAL:
    xml->unitlevel++;
    break;
  case TARGETLOCAL:
    xml->unitlevel++;
    break;
  case YOURLOCAL:
    xml->unitlevel++;
    break;
  case YOURWORLD:
    xml->unitlevel++;
    break;

  case NORMALIZE:
  case SCALE:
  case CROSS:
  case DOT:
  case MULTF:
  case ADDF:
  case FROMF:
  case TOF:  
  case ADD:
  case SUB:
  case NEG:
    xml->unitlevel++;
    break;

  case FFLOAT:
    xml->unitlevel++;
    xml->floats.push(0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case VALUE:
	topf()=parse_float((*iter).value);
	break;
      case SIMATOM:
	topf()= SIMULATION_ATOM;
      case DUPLIC:
	xml->floats.pop();//get rid of dummy vector pushed on above
	xml->floats.push (xml->floats.top());
	break;
      }
    }
    break;
  case MATCHLIN:
  case MATCHANG:
  case MATCHVEL:
    xml->unitlevel++;
    xml->acc = 0;
    xml->afterburn = false;
    xml->terminate=true;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case AFTERBURN:
	xml->afterburn=parse_bool((*iter).value);
	break;
      case TERMINATE:
	xml->terminate=parse_bool((*iter).value);
	break;
      case LOCAL:
	xml->acc=parse_bool((*iter).value);
	break;
      }
    }
    break;

  case EXECUTEFOR:
    xml->unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case TIME:
	xml->executefor=parse_float((*iter).value);
	break;
      }
    }
    break;
	
  default:
	
    break;
  }
}

void AIScript::endElement(const string &name) {
  Vector temp (0,0,0);
  Names elem = (Names)element_map.lookup(name);
  Unit * tmp;
  switch(elem) {
  case UNKNOWN:
    xml->unitlevel--;
    //	  cerr << "Unknown element end tag '" << name << "' detected " << endl;
    break;

    // Vector 
  case THREATWORLD:
    xml->unitlevel++;
    if((tmp = parent->Threat())) {
      xml->vectors.push(tmp->ToWorldCoordinates (topv()));
    } else {
      if((tmp = parent->Target())) {
	xml->vectors.push(tmp->ToWorldCoordinates (topv()));
      } else {
	xml->vectors.push(xml->defaultvec);
      }
    }
    break;
  case THREATLOCAL:
    xml->unitlevel++;
    if((tmp = parent->Threat())) {
      xml->vectors.push(tmp->ToLocalCoordinates(topv()));
    } else {
      if((tmp = parent->Target())) {
	xml->vectors.push(tmp->ToLocalCoordinates (topv()));
      } else {
	xml->vectors.push(xml->defaultvec);
      }
    }
    break;

  case TARGETWORLD:
    xml->unitlevel++;
    if((tmp = parent->Target())) {
      xml->vectors.push(tmp->ToWorldCoordinates (topv()));
    } else {
      xml->vectors.push(xml->defaultvec);
    }
    break;
  case TARGETLOCAL:
    xml->unitlevel++;
    if((tmp = parent->Target())) {
      xml->vectors.push(tmp->ToLocalCoordinates(topv()));
    } else {
      xml->vectors.push(xml->defaultvec);
    }
    break;
  case YOURLOCAL:
    xml->unitlevel++;
    xml->vectors.push(this->parent->ToLocalCoordinates(topv()));
    break;
  case YOURWORLD:
    xml->unitlevel++;
    xml->vectors.push(this->parent->ToWorldCoordinates(topv()));
    break;

  case NORMALIZE:
    xml->unitlevel--;
    if (topv().i||topv().j||topv().k) {
      topv().Normalize();
    }
    break;
  case SCALE:
    xml->unitlevel--;
    topv() *= topf();
    popf();
    break;
  case CROSS:
    xml->unitlevel--;
    temp = topv();
    popv();
    topv() = CrossProduct(topv(),temp);
    break;
  case DOT:
    xml->unitlevel--;
    xml->floats.push(0);
    temp = topv();
    popv();
    topf() = DotProduct(topv(),temp);
    popv();
    break;
  case MULTF:
    xml->unitlevel--;
    temp.i = topf();
    popf();
    topf()*=temp.i;
    break;
  case ADDF:
    xml->unitlevel--;
    temp.i = topf();
    popf();
    topf()+=temp.i;
    break;
  case FROMF:
    xml->unitlevel--;
    temp.i = topf();
    popf();
    temp.j = topf();
    popf();
    xml->vectors.push(Vector(temp.i,temp.j,topf()));
    popf();
    break;
  case TOF:  
    xml->unitlevel--;
    xml->floats.push(topv().i);
    xml->floats.push(topv().j);
    xml->floats.push(topv().k);
    popv();
    break;
  case ADD:
    xml->unitlevel--;
    temp = topv();
    popv();
    topv()+=temp;
    break;
  case SUB:
    xml->unitlevel--;
    temp = topv();
    popv();
    topv()=topv()-temp;
    break;
  case NEG:
    xml->unitlevel--;
    topv()=-topv();
    break;
  case MOVETO:
    fprintf (stderr,"Moveto <%f,%f,%f>",topv().i,topv().j,topv().k);
    xml->unitlevel--;
    xml->orders.push_back(new Orders::MoveTo(topv(),xml->afterburn,xml->acc));
    popv();
    break;
  case CHANGEHEAD:
    xml->unitlevel--;
    xml->orders.push_back(new Orders::ChangeHeading(topv(),xml->acc));
    popv();
    break;
  case MATCHANG:
    xml->unitlevel--;
    xml->orders.push_back(new Orders::MatchAngularVelocity(parent->ClampAngVel(topv()),((bool)xml->acc),xml->terminate));
    popv();
    break;
  case FACETARGET:
    xml->unitlevel--;
    if (xml->itts) {
      xml->orders.push_back (new Orders::FaceTargetITTS (xml->terminate, 
							 (bool)xml->acc));
    }else {
      xml->orders.push_back (new Orders::FaceTarget (xml->terminate, 
						     (bool)xml->acc));
    }
    break;
  case MATCHLIN:
    xml->unitlevel--;
    xml->orders.push_back(new Orders::MatchLinearVelocity(parent->ClampVelocity(topv(),xml->afterburn),((bool)xml->acc),xml->terminate));
    popv();
    break;
  case MATCHVEL:
    xml->unitlevel--;
    temp=topv();
    popv();
    if (xml->lin==1) {
      xml->orders.push_back(new Orders::MatchVelocity(parent->ClampVelocity(topv(),xml->afterburn),parent->ClampAngVel(temp),((bool)xml->acc),xml->terminate));
    } else {
      xml->orders.push_back(new Orders::MatchVelocity(parent->ClampVelocity(temp,xml->afterburn),parent->ClampAngVel(topv()),((bool)xml->acc),xml->terminate));
    }
    xml->lin=0;
    popv();
    break;
  case EXECUTEFOR:
    xml->unitlevel--;
    if (xml->executefor>0) {
      xml->orders[xml->orders.size()-1]=new ExecuteFor(xml->orders[xml->orders.size()-1],xml->executefor);
    }
    break;
  case DEFAULT:
    xml->unitlevel-=2;
    xml->defaultvec=topv();
    popv();
    xml->defaultf=topf();
    popf();
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
    return;
  }
#ifndef _WIN32
  fprintf (stderr, "Loading AIscript: %s\n", filename);
#endif
  xml = new AIScriptXML;
  xml->unitlevel=0;
  xml->terminate=true;
  xml->afterburn=true;
  xml->executefor=0;
  xml->acc=2;
  xml->defaultvec=Vector(0,0,0);
  xml->defaultf=0;
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
  for (unsigned int i=0;i<xml->orders.size();i++) {
    xml->orders[i]->SetParent(parent);
    EnqueueOrder (xml->orders[i]);
  }
  delete xml;
}
AIScript::AIScript (const char * scriptname):Order (Order::MOVEMENT|Order::FACING){
  filename = new char [strlen (scriptname)+1];
  strcpy(filename,scriptname);

}

AIScript::~AIScript () {

  if (filename) {
    delete [] filename;
  }
}

void AIScript::Execute () {
  if (filename) {
    LoadXML ();
    delete [] filename;
    filename = NULL;
	
  }
  Order::Execute();

}
