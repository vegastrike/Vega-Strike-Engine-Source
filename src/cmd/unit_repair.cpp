#include "unit.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "images.h"
#include "lin_time.h"
extern float rand01();
void GameUnit::Repair() {
  //note work slows down under time compression!
  static float repairtime =XMLSupport::parse_float(vs_config->getVariable ("physics","RepairDroidTime","1000"));
  float workunit = SIMULATION_ATOM/(repairtime*getTimeCompression());//a droid completes 1 work unit in repairtime
  switch (image->repair_droid) {
  case 6:
    //versatilize Weapons! (invent)
    if (GetNumMounts()) {
      if (rand01()<workunit) {
	int whichmount = rand()%GetNumMounts();
	mounts[whichmount]->size |=(1>>(rand()%(8*sizeof(short))));
      }
      if (rand01()<workunit) {
	int whichmount= rand()%GetNumMounts();
	if (mounts[whichmount]->ammo>0) {
	  mounts[whichmount]->volume++;
	}
      }
    }
    if (computer.max_speed<60) 
      computer.max_speed+=workunit;
    if (computer.max_ab_speed<160)
      computer.max_ab_speed+=workunit;
  case 5:
    //increase maxrange
    computer.radar.maxrange+=workunit;
    if (computer.radar.maxcone>-1) {    //Repair MaxCone full
      computer.radar.maxcone-=workunit;
    }else {
      computer.radar.maxcone=-1;
    }
    if (computer.radar.lockcone>0) {    //Repair MaxCone full
      computer.radar.lockcone-=workunit;
    }else {
      //      computer.radar.lockcone=-1;
    }
    
    if (rand01()<workunit*.25) {
      computer.itts=true;
    }
    if (computer.radar.mintargetsize>0) {
      computer.radar.mintargetsize-=rSize()*workunit;
    }//no break...please continue, colonel
  case 4:
    if (GetNumMounts()) {    //    RepairWeapon();
      if (rand01()<workunit) {
	unsigned int i=rand()%GetNumMounts();
	if (mounts[i]->status==Mount::DESTROYED) {
	  mounts[i]->status=Mount::INACTIVE;
	}
      }
    }//nobreak
  case 3:
    if (computer.radar.mintargetsize>rSize()) {
      computer.radar.mintargetsize-=rSize()*workunit;
    }
    if (rand01()<.5*workunit) {
      computer.radar.color=true;
    }
    if (rand01()<workunit) {
      if (jump.damage>0)
	jump.damage--;
    }//nobreak
  case 2:
    {
      int whichgauge=rand()%(Cockpit::NUMGAUGES+1+MAXVDUS);
      if (image->cockpit_damage[whichgauge]<1) {
	image->cockpit_damage[whichgauge]+=workunit;
	if (image->cockpit_damage[whichgauge]>1)
	  image->cockpit_damage[whichgauge]=1;
      }
    }    
  case 1:
    if (computer.radar.maxcone>0) {    //Repair MaxCone half
      computer.radar.maxcone-=workunit;
    }
    if (computer.radar.lockcone>.7) {    //Repair MaxCone half
      computer.radar.lockcone-=workunit;
    }

    if (jump.drive!=-1) {    //    RepairJumpEnergy(jump.energy,maxenergy);
      if (jump.energy>maxenergy) {
	if (rand01()<workunit) {
	  jump.energy=maxenergy-1;
	}
      }
    }
    if (computer.radar.mintargetsize>1.5*rSize()) {//    RepairMinTargetSize
      computer.radar.mintargetsize-=rSize()*workunit;
    }

  default: 
    break;
  }
}
