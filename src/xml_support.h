#ifndef _XML_SUPPORT_H_
#define _XML_SUPPORT_H_

#include <string>
#include <strstream>
#include "hashtable.h"
#include <vector.h>
#include <expat.h>

using std::string;
using std::ostrstream;

namespace XMLSupport {

  struct Attribute {
    string name;
    string value;
    Attribute(string name, string value) : name(name), value(value)
    {};
  };

  class AttributeList : public vector<Attribute> {
  public:
    AttributeList(const XML_Char **atts);
  };

  class EnumMap {
    static inline string strtoupper(const string &foo);

    struct Pair {
      string name;
      int val;
    };
    Hashtable<string,const int> forward;
    Hashtable<string,const string> reverse;
  public:
    EnumMap(const Pair *data, unsigned int num);

    int lookup(const string &str) const;
    const string &lookup(int val) const;
  };

  double parse_float(const string &str);
  int parse_int(const string &str);
  /*
    string tostring(int num);
    string tostring(float num);
  */
  template<class T> inline string tostring(T num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
  }
}
#endif
