#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "cmd/unit.h"
class Enhancement : public Unit {
  virtual enum clsptr isUnit() {return ENHANCEMENTPTR;}
 public:
  Enhancement::Enhancement (const char * filename, int faction, Flightgroup * flightgrp=NULL, int fg_subnumber=0):Unit (filename,true,false,faction,flightgrp,fg_subnumber) {
  }
  virtual void reactToCollision (Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist) {
    smaller->Upgrade (this);
    Kill();
  }
};








#endif
