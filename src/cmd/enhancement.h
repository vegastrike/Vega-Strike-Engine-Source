#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "cmd/unit.h"
#include "savegame.h"
class Enhancement : public GameUnit
{
  std::string filename;
  virtual enum clsptr isUnit() {return ENHANCEMENTPTR;}

protected:
  /// constructor only to be called by UnitFactory
  Enhancement(const char * filename, int faction, const string &modifications,Flightgroup * flightgrp=NULL, int fg_subnumber=0)
    : GameUnit(filename,false,faction,modifications,flightgrp,fg_subnumber)
    , filename(filename) {
  }

  friend class UnitFactory;

public:
  virtual void reactToCollision (Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist) {
    if (smaller->isUnit()!=ASTEROIDPTR) {
      double percent;
      char * tempdata = (char *)malloc (sizeof(this->shield));
      memcpy (tempdata,&this->shield,sizeof(this->shield));
      shield.number=2;//don't want them getting our boosted shields!
      shield.fb[0]=shield.fb[1]=shield.fb[2]=shield.fb[3]=0;
      smaller->Upgrade (this,0,0,true,true,percent);
      memcpy (&this->shield,tempdata,sizeof(this->shield));
      string fn (filename);
      string fac(FactionUtil::GetFaction(faction));
      Kill();
      _Universe.AccessCockpit()->savegame->AddUnitToSave(fn.c_str(),ENHANCEMENTPTR,fac.c_str(),(long)this);
    }
  }

private:
    /// default constructor forbidden
    Enhancement( );
    /// copy constructor forbidden
    Enhancement( const Enhancement& );
    /// assignment operator forbidden
    Enhancement& operator=( const Enhancement& );
};

#endif

