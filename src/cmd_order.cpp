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
#include "cmd_unit.h"
#include "cmd_order.h"
#include "UnitCollection.h"
#include "physics.h"
AI * Order::Execute () {
  //  int completed=0;
  if(suborders.size()) {
    vector<AI*>::iterator ord = suborders.begin();
    (*ord)->Execute();
    if((*ord)->Done()){
      delete (*ord);
      ord = suborders.erase(ord);
    }
  }
  /*
  for (unsigned int i=0;i<suborders.size();i++) {
    // huh? why the mask? can't get anything to work with this thing
    // here. i'm not going to bother to figure out what's going on
    // here until later

    if (completed&(*ord)->getType()){
      (*ord)->Execute();
      completed|=(*ord)->getType();
      if ((*ord)->Done()) {
	delete (*ord);
	ord =suborders.erase(ord);
      } else {
	ord++;
      }
      }
      }
  */
  if (suborders.size()==0) {
    delete this;
    return new Order;
  }
  return this;
}

AI* Order::EnqueueOrder (AI *ord) {
  suborders.push_back (ord);
  return this;
}
AI* Order::ReplaceOrder (AI *ord) {
  int completed=0;
  vector<AI*>::iterator ordd = suborders.begin();
  for (unsigned int i=0;i<suborders.size();i++) {
    if (!(ord->getType()&(*ordd)->getType())){
      	delete (*ordd);
	ordd =suborders.erase(ordd);
    } else {
      ordd++;
    }
  }
  suborders.push_back(ord);
  return this;
}

bool Order::AttachOrder (UnitCollection *targets1) {
  if (!(type&TARGET)) {
    if (type&SELF) {
      return AttachSelfOrder (targets1);//can use attach order to do shit
    }
    return false;
  }
  if (targets)
    delete targets;
  
  targets = new UnitCollection(true);
  UnitCollection::UnitIterator *iter = targets1->createIterator();
  Unit *u;
  while(0!=(u = iter->current())) {
    targets->prepend (u);
    iter->advance();
  }
  delete iter;
  return true;
}
bool Order::AttachSelfOrder (UnitCollection *targets1) {
  if (!(type&SELF))
    return false;
  if (group)
    delete group;
  if (targets1==NULL) {
    group = NULL;
  } else {
    group = new UnitCollection(true);
    UnitCollection::UnitIterator *iter = targets1->createIterator();
    Unit *u;
    while(0!=(u = iter->current())) {
      group->prepend (u);
      iter->advance();
    }
    delete iter;
  }
  return true;
}
bool Order::AttachOrder (Vector targetv) {
  if (type!=LOCATION) {
    return false;
  }
  targetlocation = targetv;
  return true;
}

AI * ExecuteFor::Execute() {
  child->SetParent(parent);
  type = child->getType();
  if (time>maxtime) {
    done = true;
    return NULL;
  }
  time +=SIMULATION_ATOM;
  child = child->Execute();
  return this;
}


//Doesn't work anymore (as if it ever did)
AI* FlyStraight::Execute() {
  //if(GetElapsedTime() > time)
		{
			parent->Destroy();
			delete this;
			return NULL;
		}
		//		else
		{
		  //parent->YSlide(speed);
			return this;
		}
}
