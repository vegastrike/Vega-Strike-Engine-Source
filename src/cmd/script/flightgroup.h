#ifndef _FLIGHTGROUP_H_
#define _FLIGHTGROUP_H_
#include "cmd/container.h"
#include <string>
#include <vector>
#include <map>
struct Texture;


class Flightgroup {
 public:
  UnitContainer leader;//I'm on the leader
  int leader_decision;//-1 if decision is made
  Texture * squadLogo;//null if not there
  std::string directive;//"a" for attack target "b" for attack at will "h" for help out "f" for form up.... missions can get this..and set this
  std::string name; // flightgroup name
  std::string type;
  std::string ainame; // .agg.xml and .int.xml get appended to this
  std::string faction;
  int flightgroup_nr; // running fg number
  int nr_ships; // total ships nr
  QVector pos;
  int nr_ships_left;
  int nr_waves_left;

  std::map<std::string,std::string> ordermap;
  std::vector <class varInst *> *orderlist;
  //removes a ship from the flightgroup below
  void Decrement(Unit * trashed);
  static Flightgroup * newFlightgroup (const std::string &name, 
					const std::string &type,
					const std::string &faction,
					const std::string &order,
					int num_ships,
					int num_waves,
					const std::string &texname,
					const std::string &alphname,
					class Mission * mis);
  Flightgroup ();
  Flightgroup (Flightgroup & other) {
    *this = other;
  }
  Flightgroup& operator =(Flightgroup &);
  ~Flightgroup();
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
