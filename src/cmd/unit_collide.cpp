#include "vegastrike.h"
#include "unit.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "hashtable_3d.h"

Hashtable3d <LineCollide*, char[20],char[200]> collidetable;

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
void AddCollideQueue (LineCollide &tmp) {
  collidetable.Put (&tmp,&tmp);
}
void Unit::SetCollisionParent (Unit * name) {
    for (int i=0;i<numsubunit;i++) {
      subunits[i]->CollideInfo.object = name;
      subunits[i]->SetCollisionParent (name);
    }
}
void Unit::UpdateCollideQueue () {
  CollideInfo.lastchecked =NULL;//reset who checked it last in case only one thing keeps crashing with it;
  Vector Puffmin (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector Puffmax (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  if (CollideInfo.object == NULL||TableLocationChanged(CollideInfo,Puffmin,Puffmax)) {//assume not mutable
    if (CollideInfo.object!=NULL) {
      KillCollideTable(&CollideInfo);
    }
    CollideInfo.object = this;
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
    AddCollideQueue (CollideInfo);
  } else {
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
  }
}

void Unit::CollideAll() {

  vector <LineCollide*>::const_iterator i;
  vector <LineCollide*> * colQ [HUGEOBJECT+1];
  int sizecolq = collidetable.Get (&CollideInfo,colQ);
  int j = 0;
  for (;j<sizecolq;j++) {
    for (i=colQ[j]->begin();i!=colQ[j]->end();i++) {
      if ((*i)->lastchecked==this)
	continue;//ignore duplicates
      (*i)->lastchecked = this;//now we're the last checked.
      //    if (colQ[i]->object > this||)//only compare against stuff bigger than you
      if ((!CollideInfo.hhuge||(CollideInfo.hhuge&&(*i)->type==LineCollide::UNIT))&&(((*i)->object>this||(!CollideInfo.hhuge&&j==0))))//the first stuffs are in the huge array
	if (
	    Position().i+radial_size>(*i)->Mini.i&&
	    Position().i-radial_size<(*i)->Maxi.i&&
	    Position().j+radial_size>(*i)->Mini.j&&
	    Position().j-radial_size<(*i)->Maxi.j&&
	    Position().k+radial_size>(*i)->Mini.k&&
	    Position().k-radial_size<(*i)->Maxi.k) {
      //continue;
	  switch ((*i)->type) {
	  case LineCollide::UNIT://other units!!!
	    ((Unit*)(*i)->object)->Collide(this);
	    break;
	  case LineCollide::BEAM:
	    ((Beam*)(*i)->object)->Collide(this);
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
  }
#undef COLQ
}

bool Unit::OneWayCollide (Unit * target, Vector & normal, float &dist) {//do each of these bubbled subunits collide with the other unit?
  int i;
  if (!querySphere(target->Position(),target->rSize()))
    return false;;
  if (queryBSP(target->Position(), target->rSize(), normal,dist,false)) {

    return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->OneWayCollide(target,normal,dist))
      return true;
  }

  return false;
}


bool Unit::Collide (Unit * target) {
  if (target==this||owner==target||target->owner==this||target->owner==owner) 
    return false;

  //unit v unit? use point sampling?
  //now first make sure they're within bubbles of each other...
  if ((Position()-target->Position()).Magnitude()>radial_size+target->radial_size)
    return false;
  //now do some serious checks
  Vector normal(-1,-1,-1);
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
  //UNUSED BUT GOOD  float elast = .5*(smaller->GetElasticity()+bigger->GetElasticity());
  //BAD  float speedagainst = (normal.Dot (smaller->GetVelocity()-bigger->GetVelocity()));
  //BADF  smaller->ApplyForce (normal * fabs(elast*speedagainst)/SIMULATION_ATOM);
  //BAD  bigger->ApplyForce (normal * -fabs((elast+1)*speedagainst*smaller->GetMass()/bigger->GetMass())/SIMULATION_ATOM);
  //deal damage similarly to beam damage!!  Apply some sort of repel force
  if (normal.i==-1&&normal.j==-1) {
    normal = (smaller->Position()-bigger->Position());
    if (normal.i||normal.j||normal.k)
      normal.Normalize();
  }
  //NOT USED BUT GOOD  Vector farce = normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM));
  smaller->ApplyForce (normal*.4*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  bigger->ApplyForce (normal*.4*(smaller->GetMass()*smaller->GetMass()/bigger->GetMass())*-fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  smaller->ApplyDamage (bigger->Position(),-normal,  .5*fabs(normal.Dot(smaller->GetVelocity()-bigger->GetVelocity()))*bigger->mass*SIMULATION_ATOM,GFXColor(1,1,1,1));
  bigger->ApplyDamage (smaller->Position(),normal, .5*fabs(normal.Dot(smaller->GetVelocity()-bigger->GetVelocity()))*smaller->mass*SIMULATION_ATOM,GFXColor(1,1,1,1));

  //each mesh with each mesh? naw that should be in one way collide
  return true;
}

bool Bolt::Collide () {
  vector <LineCollide *> *candidates[2];  
  collidetable.Get (cur_position,candidates);
  Vector Mini ( prev_position.Min (cur_position));
  Vector Maxi ( prev_position.Max (cur_position));
  for (unsigned int j=0;j<2;j++) {
    for (vector <LineCollide *>::iterator i=candidates[j]->begin();i!=candidates[j]->end();i++) {
      if ((*i)->type==LineCollide::UNIT) {
	if (Mini.i< (*i)->Maxi.i&&
	    Mini.j< (*i)->Maxi.j&&
	    Mini.k< (*i)->Maxi.k&&
	    Maxi.i> (*i)->Mini.i&&
	    Maxi.j> (*i)->Mini.j&&
	    Maxi.k> (*i)->Mini.k) {
	  if (this->Collide ((Unit*)(*i)->object)) {
	    delete this;
	    return true;
	  }
	}
      }
    }
  }
  return false;
}


void Beam::CollideHuge (const LineCollide & lc) {
  vector <LineCollide *> tmp = collidetable.GetHuge();
  for (unsigned int i=0;i<tmp.size();i++) {
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


