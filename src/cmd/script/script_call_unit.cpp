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

#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"

//#include "vegastrike.h"

/* *********************************************************** */


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

      if(my_unit==NULL){
	fatalError(node,mode,"no such high unit nr");
	assert(0);
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
	int fgnum=my_unit->getFgSubnumber();

	char buffer[100];
	sprintf(buffer,"%s-%d",fg->name.c_str(),fgnum);

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
