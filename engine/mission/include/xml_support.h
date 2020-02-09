#ifndef _XML_SUPPORT_H_

#define _XML_SUPPORT_H_
#include <stdio.h>
#include <string>
#ifndef WIN32
#include <strstream>
#endif
#include "hashtable.h"
#include <vector>
#include <expat.h>
#include <iostream>		// needed for cout calls in config_xml.cpp (and other places too i'm sure)


#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

using std::string;
#ifndef WIN32
using std::ostrstream;
#endif

string strtoupper(const string &foo);

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
  double parse_float(const string &str);
  int parse_int(const string &str);
  bool parse_bool (const string &str);

  class EnumMap {

    //    static inline double parse_float (const string &str) {return ::parse_float (str)};
    Hashtable<string,const int, char[1001]> forward;
    Hashtable<string,const string, char[1001]> reverse;
  public:

    struct Pair {
      string name;
      int val;
	  Pair (const string c, int v) {
		name = c;
		val = v;
	  }
    };
 
    EnumMap(const Pair *data, unsigned int num);


    int lookup(const string &str) const;
    const string &lookup(int val) const;
  };

  /*
    string tostring(int num);
    string tostring(float num);
  */
//#ifdef WIN32
  string inline tostring (int num) {
	char tmp[256];
	sprintf (tmp,"%d",num);
	return string(tmp);
  }
  string inline tostring (float num) {
	char tmp[256];
	sprintf (tmp,"%f",num);
	return string(tmp);
  }
/*#else
  template<class T> inline string tostring(T num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
	
  }
#endif*/
}
#endif
