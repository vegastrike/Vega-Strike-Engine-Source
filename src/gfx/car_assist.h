#include "xml_support.h"
using XMLSupport::EnumMap;
namespace CAR {

  const int  FORWARD_BLINKEN =1;
  const int LEFT_BLINKEN =2;
  const int RIGHT_BLINKEN =4;
  const int SIREN_BLINKEN =8;
  const int ON_NO_BLINKEN =16;

  enum ALL_TYPES {
    RUNNINGLIGHTS,HEADLIGHTS,LEFTBLINK,RIGHTBLINK,BRAKE,REVERSE,SIREN
  };
  const EnumMap::Pair type_names[8] = {
    EnumMap::Pair("UNKNOWN",RUNNINGLIGHTS),
    EnumMap::Pair("RUNNINGLIGHTS",RUNNINGLIGHTS),
    EnumMap::Pair("HEADLIGHTS", HEADLIGHTS),
    EnumMap::Pair("LEFTBLINK",LEFTBLINK),
    EnumMap::Pair("RIGHTBLINK",RIGHTBLINK),
    EnumMap::Pair("BRAKE",BRAKE),
    EnumMap::Pair("REVERSE",REVERSE),
    EnumMap::Pair("SIREN",SIREN)
  };
  const EnumMap type_map (type_names,8);
}
