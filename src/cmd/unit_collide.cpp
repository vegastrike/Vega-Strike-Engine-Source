#include "vegastrike.h"
#include "unit.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "gfx/bsp.h"
#include "collide/rapcol.h"
#include "collide/csgeom/transfrm.h"
#include "collide/collider.h"
#include "hashtable.h"
#include <string>
static Hashtable <std::string,collideTrees,char[127]> unitColliders;
collideTrees::collideTrees (const std::string &hk, BSPTree *bT, BSPTree *bS, csRapidCollider *cT, csRapidCollider *cS): hash_key(hk),bspTree(bT), colTree(cT), bspShield(bS), colShield(cS) {
  refcount=1;
  unitColliders.Put (hash_key,this);
}
collideTrees* collideTrees::Get(const std::string &hash_key) {
  return unitColliders.Get(hash_key);
}
void collideTrees::Dec() {
  refcount--;
  if (refcount==0) {
    unitColliders.Delete (hash_key);
    if (bspTree)
      delete bspTree;
    if (colTree) 
      delete colTree;
    if (bspShield)
      delete bspShield;
    if (colShield)
      delete colShield;
    delete this;
    return;
  }
}

bool TableLocationChanged (const Vector & Mini,const Vector & minz) { 
  return (_Universe->activeStarSystem()->collidetable->c.hash_int (Mini.i)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.i) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.j)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.j) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.k)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.k));
}
bool TableLocationChanged (const LineCollide &lc, const Vector &minx, const Vector & maxx) {
  return TableLocationChanged (lc.Mini,minx) || TableLocationChanged (lc.Maxi,maxx);
}
void KillCollideTable (LineCollide * lc,StarSystem * ss) {
  if (lc->type==LineCollide::UNIT) {
    ss->collidetable->c.Remove ( lc,lc->object.u);
  } else {
    printf ("such collide types as %d not allowed",lc->type);
  }
}
bool EradicateCollideTable (LineCollide * lc, StarSystem * ss) {
  if (lc->type==LineCollide::UNIT) { 
    return ss->collidetable->c.Eradicate (lc->object.u);
  } else {
    printf ("such collide types as %d not allowed",lc->type);
    return false;
  }
}
void AddCollideQueue (LineCollide &tmp,StarSystem * ss) {
  if (tmp.type==LineCollide::UNIT) { 
    ss->collidetable->c.Put (&tmp,tmp.object.u);
  } else {
    printf ("such collide types as %d not allowed",tmp.type);
  }
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
#define UNSAFE_COLLIDE_RELEASE
#if (defined SAFE_COLLIDE_DEBUG) || (defined  UNSAFE_COLLIDE_RELEASE) 
  if (CollideInfo.object.u!=NULL) {
    KillCollideTable (&CollideInfo,activeStarSystem);
    CollideInfo.object.u = NULL;
  }
#endif

#ifndef UNSAFE_COLLIDE_RELEASE
#ifdef SAFE_COLLIDE_DEBUG
    if (
#endif
	EradicateCollideTable (&CollideInfo,activeStarSystem)
#ifdef SAFE_COLLIDE_DEBUG 
	) {
      fprintf (stderr,"RECOVERED from (formerly) fatal, currently nonfatal error with unit deletion\n");      
    }
#else
    ;
#endif
#endif
#ifdef SAFE_COLLIDE_DEBUG
    for (unsigned int i=0;i<_Universe->star_system.size();i++) {
      _Universe->pushActiveStarSystem(_Universe->star_system[i]);
    
    if (EradicateCollideTable (&CollideInfo,_Universe->star_system[i])) {
      fprintf (stderr,"VERY BAD ERROR FATAL! 0x%x %s",(int)((int *)(this)),this->name.c_str());
    }
    _Universe->popActiveStarSystem();
    }
#endif
  CollideInfo.object.u=NULL;
  int j;
  for (j=0;j<nummounts;j++) {
    if (mounts[j].type.type==weapon_info::BEAM) {
      if (mounts[j].ref.gun) {
	mounts[j].ref.gun->RemoveFromSystem(true);
      }
    }
  }
  activeStarSystem=NULL;
}

void Unit::UpdateCollideQueue () {
  if (activeStarSystem==NULL) {
    activeStarSystem = _Universe->activeStarSystem();
  } else {
    assert (activeStarSystem==_Universe->activeStarSystem());
  }
  CollideInfo.lastchecked =NULL;//reset who checked it last in case only one thing keeps crashing with it;
  Vector Puffmin (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector Puffmax (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  if (CollideInfo.object.u == NULL||TableLocationChanged(CollideInfo,Puffmin,Puffmax)) {//assume not mutable
    if (CollideInfo.object.u!=NULL) {
      KillCollideTable(&CollideInfo,activeStarSystem);
    }
    CollideInfo.object.u = this;
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
    AddCollideQueue (CollideInfo,activeStarSystem);
  } else {
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
  }
}

void Unit::CollideAll() {
  if (SubUnit||killed)
    return;

  UnitCollection * colQ [tablehuge+1];
  int sizecolq = _Universe->activeStarSystem()->collidetable->c.Get (&CollideInfo,colQ);
  int j = 0;
  for (;j<sizecolq;j++) {
    Unit *un;
    for (un_iter i=colQ[j]->createIterator();(un=(*i))!=NULL;++i) {//warning CANNOT use iterator (except for this sort of collide queue now that I fixed the list
      //UNITS MAY BE DELETED FROM THE CURRENT POINTED TO colQ IN THE PROCESS OF THEIR REMOVAL!!!!       //BUG TERMINATED!
      LineCollide * tmp = &un->CollideInfo;
      if (tmp->lastchecked==this)
	continue;//ignore duplicates
      tmp->lastchecked = this;//now we're the last checked.

      if ((!CollideInfo.hhuge||(CollideInfo.hhuge&&tmp->type==LineCollide::UNIT))&&((tmp->object.u>this||(!CollideInfo.hhuge&&j==0))))//the first stuffs are in the huge array
	if (Position().i+radial_size>tmp->Mini.i&&
	    Position().i-radial_size<tmp->Maxi.i&&
	    Position().j+radial_size>tmp->Mini.j&&
	    Position().j-radial_size<tmp->Maxi.j&&
	    Position().k+radial_size>tmp->Mini.k&&
	    Position().k-radial_size<tmp->Maxi.k) {
	  un->Collide(this);
	}
    
    }
  }
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
  if (smaller->colTrees==NULL||colTrees==NULL)
    return false;
  if (smaller->colTrees->colTree==NULL||colTrees->colTree==NULL)
    return false;

    csRapidCollider::CollideReset();
    Unit * bigger =this;
    const csReversibleTransform bigtransform (bigger->cumulative_transformation_matrix);
    const csReversibleTransform smalltransform (smaller->cumulative_transformation_matrix);
    if (smaller->colTrees->colTree->Collide (*bigger->colTrees->colTree,
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
  bool usecoltree =(colTrees&&target->colTrees)
    ?colTrees->colTree&&target->colTrees->colTree
    : false;
  if (usecoltree) {
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
  BSPTree *const* tmpBsp;
  BSPTree *myNull=NULL;
  if (colTrees) {
    tmpBsp = ShieldUp(st)?&colTrees->bspShield:&colTrees->bspTree;
    if (colTrees->bspTree&&!ShieldBSP) {
      tmpBsp= &colTrees->bspTree;
    }
  } else {
    tmpBsp=&myNull;
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
  BSPTree *myNull=NULL;
  Vector st (InvTransform (cumulative_transformation_matrix,start));
  BSPTree *const* tmpBsp = &myNull;
  if (colTrees) {
    tmpBsp=ShieldUp(st)?&colTrees->bspShield:&colTrees->bspTree;
    if (colTrees->bspTree&&!ShieldBSP) {
      tmpBsp= &colTrees->bspTree;
    }
  }
  for (;tmpBsp!=NULL;tmpBsp=((ShieldUp(st)&&(tmpBsp!=((colTrees?&colTrees->bspTree:&myNull))))?((colTrees?&colTrees->bspTree:&myNull)):NULL)) {
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



float Unit::querySphere (const Vector &start, const Vector &end, float min_radius) const{
  if (!SubUnits.empty()) {
    un_fkiter i=SubUnits.constFastIterator();
    for (const Unit * un;(un=i.current())!=NULL;i.advance()) {
      float tmp;
      if ((tmp=un->querySphere (start,end,min_radius))!=0) {
	return tmp;
      }
    }
  }
 
  return querySphereNoRecurse (start,end,min_radius);
}

float Unit::querySphereNoRecurse (const Vector & start, const Vector & end, float min_radius) const {
  int i;
  float tmp;
  Vector st,dir;
  for (i=0;i<nummesh;i++) {
    float a, b,c;
    st = start - Transform (cumulative_transformation_matrix,meshdata[i]->Position());	
    dir = end-start;//now start and end are based on mesh's position
    // v.Dot(v) = r*r; //equation for sphere
    // (x0 + (x1 - x0) *t) * (x0 + (x1 - x0) *t) = r*r
    c = st.Dot (st) - (min_radius+meshdata[i]->rSize())*(meshdata[i]->rSize()+min_radius)
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


static bool lcwithin (const LineCollide & lc, const LineCollide&tmp) {
  return (lc.Mini.i< tmp.Maxi.i&&
	  lc.Mini.j< tmp.Maxi.j&&
	  lc.Mini.k< tmp.Maxi.k&&
	  lc.Maxi.i> tmp.Mini.i&&
	  lc.Maxi.j> tmp.Mini.j&&
	  lc.Maxi.k> tmp.Mini.k);
}

bool Bolt::Collide () {
  UnitCollection *candidates[2];  
  _Universe->activeStarSystem()->collidetable->c.Get (cur_position,candidates);
  LineCollide minimaxi;//might as well have this so we can utilize common function
  minimaxi.Mini= ( prev_position.Min (cur_position));
  minimaxi.Maxi= ( prev_position.Max (cur_position));
  for (unsigned int j=0;j<2;j++) {
    Unit * un;
    for (un_iter i=candidates[j]->createIterator();(un=*i)!=NULL;++i) {
      
      if (lcwithin (minimaxi,un->GetCollideInfo ())) {
	if (this->Collide (un)) {
	  delete this;
	  return true;
	}
	
      }
    }
  }
  return false;
}

void Beam::CollideHuge (const LineCollide & lc) {
  UnitCollection *colQ [tablehuge+1];
  if (!lc.hhuge) {
    int sizecolq = _Universe->activeStarSystem()->collidetable->c.Get (&lc,colQ);
    for (int j=0;j<sizecolq;j++) {
      Unit *un;
      for (un_iter i=colQ[j]->createIterator();(un=(*i))!=NULL;++i) {

	if (lcwithin(lc,	un->GetCollideInfo())) {
	  this->Collide (un);
	}
      }
    }
  }else {
    un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
    Unit *un;
    for (;(un=*i)!=NULL;++i) {
      if (lcwithin (lc,un->GetCollideInfo())) {
	this->Collide(un);
      }
    }
  }

}


