#include "gfx/aux_texture.h"
#include "mission.h"
#include "flightgroup.h"
#include "cmd/unit_generic.h"
#include <stdio.h>

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
      fg->squadLogo=new Texture (logo_tex.c_str(),0,MIPMAP);
    }else {
      fg->squadLogo=new Texture (logo_tex.c_str(),logo_alp.c_str(),0,MIPMAP);
    }
  }
  return fg;
}
Flightgroup::~Flightgroup() {
  //VSFileSystem::vs_fprintf (stderr,"deleting FG 0x%x %s %s\n",this,name.c_str(),faction.c_str());
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
