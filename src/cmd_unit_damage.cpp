#include "cmd_unit.h"
#include "gfx_animation.h"
#include "gfx_mesh.h"
#include "gfx_halo.h"
static list<Unit*> Unitdeletequeue;
void Unit::UnRef() {
  ucref--;
  if (killed&&ucref==0) {
    Unitdeletequeue.push_back(this);//delete
  }
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
  if (explosion==NULL&&timeexplode==0&&nummesh) {	//no explosion in unit data file && explosions haven't started yet
    explosion = new Animation * [nummesh];
    timeexplode=0;
    for (i=0;i<nummesh;i++){
      explosion[i]= new Animation ("explosion_orange.ani",false,.1,BILINEAR,false);
      explosion[i]->SetDimensions(4*meshdata[i]->rSize(),4*meshdata[i]->rSize());
    }    
  }
  float tmp[16];
  
  float tmp2[16];
  bool alldone =false;
  if (explosion) {
    for (i=0;i<nummesh;i++) {
      if (!explosion[i])
	continue;
      timeexplode+=GetElapsedTime();
      Translate (tmp,meshdata[i]->Position());
      MultMatrix (tmp2,cumulative_transformation_matrix,tmp);
      explosion[i]->SetPosition(tmp2[12],tmp2[13],tmp2[14]);
      if (timeexplode>i*.5){
	if (drawit) explosion[i]->Draw();
      }
      if (explosion[i]->Done()) {
	delete explosion[i];	
	explosion[i]=NULL;
      }else {
	alldone=true;
      }
    }
    if (!alldone){
      delete [] explosion;
      explosion = NULL;
    }
  }
  for (i=0;i<numsubunit;i++) {
    alldone |=subunits[i]->Explode();
  }
  return alldone;
}
