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

#include "mission.h"
#include "easydom.h"

//#include "vs_globals.h"
//#include "vegastrike.h"



varInst *Mission::call_olist(missionNode *node,int mode){

  //varInst *viret=new varInst;
  varInst *viret=NULL;

  string cmd=node->attr_value("name");

  if(cmd=="new"){
    viret=call_olist_new(node,mode);

    return viret;
  }
  else{
    varInst *ovi=getObjectArg(node,mode);
    olist_t *my_object=getOListObject(node,mode,ovi);
    
    if(cmd=="push_back"){
      missionNode *snode=getArgument(node,mode,1);
      //varInst *vi=doVariable(snode,mode); // should be getObjExpr
      varInst *vi=checkExpression(snode,mode); // should be getObjExpr
      
      debug(3,node,mode,"olist.push_back pushing variable");
      //      printf("vi is 0x%x\n",vi);
      printVarInst(3,vi);

      if(mode==SCRIPT_RUN){
	call_olist_push_back(node,mode,ovi,vi);
      }

      viret=new varInst;
      viret->type=VAR_VOID;
      return viret;
    }
    else if(cmd=="pop_back"){
      debug(3,node,mode,"olist.pop");

      if(mode==SCRIPT_RUN){
	//my_object->pop_back();
	call_olist_pop_back(node,mode,ovi);
      }
      
      viret=new varInst;
      viret->type=VAR_VOID;
      return viret;
    }
    else if(cmd=="back"){
      debug(3,node,mode,"olist.back");

      viret=new varInst;
      viret->type=VAR_OBJECT;

      if(mode==SCRIPT_RUN){
	varInst *back_vi=call_olist_back(node,mode,ovi);
	assignVariable(viret,back_vi);
      }
      return viret;
    }
    else if(cmd=="at"){
      debug(3,node,mode,"olist.at");

      missionNode *snode=getArgument(node,mode,1);
      int index=checkIntExpr(snode,mode);
      debug(3,snode,mode,"index is in that node");


      viret=new varInst;
      viret->type=VAR_ANY;

      if(mode==SCRIPT_RUN){
	varInst *back_vi=call_olist_at(node,mode,ovi,index);
	assignVariable(viret,back_vi);
      }

      return viret;
    }
    else if(cmd=="toxml"){
      if(node->subnodes.size()!=1){
	fatalError(node,mode,"olist.toxml needs no arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.toxml");

      if(mode==SCRIPT_RUN){
	call_olist_toxml(node,mode,ovi);
      }

      viret =new varInst;
      viret->type=VAR_VOID;
      return viret;
    }
    else if(cmd=="size"){
      if(node->subnodes.size()!=1){
	fatalError(node,mode,"olist.size needs one arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.size");

      viret=new varInst;

      if(mode==SCRIPT_RUN){
	int len=my_object->size();
	viret->int_val=len;
      }

      viret->type=VAR_INT;
      return viret;
    }
    else{
      fatalError(node,mode,"unknown command "+cmd+" for callback olist");
      assert(0);
    }
    
    return NULL; // never reach
  }
  return NULL; // never reach
}

olist_t *Mission::getOListObject(missionNode *node,int mode,varInst *ovi){
  olist_t *my_object=NULL;

	if(mode==SCRIPT_RUN){
	  my_object=(olist_t *)ovi->object;
	  if(my_object==NULL){
	    fatalError(node,mode,"olist: no object");
	    assert(0);
	  }
	}

	return(my_object);
}

varInst *Mission::call_olist_at(missionNode *node,int mode,varInst *ovi,int index){
  olist_t *olist=getOListObject(node,mode,ovi);
  	if(index>=olist->size()){
	  char buffer[200];
	  sprintf(buffer,"olist.at: index out of range size=%d, index=%d\n",olist->size(),index);
	  fatalError(node,mode,buffer);
	  assert(0);
	}
	varInst *back_vi=(*olist)[index];
	
	return back_vi;
}

varInst *Mission::call_olist_back(missionNode *node,int mode,varInst *ovi){
  return  getOListObject(node,mode,ovi)->back();
  
}
void Mission::call_olist_pop_back(missionNode *node,int mode,varInst *ovi){
	getOListObject(node,mode,ovi)->pop_back();
}

void Mission::call_olist_push_back(missionNode *node,int mode,varInst *ovi,varInst *push){
	varInst *push_vi=new varInst;
	push_vi->type=push->type;
	assignVariable(push_vi,push);

	getOListObject(node,mode,ovi)->push_back(push_vi);

}

void Mission::call_olist_toxml(missionNode *node,int mode,varInst *ovi){
  olist_t *my_object=getOListObject(node,mode,ovi);

  int len=my_object->size();
	
	for(int i=0;i<len;i++){
	  varInst *vi=(*my_object)[i];
	  char buffer[200];
	  sprintf(buffer,"<olist index=\"%d\"",i);
	  debug(3,node,mode,buffer);
	  var_out << buffer << " " ;
	  saveVarInst(vi,var_out);
	  var_out << "</olist>" << endl;
	}

}

varInst *Mission::call_olist_new(missionNode *node,int mode){
  varInst *viret=new varInst;

    olist_t *my_object=new olist_t;

    viret->type=VAR_OBJECT;
    viret->objectname="olist";
    
    viret->object=(void *)my_object;

    debug(3,node,mode,"olist new object: ");
    printVarInst(3,viret);

    return viret;
}
