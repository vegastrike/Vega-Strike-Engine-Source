#include "mission.h"
#include "flightgroup.h"
#include "cmd/unit.h"
#include <stdio.h>
Flightgroup::Flightgroup ():pos(0,0,0) {//betterto have a flightgroup constructor
  fprintf (stderr,"constructing FG 0x%x\n",this);
  orderlist=NULL;
  nr_waves_left=nr_ships_left=nr_ships=flightgroup_nr=0;
  leader_decision=-1;
  squadLogo=NULL;
}

void Flightgroup::Decrement(Unit * un) {
  nr_ships--;
  nr_ships_left--;
  if (leader.GetUnit()==un) {
    leader.SetUnit (NULL);
  }
}

void Flightgroup::Init( Flightgroup * fg, const std::string &name,
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

Flightgroup * Flightgroup::newFlightgroup (const std::string &name, 
					   const std::string &type,
					   const std::string &faction,
					   const std::string & order,
					   int num_ships,
					   int num_waves,
					   const std::string &logo_tex,
					   const std::string &logo_alp,
					   Mission * mis) {
  Flightgroup *fg=mis->findFlightgroup (name,faction);
  Flightgroup *fgtmp = fg;
  if( fg==NULL)
	  fg =new Flightgroup;
  fg->Init( fgtmp, name, type, faction, order, num_ships, num_waves, mis);
  if (!logo_tex.empty()) {
    
    if (logo_alp.empty()) {
      fg->squadLogoStr=logo_tex;
    }else {
      fg->squadLogoStr=logo_alp;
    }
  }
  return fg;
}
