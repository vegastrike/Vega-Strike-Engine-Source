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



#ifndef _CMD_ORDER_H
#define _CMD_ORDER_H

#include "cmd/unit.h"
#include "cmd/collection.h"
#include <vector>
class Order {
protected:
  Unit * parent;
  int type; 
  bool done;
  UnitCollection *group;
  UnitCollection *targets;
  Vector targetlocation;
  vector<Order*> suborders;
public:
  enum ORDERTYPES { MOVEMENT =1, FACING = 2, WEAPON = 4, LOCATION = 8, TARGET = 16, SELF = 32 }; 

  Order (): targetlocation(0,0,0){parent = NULL;group =targets=NULL;type=0;done=false;}
  Order(int ttype): targetlocation(0,0,0){parent = NULL;group=targets=NULL;type = ttype;done=false;}
  virtual ~Order () {}
  virtual void Execute();
  Order* queryType (int type);
  void eraseType (int type);
  bool AttachOrder (UnitCollection *targets);
  bool AttachOrder (Vector target);
  bool AttachSelfOrder (UnitCollection *targets=NULL);
  Order* EnqueueOrder (Order * ord);
  Order* ReplaceOrder (Order * ord);
  bool Done() {return done;}
  int getType() {return type;}
  void SetParent(Unit *parent1) {parent = parent1;};
};

class OrderFactory {
public:
  virtual int type () {return 0;}
  OrderFactory(){}
  virtual Order * newOrder() {return new Order;}
};

class ExecuteFor:  public Order {
  Order * child;
  float time;
  float maxtime;
 public:
  ExecuteFor (Order * chld, float seconds): Order(chld->getType()),child(chld),time(0),maxtime(seconds) {}
  void Execute ();
  ~ExecuteFor () {delete child;}
};

#endif
