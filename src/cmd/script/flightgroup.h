#ifndef _FLIGHTGROUP_H_
#define _FLIGHTGROUP_H_

#include "cmd/container.h"
#include "mission.h"
#include <string>
#include <vector>
#include <map>
class Texture;

class Flightgroup {
private:
  std::string squadLogoStr;
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
  void Decrement(Unit * trashed) {
    nr_ships--;
    nr_ships_left--;
    if (leader.GetUnit()==trashed) {
      leader.SetUnit (NULL);
    }
  }
  static Flightgroup * newFlightgroup (const std::string &name, 
					const std::string &type,
					const std::string &faction,
					const std::string &order,
					int num_ships,
					int num_waves,
					const std::string &texname,
					const std::string &alphname,
					class Mission * mis);
  Flightgroup ():pos(0,0,0) {//betterto have a flightgroup constructor
    //fprintf (stderr,"constructing FG 0x%x\n",this);
    orderlist=NULL;
    nr_waves_left=nr_ships_left=nr_ships=flightgroup_nr=0;
    leader_decision=-1;
    squadLogo=NULL;
  }
  void Init( Flightgroup * fg, const std::string &name,
					const std::string &type,
					const std::string &faction,
					const std::string & order,
					int num_ships,
					int num_waves,Mission * mis)
	{
	  bool new_fg =false;
	  if (fg==NULL) {
		new_fg=true;
		this->name=name;
		this->directive="b";
		this->faction= faction;
	  }else {
		new_fg = (this->nr_waves_left==0&&this->nr_ships_left==0);
	  }

	  if (new_fg) {
		this->nr_waves_left=num_waves-1;
		mission->AddFlightgroup (this);
	  }
	  this->ainame=order;
	  this->type = type;
	  this->nr_ships+=num_ships;
	  this->nr_ships_left+=num_ships;
	}
  Flightgroup (Flightgroup & other) {
	  *this = other;
  }
  Flightgroup& Flightgroup::operator = (Flightgroup & other);
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
