#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "enhancement_generic.h"
#include "cmd/unit.h"
#include "savegame.h"
class GameEnhancement : public GameUnit<Enhancement>
{
protected:
  /// constructor only to be called by UnitFactory
  GameEnhancement(const char * filename, int faction, const string &modifications,Flightgroup * flightgrp=NULL, int fg_subnumber=0)
    : GameUnit<Enhancement>(filename,false,faction,modifications,flightgrp,fg_subnumber)
  {
	  string file( filename);
	  this->filename = filename;
  }

  friend class UnitFactory;

private:
    /// default constructor forbidden
    GameEnhancement( );
    /// copy constructor forbidden
    GameEnhancement( const Enhancement& );
    /// assignment operator forbidden
    GameEnhancement& operator=( const Enhancement& );
};

#endif

