#include "xml_serializer.h"
#include "cmd/unit_generic.h"
#include "cmd/images.h"
#include "vs_path.h"
#ifdef _SERVER
	#include "configxml.h"
#else
	#include "config_xml.h"
#endif
#include "vs_globals.h"
#include "vegastrike.h"
///Assumes that the tag is  <Mount type=\"  and that it will finish with " ></Mount>
using namespace XMLSupport;

std::string intStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(*input.w.i);
}
std::string floatStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(*input.w.f);
}
std::string fabsFloatStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring((float)fabs(*input.w.f));
}
std::string absIntStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring((int)abs(*input.w.i));
}
std::string absShortStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring((int)abs(*input.w.s));
}
std::string scaledFloatStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring((float)((*input.w.f)/XMLSupport::parse_float(input.str)));
}

std::string angleStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring((float((*input.w.f)*180/3.1415926536)));
}
std::string doubleStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring ((float)(*input.w.d));
}
std::string boolStarHandler (const XMLType &input,void *mythis) {
  if (*input.w.b) {
    return "1";
  }
  return "0";
}
std::string charStarHandler (const XMLType &input, void*mythis) {
  return XMLSupport::tostring(*input.w.c);
}
std::string ucharStarHandler (const XMLType &input, void*mythis) {
  return XMLSupport::tostring(*input.w.uc);
}
std::string shortStarHandler (const XMLType &input, void*mythis) {
  return XMLSupport::tostring(*input.w.s);
}
std::string ushortStarHandler (const XMLType &input, void*mythis) {
  return XMLSupport::tostring(*input.w.us);
}
std::string negationCharStarHandler (const XMLType &input, void*mythis) {
  return XMLSupport::tostring(-(*input.w.c));
}
std::string negationIntStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(-(*input.w.i));
}
std::string negationFloatStarHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(-(*input.w.f));
}
std::string stringStarHandler (const XMLType &input,void *mythis) {
  if (!input.w.p) {
    return string ("");
  }
  return * ((string *)(input.w.p));
}
std::string stringHandler (const XMLType &input,void *mythis) {
  return input.str;
}
std::string intHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(input.w.hardint);
}
std::string floatHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(input.w.hardfloat);
}
std::string lessNeg1Handler (const XMLType &input, void *mythis) {
  return XMLSupport::tostring (((*input.w.c)<-1)?1:0);
}

std::string cloakHandler(const XMLType &input, void *mythis) { 
  return XMLSupport::tostring (((*input.w.s)==-1)?1:0);
}
std::string shortToFloatHandler(const XMLType &input, void *mythis) { 
  return XMLSupport::tostring ((float)(((float)(*input.w.s))/32767.));
}

void XMLElement::Write (FILE * fp, void * mythis) {
  fprintf (fp," %s=\"%s\"",elem.c_str(),((*handler)(value,mythis)).c_str());
}
static void Tab (FILE * fp) {
  fprintf (fp,"\t");
}
static void Tab (FILE * fp, int level) {
  for (int i=0;i<level;i++) {
    Tab (fp);
  }
}
void XMLnode::Write (FILE* fp, void *mythis, int level) {
  Tab(fp,level);fprintf (fp,"<%s",val.c_str());
  for (unsigned int i=0;i<elements.size();i++) {
    elements[i].Write (fp,mythis);
  }
  if (subnodes.empty()) {
    fprintf (fp,"/>\n");    
  } else {
    fprintf (fp,">\n");
    for (unsigned int i=0;i<subnodes.size();i++) {
      subnodes[i].Write (fp,mythis,level+1);
    }
    Tab(fp,level);fprintf (fp,"</%s>\n",val.c_str());
  }
}
void XMLSerializer::Write (const char * modificationname) {
  std::string savedunitpath;
  if (modificationname)
    if (strlen(modificationname)!=0) 
      savedir=modificationname;

  // If in network mode on client side we expect saves to be in ./save
  if( Network==NULL && !SERVER)
	  savedunitpath=vs_config->getVariable ("data","serialized_xml","serialized_xml");
  else if( !SERVER)
	  savedunitpath = "save";
  // With account server we expect them in the ./accounts dir
  else if( SERVER==2)
	  savedunitpath = "accounts";
  // With account server we expect them in the ./accountstmp dir
  else if( SERVER==1)
	  savedunitpath = "accountstmp";

  MakeSharedPath (savedunitpath);
  string retdir =MakeSharedPath (savedunitpath+string("/")+savedir);
  FILE * fp =fopen ((retdir+string("/")+filename).c_str(),"w");
  if (!fp) {
    return;
  }
  for (unsigned int i=0;i<topnode.subnodes.size();i++) {
    topnode.subnodes[i].Write (fp,mythis,0);
  }
  fclose (fp);
}
XMLSerializer::XMLSerializer (const char * filename, const char *modificationname, void *mythis): savedir(modificationname), mythis(mythis) {
  curnode=&topnode;
  // In network mode we don't care about saving filename, we want always to save with modification
  // name since we only work with savegames
  if( Network!=NULL)
	  this->filename = string( modificationname);
  else
	  this->filename = string( filename);
}
void XMLSerializer::AddTag (const std::string &tag) {
  curnode->subnodes.push_back (XMLnode(tag,curnode));
  curnode = &curnode->subnodes.back();
}

void XMLSerializer::AddElement (const std::string &element, XMLHandler *handler, const XMLType &input) {
  curnode->elements.push_back (XMLElement (element,input,handler));
}

void XMLSerializer::EndTag (const std::string endname) {
  if (curnode)
    if (endname==curnode->val) 
      curnode = curnode->up;
}
