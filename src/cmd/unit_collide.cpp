#include "vegastrike.h"
#include "unit.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "hashtable_3d.h"
#include "gfx/bsp.h"
#include "collide/rapcol.h"
#include "collide/csgeom/transfrm.h"
#include "collide/collider.h"

bool TableLocationChanged (const Vector & Mini,const Vector & minz) { 
  return (_Universe->activeStarSystem()->collidetable->c.hash_int (Mini.i)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.i) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.j)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.j) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.k)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.k));
}
bool TableLocationChanged (const LineCollide &lc, const Vector &minx, const Vector & maxx) {
  return TableLocationChanged (lc.Mini,minx) || TableLocationChanged (lc.Maxi,maxx);
}
void KillCollideTable (LineCollide * lc) {
  _Universe->activeStarSystem()->collidetable->c.Remove (lc, lc);
}
bool EradicateCollideTable (LineCollide * lc) {
  return _Universe->activeStarSystem()->collidetable->c.Eradicate (lc);
}
void AddCollideQueue (LineCollide &tmp) {
  _Universe->activeStarSystem()->collidetable->c.Put (&tmp,&tmp);
}
void Unit::SetCollisionParent (Unit * name) {
  assert (0); //deprecated... many less collisions with subunits out of the table
#if 0
    for (int i=0;i<numsubunit;i++) {
      subunits[i]->CollideInfo.object.u = name;
      subunits[i]->SetCollisionParent (name);
    }
#endif
}
void Unit::RemoveFromSystem() {
#if (defined SAFE_COLLIDE_DEBUG) || (defined  UNSAFE_COLLIDE_RELEASE) 
  if (CollideInfo.object.u!=NULL) {
    KillCollideTable (&CollideInfo);
    CollideInfo.object.u = NULL;
  }
#endif

#ifndef UNSAFE_COLLIDE_RELEASE
#ifdef SAFE_COLLIDE_DEBUG
    if (
#endif
	EradicateCollideTable (&CollideInfo)
#ifdef SAFE_COLLIDE_DEBUG 
	) {
      fprintf (stderr,"RECOVERED from (formerly) fatal, currently nonfatal error with unit deletion\n");      
    }
#else
    ;
#endif
#endif
  CollideInfo.object.u=NULL;
  int i;
  for (i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].ref.gun) {
	mounts[i].ref.gun->RemoveFromSystem(true);
      }
    }
  }
}

void Unit::UpdateCollideQueue () {
  CollideInfo.lastchecked =NULL;//reset who checked it last in case only one thing keeps crashing with it;
  Vector Puffmin (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector Puffmax (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  if (CollideInfo.object.u == NULL||TableLocationChanged(CollideInfo,Puffmin,Puffmax)) {//assume not mutable
    if (CollideInfo.object.u!=NULL) {
      KillCollideTable(&CollideInfo);
    }
    CollideInfo.object.u = this;
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
    AddCollideQueue (CollideInfo);
  } else {
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
  }
}

void Unit::CollideAll() {
  if (SubUnit||killed)
    return;

  vector <LineCollide*> * colQ [tablehuge+1];
  int sizecolq = _Universe->activeStarSystem()->collidetable->c.Get (&CollideInfo,colQ);
  int j = 0;
  for (;j<sizecolq;j++) {
    for (unsigned int i=0;i<colQ[j]->size();i++) {//warning CANNOT use iterator
      //UNITS MAY BE DELETED FROM THE CURRENT POINTED TO colQ IN THE PROCESS OF THEIR REMOVAL!!!! 
      //BUG TERMINATED!
      LineCollide * tmp = (*colQ[j])[i];
      if (tmp->lastchecked==this)
	continue;//ignore duplicates
      tmp->lastchecked = this;//now we're the last checked.

      if ((!CollideInfo.hhuge||(CollideInfo.hhuge&&tmp->type==LineCollide::UNIT))&&((tmp->object.u>this||(!CollideInfo.hhuge&&j==0))))//the first stuffs are in the huge array
	if (
	    Position().i+radial_size>tmp->Mini.i&&
	    Position().i-radial_size<tmp->Maxi.i&&
	    Position().j+radial_size>tmp->Mini.j&&
	    Position().j-radial_size<tmp->Maxi.j&&
	    Position().k+radial_size>tmp->Mini.k&&
	    Position().k-radial_size<tmp->Maxi.k) {
      //continue;
	  switch (tmp->type) {
	  case LineCollide::UNIT://other units!!!
	    (tmp->object.u)->Collide(this);
	    break;
	  case LineCollide::BEAM:
	    (tmp->object.b)->Collide(this);
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

bool Unit::Inside (const Vector &target, const float radius, Vector & normal, float &dist) {//do each of these bubbled subunits collide with the other unit?
  if (!querySphere(target,radius)) {
    return false;;
  }
  /* seems redudnant
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->Inside(target,radius,normal,dist)) {
      return true;
    }
  }
  */
  if (queryBSP(target, radius, normal,dist,false)) {
    return true;
  }

  return false;
}
bool Unit::InsideCollideTree (Unit * smaller, Vector & bigpos, Vector &bigNormal, Vector & smallpos, Vector & smallNormal) {
    if (smaller->colTree==NULL||colTree==NULL)
      return false;

    csRapidCollider::CollideReset();
    Unit * bigger =this;
    const csReversibleTransform bigtransform (bigger->cumulative_transformation_matrix);
    const csReversibleTransform smalltransform (smaller->cumulative_transformation_matrix);
    if (smaller->colTree->Collide (*bigger->colTree,
				  &smalltransform,
				  &bigtransform)) {
      //static int crashcount=0;
      //      fprintf (stderr,"%s Crashez to %s %d\n", bigger->name.c_str(), smaller->name.c_str(),crashcount++);
      csCollisionPair * mycollide = csRapidCollider::GetCollisions();
      int numHits = csRapidCollider::numHits;
      if (numHits) {
	smallpos.Set((mycollide[0].a1.x+mycollide[0].b1.x+mycollide[0].c1.x)/3,  
		     (mycollide[0].a1.y+mycollide[0].b1.y+mycollide[0].c1.y)/3,  
		     (mycollide[0].a1.z+mycollide[0].b1.z+mycollide[0].c1.z)/3);
	smallpos = Transform (smaller->cumulative_transformation_matrix,smallpos);
	bigpos.Set((mycollide[0].a2.x+mycollide[0].b2.x+mycollide[0].c2.x)/3,  
		   (mycollide[0].a2.y+mycollide[0].b2.y+mycollide[0].c2.y)/3,  
		   (mycollide[0].a2.z+mycollide[0].b2.z+mycollide[0].c2.z)/3);
	bigpos = Transform (bigger->cumulative_transformation_matrix,bigpos);
	csVector3 sn, bn;
	sn.Cross (mycollide[0].b1-mycollide[0].a1,mycollide[0].c1-mycollide[0].a1);
	bn.Cross (mycollide[0].b2-mycollide[0].a2,mycollide[0].c2-mycollide[0].a2);
	sn.Normalize();
	bn.Normalize();
	smallNormal.Set (sn.x,sn.y,sn.z);
	bigNormal.Set (bn.x,bn.y,bn.z);
	smallNormal = TransformNormal (smaller->cumulative_transformation_matrix,smallNormal);
	bigNormal = TransformNormal (bigger->cumulative_transformation_matrix,bigNormal);
	return true;
      }
    }
    UnitCollection::UnitIterator i;
    if (!bigger->SubUnits.empty()) {
      i=bigger->getSubUnits();
      for (Unit * un;(un=i.current())!=NULL;i.advance()) {
	if ((un->InsideCollideTree(smaller,bigpos, bigNormal,smallpos,smallNormal))) {
	  return true;
	}
      }
    }
    if (!smaller->SubUnits.empty()) {
      i=smaller->getSubUnits();
      for (Unit * un;(un=i.current())!=NULL;i.advance()) {
	if ((bigger->InsideCollideTree(un,bigpos, bigNormal,smallpos,smallNormal))) {
	  return true;
	}
      }
    }
    //FIXME
    //doesn't check all i*j options of subunits vs subunits
    return false;
}
bool Unit::Collide (Unit * target) {
  if (target==this||((target->isUnit()!=NEBULAPTR&&isUnit()!=NEBULAPTR)&&(owner==target||target->owner==this||(owner!=NULL&&target->owner==owner))))
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
  if (colTree&&target->colTree) {
    Vector bigpos,smallpos,bigNormal,smallNormal;
    if (bigger->InsideCollideTree (smaller,bigpos,bigNormal,smallpos,smallNormal)) {
      bigger->reactToCollision (smaller,bigpos, bigNormal,smallpos,smallNormal, 10   );	
    }
  } else {
    if (bigger->Inside(smaller->Position(),smaller->rSize(),normal,dist)) {
      if (normal.i==-1&&normal.j==-1) {
	normal = (smaller->Position()-bigger->Position());
	if (normal.i||normal.j||normal.k)
	  normal.Normalize();
      }

      bigger->reactToCollision (smaller,bigger->Position(), normal,smaller->Position(), -normal, dist);
    }else {
      return false;      
    }
  }
  //UNUSED BUT GOOD  float elast = .5*(smallcsReversibleTransform (cumulative_transformation_matrix),er->GetElasticity()+bigger->GetElasticity());
  //BAD  float speedagainst = (normal.Dot (smaller->GetVelocity()-bigger->GetVelocity()));
  //BADF  smaller->ApplyForce (normal * fabs(elast*speedagainst)/SIMULATION_ATOM);
  //BAD  bigger->ApplyForce (normal * -fabs((elast+1)*speedagainst*smaller->GetMass()/bigger->GetMass())/SIMULATION_ATOM);
  //deal damage similarly to beam damage!!  Apply some sort of repel force

  //NOT USED BUT GOOD  Vector farce = normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM));
  return true;
}



Unit * Unit::queryBSP (const Vector &pt, float err, Vector & norm, float &dist, bool ShieldBSP) {
  int i;
  if (!SubUnits.empty()) {
    un_fiter i = SubUnits.fastIterator();
    for (Unit * un;(un=i.current())!=NULL;i.advance()) {
      Unit * retval;
      if ((retval=un->queryBSP(pt,err, norm,dist,ShieldBSP))) {
	return retval;
      }
    }
  }
  Vector st (InvTransform (cumulative_transformation_matrix,pt));
  bool temp=false;
  for (i=0;i<nummesh&&!temp;i++) {
    temp|=meshdata[i]->queryBoundingBox (st,err);
     
  }
  if (!temp)
    return NULL;
  BSPTree *const* tmpBsp = ShieldUp(st)?&bspShield:&bspTree;
  if (bspTree&&!ShieldBSP) {
    tmpBsp= &bspTree;
  }
  if (!(*tmpBsp)) {
    dist = (st - meshdata[i-1]->Position()).Magnitude()-err-meshdata[i-1]->rSize();
    return this;
  }
  if ((*tmpBsp)->intersects (st,err,norm,dist)) {
    norm = ToWorldCoordinates (norm);
    return this;
  }
  return NULL;
}

Unit * Unit::queryBSP (const Vector &start, const Vector & end, Vector & norm, float &distance, bool ShieldBSP) {
  int i;
  Unit * tmp;
  if (!SubUnits.empty()) {
    un_fiter i(SubUnits.fastIterator());
    for (Unit * un;(un=i.current())!=NULL;i.advance()) {
      if ((tmp=un->queryBSP(start,end, norm,distance,ShieldBSP))!=0) {
	return tmp;
      }
    }
  }
  Vector st (InvTransform (cumulative_transformation_matrix,start));
  BSPTree *const* tmpBsp = ShieldUp(st)?&bspShield:&bspTree;
  if (bspTree&&!ShieldBSP) {
    tmpBsp= &bspTree;
  }
  for (;tmpBsp!=NULL;tmpBsp=((ShieldUp(st)&&(tmpBsp!=(&bspTree)))?(&bspTree):NULL)) {
    if (!(*tmpBsp)) {
      distance = querySphereNoRecurse (start,end);
      norm = (distance * (start-end));
      distance = norm.Magnitude();
      norm +=start;
      norm.Normalize();//normal points out from center
      if (distance)
	return this;
      else
	continue;
    }
    Vector ed (InvTransform (cumulative_transformation_matrix,end));
    bool temp=false;
    for (i=0;i<nummesh&&!temp;i++) {
      temp = (1==meshdata[i]->queryBoundingBox (st,ed,0));
    }
    if (!temp) {
      continue;
    }
    if ((distance = (*tmpBsp)->intersects (st,ed,norm))!=0) {
      norm = ToWorldCoordinates (norm);
      return this;
    }
  }
  return NULL;
}


bool Unit::querySphere (const Vector &pnt, float err) const{
  int i;
  const float * tmpo = cumulative_transformation_matrix;
  
  Vector TargetPoint (tmpo[0],tmpo[1],tmpo[2]);
#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));//adjust the ship radius by the scale of local coordinates
#endif
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform (tmpo,meshdata[i]->Position())-pnt;
    if (TargetPoint.Dot (TargetPoint)< 
	err*err+
	meshdata[i]->rSize()*meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
	*SizeScaleFactor*SizeScaleFactor
#endif
	+
#ifdef VARIABLE_LENGTH_PQR
	SizeScaleFactor*
#endif
	2*err*meshdata[i]->rSize()
	)
      return true;
  }
  if (!SubUnits.empty()) {
    un_fkiter i=SubUnits.constFastIterator();
    for (const Unit * un;(un=i.current())!=NULL;i.advance()) {
      if (un->querySphere (pnt,err)) {
	return true;
      }
    }
  }
  return false;
}



float Unit::querySphere (const Vector &start, const Vector &end) const{
  if (!SubUnits.empty()) {
    un_fkiter i=SubUnits.constFastIterator();
    for (const Unit * un;(un=i.current())!=NULL;i.advance()) {
      float tmp;
      if ((tmp=un->querySphere (start,end))!=0) {
	return tmp;
      }
    }
  }
 
  return querySphereNoRecurse (start,end);
}

float Unit::querySphereNoRecurse (const Vector & start, const Vector & end) const {
  int i;
  float tmp;
  Vector st,dir;
  for (i=0;i<nummesh;i++) {
    float a, b,c;
    st = start - Transform (cumulative_transformation_matrix,meshdata[i]->Position());	
    dir = end-start;//now start and end are based on mesh's position
    // v.Dot(v) = r*r; //equation for sphere
    // (x0 + (x1 - x0) *t) * (x0 + (x1 - x0) *t) = r*r
    c = st.Dot (st) - meshdata[i]->rSize()*meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
      *SizeScaleFactor*SizeScaleFactor
#endif
      ;
    b = 2 * (dir.Dot (st));
    a = dir.Dot(dir);
    //b^2-4ac
    c = b*b - 4*a*c;
    if (c<0||a==0)
      continue;
    a *=2;
      
    tmp = (-b + sqrtf (c))/a;
    c = (-b - sqrtf (c))/a;
    if (tmp>0&&tmp<=1) {
      return (c>0&&c<tmp) ? c : tmp;
    } else if (c>0&&c<=1) {
	return c;
    }
  }
  return 0;
}

void Unit::Destroy() {
  if (!killed)
    if (!Explode(false,SIMULATION_ATOM))
      Kill();
}





bool Bolt::Collide () {
  vector <LineCollide *> *candidates[2];  
  _Universe->activeStarSystem()->collidetable->c.Get (cur_position,candidates);
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
	  if (this->Collide ((*i)->object.u)) {
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
  vector <LineCollide *> tmp = _Universe->activeStarSystem()->collidetable->c.GetHuge();
  for (unsigned int i=0;i<tmp.size();i++) {
    if (tmp[i]->type==LineCollide::UNIT) {
      if (lc.Mini.i< tmp[i]->Maxi.i&&
	  lc.Mini.j< tmp[i]->Maxi.j&&
	  lc.Mini.k< tmp[i]->Maxi.k&&
	  lc.Maxi.i> tmp[i]->Mini.i&&
	  lc.Maxi.j> tmp[i]->Mini.j&&
	  lc.Maxi.k> tmp[i]->Mini.k) {
	this->Collide ((Unit*)tmp[i]->object.u);
      }
    }
  }

}


