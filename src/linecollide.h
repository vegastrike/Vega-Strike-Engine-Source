#ifndef _LINECOLLIDE_H_
#define _LINECOLLIDE_H_

#include <stdlib.h>
/**
 * Line Collide provides a complete container for a unit that is put in a collide hashtable
 * For collisions. The mini, maxi vectors of the line collide are taken
 */
class Unit;
class Beam;
class Bolt;

struct LineCollide {
  ///The object that this LineCollide approximates
  union OBJECT {
    Unit *u;
    Beam *b;
    Bolt *blt;
    int i;
  }object;
  ///The minimum x,y,z that this object has
  QVector Mini;
  ///The maximum x,y,z that this object has
  QVector Maxi;
  /**
   * The last item that checked this for collisions
   * to prevent duplicate selection 
   */
  void * lastchecked;
  ///Which type of unit it is. Used for subsequently calling object's Accurate collide func
  enum collidables {UNIT, BEAM,BALL,BOLT,PROJECTILE} type;
  ///If this object was saved as a huge object (hhuge for dos oddities)
  bool hhuge;
  LineCollide(): Mini(0,0,0), Maxi(0,0,0), type(UNIT){object.u=NULL;hhuge=false;lastchecked=NULL;}
  LineCollide (void * objec, enum collidables typ,const QVector &st, const QVector &en) {this->object.u=(Unit *)objec;this->type=typ;this->Mini=st;this->Maxi=en;hhuge=false; lastchecked=NULL;}
  LineCollide (const LineCollide &l) {object=l.object; type=l.type; Mini=l.Mini;Maxi=l.Maxi;hhuge=l.hhuge; lastchecked=NULL;}      
};
#endif
