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
#include "gfx/cockpit.h"
#include "cmd/script/mission.h"
#include "cmd/ai/communication.h"
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
  delete [] old;
  nummesh = 0;
  meshdata = new Mesh *[1];
  meshdata[0]=NULL;//the shield
  //FIXME...how the heck can they go spinning out of control!
}

void Unit::Kill(bool erasefromsave) {
  if (colTrees)
    colTrees->Dec();//might delete
  colTrees=NULL;
  if (erasefromsave)
    _Universe->AccessCockpit()->savegame->RemoveUnitFromSave((int)this);
  
  if (halos&&numhalos) {
    for (int hc=0;hc<numhalos;hc++) {
      delete halos[hc];
    }
    delete [] halos;
    halos=NULL;
    numhalos=0;
  }
  if (docked&(DOCKING_UNITS)) {
    vector <Unit *> dockedun;
    unsigned int i;
    for (i=0;i<image->dockedunits.size();i++) {
      Unit * un;
      if (NULL!=(un=image->dockedunits[i]->uc.GetUnit())) 
	dockedun.push_back (un);
    }
    while (!dockedun.empty()) {
      dockedun.back()->UnDock(this);
      dockedun.pop_back();
    }
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
  computer.velocity_ref.SetUnit(NULL);
  if(aistate) {
    aistate->ClearMessages();
    delete aistate;
  }
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


float rand01 () {
	return ((float)rand()/(float)RAND_MAX);
}
void Unit::leach (float damShield, float damShieldRecharge, float damEnRecharge) {
  recharge*=damEnRecharge;
  shield.recharge*=damShieldRecharge;
  switch (shield.number) {
  case 2:
    shield.fb[2]*=damShield;
    shield.fb[3]*=damShield;
    break;
  case 4:
    shield.fbrl.frontmax*=damShield;
    shield.fbrl.backmax*=damShield;
    shield.fbrl.leftmax*=damShield;
    shield.fbrl.rightmax*=damShield;
    break;
  case 6:
    shield.fbrltb.fbmax*=damShield;
    shield.fbrltb.rltbmax*=damShield;
    break;
  }
}
void Unit::DamageRandSys(float dam, const Vector &vec) {
	float deg = fabs(180*atan2 (vec.i,vec.k)/M_PI);
	float randnum=rand01();
	float degrees=deg;
	if (degrees>180) {
		degrees=360-degrees;
	}
	if (degrees>=0&&degrees<20) {
		//DAMAGE COCKPIT
		if (randnum>=.85) {
			computer.set_speed=(rand01()*computer.max_speed*(5/3))-(computer.max_speed*(2/3)); //Set the speed to a random speed
		} else if (randnum>=.775) {
			computer.itts=false; //Set the computer to not have an itts
		} else if (randnum>=.7) {
			computer.radar.color=false; //set the radar to not have color
		} else if (randnum>=.5) {
			computer.target=NULL; //set the target to NULL
		} else if (randnum>=.4) {
			limits.retro*=dam;
		} else if (randnum>=.325) {
			computer.radar.maxcone+=(1-dam);
			if (computer.radar.maxcone>.99)
				computer.radar.maxcone=.99;
		} else if (randnum>=.25) {
			computer.radar.mintargetsize+=(rSize()/2);
			if (computer.radar.mintargetsize>rSize())
				computer.radar.mintargetsize=rSize();
		} else if (randnum>=.175) {
			computer.radar.maxrange*=dam;
		} else {
		  int which= rand()%(1+Cockpit::NUMGAUGES+MAXVDUS);
		  image->cockpit_damage[which]*=dam;
		  if (image->cockpit_damage[which]<.1) {
		    image->cockpit_damage[which]=0;
		  }
		}
		return;
	}
	if (degrees>=20&&degrees<35) {
		//DAMAGE MOUNT
		if (nummounts) {
			unsigned int whichmount=rand()%nummounts;
			if (randnum>=.9) {
				mounts[whichmount].status=Unit::Mount::DESTROYED;
			} else if (randnum>=.6) {
			  image->ecm*=dam;
			}else if (mounts[whichmount].ammo>0) {
			  mounts[whichmount].ammo*=dam;
			}
		}
		return;
	}
	if (degrees>=35&&degrees<60) {
		//DAMAGE FUEL
		if (randnum>=.75) {
			fuel*=dam;
		} else if (randnum>=.5) {
			this->afterburnenergy+=((1-dam)*recharge);
		} else if (randnum>=.25) {
			image->cargo_volume*=dam;
		} else {  //Do something NASTY to the cargo
			if (image->cargo.size()>0) {
				int i=0;
				unsigned int cargorand;
				do {
					cargorand=rand()%image->cargo.size();
				} while (image->cargo[cargorand].quantity!=0&&++i<image->cargo.size());
				image->cargo[cargorand].quantity*=dam;
			}
		}
		return;
	}
	if (degrees>=60&&degrees<90) {
		//DAMAGE ROLL/YAW/PITCH/THRUST
		if (randnum>=.8) {
			computer.max_pitch*=dam;
		} else if (randnum>=.6) {
			computer.max_yaw*=dam;
		} else if (randnum>=.55) {
			computer.max_roll*=dam;
		} else if (randnum>=.5) {
			limits.roll*=dam;
		} else if (randnum>=.3) {
			limits.yaw*=dam;
		} else if (randnum>=.1) {
			limits.pitch*=dam;
		} else {
			limits.lateral*=dam;
		}
		return;
	}
	if (degrees>=90&&degrees<120) {
		//DAMAGE Shield
		//DAMAGE cloak
		if (randnum>=.95) {
			this->cloaking=-1;
		} else if (randnum>=.78) {
			image->cloakenergy+=((1-dam)*recharge);
		} else if (randnum>=.7) {
			cloakmin+=(rand()%(32000-cloakmin));
		}
		switch (shield.number) {
		case 2:
			if (randnum>=.35&&randnum<.7) {
				shield.fb[2]*=dam;
			} else {
				shield.fb[3]*=dam;
			}
			break;
		case 4:
			if (randnum>=.5&&randnum<.7) {
				shield.fbrl.frontmax*=dam;
			} else if (randnum>=.3) {
				shield.fbrl.backmax*=dam;
			} else if (deg>180) {
				shield.fbrl.leftmax*=dam;
			} else {
				shield.fbrl.rightmax*=dam;
			}
			break;
		case 6:
			if (randnum>=.4&&randnum<.7) {
				shield.fbrltb.fbmax*=dam;
			} else {
				shield.fbrltb.rltbmax*=dam;
			}
			break;
		}
		return;
	}
	if (degrees>=120&&degrees<150) {
		//DAMAGE Reactor
		//DAMAGE JUMP
		if (randnum>=.9) {
			shield.leak+=((1-dam)*100);
		} else if (randnum>=.7) {
			shield.recharge*=dam;
		} else if (randnum>=.5) {
			this->recharge*=dam;
		} else if (randnum>=.3) {
			this->maxenergy*=dam;
		} else if (randnum>=.2) {
			this->jump.energy*=(2-dam);
		} else if (randnum>=.03){
			this->jump.damage+=100*(1-dam);
		} else {
		  if (image->repair_droid>0) {
		    image->repair_droid--;
		  }
		}
		return;
	}
	if (degrees>=150&&degrees<=180) {
		//DAMAGE ENGINES
		if (randnum>=.8) {
			computer.max_ab_speed*=dam;
		} else if (randnum>=.6) {
			computer.max_speed*=dam;
		} else if (randnum>=.4) {
			limits.afterburn*=dam;
		} else if (randnum>=.2) {
			limits.vertical*=dam;
		} else {
			limits.forward*=dam;
		}
		return;
	}
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
    if (_Universe->AccessCockpit()->GetParent()!=this||_Universe->AccessCockpit()->godliness<=0||hull>damage) {
      static float system_failure=XMLSupport::parse_float(vs_config->getVariable ("physics","indiscriminate_system_destruction",".25"));
      DamageRandSys(system_failure*rand01()+(1-system_failure)*(1-(damage/hull)),pnt);
      hull -=damage;
    }else {
      _Universe->AccessCockpit()->godliness-=damage;
      DamageRandSys(rand01(),pnt);//get system damage...but live!
    }

  }
  if (hull <0) {
    static float autoejectpercent = XMLSupport::parse_float(vs_config->getVariable ("physics","autoeject_percent",".5"));
    if (!SubUnit&&rand()<(RAND_MAX*autoejectpercent)) {
      EjectCargo ((unsigned int)-1);
    }
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


void Unit::ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit, const GFXColor & color, Unit * ownerDoNotDereference, float phasedamage) {
  if (ownerDoNotDereference==_Universe->AccessCockpit()->GetParent()) {
    if (ownerDoNotDereference) {
      //now we can dereference it because we checked it against the parent
      CommunicationMessage c(ownerDoNotDereference,this,NULL,0);
      c.SetCurrentState(c.fsm->GetHitNode(),NULL,0);
      this->getAIState()->Communicate (c);      
    }
  }
  Vector localpnt (InvTransform(cumulative_transformation_matrix,pnt));
  Vector localnorm (ToLocalCoordinates (normal));
  ApplyLocalDamage(localpnt, localnorm, amt,affectedUnit,color,phasedamage);
}


float Unit::ExplosionRadius() {
  static float expsize=XMLSupport::parse_float(vs_config->getVariable ("graphics","explosion_size","3"));
  return expsize*rSize();
}
bool Unit::Explode (bool drawit, float timeit) {


  if (image->explosion==NULL&&image->timeexplode==0) {	//no explosion in unit data file && explosions haven't started yet

  // notify the director that a ship got destroyed
  mission->DirectorShipDestroyed(this);

    image->timeexplode=0;
	static std::string expani = vs_config->getVariable ("graphics","explosion_animation","explosion_orange.ani");
    image->explosion= new Animation (expani.c_str(),false,.1,BILINEAR,false);
    image->explosion->SetDimensions(ExplosionRadius(),ExplosionRadius());
	if (!SubUnit){
		Vector exploc = cumulative_transformation.position;
		Unit * un;
		if (NULL!=(un=_Universe->AccessCockpit(0)->GetParent())) {
			exploc = un->Position();						
		}
	    AUDPlay (sound->explode,exploc,Velocity,1);

	}
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
