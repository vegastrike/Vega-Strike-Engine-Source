
#include "vegastrike.h"
//#include "unit.h"
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
#include "vs_globals.h"
#include "configxml.h"
#include "collide.h"

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
      fprintf (stderr,"VERY BAD ERROR FATAL! 0x%lx %s",(long)((long *)(this)),this->name.c_str());
    }
    _Universe->popActiveStarSystem();
    }
#endif
  CollideInfo.object.u=NULL;
  int j;
  for (j=0;j<GetNumMounts();j++) {
    if (mounts[j].type->type==weapon_info::BEAM) {
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
  QVector Puffmin (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  QVector Puffmax (Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  if (CollideInfo.object.u == NULL||TableLocationChanged(CollideInfo,Puffmin,Puffmax)) {//assume not mutable
    if (CollideInfo.object.u!=NULL) {
      KillCollideTable(&CollideInfo,activeStarSystem);
    }
    CollideInfo.object.u = this;
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
	CollideInfo.type=LineCollide::UNIT;
    AddCollideQueue (CollideInfo,activeStarSystem);
  } else {
    CollideInfo.Mini= Puffmin;
    CollideInfo.Maxi=Puffmax;
  }
}
extern bool usehuge_table();
void Unit::CollideAll() {
  if (SubUnit||killed)
    return;

  UnitCollection * colQ [tablehuge+1];
  bool usehuge = usehuge_table()||GetJumpStatus().drive>=0;
  int sizecolq = _Universe->activeStarSystem()->collidetable->c.Get (&CollideInfo,colQ,usehuge);
  if (CollideInfo.hhuge&&GetJumpStatus().drive>=0) {
    _Universe->activeStarSystem()->collidetable->c.AddHugeToActive(this);
  }

  int j = 0;
  for (;j<sizecolq;j++) {
    Unit *un;
    for (un_iter i=colQ[j]->createIterator();(un=(*i))!=NULL;++i) {//warning CANNOT use iterator (except for this sort of collide queue now that I fixed the list
      //UNITS MAY BE DELETED FROM THE CURRENT POINTED TO colQ IN THE PROCESS OF THEIR REMOVAL!!!!       //BUG TERMINATED!
      LineCollide * tmp = &(un)->CollideInfo;
      if (tmp->lastchecked==this)
	continue;//ignore duplicates
      tmp->lastchecked = this;//now we're the last checked.

	  if ((!Unit::CollideInfo.hhuge||(CollideInfo.hhuge&&tmp->type==LineCollide::UNIT))&&((tmp->object.u>this||GetJumpStatus().drive>=0||un->GetJumpStatus().drive>=0||(!CollideInfo.hhuge&&j==0))))//the first stuffs are in the huge array
	if (Position().i+radial_size>tmp->Mini.i&&
	    Position().i-radial_size<tmp->Maxi.i&&
	    Position().j+radial_size>tmp->Mini.j&&
	    Position().j-radial_size<tmp->Maxi.j&&
	    Position().k+radial_size>tmp->Mini.k&&
	    Position().k-radial_size<tmp->Maxi.k) {
	  if (un->Collide(this)) {
	    if (j==0&&usehuge) {
	      _Universe->activeStarSystem()->collidetable->c.AddHugeToActive(un);
	    }
	  }
	}
    
    }
  }
}

Vector Vabs (const Vector &in) {
	return Vector (in.i>=0?in.i:-in.i,
				   in.j>=0?in.j:-in.j,
				   in.k>=0?in.k:-in.k);
}

Matrix WarpMatrixForCollisions (Unit * un, const Matrix& ctm) {
	if (un->GetVelocity().MagnitudeSquared()*SIMULATION_ATOM*SIMULATION_ATOM<un->rSize()*un->rSize()) {
		return ctm;
	}else {
		Matrix k(ctm);
		const Vector v(Vector(1,1,1)+Vabs(ctm.getR()*ctm.getR().Dot(un->GetVelocity().Scale(100*SIMULATION_ATOM/un->rSize()))));
		
/*		k.r[0]*=v.i;
		k.r[1]*=v.i;
		k.r[2]*=v.i;

		k.r[3]*=v.j;
		k.r[4]*=v.j;
s		k.r[5]*=v.j;

		k.r[6]*=v.k;
		k.r[7]*=v.k;
		k.r[8]*=v.k;*/
		k.r[0]*=v.i;
		k.r[1]*=v.j;
		k.r[2]*=v.k;

		k.r[3]*=v.i;
		k.r[4]*=v.j;
		k.r[5]*=v.k;

		k.r[6]*=v.i;
		k.r[7]*=v.j;
		k.r[8]*=v.k;		
		return k;
	}
}
bool Unit::Inside (const QVector &target, const float radius, Vector & normal, float &dist) {//do each of these bubbled subunits collide with the other unit?
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
static float tmpmax (float a, float b) {
  return a>b?a:b;
}
bool Unit::InsideCollideTree (Unit * smaller, QVector & bigpos, Vector &bigNormal, QVector & smallpos, Vector & smallNormal) {
  if (smaller->colTrees==NULL||this->colTrees==NULL)
    return false;
  if (smaller->colTrees->usingColTree()==false||this->colTrees->usingColTree()==false)
    return false;

    csRapidCollider::CollideReset();
    //    printf ("Col %s %s\n",name.c_str(),smaller->name.c_str());
    Unit * bigger =this;

#ifdef SUPERCOLLIDER
    float unitsmovement = tmpmax((bigger->curr_physical_state.position-bigger->prev_physical_state.position).Cast().Magnitude()/bigger->rSize(),(smaller->curr_physica
																																				 l_state.position-smaller->prev_physical_state.position).Cast().Magnitude()/smaller->rSize())/2;
    static float max_collision_accuracy = XMLSupport::parse_float (vs_config->getVariable("physics","max_collision_accuracy","10"));
    if (unitsmovement>max_collision_accuracy)
      unitsmovement=max_collision_accuracy;
    if (unitsmovement<1)
     unitsmovement=1;
    int um = (int)unitsmovement;
    if (um>1) {
      //printf ("um >1 for %s with %s\n",bigger->name.c_str(),smaller->name.c_str());
    }
#endif
    const csReversibleTransform bigtransform (/*WarpMatrixForCollisions(bigger)*/bigger->cumulative_transformation_matrix);
	const csReversibleTransform smalltransform (/*WarpMatrixForCollisions(smaller)*/smaller->cumulative_transformation_matrix);
#ifdef SUPERCOLLIDER
    for (int iter=1;iter<=/*um*/1;++iter) 
#endif
      {
      //we're only gonna lerp the positions for speed here... gahh!
#ifdef SUPERCOLLIDER
      float nowness=(((float)iter)/um);
      float thenness = ((float)( um-iter))/um;
      {
	QVector bigorig(nowness*bigger->curr_physical_state.position+thenness*bigger->prev_physical_state.position);
	bigtransform.SetOrigin(bigorig.Cast());
      }{
	QVector smallorig(nowness*smaller->curr_physical_state.position+thenness*smaller->prev_physical_state.position);
	smalltransform.SetOrigin(smallorig.Cast());
      }
#endif
      if (smaller->colTrees->colTree(smaller)->Collide (*bigger->colTrees->colTree(bigger),
													 &smalltransform,
													 &bigtransform)) {
	//static int crashcount=0;
	//      fprintf (stderr,"%s Crashez to %s %d\n", bigger->name.c_str(), smaller->name.c_str(),crashcount++);
	csCollisionPair * mycollide = csRapidCollider::GetCollisions();
	int numHits = csRapidCollider::numHits;
	if (numHits) {
		printf ("%s hit %s\n",smaller->name.c_str(),bigger->name.c_str());
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
    }
    UnitCollection::UnitIterator i;
    static float rsizelim = XMLSupport::parse_float (vs_config->getVariable ("physics","smallest_subunit_to_collide",".2"));
    if (!bigger->SubUnits.empty()) {
      i=bigger->getSubUnits();
      for (Unit * un;(un=i.current())!=NULL;i.advance()) {
	if ((bigger->isUnit()!=ASTEROIDPTR)&&(un->rSize()/bigger->rSize()<rsizelim)) {
	  break;
	}else {
	  //	  printf ("s:%f",un->rSize()/bigger->rSize());
	}
	if ((un->InsideCollideTree(smaller,bigpos, bigNormal,smallpos,smallNormal))) {
	  return true;
	}
      }
    }
    if (!smaller->SubUnits.empty()) {
      i=smaller->getSubUnits();
      for (Unit * un;(un=i.current())!=NULL;i.advance()) {
	if ((smaller->isUnit()!=ASTEROIDPTR)&&(un->rSize()/smaller->rSize()<rsizelim)) {
	  //	  printf ("s:%f",un->rSize()/smaller->rSize());
	  break;

	}
	if ((bigger->InsideCollideTree(un,bigpos, bigNormal,smallpos,smallNormal))) {
	  return true;
	}
      }
    }
    //FIXME
    //doesn't check all i*j options of subunits vs subunits
    return false;
}

Unit * Unit::BeamInsideCollideTree (const QVector & start,const QVector & end, QVector & pos, Vector &norm, double &distance) {
  QVector r (end-start);
  double mag = r.Magnitude();
  if (mag>0) {
    r = r*(1./mag);
  }
  {
    bool temp=true;
    if (this->colTrees==NULL) {
      temp=true;
    }else if (this->colTrees->colTree(this)==NULL) {
      temp=true;
    }
    if (temp) {
      float dis=distance;
      Unit * ret= queryBSP(start,end,norm,dis);
      distance=dis;
      pos=start+r*distance;
      return ret;
    }
  }
  QVector p(-r.k,r.i,-r.j);
  QVector q;
  ScaledCrossProduct(r,p,q);
  ScaledCrossProduct(q,r,p);
  csRapidCollider::CollideReset();
  //    printf ("Col %s %s\n",name.c_str(),smaller->name.c_str());
  const csReversibleTransform bigtransform (cumulative_transformation_matrix);
  Matrix smallerMat(p.Cast(),q.Cast(),r.Cast());
  smallerMat.p = start;
  const csReversibleTransform smalltransform (smallerMat);
  bsp_polygon tri;
  tri.v.push_back(Vector(-mag/1024,0,0));
  tri.v.push_back(Vector(-mag/1024,0,mag));
  tri.v.push_back(Vector(mag/1024,0,mag));
  tri.v.push_back(Vector(mag/1024,0,0));
  vector <bsp_polygon> mesh;
  mesh.push_back(tri);
  csRapidCollider smallColTree(mesh);
  if (smallColTree.Collide (*(this->colTrees)->colTree(this),
							&smalltransform,
							&bigtransform)) {
      static int crashcount=0;
    
            fprintf (stderr,"%s Beam Crashez %d\n", name.c_str(),crashcount++);
      csCollisionPair * mycollide = csRapidCollider::GetCollisions();
      int numHits = csRapidCollider::numHits;
      if (numHits) {
	printf ("num hits %d",numHits);
	/*
	pos.Set((mycollide[0].a1.x+mycollide[0].b1.x+mycollide[0].c1.x)/3,  
		     (mycollide[0].a1.y+mycollide[0].b1.y+mycollide[0].c1.y)/3,  
		     (mycollide[0].a1.z+mycollide[0].b1.z+mycollide[0].c1.z)/3);
	pos = Transform (smaller->cumulative_transformation_matrix,smallpos);
	*/
	pos.Set((mycollide[0].a2.x+mycollide[0].b2.x+mycollide[0].c2.x)/3,  
		   (mycollide[0].a2.y+mycollide[0].b2.y+mycollide[0].c2.y)/3,  
		   (mycollide[0].a2.z+mycollide[0].b2.z+mycollide[0].c2.z)/3);
	pos = Transform (cumulative_transformation_matrix,pos);
	csVector3 sn, bn;
	sn.Cross (mycollide[0].b1-mycollide[0].a1,mycollide[0].c1-mycollide[0].a1);
	bn.Cross (mycollide[0].b2-mycollide[0].a2,mycollide[0].c2-mycollide[0].a2);
	sn.Normalize();
	bn.Normalize();
	//	smallNormal.Set (sn.x,sn.y,sn.z);
	norm.Set (bn.x,bn.y,bn.z);
	//smallNormal = TransformNormal (smaller->cumulative_transformation_matrix,smallNormal);
	norm = TransformNormal (cumulative_transformation_matrix,norm);
	distance = (pos-start).Magnitude();
	return this;
      }
    }
#if 0
    UnitCollection::UnitIterator i;
    static float rsizelim = XMLSupport::parse_float (vs_config->getVariable ("physics","smallest_subunit_to_collide",".2"));
    if (!bigger->SubUnits.empty()) {
      i=bigger->getSubUnits();
      for (Unit * un;(un=i.current())!=NULL;i.advance()) {
	if ((bigger->isUnit()!=ASTEROIDPTR)&&(un->rSize()/bigger->rSize()<rsizelim)) {
	  break;
	}else {
	  //	  printf ("s:%f",un->rSize()/bigger->rSize());
	}
	if ((un->InsideCollideTree(smaller,bigpos, bigNormal,smallpos,smallNormal))) {
	  return true;
	}
      }
    }
    if (!smaller->SubUnits.empty()) {
      i=smaller->getSubUnits();
      for (Unit * un;(un=i.current())!=NULL;i.advance()) {
	if ((smaller->isUnit()!=ASTEROIDPTR)&&(un->rSize()/smaller->rSize()<rsizelim)) {
	  //	  printf ("s:%f",un->rSize()/smaller->rSize());
	  break;

	}
	if ((bigger->InsideCollideTree(un,bigpos, bigNormal,smallpos,smallNormal))) {
	  return true;
	}
      }
    }
#endif

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
  if ((this->DockedOrDocking()&(DOCKED_INSIDE|DOCKED))||(target->DockedOrDocking()&(DOCKED_INSIDE|DOCKED))) {
    return false;
  }
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
  bool usecoltree =(this->colTrees&&target->colTrees)
	  ?this->colTrees->colTree(this)&&target->colTrees->colTree(this)
    : false;
  if (usecoltree) {
    QVector bigpos,smallpos;
    Vector bigNormal,smallNormal;
    if (bigger->InsideCollideTree (smaller,bigpos,bigNormal,smallpos,smallNormal)) {
      if (!bigger->isDocked(smaller)&&!smaller->isDocked(bigger)) {
	bigger->reactToCollision (smaller,bigpos, bigNormal,smallpos,smallNormal, 10   ); 
      } else return false;
    } else return false;
  } else {
    if (bigger->Inside(smaller->Position(),smaller->rSize(),normal,dist)) {
      if (normal.i==-1&&normal.j==-1) {
	normal = (smaller->Position()-bigger->Position()).Cast();
	if (normal.i||normal.j||normal.k)
	  normal.Normalize();
      }
      if (!bigger->isDocked(smaller)&&!smaller->isDocked(bigger)) {
	bigger->reactToCollision (smaller,bigger->Position(), normal,smaller->Position(), -normal, dist);
      }else return false;
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

 
Unit * Unit::queryBSP (const QVector &pt, float err, Vector & norm, float &dist, bool ShieldBSP) {
  int i;
  if (RecurseIntoSubUnitsOnCollision)
  if (!SubUnits.empty()) {
    un_fiter i = SubUnits.fastIterator();
    for (Unit * un;(un=i.current())!=NULL;i.advance()) {
      Unit * retval;
      if ((retval=un->queryBSP(pt,err, norm,dist,ShieldBSP))) {
	return retval;
      }
    }
  }
  QVector st (InvTransform (cumulative_transformation_matrix,pt));
  bool temp=false;
  for (i=0;i<nummesh()&&!temp;i++) {
    temp|=meshdata[i]->queryBoundingBox (st,err);
  }
  if (!temp)
    return NULL;
  BSPTree *const* tmpBsp;
  BSPTree *myNull=NULL;
  if (this->colTrees) {
    tmpBsp = ShieldUp(st.Cast())?&this->colTrees->bspShield:&this->colTrees->bspTree;
    if (this->colTrees->bspTree&&!ShieldBSP) {
      tmpBsp= &this->colTrees->bspTree;
    }
  } else {
    tmpBsp=&myNull;
  }
  if (!(*tmpBsp)) {
    dist = (st - meshdata[i-1]->Position().Cast()).Magnitude()-err-meshdata[i-1]->rSize();
    return this;
  }
  if ((*tmpBsp)->intersects (st.Cast(),err,norm,dist)) {
    norm = ToWorldCoordinates (norm);
    return this;
  }
  return NULL;
}


Unit * Unit::queryBSP (const QVector &start, const QVector & end, Vector & norm, float &distance, bool ShieldBSP) {
  Unit * tmp;
  if (RecurseIntoSubUnitsOnCollision)
  if (!SubUnits.empty()) {
    un_fiter i(SubUnits.fastIterator());
    for (Unit * un;(un=i.current())!=NULL;i.advance()) {
      if ((tmp=un->queryBSP(start,end, norm,distance,ShieldBSP))!=0) {
	return tmp;
      }
    }
  }
  BSPTree *myNull=NULL;
  BSPTree *const* tmpBsp = &myNull;
#ifdef OLDSHITWHERESHIELDSUSEDTOMATTER
  if (this->colTrees) {
    tmpBsp=ShieldUp(st.Cast())?&this->colTrees->bspShield:&this->colTrees->bspTree;
    if (this->colTrees->bspTree&&!ShieldBSP) {
      tmpBsp= &this->colTrees->bspTree;
    }
    tmpBsp = &this->colTrees->bspTree;
  }
#endif
  //for (;tmpBsp!=NULL;tmpBsp=((ShieldUp(st.Cast())&&(tmpBsp!=((this->colTrees?&this->colTrees->bspTree:&myNull))))?((this->colTrees?&this->colTrees->bspTree:&myNull)):NULL)) {
    distance = querySphereNoRecurse (start,end);
    if (distance) {
      if (!(*tmpBsp)) {
	norm = (distance * (start-end)).Cast();
	distance = norm.Magnitude();
	norm= (norm.Cast()+start).Cast();
	norm.Normalize();//normal points out from center
	return this;
      }
    }else
      return NULL;
    QVector st (InvTransform (cumulative_transformation_matrix,start));
    QVector ed (InvTransform (cumulative_transformation_matrix,end));
    /*bool temp=false;
        for (i=0;i<nummesh()&&!temp;i++) {
      temp = (1==meshdata[i]->queryBoundingBox (st,ed,0));
    }
    if (!temp) {
      return NULL;
    }*/
    if ((distance = (*tmpBsp)->intersects (st.Cast(),ed.Cast(),norm))!=0) {
      norm = ToWorldCoordinates (norm);
      return this;
    }
    //}
  return NULL;
}



bool Unit::querySphere (const QVector &pnt, float err) const{
  int i;
  const Matrix * tmpo = &cumulative_transformation_matrix;
  
  Vector TargetPoint (tmpo->getP());
#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));//adjust the ship radius by the scale of local coordinates
#endif
  for (i=0;i<nummesh();i++) {
    TargetPoint = (Transform (*tmpo,meshdata[i]->Position()).Cast()-pnt).Cast();
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
  if (RecurseIntoSubUnitsOnCollision) 
  if (!SubUnits.empty()) {
    un_fkiter i=SubUnits.constFastIterator();
    for (const Unit * un;(un=i.current())!=NULL;i.advance()) {
      if ((un)->querySphere (pnt,err)) {
	return true;
      }
    }
  }
  return false;
}


float Unit::querySphere (const QVector &start, const QVector &end, float min_radius) const{
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


float Unit::querySphereNoRecurse (const QVector & start, const QVector & end, float min_radius) const {
  int i;
  float tmp;
  QVector st,dir;
  //if( min_radius<0.00001)
  // min_radius = 0;
  for (i=0;i<nummesh();i++) {
	if ((meshdata[i]->Position().Magnitude()>this->rSize())||(meshdata[i]->rSize()>30+this->rSize())) {
		continue;
	}
	if (isUnit()==PLANETPTR&&i>0)
		break;
    float a, b,c;
    st = start - Transform (cumulative_transformation_matrix,meshdata[i]->Position()).Cast();	
    dir = end-start;//now start and end are based on mesh's position
    // v.Dot(v) = r*r; //equation for sphere
    // (x0 + (x1 - x0) *t) * (x0 + (x1 - x0) *t) = r*r
    c = st.Dot (st);
	float temp1 = (min_radius+meshdata[i]->rSize());
	if( min_radius!=-FLT_MAX)
		c = c - temp1*temp1;
	else
		c = temp1;
#ifdef VARIABLE_LENGTH_PQR
    c *= SizeScaleFactor*SizeScaleFactor;
#endif
    b = 2 * (dir.Dot (st));
    a = dir.Dot(dir);
    //b^2-4ac
	if( min_radius!=-FLT_MAX)
  	  c = b*b - 4*a*c;
	else
      c = FLT_MAX;
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
