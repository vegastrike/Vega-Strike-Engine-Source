#ifndef _CMD_AI_H_
#define _CMD_AI_H_
#include "cmd_unit.h"
class Order;
class AI{
protected:
	Unit *parent;
public:
	AI(Unit *parent1) {SetParent(parent1);};
	AI() {parent = NULL;};
	void SetParent(Unit *parent1) {parent = parent1;};
	virtual AI *Execute() {return this;}
	virtual bool Done() {return false;}
        virtual int getType(){ return 0;}
	virtual AI* EnqueueOrder (AI * tmp) {delete this; return tmp;}
	virtual AI* ReplaceOrder (AI * tmp) {delete this; return tmp;}
};
class AIFactory {
public:
  AIFactory(){}
  virtual AI* newAI() {return new AI;}
};

#endif
