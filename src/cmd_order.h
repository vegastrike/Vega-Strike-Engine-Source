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

/* This should REALLY be "basic behavior"... oh well */

#ifndef _CMD_ORDER_H
#define _CMD_ORDER_H

#include "cmd_ai.h"

#include "UnitCollection.h"
#include <vector>
const int LOCATION =1;
const int TARGET = 2;
const int SELF = 4; //the order types are orthogonal...you can form up while attacking and moving to a location
class Order:public AI {
protected:
  int type; 
  bool done;
  UnitCollection *group;
  UnitCollection *targets;
  Vector targetlocation;
  vector<AI*> suborders;
public:
  Order (): AI(), targetlocation(0,0,0){group =targets=NULL;type=0;done=false;}
  Order(int ttype):AI(), targetlocation(0,0,0){targets=NULL;type = ttype;done=false;}
  virtual ~Order () {}
  virtual AI *Execute();
  bool AttachOrder (UnitCollection *targets);
  bool AttachOrder (Vector target);
  bool AttachSelfOrder (UnitCollection *targets=NULL);
  AI* EnqueueOrder (AI * ord);
  AI* ReplaceOrder (AI * ord);
  bool Done() {return done;}
  int getType() {return type;}
};

class OrderFactory: public AIFactory {
public:
  virtual int type () {return 0;}
  OrderFactory():AIFactory(){}
  virtual AI * newAI () {return new Order;}
  virtual Order * newOrder() {return new Order;}
};

class ExecuteFor:  public Order {
  AI * child;
  float time;
  float maxtime;
 public:
  ExecuteFor (AI * chld, float seconds): Order(),child(chld),time(0),maxtime(seconds) {}
  AI * Execute ();
  ~ExecuteFor () {delete child;}
};


class FlyStraight:public AI{
	float speed;
	float time;

public:
	FlyStraight(float speed1, float time1) {parent = NULL; speed = speed1; time = time1;};
	
	AI *Execute();
	bool Done() {return false;}
	int getType() {return 0;}
        bool AppendOrder (Order * tmp) {return false;}

};
#endif
