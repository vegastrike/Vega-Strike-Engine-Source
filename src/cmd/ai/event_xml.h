#ifndef _CMD_AI_EVENT_XML_H_
#define _CMD_AI_EVENT_XML_H_
#include "xml_support.h"
#include <string>
#include <vector>
#include <list>
/**
 * General namespace that does nothing on its own, but
 * Deals with the parsing of an XML file tha tcontains a number of
 * event "if" statements
 * Each statement can have a minimum, max value and a "not" flag to invert it
 * and the type references the enum in the class using this one, and 
 * is eventually only used for the "not" tag
 */
namespace AIEvents {
  ///A struct indicating an event that may or may not be executed
  struct AIEvresult {
    ///will never be zero...negative indicates "not"
    int type;
    ///The maximum/minimum values that will cause this event
    float max, min;
    float timetofinish;
    float timetointerrupt;
    float priority;
    ///The string indicating what type of thing this event evaluates
    std::string script;
    AIEvresult (int type, float const min, const float max, float timetofinish,float timetointerrupt, float priority, const std::string &aiscript);
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
    XMLSupport::EnumMap element_map;
    int level; float curtime; float maxtime; float obedience; //short fix
    std::vector <std::list <AIEvresult> >result;
    ElemAttrMap (const XMLSupport::EnumMap &el):
      element_map (el), level(0) { } 
  };
  void LoadAI (const char * filename, ElemAttrMap &result,const std::string &faction );//num seconds
}
#endif
