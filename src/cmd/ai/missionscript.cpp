/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  AI for mission scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include "script.h"
#include "navigation.h"
#include "xml_support.h"
#include "flybywire.h"
#include <stdio.h>
#include <vector>
#include <stack>
#include "vs_path.h"
#include "tactics.h"

#include "missionscript.h"
#include "cmd/script/mission.h"

//AImissionScript::AImissionScript (string modname):Order (Order::MOVEMENT|Order::FACING){
AImissionScript::AImissionScript (string modname){
  //  cout << "AImissionScript " << modname << endl;
  //printf("addr %x\n",(int)this);
  //  mission->addModule(modname);

  modulename=modname;

  classid=mission->createClassInstance(modulename);
  
  first_run=true;
}

AImissionScript::~AImissionScript () {
  mission->runScript(modulename,"quitai",classid);
#ifdef ORDERDEBUG
  fprintf (stderr,"aims%x",this);
  fflush (stderr);
#endif
}

  //  cout << "ai quitting" << endl;


void AImissionScript::Execute () {
  desired_ang_velocity=Vector(0,0,0);
  desired_velocity=Vector(0,0,0);

  mission->setCurrentAIUnit(parent);
  mission->setCurrentAIOrder(this);

  if(first_run){
    mission->runScript(modulename,"initai",classid);
    first_run=false;
  }

  mission->runScript(modulename,"executeai",classid);

  varInst *vi=mission->lookupClassVariable(modulename,"aistyle",classid);
  if(vi==NULL || vi->type!=VAR_INT){
    Order::Execute();
  }
  else{
    if(vi->int_val==0){
      Order::Execute();
    }
    else if(vi->int_val==1){
      FlyByWire::Execute();
    }
  }
  mission->deleteVarInst(vi);
  done=false;

  varInst *done_vi=mission->lookupClassVariable(modulename,"_done",classid);
  if(done_vi!=NULL && done_vi->type==VAR_BOOL && done_vi->bool_val==true){
    done=true;
  }
  mission->deleteVarInst(done_vi);
}


AIFlyToWaypoint::AIFlyToWaypoint(Vector wp,float velo,bool afburn,float rng) : AImissionScript("ai_flyto_waypoint") {
  waypoint=wp;
  vel=velo;
  range=rng;
  aburn=afburn;

  varInst *vi_wp=mission->lookupClassVariable(modulename,"waypoint",classid);
  mission->call_vector_into_olist(vi_wp,waypoint);

  varInst *vi_range=mission->lookupClassVariable(modulename,"abort_range",classid);
  vi_range->float_val=range;

  varInst *vi_vel=mission->lookupClassVariable(modulename,"vel",classid);
  vi_vel->float_val=vel;

  varInst *vi_aburn=mission->lookupClassVariable(modulename,"afterburner",classid);
  vi_vel->bool_val=aburn;


}



AISuperiority::AISuperiority() : AImissionScript("ai_superiority") {

}

AIPatrol::AIPatrol(int mode,Vector area,float range,Unit *around_unit) : AImissionScript("ai_patrol") {

  varInst *vi_wp=mission->lookupClassVariable(modulename,"area",classid);
  mission->call_vector_into_olist(vi_wp,area);

  varInst *vi_range=mission->lookupClassVariable(modulename,"range",classid);
  vi_range->float_val=range;

  varInst *vi_mode=mission->lookupClassVariable(modulename,"patrol_mode",classid);
  vi_mode->int_val=mode;

  varInst *vi_unit=mission->lookupClassVariable(modulename,"around_unit",classid);
  vi_unit->objectname="unit";
  vi_unit->object=around_unit;

}
