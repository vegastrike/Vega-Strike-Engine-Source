#ifndef _TURRETAI_H_
#define _TURRETAI_H_
#include "navigation.h"
namespace Orders {
class TurretAI :public FaceTarget {
  float range;
 public:
  TurretAI();
  virtual void Execute ();
};
}
#endif
