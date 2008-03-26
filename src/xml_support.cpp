#include <string>
#include <sstream>
#include <stdlib.h>
//#include <strstream.h>
#include <ctype.h>
#include <assert.h>
#include "xml_support.h"

using std::string;


string strtoupper(const string &foo) 
{
  string rval(foo);
  for (string::iterator it = rval.begin(); it != rval.end(); ++it)
	  *it = toupper(*it);
  return rval;
}
string strtolower(const string &foo) 
{
  string rval(foo);
  for (string::iterator it = rval.begin(); it != rval.end(); ++it)
	  *it = tolower(*it);
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
    if (!result) result = forward.Get("UNKNOWN");
    return result?*result:0;
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
	if ((!str.empty())&&((*str.begin())=='t'||(*str.begin())=='T'||(*str.begin())=='y'||(*str.begin())=='Y'||(*str.begin())=='1')) {
		return true;
	} else {
		return false;
	}
  }
  double parse_float(const string &str) 
  {
  	double ret = 0.0f;
  	std::stringstream ss(str);
  	ss >> ret;
    return(ret);
  }
  
  float parse_floatf(const string &str) 
  {
	float ret = 0.0f;
	std::stringstream ss(str);
	ss >> ret;
	return(ret);
  }
  
  int parse_int(const string &str) {
  	int ret = 0;
  	std::stringstream ss(str);
  	ss >> ret;
  	return(ret);
  }
  string::size_type parse_option_find(const string &str, const string &opt, const string &sep, const string &vsep) {
    bool ini=true;
    string::size_type pos=0;
    string::size_type optlen=opt.length();
    string::size_type strlen=str.length();
    string allsep=sep+vsep;
    if ((optlen==0)||(strlen==0)) 
        return string::npos;
    bool found=false;
    while (!found&&(pos!=string::npos)&&((pos=str.find(opt,pos+(ini?0:1)))!=string::npos)) {
        ini=false;
        found = (  ((pos==0)||(sep.find(str[pos-1])!=string::npos))
                 &&((pos+optlen>=strlen)||(allsep.find(str[pos+optlen])!=string::npos))  );
        if (!found) pos=str.find_first_of(sep,pos+optlen); //quick advancement
    }
    return found?pos:string::npos;
  }
  bool parse_option_ispresent(const string &str, const string &opt, const string &sep, const string &vsep) {
    return parse_option_find(str,opt,sep,vsep)!=string::npos;
  }
  string parse_option_value(const string &str, const string &opt, const string &defvalue, const string &sep, const string &vsep) {
    string::size_type pos=parse_option_find(str,opt,sep,vsep);
    string::size_type vpos=str.find_first_of(vsep,pos+opt.length());
    string value;
    if (pos!=string::npos && vpos!=string::npos) {
        string::size_type vend=str.find_first_of(sep,vpos+1);
        value = str.substr(vpos+1,((vend!=string::npos)?vend-vpos-1:string::npos));
    } else if (pos!=string::npos) {
        value = "true";
    } else {
        value = defvalue;
    }
    return value;
  }

  string escaped_string(const string &str)
  {
#define ESCAPE_CASE(e,c,skip) case e: if (rp&&str[rp-1]=='\\') rv[ip++] = c; else if (!skip) rv[ip++] = e; break
	string::size_type rp,ip,n=str.length();
	string rv;
	rv.resize(n);
	for (rp=ip=0; rp<n; ++rp) {
	  switch(str[rp]) {
	    ESCAPE_CASE('\\','\\',true);
	    ESCAPE_CASE('n','\n',false);
	    ESCAPE_CASE('r','\r',false);
		ESCAPE_CASE('t','\t',false);
		default: rv[ip++] = str[rp];
	  }
	}
#undef ESCAPE_CASE
	rv.resize(ip);
	return rv;
  }

}//namespace XMLSupport

