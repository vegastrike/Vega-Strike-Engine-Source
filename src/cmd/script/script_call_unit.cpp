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
#include "collection.h"
#include "iterator.h"
#include "unit.h"
#include "ai/order.h"
#include "ai/aggressive.h"

#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"

//#include "vegastrike.h"

/* *********************************************************** */

extern void SetTurretAI (Unit * fighter);

extern Unit *player_unit;

varInst *Mission::call_unit(missionNode *node,int mode){

  varInst *viret=NULL;

  string cmd=node->attr_value("name");

  if(cmd=="getUnit"){
    missionNode *nr_node=getArgument(node,mode,0);
    int unit_nr=(int)doFloatVar(nr_node,mode);
    Unit *my_unit=NULL;

    if(mode==SCRIPT_RUN){
      StarSystem *ssystem=_Universe->activeStarSystem();
      UnitCollection *unitlist=ssystem->getUnitList();
      //UnitCollection::UnitIterator *uiter=unitlist->createIterator();
      Iterator *uiter=unitlist->createIterator();


      int i=0;
      Unit *unit=uiter->current();
      while(unit!=NULL){
	if(i==unit_nr){
	  my_unit=uiter->current();
	  unit=NULL;
	}
	else{
	  unit=uiter->advance();
	  i++;
	}
      }
    }

    viret=new varInst;
    viret->type=VAR_OBJECT;
    viret->objectname="unit";
    
    viret->object=(void *)my_unit;

    debug(3,node,mode,"unit getUnit: ");
    printVarInst(3,viret);

    return viret;
  }
  else if(cmd=="getPlayer"){

    Unit *my_unit=NULL;

    if(mode==SCRIPT_RUN){
      my_unit=player_unit;
    }

    viret=new varInst;
    viret->type=VAR_OBJECT;
    viret->objectname="unit";
    
    viret->object=(void *)my_unit;

    debug(3,node,mode,"unit getUnit: ");
    printVarInst(3,viret);

    return viret;

  }
  else if(cmd=="launch"){
    missionNode *name_node=getArgument(node,mode,0);
    varInst *name_vi=checkObjectExpr(name_node,mode);
    
    missionNode *faction_node=getArgument(node,mode,1);
    varInst *faction_vi=checkObjectExpr(faction_node,mode);
    
    missionNode *type_node=getArgument(node,mode,2);
    varInst *type_vi=checkObjectExpr(type_node,mode);
    
    missionNode *ai_node=getArgument(node,mode,3);
    varInst *ai_vi=checkObjectExpr(ai_node,mode);
    
    missionNode *nr_node=getArgument(node,mode,4);
    int nr_of_ships=(int)checkFloatExpr(nr_node,mode);

    missionNode *pos_node[3];
    float pos[3];
    for(int i=0;i<3;i++){
      pos_node[i]=getArgument(node,mode,5+i);
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
      fg->waves=1; // not yet
      fg->nr_ships=nr_of_ships;
      fg->pos[0]=pos[0];
      fg->pos[1]=pos[1];
      fg->pos[2]=pos[2];
      for(int i=0;i<3;i++){
	fg->rot[i]=0.0;
      }
      
      call_unit_launch(fg);
    }

      viret=new varInst;
      viret->type=VAR_VOID;
      return viret;
  }
  else{
    varInst *ovi=getObjectArg(node,mode);
    Unit *my_unit=getUnitObject(node,mode,ovi);

      debug(3,node,mode,"unit object: ");
      printVarInst(3,ovi);

    if(cmd=="getPosition"){
      if(mode==SCRIPT_RUN){
	varInst *vec3_vi=call_olist_new(node,mode);

	Vector pos=my_unit->Position();

	varInst *pos_vi;

	pos_vi=new varInst;
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.i;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	pos_vi=new varInst;
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.j;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	pos_vi=new varInst;
	pos_vi->type=VAR_FLOAT;
	pos_vi->float_val=pos.k;
	call_olist_push_back(node,mode,vec3_vi,pos_vi);

	return vec3_vi;
      }
      else{
	viret=new varInst;
	viret->type=VAR_OBJECT;
	viret->objectname="olist";
	return viret;
      }
    }
    else if(cmd=="getFgId"){
      if(mode==SCRIPT_RUN){
	
	Flightgroup *fg=my_unit->getFlightgroup();
	  char buffer[100];
	  int fgnum=0;

	if(fg){
	  fgnum=my_unit->getFgSubnumber();
	  sprintf(buffer,"%s-%d",fg->name.c_str(),fgnum);
	}
	else{
	  sprintf(buffer,"unknown");
	}


	varInst *str_vi=call_string_new(node,mode,buffer);

	return str_vi;
      }
      else{
	varInst *vi=new varInst;
	vi->type=VAR_OBJECT;
	vi->objectname="string";
	return vi;
      }
    }
    else{
      fatalError(node,mode,"no such method "+cmd);
      assert(0);
    }
    return NULL; // never reach
  }
}

Unit *Mission::getUnitObject(missionNode *node,int mode,varInst *ovi){
  Unit *my_object=NULL;

	if(mode==SCRIPT_RUN){
	  my_object=(Unit *)ovi->object;
	  if(my_object==NULL){
	    fatalError(node,mode,"unit: no object");
	    assert(0);
	  }
	}

	return(my_object);
}

// void call_unit_launch(missionNode *node,int mode,string name,string faction,string type,string ainame,int nr_ships,Vector & pos){

void Mission::call_unit_launch(Flightgroup *fg){

   int faction_nr=_Universe->GetFaction(fg->faction.c_str());
   //   printf("faction nr: %d %s\n",faction_nr,fg->faction.c_str());
   Unit *units[20];

   for(int u=0;u<fg->nr_ships;u++){
     Unit *my_unit=new Unit(fg->type.c_str(),true,false,faction_nr,fg,u);
     units[u]=my_unit;
   }

   float fg_radius=units[0]->rSize();

   for(int u=0;u<fg->nr_ships;u++){
     Unit *my_unit=units[u];

     Vector pox;

     pox.i=fg->pos[0]+u*fg_radius*3;
     pox.j=fg->pos[1]+u*fg_radius*3;
     pox.k=fg->pos[2]+u*fg_radius*3;

     my_unit->SetPosAndCumPos(pox);

     string ai_agg=fg->ainame+".agg.xml";
     string ai_int=fg->ainame+".int.xml";

     char ai_agg_c[1024];
     char ai_int_c[1024];
     strcpy(ai_agg_c,ai_agg.c_str());
     strcpy(ai_int_c,ai_int.c_str());
     //      printf("1 - %s  2 - %s\n",ai_agg_c,ai_int_c);

     my_unit->EnqueueAI( new Orders::AggressiveAI (ai_agg_c, ai_int_c));
	    
     SetTurretAI (my_unit);

     //     cout << fg->name << endl;

     _Universe->activeStarSystem()->AddUnit(my_unit);

     findNextEnemyTarget(my_unit);

   }


 }

void Mission::findNextEnemyTarget(Unit *my_unit){
      StarSystem *ssystem=_Universe->activeStarSystem();
      UnitCollection *unitlist=ssystem->getUnitList();
      //UnitCollection::UnitIterator *uiter=unitlist->createIterator();
      Iterator *uiter=unitlist->createIterator();


      int i=0;
      Unit *unit=uiter->current();
      Unit *target_unit=NULL;
      //      int my_faction=_Universe->GetFaction(my_unit->getFlightgroup()->faction.c_str());
      int my_faction=my_unit->faction;
      while(unit!=NULL){
	//	int other_faction=_Universe->GetFaction(unit->getFlightgroup()->faction.c_str());
	int other_faction=unit->faction;

	if(_Universe->GetRelation(my_faction,other_faction)<0.0){
	  target_unit=uiter->current();
	  unit=NULL;
	}
	else{
	  //	  printf("relation was: %f %d %d\n",_Universe->GetRelation(my_faction,other_faction),my_faction,other_faction);
	  unit=uiter->advance();
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
