#include "cmd_unit.h"
#include "cmd_beam.h"
#include "gfx_mesh.h"
#include "cmd_collide.h"
#include "physics.h"
#include "hashtable_3d.h"

Hashtable3d <const LineCollide*, char[20],char[200]> collidetable;

bool TableLocationChanged (const Vector & Mini,const Vector & minz) { 
  return (collidetable.hash_int (Mini.i)!=collidetable.hash_int (minz.i) ||
	  collidetable.hash_int (Mini.j)!=collidetable.hash_int (minz.j) ||
	  collidetable.hash_int (Mini.k)!=collidetable.hash_int (minz.k));
}
bool TableLocationChanged (const LineCollide &lc, const Vector &minx, const Vector & maxx) {
  return TableLocationChanged (lc.Mini,minx) || TableLocationChanged (lc.Maxi,maxx);
}
void KillCollideTable (LineCollide * lc) {
  collidetable.Remove (lc, lc);
}
void AddCollideQueue (const LineCollide &tmp) {
  collidetable.Put (&tmp,&tmp);
}
void Unit::UpdateCollideQueue () {
  Vector Puffmin (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector Puffmax (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  if (CollideInfo.object == NULL||TableLocationChanged(CollideInfo,Puffmin,Puffmax)) {//assume not mutable
    if (CollideInfo.object!=NULL)
      KillCollideTable(&CollideInfo);
    CollideInfo.object = this;
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
    AddCollideQueue (CollideInfo);
  }
}

void Unit::CollideAll() {
  unsigned int i;
#define COLQ colQ
  vector <const LineCollide*> colQ;
  bool huge = collidetable.Get (CollideInfo.Mini,CollideInfo.Maxi,colQ);
  for (i=0;i<COLQ.size();i++) {
    //    if (colQ[i]->object > this||)//only compare against stuff bigger than you
    if ((!huge||(huge&&COLQ[i]->type==LineCollide::UNIT))&&((COLQ[i]->object>this||(!huge&&i<collidetable.GetHuge().size()))))//the first stuffs are in the huge array
      if (
	  Position().i+radial_size>COLQ[i]->Mini.i&&
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
#undef COLQ
}

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
  fprintf (stderr,"Collidison %s %s",name.c_str(),target->name.c_str());
  //GOODsmaller->ApplyForce (normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  //GOODbigger->ApplyForce (normal*(smaller->GetMass()*smaller->GetMass()/bigger->GetMass())*-fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  //each mesh with each mesh? naw that should be in one way collide
  return true;
}

void Beam::CollideHuge (const LineCollide & lc) {
  vector <const LineCollide *> tmp = collidetable.GetHuge();
  for (int i=0;i<tmp.size();i++) {
    if (tmp[i]->type==LineCollide::UNIT) {
      if (lc.Mini.i< tmp[i]->Maxi.i&&
	  lc.Mini.j< tmp[i]->Maxi.j&&
	  lc.Mini.k< tmp[i]->Maxi.k&&
	  lc.Maxi.i> tmp[i]->Mini.i&&
	  lc.Maxi.j> tmp[i]->Mini.j&&
	  lc.Maxi.k> tmp[i]->Mini.k) {
	this->Collide ((Unit*)tmp[i]->object);
      }
    }
  }

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
    fprintf (stderr, "beam delivers damage to %s", target->name.c_str());
    //deliver float tmp=(curlength/range)); (damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
    return true;
  }
  return false;
}
