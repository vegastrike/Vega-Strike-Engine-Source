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
      fg->squadLogoStr=logo_tex;
    }else {
      fg->squadLogoStr=logo_alp;
    }
  }
  return fg;
}

Flightgroup& Flightgroup::operator = (Flightgroup & other) {
  *this = other;
  return other;
}

Flightgroup::~Flightgroup() {}
