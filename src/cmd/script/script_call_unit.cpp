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
  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"
#include "cmd/collection.h"
#include "cmd/iterator.h"
#include "cmd/unit.h"
#include "cmd/ai/order.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/missionscript.h"

#include "mission.h"
#include "easydom.h"
#include "msgcenter.h"

#include "vs_globals.h"
#include "config_xml.h"
#include "gfx/cockpit.h"
//#include "vegastrike.h"

/* *********************************************************** */


#if 0
NEVER NEVER NEVER use Unit* to save a unit across frames
extern Unit *player_unit;
BAD BAD BAD

Better:
extern UnitContainer player_unit;

Best:
_Universe->AccessCockpit()->GetParent();
#endif

varInst *Mission::call_unit(missionNode *node,int mode){
#ifdef ORDERDEBUG
  fprintf (stderr,"callun%x",this);
  fflush (stderr);
#endif

  varInst *viret=NULL;


  if(mode==SCRIPT_PARSE){
    string cmd=node->attr_value("name");
    node->script.method_id=module_unit_map[cmd];
  }

   callback_module_unit_type method_id=(callback_module_unit_type) node->script.method_id;

  if(method_id==CMT_UNIT_getUnit){
    missionNode *nr_node=getArgument(node,mode,0);
    int unit_nr=doIntVar(nr_node,mode);
    Unit *my_unit=NULL;

    if(mode==SCRIPT_RUN){
      StarSystem *ssystem=_Universe->scriptStarSystem();
      //UnitCollection::UnitIterator *uiter=unitlist->createIterator();
      un_iter uiter=ssystem->getUnitList().createIterator();

      int i=0;
      Unit *unit=uiter.current();
      while(unit!=NULL){
	if(i==unit_nr){
	  my_unit=uiter.current();
	  unit=NULL;
	}
	else{
	  unit=++(uiter);
	  i++;
	}
      }
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="unit";
    
    viret->object=(void *)my_unit;

    debug(3,node,mode,"unit getUnit: ");
    printVarInst(3,viret);

    return viret;
  }
  else if(method_id==CMT_UNIT_getPlayer){

    Unit *my_unit=NULL;

    if(mode==SCRIPT_RUN){
      my_unit=_Universe->AccessCockpit()->GetParent();;
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="unit";
    
    viret->object=(void *)my_unit;

    debug(3,node,mode,"unit getUnit: ");
    printVarInst(3,viret);

    return viret;

  }
  else if(method_id==CMT_UNIT_launch){
    missionNode *name_node=getArgument(node,mode,0);
    varInst *name_vi=checkObjectExpr(name_node,mode);
    
    missionNode *faction_node=getArgument(node,mode,1);
    varInst *faction_vi=checkObjectExpr(faction_node,mode);
    
    missionNode *type_node=getArgument(node,mode,2);
    varInst *type_vi=checkObjectExpr(type_node,mode);
    
    missionNode *ai_node=getArgument(node,mode,3);
    varInst *ai_vi=checkObjectExpr(ai_node,mode);
    
    missionNode *nr_node=getArgument(node,mode,4);
    int nr_of_ships=checkIntExpr(nr_node,mode);

    missionNode *nrw_node=getArgument(node,mode,5);
    int nr_of_waves=checkIntExpr(nrw_node,mode);

    missionNode *pos_node[3];
    float pos[3];
    for(int i=0;i<3;i++){
      pos_node[i]=getArgument(node,mode,6+i);
      pos[i]=checkFloatExpr(pos_node[i],mode);
    }

    if(mode==SCRIPT_RUN){
      string name_string=*((string *)name_vi->object);
      string faction_string=*((string *)faction_vi->object);
      string type_string=*((string *)type_vi->object);
      string ai_string=*((string *)ai_vi->object);


      Flightgroup *fg=new Flightgroup;

      fg->name=name_string;
      fg->type=type_string;
      fg->ainame=ai_string;
      fg->faction=faction_string;
      fg->unittype=Flightgroup::UNIT;
      fg->terrain_nr=-1;
      fg->waves=nr_of_waves;
      fg->nr_ships=nr_of_ships;
      fg->pos[0]=pos[0];
      fg->pos[1]=pos[1];
      fg->pos[2]=pos[2];
      for(int i=0;i<3;i++){
	fg->rot[i]=0.0;
      }
      fg->nr_ships_left=fg->nr_ships;
      fg->nr_waves_left=fg->waves-1;
      fg->orderlist=NULL;
#ifdef ORDERDEBUG
  fprintf (stderr,"cunl%x",this);
  fflush (stderr);
#endif

      call_unit_launch(fg);
#ifdef ORDERDEBUG
  fprintf (stderr,"ecun");
  fflush (stderr);
#endif

    }

    deleteVarInst(name_vi);
    deleteVarInst(faction_vi);
    deleteVarInst(type_vi);
    deleteVarInst(ai_vi);

    viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
      return viret;
  }else if (method_id==CMT_UNIT_getCredits) {
     viret=newVarInst(VI_TEMP);
     viret->type=VAR_FLOAT;
     viret->float_val=0;
     if (mode==SCRIPT_RUN) {
       viret->float_val=_Universe->AccessCockpit()->credits;
     }
   } else if (method_id==CMT_UNIT_addCredits) {
     missionNode *nr_node=getArgument(node,mode,0);
     float credits=doFloatVar(nr_node,mode);
     if (mode==SCRIPT_RUN) {
       _Universe->AccessCockpit()->credits+=credits;
     }
   }
  else{
    varInst *ovi=getObjectArg(node,mode);
    Unit *my_unit=getUnitObject(node,mode,ovi);

      debug(3,node,mode,"unit object: ");
      printVarInst(3,ovi);

    if(method_id==CMT_UNIT_getPosition){
      if(mode==SCRIPT_RUN){
	varInst *vec3_vi=call_olist_new(node,mode);

	Vector pos=my_unit->Position();

	varInst *pos_vi;

	pos_vi=newVarInst(VI_TEMP);
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.i;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	pos_vi=newVarInst(VI_TEMP);
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.j;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	pos_vi=newVarInst(VI_TEMP);
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.k;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	//	deleteVarInst(vec3_vi);
	viret=vec3_vi;
	//return vec3_vi;
      }
      else{
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_OBJECT;
	viret->objectname="olist";
	//return viret;
      }
    }
    else if(method_id==CMT_UNIT_getFaction){
      if(mode==SCRIPT_RUN){
	    varInst *ovi=getObjectArg(node,mode);
	    Unit *my_unit=getUnitObject(node,mode,ovi);
		string factionname=_Universe->GetFaction(my_unit->faction);
		varInst *str_vi=call_string_new(node,mode,factionname);
		viret=str_vi;
      }else {
		varInst *str_vi=call_string_new(node,mode,"");
		viret =str_vi;
      }
      return viret;
    }
    else if(method_id==CMT_UNIT_getVelocity){
      if(mode==SCRIPT_RUN){
	varInst *vec3_vi=call_olist_new(node,mode);

	float vel_mag=my_unit->GetVelocity().Magnitude();
	Vector pos;
	
	printf("VELMAG:%f\n",vel_mag);
	if(fabs (vel_mag)> 10e-6){
	  pos=my_unit->GetVelocity()/vel_mag;
	}
	else{
	  Vector p,q,r;
	  
	  my_unit->GetOrientation(p,q,r);
	  pos=r;
	}

	varInst *pos_vi;

	pos_vi=newVarInst(VI_TEMP);
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.i;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	pos_vi=newVarInst(VI_TEMP);
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.j;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	pos_vi=newVarInst(VI_TEMP);
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.k;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	//	deleteVarInst(vec3_vi);
	viret=vec3_vi;
	//return vec3_vi;
      }
      else{
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_OBJECT;
	viret->objectname="olist";
	//return viret;
      }
    }
    else if(method_id==CMT_UNIT_getTarget){
      Unit *res_unit=NULL;
      if(mode==SCRIPT_RUN){
	res_unit=my_unit->Target();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="unit";
      viret->object=res_unit;
    }
    else if(method_id==CMT_UNIT_getThreat){
      Unit *res_unit=NULL;
      if(mode==SCRIPT_RUN){
	res_unit=my_unit->Threat();
	//	printf("threat: %x\n",(int)res_unit);
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="unit";
      viret->object=res_unit;
    }
    else if(method_id==CMT_UNIT_setTarget){
      Unit *other_unit=getUnitArg(node,mode,1);
      if(mode==SCRIPT_RUN){
	my_unit->Target(other_unit);
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(method_id==CMT_UNIT_getMinDis){
      Vector vec3=getVec3Arg(node,mode,1);

	float dist=0.0;
	if(mode==SCRIPT_RUN){
	  dist=my_unit->getMinDis(vec3);
	}
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_FLOAT;
	viret->float_val=dist;
    }
    else if(method_id==CMT_UNIT_getAngle){
      Unit *other_unit=getUnitArg(node,mode,1);

	float angle=0.0;
	float dist=0.0;
	if(mode==SCRIPT_RUN){
	  Vector p,q,r;
	  Vector vectothem=Vector(other_unit->Position() - my_unit->Position()).Normalize();
	  my_unit->GetOrientation(p,q,r);
	  //angle=my_unit->cosAngleTo(other_unit,dist);
	  angle=acos( vectothem.Dot(r) );
	  angle=(angle/PI)*180.0;
	  //printf("angle: %f\n",angle);
	}
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_FLOAT;
	viret->float_val=angle;
    }
    else if(method_id==CMT_UNIT_getAngleToPos){
      Vector other_pos=getVec3Arg(node,mode,1);

	float angle=0.0;
	float dist=0.0;
	if(mode==SCRIPT_RUN){
	  Vector p,q,r;
	  Vector vectothem=Vector(other_pos - my_unit->Position()).Normalize();
	  my_unit->GetOrientation(p,q,r);
	  //angle=my_unit->cosAngleTo(other_unit,dist);
	  angle=acos( vectothem.Dot(r) );
	  angle=(angle/PI)*180.0;
	  //printf("angle: %f\n",angle);
	}
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_FLOAT;
	viret->float_val=angle;
    }
    else if(method_id==CMT_UNIT_getFShieldData){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->FShieldData();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_getRShieldData){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->RShieldData();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_getLShieldData){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->LShieldData();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_getBShieldData){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->BShieldData();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_getEnergyData){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->EnergyData();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_getHullData){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->GetHull();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_getRSize){
      float res=0.0;
      if(mode==SCRIPT_RUN){
	res=my_unit->rSize();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_isStarShip){
      bool res=false;
      if(mode==SCRIPT_RUN){
	res=my_unit->isStarShip();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_BOOL;
      viret->bool_val=res;
    }
    else if(method_id==CMT_UNIT_isPlanet){
      bool res=false;
      if(mode==SCRIPT_RUN){
	res=my_unit->isPlanet();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_BOOL;
      viret->bool_val=res;
    }
    else if(method_id==CMT_UNIT_isJumppoint){
      bool res=false;
      if(mode==SCRIPT_RUN){
	res=my_unit->isJumppoint();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_BOOL;
      viret->bool_val=res;
    }
    else if(method_id==CMT_UNIT_getRelation){
      float res=0.0;
      Unit *other_unit=getUnitArg(node,mode,1);

      if(mode==SCRIPT_RUN){
	res=my_unit->getRelation(other_unit);
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=res;
    }
    else if(method_id==CMT_UNIT_Jump){
      if(mode==SCRIPT_RUN){
	my_unit->ActivateJumpDrive();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(method_id==CMT_UNIT_getOrientationP){
#if 0
      //TODO
      if(mode==SCRIPT_RUN){
	res=my_unit->FShieldData();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
      viret->float_val=res;
#endif
    }
    else if(method_id==CMT_UNIT_getOrder){
      Order *my_order=NULL;

      if(mode==SCRIPT_RUN){
	my_order=my_unit->getAIState();
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="order";
      viret->object=(void *)my_order;
    }
    else if(method_id==CMT_UNIT_removeFromGame){

      if(mode==SCRIPT_RUN){
	my_unit->Kill();
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(method_id==CMT_UNIT_getFgName){
      if(mode==SCRIPT_RUN){
	string fgname;
	Flightgroup *fg=my_unit->getFlightgroup();
	if(fg){
	  fgname=fg->name;
	}
	if(fgname.empty()){
	  fgname="-none-";
	}

	varInst *str_vi=call_string_new(node,mode,fgname);
	viret=str_vi;
      }
      else{
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_OBJECT;
	viret->objectname="string";
      }
    }
    else if(method_id==CMT_UNIT_getFgSubnumber){
      int num=0;
      if(mode==SCRIPT_RUN){
	num=my_unit->getFgSubnumber();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_INT;
      viret->int_val=num;
    }
    else if(method_id==CMT_UNIT_scanSystem){
      if(mode==SCRIPT_RUN){
	my_unit->scanSystem();
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(method_id==CMT_UNIT_scannerNearestEnemy){
      Unit *ret_unit=NULL;
      if(mode==SCRIPT_RUN){
	ret_unit=my_unit->getScanner()->nearest_enemy;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="unit";
      viret->object=(void *)ret_unit;
    }
    else if(method_id==CMT_UNIT_scannerNearestFriend){
      Unit *ret_unit=NULL;
      if(mode==SCRIPT_RUN){
	ret_unit=my_unit->getScanner()->nearest_friend;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="unit";
      viret->object=(void *)ret_unit;
    }
    else if(method_id==CMT_UNIT_scannerNearestShip){
      Unit *ret_unit=NULL;
      if(mode==SCRIPT_RUN){
	ret_unit=my_unit->getScanner()->nearest_ship;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="unit";
      viret->object=(void *)ret_unit;
    }
    else if(method_id==CMT_UNIT_scannerLeader){
      Unit *ret_unit=NULL;
      if(mode==SCRIPT_RUN){
	ret_unit=my_unit->getScanner()->leader;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="unit";
      viret->object=(void *)ret_unit;
    }
    else if(method_id==CMT_UNIT_scannerNearestEnemyDist){
      float ret=9999999.0;
      if(mode==SCRIPT_RUN){
	ret=my_unit->getScanner()->nearest_enemy_dist;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=ret;
    }
    else if(method_id==CMT_UNIT_scannerNearestFriendDist){
      float ret=9999999.0;
      if(mode==SCRIPT_RUN){
	ret=my_unit->getScanner()->nearest_friend_dist;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=ret;
    }
    else if(method_id==CMT_UNIT_scannerNearestShipDist){
      float ret=9999999.0;
      if(mode==SCRIPT_RUN){
	ret=my_unit->getScanner()->nearest_ship_dist;
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_FLOAT;
      viret->float_val=ret;
    }
    else if(method_id==CMT_UNIT_getFgId){
      if(mode==SCRIPT_RUN){
	
		string fgname=my_unit->getFgID();

		if(fgname.empty()){
		  fgname="-unknown";
		}


		varInst *str_vi=call_string_new(node,mode,fgname);

		viret=str_vi;
		//return str_vi;
      }

      else{
	viret=newVarInst(VI_TEMP);
	viret->type=VAR_OBJECT;
	viret->objectname="string";
	//return vi;
      }
    }
    else if(method_id==CMT_UNIT_toxml){
      if(mode==SCRIPT_RUN){
	call_unit_toxml(node,mode,ovi);
      }
      viret =newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
      //return viret;
    }
    else{
      fatalError(node,mode,"no such method "+node->script.name);
      assert(0);
    }
#ifdef ORDERDEBUG
  fprintf (stderr,"callundel%x",ovi);
  fflush (stderr);
#endif

    deleteVarInst(ovi);
#ifdef ORDERDEBUG
  fprintf (stderr,"undel1");
  fflush (stderr);
#endif

    return viret;
  }//else (objects)
#ifdef ORDERDEBUG
  fprintf (stderr,"endcallun%x",this);
  fflush (stderr);
#endif

  return NULL; //never reach
}

Unit *Mission::getUnitObject(missionNode *node,int mode,varInst *ovi){
  Unit *my_object=NULL;

	if(mode==SCRIPT_RUN){
	  my_object=(Unit *)ovi->object;
#if 0
	  if(my_object==NULL){
	    fatalError(node,mode,"unit: no object");
	    assert(0);
	  }
#endif
	}

	return(my_object);
}

// void call_unit_launch(missionNode *node,int mode,string name,string faction,string type,string ainame,int nr_ships,Vector & pos){

void Mission::call_unit_launch(Flightgroup *fg){

   int faction_nr=_Universe->GetFaction(fg->faction.c_str());
   //   printf("faction nr: %d %s\n",faction_nr,fg->faction.c_str());
   Unit *units[20];
   int u;
   for(u=0;u<fg->nr_ships;u++){
     Unit *my_unit=new Unit(fg->type.c_str(),false,faction_nr,string(""),fg,u);
     units[u]=my_unit;
   }

   float fg_radius=units[0]->rSize();

   for(u=0;u<fg->nr_ships;u++){
     Unit *my_unit=units[u];

     Vector pox;

     pox.i=fg->pos[0]+u*fg_radius*3;
     pox.j=fg->pos[1]+u*fg_radius*3;
     pox.k=fg->pos[2]+u*fg_radius*3;

     my_unit->SetPosAndCumPos(pox);

     if(fg->ainame[0]!='_'){
       string ai_agg=fg->ainame+".agg.xml";
       string ai_int=fg->ainame+".int.xml";

       char ai_agg_c[1024];
       char ai_int_c[1024];
       strcpy(ai_agg_c,ai_agg.c_str());
       strcpy(ai_int_c,ai_int.c_str());
       //      printf("1 - %s  2 - %s\n",ai_agg_c,ai_int_c);

       my_unit->EnqueueAI( new Orders::AggressiveAI (ai_agg_c, ai_int_c));
     }
     else{
	      string modulename=fg->ainame.substr(1);

	      if(fg->orderlist==NULL){
		my_unit->EnqueueAI( new AImissionScript(modulename));
	      }
	      else{
		my_unit->EnqueueAI( new AIOrderList(fg->orderlist));
		printf("LAUNCHING a new orderlist ai\n");
	      }
	      //fighters[a]->SetAI( new AImissionScript(modulename));
     }

     my_unit->SetTurretAI ();

     //     cout << fg->name << endl;

     _Universe->scriptStarSystem()->AddUnit(my_unit);

     //findNextEnemyTarget(my_unit);
     my_unit->Target(NULL);
   }

   char buffer[200];
   sprintf(buffer,"%s launched %s:%s %d-%d",fg->faction.c_str(),fg->name.c_str(),fg->type.c_str(),0,fg->nr_ships);
   msgcenter->add("game","all",buffer);
}

void Mission::findNextEnemyTarget(Unit *my_unit){
      StarSystem *ssystem=_Universe->scriptStarSystem();

      un_iter uiter(ssystem->getUnitList().createIterator());

      int i=0;
      Unit *unit=uiter.current();
      Unit *target_unit=NULL;
      //      int my_faction=_Universe->GetFaction(my_unit->getFlightgroup()->faction.c_str());
      int my_faction=my_unit->faction;
      while(unit!=NULL){
	//	int other_faction=_Universe->GetFaction(unit->getFlightgroup()->faction.c_str());
	int other_faction=unit->faction;

	if(_Universe->GetRelation(my_faction,other_faction)<0.0){
	  target_unit=uiter.current();
	  unit=NULL;
	}
	else{
	  //	  printf("relation was: %f %d %d\n",_Universe->GetRelation(my_faction,other_faction),my_faction,other_faction);
	  unit=++(uiter);
	  i++;
	}
      }

      if(target_unit==NULL){
	//
      }
      else{
	my_unit->Target(target_unit);
      }

      return;
}


void Mission::call_unit_toxml(missionNode *node,int mode,varInst *ovi){
  Unit *my_object=getUnitObject(node,mode,ovi);

  if (my_object) {
   string fgid=my_object->getFgID();
   Flightgroup *fg=my_object->getFlightgroup();

   Vector pos=my_object->Position();
  
   var_out << "<unit fgid=" << qu(fgid) << "/>\n";
  }
}
