#include "vegastrike.h"
#include "unit.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "hashtable_3d.h"
#include "gfx/bsp.h"
const int tablehuge=12;
Hashtable3d <LineCollide*, char[20],char[200], char [tablehuge]> collidetable;

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
      subunits[i]->CollideInfo.object.u = name;
      subunits[i]->SetCollisionParent (name);
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
  if (SubUnit)
    return;

  vector <LineCollide*> * colQ [tablehuge+1];
  int sizecolq = collidetable.Get (&CollideInfo,colQ);
  int j = 0;
  for (;j<sizecolq;j++) {
    for (unsigned int i=0;i<colQ[j]->size();i++) {//warning CANNOT use iterator
      //UNITS MAY BE DELETED FROM THE CURRENT POINTED TO colQ IN THE PROCESS OF THEIR REMOVAL!!!! 
      //BUG TERMINATED!
      LineCollide * tmp = (*colQ[j])[i];
      if (tmp->lastchecked==this)
	continue;//ignore duplicates
      tmp->lastchecked = this;//now we're the last checked.
      //    if (colQ[i]->object.u > this||)//only compare against stuff bigger than you
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

bool Unit::Inside (const Vector &target, const float radius, Vector & normal, float &dist) const {//do each of these bubbled subunits collide with the other unit?
  int i;
  if (!querySphere(target,radius)) {
    return false;;
  }
  if (queryBSP(target, radius, normal,dist,false)) {
    return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->Inside(target,radius,normal,dist)) {
      return true;
    }
  }

  return false;
}


bool Unit::Collide (Unit * target) {
  if (target==this||owner==target||target->owner==this||(owner!=NULL&&target->owner==owner)) 
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
  if (!bigger->Inside(smaller->Position(),smaller->rSize(),normal,dist))
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
  bigger->reactToCollision (smaller,normal,dist);
  //NOT USED BUT GOOD  Vector farce = normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM));
  return true;
}
void Unit::reactToCollision(Unit * smalle, const Vector & normal, float dist) {
  smalle->ApplyForce (normal*.4*smalle->GetMass()*fabs(normal.Dot (((smalle->GetVelocity()-this->GetVelocity())/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  this->ApplyForce (normal*.4*(smalle->GetMass()*smalle->GetMass()/this->GetMass())*-fabs(normal.Dot ((smalle->GetVelocity()-this->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
  smalle->ApplyDamage (this->Position(),-normal,  .5*fabs(normal.Dot(smalle->GetVelocity()-this->GetVelocity()))*this->mass*SIMULATION_ATOM,GFXColor(1,1,1,1));
  this->ApplyDamage (smalle->Position(),normal, .5*fabs(normal.Dot(smalle->GetVelocity()-this->GetVelocity()))*smalle->mass*SIMULATION_ATOM,GFXColor(1,1,1,1));
  //each mesh with each mesh? naw that should be in one way collide

}



bool Unit::queryBSP (const Vector &pt, float err, Vector & norm, float &dist, bool ShieldBSP) const{
  int i;
  for (i=0;i<numsubunit;i++) {
    if ((subunits[i]->queryBSP(pt,err, norm,dist,ShieldBSP)))
      return true;
  }
  Vector st (InvTransform (cumulative_transformation_matrix,pt));
  bool temp=false;
  for (i=0;i<nummesh&&!temp;i++) {
    temp|=meshdata[i]->queryBoundingBox (st,err);
     
  }
  if (!temp)
    return false;
  BSPTree *const* tmpBsp = ShieldUp(st)?&bspShield:&bspTree;
  if (bspTree&&!ShieldBSP) {
    tmpBsp= &bspTree;
  }
  if (!(*tmpBsp)) {
    dist = (st - meshdata[i-1]->Position()).Magnitude()-err-meshdata[i-1]->rSize();
    return true;
  }
  if ((*tmpBsp)->intersects (st,err,norm,dist)) {
    norm = ToWorldCoordinates (norm);
    return true;
  }
  return false;
}

float Unit::queryBSP (const Vector &start, const Vector & end, Vector & norm, bool ShieldBSP) const{
  int i;
  float tmp;

  for (i=0;i<numsubunit;i++) {
    if ((tmp = subunits[i]->queryBSP(start,end,norm,ShieldBSP))!=0)
      return tmp;
  }
  Vector st (InvTransform (cumulative_transformation_matrix,start));
  BSPTree *const* tmpBsp = ShieldUp(st)?&bspShield:&bspTree;
  if (bspTree&&!ShieldBSP) {
    tmpBsp= &bspTree;
  }
  if (!(*tmpBsp)) {
    tmp = querySphere (start,end);
    norm = (tmp * (start-end));
    tmp = norm.Magnitude();
    norm +=start;
    norm.Normalize();//normal points out from center
    return tmp;
  }
  Vector ed (InvTransform (cumulative_transformation_matrix,end));
  bool temp=false;
  for (i=0;i<nummesh&&!temp;i++) {
    temp = (1==meshdata[i]->queryBoundingBox (st,ed,0));
  }
  if (!temp)
    return false;
  if ((tmp = (*tmpBsp)->intersects (st,ed,norm))!=0) {
    norm = ToWorldCoordinates (norm);
    return tmp;
  }
  return 0;
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
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->querySphere (pnt,err))
      return true;
  }
  return false;
}



float Unit::querySphere (const Vector &start, const Vector &end) const{
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
  for (i=0;i<numsubunit;i++) {
    if ((tmp = subunits[i]->querySphere (start,end))!=0) {
      return tmp;
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
  vector <LineCollide *> tmp = collidetable.GetHuge();
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


