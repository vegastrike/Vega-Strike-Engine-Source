#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "unit.h"
#define _TEST_
#include "iterator.h"
#include "collection.h"

#define SIZE 64

void Iterate (UnitCollection &c) {
  Unit * unit=NULL;
  un_iter ui = c.createIterator();
  while ((unit = ui.current())!=NULL) {
    UnitCollection::UnitListNode * uln = ui.pos->next;
    bool cachunk=false;
    if (rand()<RAND_MAX/128) {
      cachunk=true;
      unit->Kill();
    }    

    if (rand()<RAND_MAX/100) {
      Iterate(c);
    }

    if (!cachunk&&rand()<RAND_MAX/128) {
      unit->Kill();
    }    

    ui.advance();
  }
}

int main () {
  Unit * unit;
  srand (124);
  UnitCollection c;
  Unit *u[SIZE];
  unsigned int i;
  for (unsigned int j=0;j<64;j++) {
    for (unsigned int k=0;k<64;k++) {
      for (i=0;i<SIZE;i++) {
	u[i]= new Unit(rand()<RAND_MAX/10);
	c.prepend (u[i]);
	if (rand()<RAND_MAX/200) {
	  u[i]->Kill();
	}
      }
    }
    for (unsigned int i=0;i<10;i++) {
      un_iter iter = c.createIterator();
      i=0;  
      while ((unit = iter.current())!=NULL) {
	assert (!unit->Killed()&&!unit->zapped);
	if (rand()<RAND_MAX/8) {
	  Iterate(c);
	}
	
	if (rand()<RAND_MAX/400) {
	  unit->Kill();
	}
	//    printf ("%d %d %d", i,unit->ucref,unit->zapped);
	iter.advance();
	i++;
      }
      UnitCollection::FreeUnusedNodes();
    }
    UnitCollection::FreeUnusedNodes();
  }
  for (unsigned int i=0;i<SIZE;i++) {
    assert (u[i]->zapped==u[i]->killed);
  }
  return 0;
}
