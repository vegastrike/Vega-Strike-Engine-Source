#ifndef _TURRETAI_H_
#define _TURRETAI_H_
#include "navigation.h"
namespace Orders {
class TurretAI :public FaceTarget {
  float range;
  float speed;
  float mrange;
 public:
  TurretAI();
  virtual void Execute ();
  virtual void getAverageGunSpeed (float &speed, float & range, float & mrange) const;
};
}
#endif
