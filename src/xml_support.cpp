#include <string.h>
#include <strstream.h>
#include <ctype.h>
#include <assert.h>
#include "xml_support.h"

namespace XMLSupport {
  string int_to_string(int num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
  }

  string EnumMap::strtoupper(const string &foo) {
    string rval;
    string::const_iterator src = foo.begin();
    while(src!=foo.end()) rval += toupper(*src++);
    return rval;
  }

  EnumMap::EnumMap(const Pair *data, unsigned int num) {
    for(unsigned int a=0; a<num; a++) {
      forward.Put(strtoupper(data[a].name), &data[a].val);
      reverse.Put(int_to_string(data[a].val), &data[a].name);
    }
  }

  int EnumMap::lookup(const string &str) const {
    const int *result = forward.Get(strtoupper(str));
    if(0!=result) return *result;
    else return *forward.Get("UNKNOWN");
  }
  const string &EnumMap::lookup(int val) const {
    const string *result = reverse.Get(int_to_string(val));
    assert(0!=result);
    return *result;
  }

  AttributeList::AttributeList (const XML_Char **atts) {
    for(;*atts!=NULL;atts+=2) {
      push_back(Attribute(atts[0], atts[1]));
    }
  }
}
