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
#include "cmd/unit.h"
#include "order.h"
#include "cmd/collection.h"
#include "vegastrike.h"
//#define ORDERDEBUG
void Order::Execute () {
#ifdef AGRDEBUG
  fprintf (stderr,"ox%x",this);
  fflush (stderr);
#endif

  int completed=0;
//	fprintf (stderr,"size: %d done %d", suborders.size(), (*suborders.begin())->Done());
  /*
  if(suborders.size()) {
    vector<Order*>::iterator ord = suborders.begin();
    (*ord)->Execute();
    if((*ord)->Done()){
      delete (*ord);
      ord = suborders.erase(ord);
    }
  }
  */

  unsigned int i=0;

  for (i=0;i<suborders.size();i++) {
    // huh? why the mask? can't get anything to work with this thing
    // here. i'm not going to bother to figure out what's going on
    // here until later
    
#ifdef AGRDEBUG
  fprintf (stderr,"sub i%d ",i);
  fflush (stderr);
#endif

    if ((completed& ((suborders[i])->getType()&(MOVEMENT|FACING|WEAPON)))==0) {
#ifdef AGRDEBUG
  fprintf (stderr,"ldi%x ",suborders[i]);
  fflush (stderr);
#endif
      (suborders[i])->Execute();
#ifdef AGRDEBUG
  fprintf (stderr,"ldci%x \n",suborders[i]);
  fflush (stderr);
#endif
      completed|=(suborders[i])->getType();
      if ((suborders[i])->Done()) {
	vector<Order*>::iterator ord = suborders.begin()+i;
#ifdef AGRDEBUG
  fprintf (stderr,"deex%x",*ord);
  fflush (stderr);
#endif
	delete (*ord);
#ifdef AGRDEBUG
  fprintf (stderr,"delex\n");
  fflush (stderr);
#endif
	ord =suborders.erase(ord);
	i--;
      } 
    }
  }
  
  if (suborders.size()==0) {
    done = true;
  } else{
    done = false;
  }
#ifdef AGRDEBUG
  fprintf (stderr,"eox");
  fflush (stderr);
#endif

  return;
}

Order * Order::queryType (int type) {
  for (unsigned int i=0;i<suborders.size();i++) {
    if ((suborders[i]->type&type)==type) {
      return suborders[i];
    }
  }
  return NULL;
}


void Order::eraseType (int type) {
  for (unsigned int i=0;i<suborders.size();i++) {
    if ((suborders[i]->type&type)==type) {
#ifdef ORDERDEBUG
  fprintf (stderr,"delord %x",suborders[i]);
  fflush (stderr);
#endif
      delete suborders[i];
#ifdef ORDERDEBUG
  fprintf (stderr,"\\delord");
  fflush (stderr);
#endif
      vector <Order *>::iterator j= suborders.begin()+i;
      suborders.erase(j);
      i--;
    }
  }
}

Order* Order::EnqueueOrder (Order *ord) {
#ifdef ORDERDEBUG
  fprintf (stderr,"enqord");
  fflush (stderr);
#endif
  ord->SetParent(parent);
  suborders.push_back (ord);
#ifdef ORDERDEBUG
  fprintf (stderr,"\\enqord");
  fflush (stderr);
#endif
  return this;
}
Order* Order::EnqueueOrderFirst (Order *ord) {
  ord->SetParent(parent);

  vector<Order*>::iterator first_elem = suborders.begin();
  suborders.insert (first_elem,ord);
  return this;
}
Order* Order::ReplaceOrder (Order *ord) {
#ifdef ORDERDEBUG
  fprintf (stderr,"repord");
  fflush (stderr);
#endif

  int completed=0;
  vector<Order*>::iterator ordd = suborders.begin();
  for (unsigned int i=0;i<suborders.size();i++) {
    if (!(ord->getType()&(*ordd)->getType()&(FACING|WEAPON|MOVEMENT))){
#ifdef ORDERDEBUG
  fprintf (stderr,"delpord%x",*ordd);
  fflush (stderr);
#endif
      	delete (*ordd);
	ordd =suborders.erase(ordd);
    } else {
      ordd++;
    }
  }
  suborders.push_back(ord);
#ifdef ORDERDEBUG
  fprintf (stderr,"repord");
  fflush (stderr);
#endif

  return this;

}

bool Order::AttachOrder (UnitCollection *targets1) {
#ifdef ORDERDEBUG
  fprintf (stderr,"atachord");
  fflush (stderr);
#endif
  if (!(type&TARGET)) {
    if (type&SELF) {
      return AttachSelfOrder (targets1);//can use attach order to do shit
    }
    return false;
  }
  if (targets)
 {
#ifdef ORDERDEBUG
  fprintf (stderr,"targ%x",targets);
  fflush (stderr);
#endif
    delete targets;
  
  }
  targets = new UnitCollection();
  UnitCollection::UnitIterator *iter = targets1->createIterator();
  Unit *u;
  while(0!=(u = iter->current())) {
    targets->prepend (u);
    iter->advance();
  }
#ifdef ORDERDEBUG
  fprintf (stderr,"deliterxx%x",iter);
  fflush (stderr);
#endif
  delete iter;
  return true;
#ifdef ORDERDEBUG
  fprintf (stderr,"\\atachord");
  fflush (stderr);
#endif

}
bool Order::AttachSelfOrder (UnitCollection *targets1) {
  if (!(type&SELF))
    return false;
  if (group)
    delete group;
  if (targets1==NULL) {
    group = NULL;
  } else {
    group = new UnitCollection();
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
  if (!(type&LOCATION)) {
    return false;
  }
  targetlocation = targetv;
  return true;
}

void ExecuteFor::Execute() {
  if (child) {
    child->SetParent(parent);
    type = child->getType();
  }
  if (time>maxtime) {
    done = true;

    return;
  }
  time +=SIMULATION_ATOM;
  if (child)
    child->Execute();
  return;
}

Order* Order::findOrder(Order *ord){
  for (unsigned int i=0;i<suborders.size();i++) {
    if (suborders[i]==ord) {
      return suborders[i];
    }
  }
  return NULL;
}
Order::~Order () {
#ifdef ORDERDEBUG
  fprintf (stderr,"ofr%x\n",this);
  fflush (stderr);
#endif
}
void Order::eraseOrder(Order *ord){
  bool found=false;

  for (unsigned int i=0;i<suborders.size() && found==false;i++) {
    if (suborders[i]==ord){
#ifdef ORDERDEBUG
  fprintf (stderr,"eraseord%x",suborders[i]);
  fflush (stderr);
#endif
      delete suborders[i];
#ifdef ORDERDEBUG
  fprintf (stderr,"eraseord");
  fflush (stderr);
#endif

      vector <Order *>::iterator j= suborders.begin()+i;
      suborders.erase(j);
      found=true;
      //      printf("erased order\n");
    }
  }

  
}
