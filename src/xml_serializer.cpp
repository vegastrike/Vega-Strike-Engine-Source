#include "xml_serializer.h"
#include "cmd/unit.h"
///Assumes that the tag is  <Mount type=\"  and that it will finish with " ></Mount>
using namespace XMLSupport;
std::string Unit::mountSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  int i=input.hardint;
  if (un->nummounts>i) {
    string result(lookupMountSize(un->mounts[i].size));
    if (un->mounts[i].status==Mount::INACTIVE||un->mounts[i].status==Mount::ACTIVE)
      result+=string("\" type=\"")+(un->mounts[i].type.weapon_name);
    if (un->mounts[i].ammo!=-1)
      result+=string("\" ammo=\"")+XMLSupport::tostring(un->mounts[i].ammo);
    
    Matrix m;
    un->mounts[i].GetMountLocation().to_matrix(m);
    result+=string ("\" x=\"")+tostring(m[12]);
    result+=string ("\" y=\"")+tostring(m[13]);
    result+=string ("\" z=\"")+tostring(m[14]);

    result+=string ("\" qi=\"")+tostring(m[4]);
    result+=string ("\" qj=\"")+tostring(m[5]);
    result+=string ("\" qk=\"")+tostring(m[6]);
     
    result+=string ("\" ri=\"")+tostring(m[8]);    
    result+=string ("\" rj=\"")+tostring(m[9]);    
    result+=string ("\" rk=\"")+tostring(m[10]);    
    return result;
  }else {
    return string("");
  }
}
std::string Unit::subunitSerializer (const XMLType &input, void * mythis) {
  Unit * un=(Unit *)mythis;
  int index=input.hardint;
  Unit *su;
  int i=0;
  for (un_iter ui=un->getSubUnits();NULL!= (su=ui.current());++ui,++i) {
    if (i==index) {
      return su->name;
    }
  }
  return string("destroyed_turret");
}
XMLSerializer::XMLSerializer (const char * filename):filename(filename) {
  
}
void XMLSerializer::AddTag (const std::string &tag) {
  curnode->subnodes.push_back (XMLnode(tag,curnode));
}

void XMLSerializer::AddElement (const std::string &element, XMLHandler handler, const XMLType &input) {
  curnode->elements.push_back (XMLElement (element,input,handler));
}

void XMLSerializer::EndTag () {
  curnode = curnode->up;
}
