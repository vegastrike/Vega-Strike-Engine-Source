#include "cmd_unit.h"
#include "cmd_beam.h"

vector <LineCollide> collidequeue;

void Unit::CollideAll() {
  unsigned int i;
  //target->curr_physical_state.position;, rSize();
  for (i=0;i<collidequeue.size();i++) {
    if (Position().i+radial_size>collidequeue[i].Mini.i&&
	Position().i-radial_size<collidequeue[i].Maxi.i&&
	Position().j+radial_size>collidequeue[i].Mini.j&&
	Position().j-radial_size<collidequeue[i].Maxi.j&&
	Position().k+radial_size>collidequeue[i].Mini.k&&
	Position().k-radial_size<collidequeue[i].Maxi.k) {
      switch (collidequeue[i].type) {
      case LineCollide::UNIT://other units!!!
	return;
      case LineCollide::BEAM:
	((Beam*)collidequeue[i].object)->Collide(this);
	break;
      case LineCollide::BALL:
	break;
      case LineCollide::BOLT:
	break;
      case LineCollide::PROJECTILE:
	break;
      }
    }
  }
    //add self to the queue??? using prev and cur physical state as an UNKNOWN
}
bool Unit::Collide (Unit * target) {
  if (target==this) 
    return false;
  //unit v unit? use point sampling?
  //now first make sure they're within bubbles of each other...
  if ((Position()-target->Position()).Magnitude()>radial_size+target->radial_size)
    return false;
  //now do some serious checks
  //each subunit with each other subunit
  //each mesh with each mesh

  //blah
  return true;
}


bool Beam::Collide (Unit * target) {
  if (target==owner) 
    return false;
  float distance = target->querySphere (center,direction,0);
  if (distance==0||distance>curlength) {
    return false;
  }
  if (target->queryBoundingBox(center,direction,0)==0)
    return false;
  curlength = distance;
  impact=IMPACT;
  //deliver float tmp=(curlength/range)); (damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
  return true;
}
