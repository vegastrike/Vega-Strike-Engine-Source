#include "xml_support.h"
#include <vector>
#include <string>
using std::string;
using std::vector;
union XMLType {
  int *i;
  float *f;
  void * p;
  int hardint;
  XMLType (int *myint) {i =myint;}
  XMLType (int myhardint) {hardint=myhardint;}
  XMLType (float  *myfloat) {f =myfloat;}
  XMLType (void * myvoid) {p=myvoid;}
};
typedef std::string (*XMLHandler)(const XMLType &input, void * mythis);
std::string intHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(*input.i);
}
std::string floatHandler (const XMLType &input,void *mythis) {
  return XMLSupport::tostring(*input.f);
}
std::string stringHandler (const XMLType &input,void *mythis) {
  return * ((string *)(input.p));
}

struct XMLElement {
  std::string elem;
  XMLType value;
  XMLHandler handler;
  XMLElement (const std::string &ele, const XMLType &val, XMLHandler hand):value(val) {
    elem = ele;
    handler = hand;
  }
};
struct XMLnode {
  XMLnode *up;
  std::string val;
  vector <XMLElement> elements;
  vector <XMLnode> subnodes;
  XMLnode () {up=NULL;}
  XMLnode (const std::string &val, XMLnode * newup) {this->val=val;up=newup;}

};
class XMLSerializer {
  std::string filename;
  void * mythis;
  XMLnode topnode; 
  XMLnode * curnode;
 public:
  XMLSerializer(const char * filename);
  void AddTag (const std::string &tag);
  void AddElement (const std::string &element, XMLHandler handler, const XMLType &input);
  void EndTag ();
};
