#include "cmd_unit.h"
#include "cmd_beam.h"
#include "gfx_mesh.h"
#include "cmd_collide.h"
#include "physics.h"
#include "hashtable_3d.h"
vector <LineCollide*> collidequeue;
//const int COLLIDETABLESIZE=20;//cube root of entries
//const int COLLIDETABLEACCURACY=200;// "1/largeness of sectors"

#define _USE_COLLIDE_TABLE
Hashtable3d <LineCollide*, char[20],char[200]> collidetable;

void AddCollideQueue (const LineCollide &tmp, bool hhuge) {
  int size = collidequeue.size();
  collidequeue.push_back (new LineCollide(tmp));
#ifdef _USE_COLLIDE_TABLE
  collidetable.Put (collidequeue[size],collidequeue[size],hhuge);
#endif

  
}
void ClearCollideQueue() {
#ifdef _USE_COLLIDE_TABLE
  collidetable.Clear();//blah might take some time
#endif
  for (unsigned int i=0;i<collidequeue.size();i++) {
    delete collidequeue[i];
  }
  collidequeue = vector<LineCollide*>();
}

void Unit::CollideAll() {
  unsigned int i;
  bool hhuge = false;
  Vector minx (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector maxx(Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  //target->curr_physical_state.position;, rSize();
#ifdef _USE_COLLIDE_TABLE
  #define COLQ colQ
  vector <LineCollide*> colQ;
  hhuge = collidetable.Get (minx,maxx,colQ);
#else
  #define COLQ collidequeue
#endif
  for (i=0;i<COLQ.size();i++) {
    if (Position().i+radial_size>COLQ[i]->Mini.i&&
	Position().i-radial_size<COLQ[i]->Maxi.i&&
	Position().j+radial_size>COLQ[i]->Mini.j&&
	Position().j-radial_size<COLQ[i]->Maxi.j&&
	Position().k+radial_size>COLQ[i]->Mini.k&&
	Position().k-radial_size<COLQ[i]->Maxi.k) {
      //continue;
      switch (COLQ[i]->type) {
      case LineCollide::UNIT://other units!!!
	((Unit*)COLQ[i]->object)->Collide(this);
	break;
      case LineCollide::BEAM:
	((Beam*)COLQ[i]->object)->Collide(this);
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
  AddCollideQueue (LineCollide(this,LineCollide::UNIT,minx,maxx),hhuge);
#undef COLQ
}
/*
bool Mesh::Collide (Unit * target, const Transformation &cumtrans, Matrix cumtransmat) {
  Transformation cumulative_transformation(cumtrans);
  cumulative_transformation.position = local_pos.Transform (cumtransmat);

  if (bspTree||target->querySphere (cumulative_transformation.position,rSize())//test0808
   ) {
    float localTrans [16];// {1,0,0,0,0,1,0,0,0,0,1,0,target->Position().i,target->Position.j,target->Position().k,1};
    if (QueryBSP (InvTransform(cumtransmat,target->Position()),target->rSize())) {//bsp      
      fprintf (stderr,"mesh %s intersects unit %s", hash_name->c_str(), target->name.c_str());
      return true;
    }
  }
  return false;
}
*/
bool Unit::OneWayCollide (Unit * target, Vector & normal, float &dist) {//do each of these bubbled subunits collide with the other unit?
  int i;
  if (!querySphere(target->Position(),target->rSize()))
    return false;;
  if (queryBSP(target->Position(), target->rSize(), normal,dist)) {
    normal = ToWorldCoordinates (normal);
    return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->OneWayCollide(target,normal,dist))
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
  Vector normal;
  float dist;
  Unit * bigger;
  Unit * smaller;
  if (radial_size<target->radial_size) {
    bigger = target;
    smaller = this;
  } else {
    bigger = this;
    smaller = target;
  }
  if (!bigger->OneWayCollide(smaller,normal,dist))
    return false;
  float elast = .5*(smaller->GetElasticity()+bigger->GetElasticity());
  //  float speedagainst = (normal.Dot (smaller->GetVelocity()-bigger->GetVelocity()));
  //  smaller->ApplyForce (normal * fabs(elast*speedagainst)/SIMULATION_ATOM);
  //  bigger->ApplyForce (normal * -fabs((elast+1)*speedagainst*smaller->GetMass()/bigger->GetMass())/SIMULATION_ATOM);
  //deal damage similarly to beam damage!!  Apply some sort of repel force
  smaller->ApplyForce (normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  bigger->ApplyForce (normal*(smaller->GetMass()*smaller->GetMass()/bigger->GetMass())*-fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  //each mesh with each mesh? naw that should be in one way collide
  return true;
}


bool Beam::Collide (Unit * target) {

  if (target==owner) 
    return false;
  float distance = target->querySphere (center,direction,0);
  if (distance<0||distance>curlength+target->rSize()) {
  }
  Vector normal;//apply shields

  if (distance = target->queryBSP(center,center+direction*curlength,normal)) { 

    curlength = distance;
    impact|=IMPACT;
    
    //deliver float tmp=(curlength/range)); (damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
    return true;
  }
  return false;
}
