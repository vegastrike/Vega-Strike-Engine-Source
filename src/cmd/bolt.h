#ifndef _CMD_BOLT_H_
#define _CMD_BOLT_H_
#include "weapon_xml.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include <vector>
using std::vector;




class Bolt {
  Transformation cumulative_transformation;
  int ref;
  float damage, longrange;
  float speed, range;
  
  
};

#endif
