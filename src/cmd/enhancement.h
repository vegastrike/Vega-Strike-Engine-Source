#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "cmd/unit.h"
#include "savegame.h"
class Enhancement : public Unit {
  std::string filename;
  virtual enum clsptr isUnit() {return ENHANCEMENTPTR;}
 public:
  Enhancement::Enhancement (const char * filename, int faction, Flightgroup * flightgrp=NULL, int fg_subnumber=0):Unit (filename,true,false,faction,flightgrp,fg_subnumber),filename(filename) {
  }
  virtual void reactToCollision (Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist) {
    if (smaller->isUnit()!=ASTEROIDPTR) {
      smaller->Upgrade (this);
      string fn (filename);
      string fac(_Universe->GetFaction(faction));
      Kill();
      AddUnitToSave(fn.c_str(),ENHANCEMENTPTR,fac.c_str(),(int)this);
    }
  }
};








#endif
