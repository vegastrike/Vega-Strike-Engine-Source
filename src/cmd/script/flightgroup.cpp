#include "gfx/aux_texture.h"

#include "mission.h"
#include "flightgroup.h"
#include <stdio.h>
Flightgroup::Flightgroup ():pos(0,0,0) {//betterto have a flightgroup constructor
  fprintf (stderr,"constructing FG 0x%x\n",this);
  squadLogo=NULL;
  orderlist=NULL;
  nr_waves_left=nr_ships_left=nr_ships=flightgroup_nr=0;
  leader_decision=-1;
}

void Flightgroup::Decrement(Unit * un) {
  nr_ships--;
  nr_ships_left--;
  if (leader.GetUnit()==un) {
    leader.SetUnit (NULL);
  }
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
  bool new_fg =false;
  if (fg==NULL) {
    new_fg=true;
    fg =new Flightgroup;
    fg->name=name;
    fg->directive="b";
    fg->faction= faction;
  }else {
    new_fg = (fg->nr_waves_left==0&&fg->nr_ships_left==0);
  }
  if (!logo_tex.empty()) {
    
    if (logo_alp.empty()) {
      fg->squadLogo=new Texture (logo_tex.c_str(),0,MIPMAP);
    }else {
      fg->squadLogo=new Texture (logo_tex.c_str(),logo_alp.c_str(),0,MIPMAP);
    }
  }


  if (new_fg) {
    fg->nr_waves_left=num_waves-1;
    mission->AddFlightgroup (fg);
  }
  fg->ainame=order;
  fg->type = type;
  fg->nr_ships+=num_ships;
  fg->nr_ships_left+=num_ships;
  return fg;
}
Flightgroup::~Flightgroup() {
  fprintf (stderr,"deleting FG 0x%x %s %s\n",this,name.c_str(),faction.c_str());
  if (squadLogo){
    delete squadLogo;
  }

}

Flightgroup& Flightgroup::operator = (Flightgroup & other) {
  printf ("warning: may not work properly");
  if (squadLogo) {
    squadLogo = other.squadLogo->Clone();
  }
  return other;
}
