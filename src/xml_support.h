#ifndef _XML_SUPPORT_H_

#define _XML_SUPPORT_H_
#include <stdio.h>
#include <string>
#ifndef WIN32
#include <sstream>
#endif
#include "hashtable.h"
#include <vector>
#include <expat.h>
#include <iostream>		// needed for cout calls in config_xml.cpp (and other places too i'm sure)
#include "gfx/vec.h"

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

using std::string;

string strtoupper(const string &foo);

namespace XMLSupport {

  struct Attribute {
    string name;
    string value;
    Attribute(string name, string value) : name(name), value(value)
    {};
  };

  class AttributeList : public std::vector<Attribute> {
  public:
    AttributeList(const XML_Char **atts);
  };
  double parse_float(const string &str);
  string replace_space(const string &str);
  int parse_int(const string &str);
  bool parse_bool (const string &str);
  bool parse_option_ispresent(const string &str, const string &opt, const string &sep=",\r\n", const string &vsep="=");
  string parse_option_value(const string &str, const string &opt, const string &defvalue, const string &sep=",\r\n", const string &vsep="=");

  class EnumMap {

    //    static inline double parse_float (const string &str) {return ::parse_float (str)};
    Hashtable<string,const int,1001>    forward;
    Hashtable<string,const string,1001> reverse;
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
  inline string tostring5 (unsigned short num) {
	char tmp[256];
	sprintf (tmp,"%.5d",num);
	return string(tmp);
  }
  inline string tostring (int num) {
	char tmp[256];
	sprintf (tmp,"%d",num);
	return string(tmp);
  }
  inline string tostring (unsigned int num) {
	char tmp[256];
	sprintf (tmp,"%u",num);
	return string(tmp);
  }
  inline string tostring (long num) {
	char tmp[256];
	sprintf (tmp,"%ld",num);
	return string(tmp);
  }
  inline string tostring (float num) {
	char tmp[256];
	sprintf (tmp,"%f",num);
	return string(tmp);
  }
  inline string floattostringh (float f) {
	char c[128];
	sprintf (c,"%2.2f",f);
	return string(c);
  }
  inline string VectorToString(const Vector & v) {
	string ret(floattostringh(v.i));
	if (v.i!=v.j||v.j!=v.k) {
		ret+=string(",")+floattostringh(v.j)+string(",")+floattostringh(v.k);
	}
	return ret;
  }
/*#else
  inline template<class T> string tostring(T num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
	
  }
#endif*/
}
#endif
