#ifndef _FLIGHTGROUP_H_
#define _FLIGHTGROUP_H_
#include "flightgroup_generic.h"

class GameFlightgroup: public Flightgroup
{
 public:
  static Flightgroup * newGameFlightgroup (const std::string &name, 
					const std::string &type,
					const std::string &faction,
					const std::string &order,
					int num_ships,
					int num_waves,
					const std::string &texname,
					const std::string &alphname,
					class Mission * mis);
  GameFlightgroup();
  GameFlightgroup (GameFlightgroup & other) {
    *this = other;
  }
  GameFlightgroup& operator =(GameFlightgroup &);
  ~GameFlightgroup();
};

class CreateFlightgroup {
 public:
  Flightgroup * fg;
  int terrain_nr;// which terrain to use. -1 for normal unit -2 for mission ter
  enum {UNIT,VEHICLE, BUILDING} unittype;
  int waves,nr_ships; // number of waves, number of ships per wave
  float rot[3];
  class easyDomNode * domnode;
};

#endif
