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
#include "cmd/ai/navigation.h"
#include "cmd/ai/flybywire.h"
#include "cmd/ai/tactics.h"

#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"

//#include "vegastrike.h"

/* *********************************************************** */

varInst *Mission::call_order(missionNode *node,int mode){

  varInst *viret=NULL;

  string cmd=node->attr_value("name");

  if(cmd=="newAggressiveAI"){
    string filestr=getStringArgument(node,mode,0);
    string intstr=getStringArgument(node,mode,1);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      my_order=new Orders::AggressiveAI(filestr.c_str(),intstr.c_str());
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newMoveTo"){
    missionNode *pos_node=getArgument(node,mode,0);
    varInst *pos_vi=checkObjectExpr(pos_node,mode);
    olist_t *pos_olist=getOListObject(pos_node,mode,pos_vi);

    missionNode *ab_node=getArgument(node,mode,1);
    bool afterburn=checkBoolExpr(ab_node,mode);

    missionNode *sw_node=getArgument(node,mode,2);
    int nr_switchbacks=checkIntExpr(sw_node,mode);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      Vector vec3=call_olist_tovector(pos_node,mode,pos_vi);
      
      my_order=new Orders::MoveTo(vec3,afterburn,nr_switchbacks);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newChangeHeading"){
    missionNode *pos_node=getArgument(node,mode,0);
    varInst *pos_vi=checkObjectExpr(pos_node,mode);
    olist_t *pos_olist=getOListObject(pos_node,mode,pos_vi);

    missionNode *sw_node=getArgument(node,mode,1);
    int nr_switchbacks=checkIntExpr(sw_node,mode);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      Vector vec3=call_olist_tovector(pos_node,mode,pos_vi);
      
      my_order=new Orders::ChangeHeading(vec3,nr_switchbacks);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newFaceTarget"){
    missionNode *itts_node=getArgument(node,mode,0);
    bool itts=checkBoolExpr(itts_node,mode);

    missionNode *fini_node=getArgument(node,mode,1);
    bool fini=checkBoolExpr(fini_node,mode);

    missionNode *acc_node=getArgument(node,mode,2);
    int acc=checkIntExpr(acc_node,mode);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      if(itts){
	my_order=new Orders::FaceTargetITTS(fini,acc);
      }
      else{
	my_order=new Orders::FaceTarget(fini,acc);
      }
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newFireAt"){
    missionNode *reaction_node=getArgument(node,mode,0);
    float reaction=checkFloatExpr(reaction_node,mode);

    missionNode *aggr_node=getArgument(node,mode,1);
    float aggr=checkFloatExpr(aggr_node,mode);

    Order *my_order=NULL;
    if(mode==SCRIPT_RUN){
      my_order=new Orders::FireAt(reaction,aggr);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newExecuteFor"){
      missionNode *enq_node=getArgument(node,mode,0);
      varInst *enq_vi=checkObjectExpr(enq_node,mode);
      Order *enq_order=getOrderObject(enq_node,mode,enq_vi);

    missionNode *time_node=getArgument(node,mode,1);
    float fortime=checkFloatExpr(time_node,mode);

    Order *my_order=NULL;
    if(mode==SCRIPT_RUN){
      my_order=new ExecuteFor(enq_order,fortime);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newCloakFor"){
      missionNode *val_node=getArgument(node,mode,0);
      bool res=checkBoolExpr(val_node,mode);

    missionNode *time_node=getArgument(node,mode,1);
    float fortime=checkFloatExpr(time_node,mode);

    Order *my_order=NULL;
    if(mode==SCRIPT_RUN){
      my_order=new CloakFor(res,fortime);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newMatchVelocity"){
    missionNode *des_node=getArgument(node,mode,0);
    varInst *des_vi=checkObjectExpr(des_node,mode);
    olist_t *des_olist=getOListObject(des_node,mode,des_vi);

    missionNode *desa_node=getArgument(node,mode,1);
    varInst *desa_vi=checkObjectExpr(desa_node,mode);
    olist_t *desa_olist=getOListObject(desa_node,mode,desa_vi);

    missionNode *local_node=getArgument(node,mode,2);
    bool local=checkBoolExpr(local_node,mode);

    missionNode *afburn_node=getArgument(node,mode,3);
    bool afburn=checkBoolExpr(afburn_node,mode);

    missionNode *fini_node=getArgument(node,mode,4);
    bool fini=checkBoolExpr(fini_node,mode);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      Vector des3=call_olist_tovector(des_node,mode,des_vi);
      Vector desa3=call_olist_tovector(desa_node,mode,desa_vi);
      
      my_order=new Orders::MatchVelocity(des3,desa3,local,afburn,fini);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newMatchAngularVelocity"){
    missionNode *des_node=getArgument(node,mode,0);
    varInst *des_vi=checkObjectExpr(des_node,mode);
    olist_t *des_olist=getOListObject(des_node,mode,des_vi);

    missionNode *local_node=getArgument(node,mode,1);
    bool local=checkBoolExpr(local_node,mode);

    missionNode *fini_node=getArgument(node,mode,2);
    bool fini=checkBoolExpr(fini_node,mode);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      Vector des3=call_olist_tovector(des_node,mode,des_vi);
      
      my_order=new Orders::MatchAngularVelocity(des3,local,fini);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else if(cmd=="newMatchLinearVelocity"){
    missionNode *des_node=getArgument(node,mode,0);
    varInst *des_vi=checkObjectExpr(des_node,mode);
    olist_t *des_olist=getOListObject(des_node,mode,des_vi);

    missionNode *local_node=getArgument(node,mode,1);
    bool local=checkBoolExpr(local_node,mode);

    missionNode *afburn_node=getArgument(node,mode,2);
    bool afburn=checkBoolExpr(afburn_node,mode);

    missionNode *fini_node=getArgument(node,mode,3);
    bool fini=checkBoolExpr(fini_node,mode);

    Order *my_order=NULL;

    if(mode==SCRIPT_RUN){
      Vector des3=call_olist_tovector(des_node,mode,des_vi);
      
      my_order=new Orders::MatchLinearVelocity(des3,local,afburn,fini);
    }

    viret=newVarInst(VI_TEMP);
    viret->type=VAR_OBJECT;
    viret->objectname="order";
    
    viret->object=(void *)my_order;

    return viret;
  }
  else{
    varInst *ovi=getObjectArg(node,mode);
    Order *my_order=getOrderObject(node,mode,ovi);

    if(cmd=="enqueueOrder"){
      missionNode *enq_node=getArgument(node,mode,1);
      varInst *enq_vi=checkObjectExpr(enq_node,mode);
      Order *enq_order=getOrderObject(enq_node,mode,enq_vi);

      if(mode==SCRIPT_RUN){
	Order *res_order=my_order->EnqueueOrder(enq_order);
	debug(3,node,mode,"enqueueing order");
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="eraseOrder"){
      missionNode *enq_node=getArgument(node,mode,1);
      varInst *enq_vi=checkObjectExpr(enq_node,mode);
      Order *enq_order=getOrderObject(enq_node,mode,enq_vi);

      if(mode==SCRIPT_RUN){
	my_order->eraseOrder(enq_order);
	debug(3,node,mode,"erasing order");
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="findOrder"){
      missionNode *enq_node=getArgument(node,mode,1);
      varInst *enq_vi=checkObjectExpr(enq_node,mode);
      Order *enq_order=getOrderObject(enq_node,mode,enq_vi);

      Order *res_order=NULL;
      if(mode==SCRIPT_RUN){
	res_order=my_order->findOrder(enq_order);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_OBJECT;
      viret->objectname="order";
      viret->object=(void *)res_order;
    }
    else if(cmd=="SteerUp"){
      missionNode *val_node=getArgument(node,mode,1);
      float val=checkFloatExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->Up(val);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="SteerRight"){
      missionNode *val_node=getArgument(node,mode,1);
      float val=checkFloatExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->Right(val);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="SteerRollRight"){
      missionNode *val_node=getArgument(node,mode,1);
      float val=checkFloatExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->RollRight(val);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="SteerStop"){
      missionNode *val_node=getArgument(node,mode,1);
      float val=checkFloatExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->Stop(val);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="SteerAccel"){
      missionNode *val_node=getArgument(node,mode,1);
      float val=checkFloatExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->Accel(val);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="SteerAfterburn"){
      missionNode *val_node=getArgument(node,mode,1);
      float val=checkFloatExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->Afterburn(val);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="SteerSheltonSlide"){
      missionNode *val_node=getArgument(node,mode,1);
      bool res=checkBoolExpr(val_node,mode);

      if(mode==SCRIPT_RUN){
	// this will crash if order is no FlyByWire
	// is there a way to check that?
	((FlyByWire *)my_order)->SheltonSlide(res);
      }

      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else if(cmd=="print"){
      if(mode==SCRIPT_RUN){
	printf("print: order=%x\n",(int)my_order);
      }
      viret=newVarInst(VI_TEMP);
      viret->type=VAR_VOID;
    }
    else{
      fatalError(node,mode,"no such method "+cmd);
      assert(0);
    }

    deleteVarInst(ovi);
    return viret;
  }//else (objects)

  return NULL; //never reach
}

Order *Mission::getOrderObject(missionNode *node,int mode,varInst *ovi){
  Order *my_object=NULL;

	if(mode==SCRIPT_RUN){
	  my_object=(Order *)ovi->object;
	  if(my_object==NULL){
	    fatalError(node,mode,"order: no object");
	    assert(0);
	  }
	}

	return(my_object);
}
