#include <string.h>
#include <stdlib.h>
//#include <strstream.h>
#include <ctype.h>
#include <assert.h>
#include "xml_support.h"

string strtoupper(const string &foo) {
  string rval;
  string::const_iterator src = foo.begin();
  while(src!=foo.end()) rval += toupper(*src++);
  return rval;
}
string strtolower(const string &foo) {
	
  string rval;
  string::const_iterator src = foo.begin();
  while(src!=foo.end()) rval += tolower(*src++);
  return rval;
}
namespace XMLSupport {
  /*
  string tostring(int num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
  }

  string tostring(float num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
  }
  */

  EnumMap::EnumMap(const Pair *data, unsigned int num) {
    for(unsigned int a=0; a<num; a++) {
      forward.Put(strtoupper(data[a].name), &data[a].val);
      reverse.Put(tostring(data[a].val), &data[a].name);
    }
  }

  int EnumMap::lookup(const string &str) const {
    const int *result = forward.Get(strtoupper(str));
    if(0!=result) return *result;
    else return *forward.Get("UNKNOWN");
  }
  const string &EnumMap::lookup(int val) const {
    const string *result = reverse.Get(tostring(val));
    assert(0!=result);
    return *result;
  }

  AttributeList::AttributeList (const XML_Char **atts) {
    for(;*atts!=NULL;atts+=2) {
      push_back(Attribute(atts[0], atts[1]));
    }
  }


  string replace_space (const string &str) {
    string retval(str);
    for (string::iterator i=retval.begin();i!=retval.end();i++) {
      if (*i==' ') {
	*i='_';
      }
    }
    return retval;
  }
  bool parse_bool (const string &str) {
	if ((*str.begin())=='t'||(*str.begin())=='y'||(*str.begin())=='1') {
		return true;
	} else {
		return false;
	}
  }
  double parse_float(const string &str) {
    return atof (str.c_str());
  }
  int parse_int(const string &str) {
    return atoi (str.c_str());
  }
}
