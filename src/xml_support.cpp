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
	if ((!str.empty())&&((*str.begin())=='t'||(*str.begin())=='T'||(*str.begin())=='y'||(*str.begin())=='Y'||(*str.begin())=='1')) {
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

