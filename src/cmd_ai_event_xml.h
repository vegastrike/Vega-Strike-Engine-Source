#include <string>
#include <vector>
#include <list>
namespace AIEvent {
  struct AIEvresult {
    int type;
    float max, min;//values that will cause this event
    string script;
    AIEvresult (int type, float const min, const float max, const string &aiscript):
      type (type), max(max), min(min), script(aiscript) {}
    
  }
  struct ElemAttrMap {
    int numelem;
    EnumMap element_map;
    int level;
    std::vector <std::list <AIEvresult> >result;
    ElemAttrMap (int nelem, int nattr, const EnumMap &el, const EnumMap &at):
      numelem(nelem), element_map (el), level(0) { } 
  };
  void LoadAI (const char * filename, ElemAttrMap &result);
}
