#ifndef _CMD_COLLIDE_H_
#define _CMD_COLLIDE_H_
struct LineCollide {
  void * object;
  enum collidables {UNIT, BEAM,BALL,BOLT,PROJECTILE} type;
  Vector Mini;
  Vector Maxi;
  LineCollide (void * objec, enum collidables typ,const Vector &st, const Vector &en) {object=objec;type=typ;Mini=st;Maxi=en;}	       
};








#endif
