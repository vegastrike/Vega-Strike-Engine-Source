#include "unit.h"
#include "ai/order.h"
#include "gfx/animation.h"
#include "gfx/mesh.h"
#include "gfx/halo.h"
#include "gfx/bsp.h"
#include "vegastrike.h"
#include "unit_collide.h"
#include <float.h>
#include "audiolib.h"
#include "images.h"
#include "beam.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "savegame.h"
//#define DESTRUCTDEBUG
static list<Unit*> Unitdeletequeue;
void Unit::UnRef() {
  ucref--;
  if (killed&&ucref==0) {
    Unitdeletequeue.push_back(this);//delete
#ifdef DESTRUCTDEBUG
    fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
  }
}

void Unit::Upgrade (Unit * up) {
  if (up->nummounts>nummounts) {
    Mount * mnt = mounts;
    mounts = up->mounts;
    mounts = mnt;
    if (nummounts) {
      for (int i=nummounts;i<up->nummounts;i++) {
	Transformation t = (mnt[i%nummounts].GetMountLocation());
	Transformation q = (mnt[(i+1)%nummounts].GetMountLocation());
	t.position=(t.position+q.position)*.5;
	mounts[i%up->nummounts].SetMountPosition(t);
      }
      int tmp = nummounts;
      nummounts = up->nummounts;
      up->nummounts=tmp;
    }
  }else for (int i=0;i<up->nummounts&&i<nummounts;i++) {
    Mount * mnt = (Mount *) malloc (sizeof (Mount));
    memcpy (mnt,&mounts[i],sizeof (Mount));
    memcpy (&mounts[i],&up->mounts[i],sizeof (Mount));
    memcpy (&up->mounts[i],mnt,sizeof (Mount));
    free (mnt);
  }
  int i;
  for (i=0;i<nummounts&&i<up->nummounts;i++) {
    Transformation t =up->mounts[i].GetMountLocation();
    mounts[i].SetMountPosition(up->mounts[i].GetMountLocation());
    up->mounts[i].SetMountPosition (t);
  }
  armor.front+=up->armor.front;
  armor.back+=up->armor.back;
  armor.right+=up->armor.right;
  armor.left+=up->armor.left;
  shield.recharge+=up->shield.recharge;
  if (shield.leak>up->shield.leak) {
    shield.leak-=up->shield.leak;
  }else {
    shield.leak=0;
  }
  hull+=up->hull;
  recharge +=up->recharge;
  if (maxenergy+up->maxenergy>0)
    maxenergy+=up->maxenergy;
  if (energy+up->energy>0)
    energy+=up->energy;
  mass +=up->mass;
  fuel+=up->fuel;
  if (jump.drive==-2)
    jump.drive = up->jump.drive;
  if (fabs(jump.energy)>fabs (up->jump.energy))
    jump.energy = up->jump.energy;
  MomentOfInertia += up->MomentOfInertia;
  limits.yaw+=up->limits.yaw;
  limits.pitch+=up->limits.pitch;
  limits.roll+=up->limits.roll;
  limits.lateral+=up->limits.lateral;
  limits.vertical+=up->limits.vertical;
  limits.forward+=up->limits.forward;
  limits.retro+=up->limits.retro;
  limits.afterburn+=up->limits.afterburn;
  computer.radar.maxrange+=up->computer.radar.maxrange;
  if (computer.radar.maxcone>up->computer.radar.maxcone) 
    computer.radar.maxcone=up->computer.radar.maxcone;
  computer.max_speed+=up->computer.max_speed;
  computer.max_ab_speed+=up->computer.max_ab_speed;
  computer.max_yaw+=up->computer.max_yaw;
  computer.max_pitch+=up->computer.max_pitch;
  computer.max_roll+=up->computer.max_roll;
  computer.itts|=up->computer.itts;
  if (cloaking==-1) {
    cloaking = up->cloaking;
  }
  if (cloakmin>up->cloakmin)
    cloakmin=up->cloakmin;
  
}


void Unit::Split (int level) {
  int i;
  int nm = nummesh;
  Vector PlaneNorm;
  Mesh ** old = meshdata;
  if (halos) {//this looks ridiculous
    for (int hc=0;hc<numhalos;hc++) {
      delete halos[hc];
    }
    delete [] halos;
    halos=NULL;
    numhalos=0;
  }

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
  for (i=0;i<nm;i++) {
    Unit * splitsub;
    SubUnits.prepend(splitsub = new Unit (old+i,1,true,faction));
    splitsub->mass = mass/level;
    splitsub->image->timeexplode=.1;
    if (splitsub->meshdata[0]) {
      Vector loc = splitsub->meshdata[0]->Position();
      splitsub->ApplyForce(splitsub->meshdata[0]->rSize()*10*mass*loc/loc.Magnitude());
      loc.Set (rand(),rand(),rand());
      loc.Normalize();
      splitsub->ApplyLocalTorque(loc*mass*rSize()*(1+rand()%(int)(1+rSize())));
    }
  }
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

void Unit::Kill(bool erasefromsave) {
  if (erasefromsave)
    RemoveUnitFromSave((int)this);
  
  if (halos&&numhalos) {
    for (int hc=0;hc<numhalos;hc++) {
      delete halos[hc];
    }
    delete [] halos;
    halos=NULL;
    numhalos=0;
  }
  for (int beamcount=0;beamcount<nummounts;beamcount++) {
    AUDDeleteSound(mounts[beamcount].sound);
    if (mounts[beamcount].ref.gun&&mounts[beamcount].type.type==weapon_info::BEAM)
      delete mounts[beamcount].ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }
  if (mounts) {
    delete []mounts;
    nummounts=0;
    mounts = NULL;
  }
  //eraticate everything. naturally (see previous line) we won't erraticate beams erraticated above
  if (!SubUnit) 
    RemoveFromSystem();
  killed = true;
  computer.target.SetUnit (NULL);

  //God I can't believe this next line cost me 1 GIG of memory until I added it
  computer.threat.SetUnit (NULL);
  if(aistate)
    delete aistate;
  aistate=NULL;
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit * un;
  while ((un=iter.current())) {
    un->Kill();
    iter.advance();
  }
  if (ucref==0) {
    Unitdeletequeue.push_back(this);

#ifdef DESTRUCTDEBUG
    fprintf (stderr,"%s 0x%x - %d\n",name.c_str(),this,Unitdeletequeue.size());
#endif
  }
}
void Unit::ProcessDeleteQueue() {
  while (!Unitdeletequeue.empty()) {
#ifdef DESTRUCTDEBUG
    fprintf (stderr,"Eliminatin' 0x%x - %d",Unitdeletequeue.back(),Unitdeletequeue.size());
    fflush (stderr);
    fprintf (stderr,"Eliminatin' %s\n",Unitdeletequeue.back()->name.c_str());
#endif
#ifdef DESTRUCTDEBUG
    if (Unitdeletequeue.back()->SubUnit) {

      fprintf (stderr,"Subunit Deleting (related to double dipping)");

    }
#endif
    Unit * mydeleter = Unitdeletequeue.back();
    Unitdeletequeue.pop_back();
    delete mydeleter;///might modify unitdeletequeue
    
#ifdef DESTRUCTDEBUG
    fprintf (stderr,"Completed %d\n",Unitdeletequeue.size());
    fflush (stderr);
#endif

  }
}


unsigned short apply_float_to_short (float tmp) {
  unsigned  short ans = (unsigned short) tmp;
  tmp -=ans;//now we have decimal;
  if (((float)rand())/((float)RAND_MAX)<tmp)
    ans +=1;
  return ans;
}

static int applyto (unsigned short &shield, const unsigned short max, const float amt) {
  shield+=apply_float_to_short(amt);
  if (shield>max)
    shield=max;
  return (shield>=max)?1:0;
}

float Unit::FShieldData() const{
  switch (shield.number) {
  case 2: return shield.fb[0]/shield.fb[2];
  case 4: return ((float)shield.fbrl.front)/shield.fbrl.frontmax;
  case 6: return ((float)shield.fbrltb.v[0])/shield.fbrltb.fbmax;
  }
  return 0;
}
void Unit::ArmorData (unsigned short armor[4]) const{
  //  memcpy (&armor[0],&this->armor.front,sizeof (unsigned short)*4);
  armor[0]=this->armor.front;
  armor[1]=this->armor.back;
  armor[2]=this->armor.right;
  armor[3]=this->armor.left;
}

float Unit::FuelData () const{
  return fuel;
}
float Unit::EnergyData() const{
  return ((float)energy)/maxenergy;
}

float Unit::BShieldData() const{
  switch (shield.number) {
  case 2: return shield.fb[1]/shield.fb[3];
  case 4: return ((float)shield.fbrl.back)/shield.fbrl.backmax;
  case 6: return ((float)shield.fbrltb.v[1])/shield.fbrltb.fbmax;
  }
  return 0;
}
float Unit::LShieldData() const{
  switch (shield.number) {
  case 2: return 0;//no data, captain
  case 4: return ((float)shield.fbrl.left)/shield.fbrl.leftmax;
  case 6: return ((float)shield.fbrltb.v[3])/shield.fbrltb.rltbmax;
  }
  return 0;
}
float Unit::RShieldData() const{
  switch (shield.number) {
  case 2: return 0;//don't react to stuff we have no data on
  case 4: return ((float)shield.fbrl.right)/shield.fbrl.rightmax;
  case 6: return ((float)shield.fbrltb.v[2])/shield.fbrltb.rltbmax;
  }
  return 0;
}

void Unit::RegenShields () {
  int rechargesh=1;
  energy +=apply_float_to_short (recharge*SIMULATION_ATOM);
  float rec = shield.recharge*SIMULATION_ATOM>energy?energy:shield.recharge*SIMULATION_ATOM;
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  if (GetNebula()!=NULL)
    rec *=nebshields;
  switch (shield.number) {
  case 2:
    shield.fb[0]+=rec;
    shield.fb[1]+=rec;
    if (shield.fb[0]>shield.fb[2]) {
      shield.fb[0]=shield.fb[2];
    } else {
      rechargesh=0;
    }
    if (shield.fb[1]>shield.fb[3]) {
      shield.fb[1]=shield.fb[3];

    } else {
      rechargesh=0;
    }
    break;
  case 4:
    rechargesh = applyto (shield.fbrl.front,shield.fbrl.frontmax,rec)*(applyto (shield.fbrl.back,shield.fbrl.backmax,rec))*applyto (shield.fbrl.right,shield.fbrl.rightmax,rec)*applyto (shield.fbrl.left,shield.fbrl.leftmax,rec);
    break;
  case 6:
    rechargesh = (applyto(shield.fbrltb.v[0],shield.fbrltb.fbmax,rec))*applyto(shield.fbrltb.v[1],shield.fbrltb.fbmax,rec)*applyto(shield.fbrltb.v[2],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[3],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[4],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[5],shield.fbrltb.rltbmax,rec);
    break;
  }
  if (rechargesh==0)
    energy-=rec;
  if (energy>maxenergy)//allow shields to absorb xtra power
    energy=maxenergy;  

}

float Unit::DealDamageToHull (const Vector & pnt, float damage ) {
  float percent;
  unsigned short * targ;
  if (fabs  (pnt.k)>fabs(pnt.i)) {
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
    if (!AUDIsPlaying (sound->armor))
      AUDPlay (sound->armor,ToWorldCoordinates(pnt)+cumulative_transformation.position,Velocity,1);
    else
      AUDAdjustSound (sound->armor,ToWorldCoordinates(pnt)+cumulative_transformation.position,Velocity);
    *targ -= apply_float_to_short (damage);
  }else {
    if (!AUDIsPlaying (sound->hull))
      AUDPlay (sound->hull,ToWorldCoordinates(pnt)+cumulative_transformation.position,Velocity,1);
    else
      AUDAdjustSound (sound->hull,ToWorldCoordinates(pnt)+cumulative_transformation.position,Velocity);
    damage -= ((float)*targ);
    *targ= 0;
    hull -=damage;
  }
  if (hull <0) {
    Destroy();
    SetAI (new Order());
    Split (rand()%3+1);
  }
  if (!FINITE (percent))
    percent = 0;
  return percent;
}
bool Unit::ShieldUp (const Vector &pnt) const{
  const int shieldmin=5;
  int index;
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  if (nebula!=NULL||nebshields>0)
    return false;
  switch (shield.number){
  case 2:
    index = (pnt.k>0)?0:1;
    return shield.fb[index]>shieldmin;
    break;
  case 6:
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
      if (pnt.k>0) {
	index = 0;
      } else {
	index = 1;
      }
    }
    return shield.fbrltb.v[index]>shieldmin;
    break;
  case 4:
  default:
    if (fabs(pnt.k)>fabs (pnt.i)) {
      if (pnt.k>0) {
	return shield.fbrl.front>shieldmin;
      } else {
	return shield.fbrl.back>shieldmin;
      }
    } else {
      if (pnt.i>0) {
	return shield.fbrl.left>shieldmin;
      } else {
	return shield.fbrl.right>shieldmin;
      }
    }
    return false;
  }
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
  if (percent&&!AUDIsPlaying (sound->shield))
	AUDPlay (sound->shield,ToWorldCoordinates(pnt)+cumulative_transformation.position,Velocity,1);
  else
	AUDAdjustSound (sound->shield,ToWorldCoordinates(pnt)+cumulative_transformation.position,Velocity);

  return percent;
}
void Unit::ApplyLocalDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit,const GFXColor &color, float phasedamage) {
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  if (affectedUnit!=this) {
    affectedUnit->ApplyLocalDamage (pnt,normal,amt,affectedUnit,color,phasedamage);
    return;
  }
  float leakamt = phasedamage+amt*.01*shield.leak;
  amt *= 1-.01*shield.leak;
  float percentage=0;
  if (GetNebula()==NULL||(nebshields>0)) {
    percentage = DealDamageToShield (pnt,amt);
    if (meshdata[nummesh]&&percentage>0&&amt==0) {//shields are up
      /*      meshdata[nummesh]->LocalFX.push_back (GFXLight (true,
	      GFXColor(pnt.i+normal.i,pnt.j+normal.j,pnt.k+normal.k),
	      GFXColor (.3,.3,.3), GFXColor (0,0,0,1), 
	      GFXColor (.5,.5,.5),GFXColor (1,0,.01)));*/
      //calculate percentage
      if (GetNebula()==NULL) 
	meshdata[nummesh]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),percentage,color);
    }
  }
  if (shield.leak>0||!meshdata[nummesh]||percentage==0||amt>0||phasedamage) {
    percentage = DealDamageToHull (pnt, leakamt+amt);
    for (int i=0;i<nummesh;i++) {
      if (percentage)
	meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),percentage,color);
    }
  }
}


void Unit::ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit, const GFXColor & color, float phasedamage) {
  Vector localpnt (InvTransform(cumulative_transformation_matrix,pnt));
  Vector localnorm (ToLocalCoordinates (normal));
  ApplyLocalDamage(localpnt, localnorm, amt,affectedUnit,color,phasedamage);
}



bool Unit::Explode (bool drawit, float timeit) {
  if (image->explosion==NULL&&image->timeexplode==0) {	//no explosion in unit data file && explosions haven't started yet
    image->timeexplode=0;
    image->explosion= new Animation ("explosion_orange.ani",false,.1,BILINEAR,false);
    image->explosion->SetDimensions(3*rSize(),3*rSize());
    AUDPlay (sound->explode,cumulative_transformation.position,Velocity,1);
  }
  if (image->explosion) {
      image->timeexplode+=timeit;
      //Translate (tmp,meshdata[i]->Position());
      //MultMatrix (tmp2,cumulative_transformation_matrix,tmp);
      image->explosion->SetPosition(cumulative_transformation_matrix[12],cumulative_transformation_matrix[13],cumulative_transformation_matrix[14]);
      Vector p,q,r;
      GetOrientation (p,q,r);
      image->explosion->SetOrientation(p,q,r);
      if (image->explosion->Done()) {
	delete image->explosion;	
	image->explosion=NULL;
      }
      if (drawit&&image->explosion) { 
	image->explosion->Draw();//puts on draw queue... please don't delete
      }
      
  }
  bool alldone = image->explosion?!image->explosion->Done():false;
  if (!SubUnits.empty()) {
    UnitCollection::UnitIterator ui = getSubUnits();
    Unit * su;
    while ((su=ui.current())) {
      alldone |=su->Explode(drawit,timeit);
      ui.advance();
    }
  }
  return alldone;
}
