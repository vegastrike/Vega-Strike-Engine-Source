#include "cmd_unit.h"
#include "gfx_animation.h"
#include "gfx_mesh.h"
#include "gfx_halo.h"
#include "gfx_bsp.h"
static list<Unit*> Unitdeletequeue;
void Unit::UnRef() {
  ucref--;
  if (killed&&ucref==0) {
    Unitdeletequeue.push_back(this);//delete
  }
}

void Unit::Split (int level) {
  int i;
  int nm = nummesh;
  Vector PlaneNorm;
  Mesh ** old = meshdata;
  for (int split=0;split<level;split++) {
    Mesh ** nw= new Mesh *[nm*2+1];
    nw[nm*2]=old[nm];//copy shield
    for (i=0;i<nm;i++) {
      PlaneNorm.Set (rand()-RAND_MAX/2,rand()-RAND_MAX/2,rand()-RAND_MAX/2+.5);
      PlaneNorm.Normalize();  
      old[i]->Fork (nw[i*2], nw[i*2+1],PlaneNorm.i,PlaneNorm.j,PlaneNorm.k,-PlaneNorm.Dot(old[i]->Position()));//splits somehow right down the middle.
      if (nw[i*2]&&nw[i*2+1]) {
	delete old[i];
      }else {
	nw[i*2+1]= NULL;
	nw[i*2]=old[i];
      }
    }
    nm*=2;
    for (i=0;i<nm;i++) {
      if (nw[i]==NULL) {
	for (int j=i+1;j<nm;j++) {
	  nw[j-1]=nw[j];
	}
	nm--;
	nw[nm]=NULL;
      }
    }
    delete [] old;
    old = nw;
  }
  if (old[nm])
    delete old[nm];
  old[nm]=NULL;
  if (subunits&&numsubunit) {
    subunits = (Unit **)realloc (subunits, (numsubunit+nm)*sizeof (Unit *));
  }else {
    subunits = (Unit **)malloc (nm*sizeof (Unit *));
  }
  for (int i=0;i<nm;i++) {
    subunits[i+numsubunit] = new Unit (old+i,1);
    subunits[i+numsubunit]->mass = mass/level;
    subunits[i+numsubunit]->timeexplode=.1;
    if (subunits[i+numsubunit]->meshdata[0]) {
      Vector loc = subunits[i+numsubunit]->meshdata[0]->Position();
      subunits[i+numsubunit]->ApplyForce(subunits[i+numsubunit]->meshdata[0]->rSize()*mass*loc/loc.Magnitude());
      loc.Set (rand(),rand(),rand());
      loc.Normalize();
      subunits[i+numsubunit]->ApplyLocalTorque(loc*mass*rSize()*(1+rand()%(int)rSize()));
    }
  }
  numsubunit = numsubunit+nm;
  if (bspTree) {
    delete bspTree;
    bspTree=NULL;
  }
  delete [] old;
  nummesh = 0;
  meshdata = new Mesh *[1];
  meshdata[0]=NULL;//the shield
  //FIXME...how the heck can they go spinning out of control!
}

void Unit::Kill() {
  if (halos&&numhalos) {
    for (int hc=0;hc<numhalos;hc++) {
      delete halos[hc];
    }
    delete [] halos;
    halos=NULL;
    numhalos=0;
  }
  if (mounts) {
    delete []mounts;
    nummounts=0;
    mounts = NULL;
  }
  killed = true;
  if (CollideInfo.object) {
    KillCollideTable (&CollideInfo);
    CollideInfo.object=NULL;
  }
  Target((Unit *)NULL);
  if (ucref==0)
    Unitdeletequeue.push_back(this);
}
void Unit::ProcessDeleteQueue() {
  while (Unitdeletequeue.size()) {
    delete Unitdeletequeue.back();
    Unitdeletequeue.pop_back();
  }
}


static unsigned short apply_float_to_short (float tmp) {
  unsigned  short ans = (unsigned short) tmp;
  tmp -=ans;//now we have decimal;
  if (((float)rand())/((float)RAND_MAX)<tmp)
    ans +=1;
  return ans;
}
float Unit::DealDamageToHull (const Vector & pnt, float damage ) {
  float percent;
  unsigned short * targ;
  if (fabs (pnt.k)>fabs(pnt.i)) {
    if (pnt.k>0) {
      targ = &armor.front;
    }else {
      targ = &armor.back;
    }
  }else {
    if (pnt.i>0) {
      targ = &armor.left;
    }else {
      targ = &armor.right;
    }
  }
  percent = damage/(*targ+hull);
  if (damage<*targ) {
    *targ -= apply_float_to_short (damage);
  }else {
    damage -= ((float)*targ);
    *targ= 0;
    hull -=damage;
  }
  if (hull <0) {
    Destroy();
    Split (rand()%3+1);
  }
  if (!FINITE (percent))
    percent = 0;
  return percent;
}
float Unit::DealDamageToShield (const Vector &pnt, float &damage) {
  int index;
  float percent=0;
  unsigned short * targ=NULL;
  switch (shield.number){
  case 2:
    index = (pnt.k>0)?0:1;
    percent = damage/shield.fb[index+2];//comparing with max
    shield.fb[index]-=damage;
    damage =0;
    if (shield.fb[index]<0) {
      damage = -shield.fb[index];
      shield.fb[index]=0;
    }
    break;
  case 6:
    percent = damage/shield.fbrltb.rltbmax;
    if (fabs(pnt.i)>fabs(pnt.j)&&fabs(pnt.i)>fabs(pnt.k)) {
      if (pnt.i>0) {
	index = 3;//left
      } else {
	index = 2;//right
      }
    }else if (fabs(pnt.j)>fabs (pnt.k)) {
      if (pnt.j>0) {
	index = 4;//top;
      } else {
	index = 5;//bot;
      }
    } else {
      percent = damage/shield.fbrltb.fbmax;
      if (pnt.k>0) {
	index = 0;
      } else {
	index = 1;
      }
    }
    if (damage>shield.fbrltb.v[index]) {
      damage -= shield.fbrltb.v[index];
      shield.fbrltb.v[index]=0;
    } else {
      shield.fbrltb.v[index]-=apply_float_to_short (damage);
      damage = 0;
    }
    break;
  case 4:
  default:
    if (fabs(pnt.k)>fabs (pnt.i)) {
      if (pnt.k>0) {
	targ = &shield.fbrl.front;
	percent = damage/shield.fbrl.frontmax;
      } else {
	targ = &shield.fbrl.back;
	percent = damage/shield.fbrl.backmax;
      }
    } else {
      if (pnt.i>0) {
	percent = damage/shield.fbrl.leftmax;
	targ = &shield.fbrl.left;
      } else {
	targ = &shield.fbrl.right;
	percent = damage/shield.fbrl.rightmax;
      }
    }
    if (damage>*targ) {
      damage-=*targ;
      *targ=0;
    } else {
      *targ -= apply_float_to_short (damage);	
      damage=0;
    }
    break;
  }
  if (!FINITE (percent))
    percent = 0;
  return percent;
}
void Unit::ApplyLocalDamage (const Vector & pnt, const Vector & normal, float amt, const GFXColor &color) {
  float leakamt = amt*.01*shield.leak;
  amt *= 1-.01*shield.leak;
  float percentage = DealDamageToShield (pnt,amt);
  if (meshdata[nummesh]&&percentage>0&&amt==0) {//shields are up
    /*      meshdata[nummesh]->LocalFX.push_back (GFXLight (true,
						      GFXColor(pnt.i+normal.i,pnt.j+normal.j,pnt.k+normal.k),
						      GFXColor (.3,.3,.3), GFXColor (0,0,0,1), 
						      GFXColor (.5,.5,.5),GFXColor (1,0,.01)));*/
    //calculate percentage
    meshdata[nummesh]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),percentage,color);
  }
  if (shield.leak>0||!meshdata[nummesh]||percentage==0||amt>0) {
    percentage = DealDamageToHull (pnt, leakamt+amt);
    for (int i=0;i<nummesh;i++) {
      if (percentage)
	meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),percentage,color);
    }
  }
}


void Unit::ApplyDamage (const Vector & pnt, const Vector & normal, float amt, const GFXColor & color) {
  Vector localpnt (InvTransform(cumulative_transformation_matrix,pnt));
  Vector localnorm (ToLocalCoordinates (normal));
  ApplyLocalDamage(localpnt, localnorm, amt,color);
}



bool Unit::Explode (bool drawit) {
  int i;
  if (explosion==NULL&&timeexplode==0) {	//no explosion in unit data file && explosions haven't started yet
    timeexplode=0;
    explosion= new Animation ("explosion_orange.ani",false,.1,BILINEAR,false);
    explosion->SetDimensions(3*rSize(),3*rSize());
        
  }
  float tmp[16];
  
  float tmp2[16];
  if (explosion) {
      timeexplode+=GetElapsedTime();
      //Translate (tmp,meshdata[i]->Position());
      //MultMatrix (tmp2,cumulative_transformation_matrix,tmp);
      explosion->SetPosition(cumulative_transformation_matrix[12],cumulative_transformation_matrix[13],cumulative_transformation_matrix[14]);
      if (explosion->Done()) {
	delete explosion;	
	explosion=NULL;
      }
      if (drawit&&explosion) { 
	explosion->Draw();//puts on draw queue... please don't delete
      }
      
  }
  bool alldone = explosion?!explosion->Done():false;
  for (i=0;i<numsubunit;i++) {
    alldone |=subunits[i]->Explode();
  }
  return alldone;
}
