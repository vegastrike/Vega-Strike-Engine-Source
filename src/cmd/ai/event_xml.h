#ifndef _CMD_AI_EVENT_XML_H_
#define _CMD_AI_EVENT_XML_H_
#include "xml_support.h"
#include <string>
#include <vector>
#include <list>

namespace AIEvents {
  struct AIEvresult {
    int type;//will never be zero...negative indicates "not"
    float max, min;//values that will cause this event
	std::string script;
    AIEvresult (int type, float const min, const float max, const std::string &aiscript);
    bool Eval (const float eval) const {//return ((eval>=min)&&(eval<max)&&type>0)||(eval>=max&&eval<min&&type<0);
      if (eval>=min)
	if (eval < max)
	  if (type>0)
	    return true;
      if (eval<min)
	if (eval>= max)
	  if (type<0)
	    return true;
      return false;
    } 
  };
  struct ElemAttrMap {
    float distclose;
    XMLSupport::EnumMap element_map;
    int level; short curtime; short maxtime;
    std::vector <std::list <AIEvresult> >result;
    ElemAttrMap (const XMLSupport::EnumMap &el):
      element_map (el), level(0) { } 
  };
  void LoadAI (const char * filename, ElemAttrMap &result);//num seconds
}
#endif
