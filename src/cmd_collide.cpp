#include "cmd_unit.h"
#include "cmd_beam.h"
#include "gfx_mesh.h"
#include "cmd_collide.h"
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
	((Unit*)collidequeue[i].object)->Collide(this);
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
  collidequeue.push_back (LineCollide(this,LineCollide::UNIT,Vector (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size),Vector (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size)));
}

bool Mesh::Collide (Unit * target, const Transformation &cumtrans, Matrix cumtransmat) {
  Transformation cumulative_transformation = local_transformation;
  cumulative_transformation.Compose(cumtrans, cumtransmat);
  //cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  //cumulative_transformation.position//rSize()
  if (target->querySphere (cumulative_transformation.position,rSize())&&
      target->queryBoundingBox (cumulative_transformation.position,rSize())
      //&&IntersectBSP (cumulative_transformation.position,rSize())//bsp
      )
    return true;
  //
  return false;
}

bool Unit::OneWayCollide (Unit * target) {//do each of these bubbled subunits collide with the other unit?
  int i;
  for (i=0;i<nummesh;i++) {
    if (meshdata[i]->Collide(target,cumulative_transformation,cumulative_transformation_matrix))
      return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->OneWayCollide(target))
      return true;
  }
  return false;
}

bool Unit::Collide (Unit * target) {
  if (target==this) 
    return false;
  //unit v unit? use point sampling?
  //now first make sure they're within bubbles of each other...
  if ((Position()-target->Position()).Magnitude()>radial_size+target->radial_size)
    return false;
  //now do some serious checks
  if (radial_size>target->radial_size) {
    if ((!target->OneWayCollide(this))||(!OneWayCollide(target)))
      return false;
  }else {
    if ((!OneWayCollide(target))||(!target->OneWayCollide(this)))
      return false;
  }
  //deal damage similarly to beam damage!!  Apply some sort of repel force

  //each mesh with each mesh? naw that should be in one way collide
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
