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

#include "gfx/vec.h"
#include "cmd/collection.h"
#include <vector>

#include <cmd/script/mission.h>

/**
 * Order is the base class for all orders.  All orders inherit from Order
 * Each fighter may have an order they are currently executing. Orders naturally
 * can queue other orders, and decide which orders may be executed in parallel by those orders' bit codes.
 * Orders affect their "parent" and may call any function within that parent
 * Limitation (perhaps resolvable) Orders do not "know" what their parent
 * is until Execute() time.  Perhaps "SetParent" Should be made virtual so that
 * various variables may be set earlier than at every execution time
 */
//#define ORDERDEBUG

class Order {
protected:
  ///The unit this order is attached to
  Unit * parent;
  ///The bit code (from ORDERTYPES) that this order is (for parallel execution)
  int type; 
  ///Whether or not this order is done
  bool done;
  ///If this order applies to a group of units (as in form up with this group)
  UnitCollection *group;
  ///If this order applies to a group of targets (somewhat deprecated, use parent->Target() now
  UnitCollection *targets;
  ///If this order applies to a physical location in world space
  Vector targetlocation;
  ///The queue of suborders that will be executed in parallel according to bit code
  std::vector<Order*> suborders;
public:
  ///The varieties of order types  MOVEMENT,FACING, and WEAPON orders may not be mutually executed (lest one engine goes left, the other right)
  enum ORDERTYPES { MOVEMENT =1, FACING = 2, WEAPON = 4, LOCATION = 8, TARGET = 16, SELF = 32 }; 
  ///The default constructor setting everything to NULL and no dependency on order
  Order (): targetlocation(0,0,0){parent = NULL;group =targets=NULL;type=0;done=false;}
  ///The constructor that specifies what order dependencies this order has
  Order(int ttype): targetlocation(0,0,0){parent = NULL;group=targets=NULL;type = ttype;done=false;}
  ///The virutal destructor
  virtual ~Order ();
  ///The function that gets called and executes all queued suborders 
  virtual void Execute();
  ///returns a pointer to the first order that may be bitwised ored with that type
  Order* queryType (int type);
  ///Erases all orders that bitwise OR with that type
  void eraseType (int type);
  ///Attaches a group of targets to this order (used for strategery-type games)
  bool AttachOrder (UnitCollection *targets);
  ///Attaches a navigation point to this order
  bool AttachOrder (Vector target);
  ///Attaches a group (form up) to this order
  bool AttachSelfOrder (UnitCollection *targets=NULL);
  ///Enqueues another order that will be executed (in parallel perhaps) when next void Execute() is called
  Order* EnqueueOrder (Order * ord);
  ///Replaces the first order of that type in the order queue
  Order* ReplaceOrder (Order * ord);
  bool Done() {return done;}
  int getType() {return type;}
  ///Sets the parent of this Unit.  Any virtual functions must call this one
  virtual void SetParent(Unit *parent1) {parent = parent1;};

  /// return pointer to order or NULL if not found
  Order *findOrder(Order *ord);
  /// erase that order from the list
  void eraseOrder(Order *ord);
  /// enqueue order as first order
  Order* EnqueueOrderFirst (Order *ord);

  virtual olist_t* getOrderList(){ return NULL;};
};
///Convenience order factory for "clicking to create an order"
class OrderFactory {
public:
  virtual int type () {return 0;}
  OrderFactory(){}
  virtual Order * newOrder() {return new Order;}
};
///Executes another order for a number of seconds
class ExecuteFor:  public Order {
  ///The child order to execute
  Order * child;
  ///the time it has executed the child order for
  float time;
  ///the total time it can execute child order
  float maxtime;
 public:
  ExecuteFor (Order * chld, float seconds): Order(chld->getType()),child(chld),time(0),maxtime(seconds) {}
  ///Executes child order and then any suborders that may be pertinant
  void Execute ();
  ///Removes this order
  ~ExecuteFor () {delete child;}
};

#endif
