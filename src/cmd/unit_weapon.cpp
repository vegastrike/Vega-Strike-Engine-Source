#include "unit.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/lerp.h"
#include "vs_globals.h"
#include "audiolib.h"
#include "vegastrike.h"
#include <string>
#include "ai/script.h"
#include "ai/navigation.h"
#include "ai/flybywire.h"
#include "images.h"
#include "missile.h"

#include "gfx/cockpit.h"

#include "force_feedback.h"

void Unit::UnFire () {
  for (int i=0;i<nummounts;i++) {
    mounts[i].UnFire();//turns off beams;
  }
}
extern unsigned short apply_float_to_short (float tmp);

void Unit::Fire (bool Missile) {

  if (cloaking>=0)
    return;
  for (int i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].type.EnergyRate*SIMULATION_ATOM>energy) {
	mounts[i].UnFire();
	continue;
      }
    }else{ 
      if (mounts[i].type.EnergyRate>energy) 
	continue;
    }
    
    if (mounts[i].Fire(owner==NULL?this:owner,Missile)) {
      energy -=apply_float_to_short( mounts[i].type.type==weapon_info::BEAM?mounts[i].type.EnergyRate*SIMULATION_ATOM:mounts[i].type.EnergyRate);
    }
  }
}


void Unit::Mount::Activate (bool Missile) {
  if ((type.type==weapon_info::PROJECTILE)==Missile) {
    if (status==INACTIVE)
      status = ACTIVE;
  }
}
///Sets this gun to inactive, unless unchosen or destroyed
void Unit::Mount::DeActive (bool Missile) {
  if ((type.type==weapon_info::PROJECTILE)==Missile) {
    if (status==ACTIVE)
      status = INACTIVE;
  }
}
void Unit::SelectAllWeapon (bool Missile) {
  for (int i=0;i<nummounts;i++) {
    mounts[i].Activate (Missile);
  }
}

///In short I have NO CLUE how this works! It just...grudgingly does
void Unit::ToggleWeapon (bool Missile) {
  int activecount=0;
  int totalcount=0;
  bool lasttotal=true;
  weapon_info::MOUNT_SIZE sz = weapon_info::NOWEAP;
  if (nummounts<1)
    return;
  sz = mounts[0].type.size;
  for (int i=0;i<nummounts;i++) {
    if ((mounts[i].type.type==weapon_info::PROJECTILE)==Missile&&!Missile&&mounts[i].status<Mount::DESTROYED) {
      totalcount++;
      lasttotal=false;
      if (mounts[i].status==Mount::ACTIVE) {
	activecount++;
	lasttotal=true;
	mounts[i].DeActive (Missile);
	if (i==nummounts-1) {
	  sz=mounts[0].type.size;
	}else {
	  sz =mounts[i+1].type.size;
	}
      }
    }
    if ((mounts[i].type.type==weapon_info::PROJECTILE)==Missile&&Missile&&mounts[i].status<Mount::DESTROYED) {
      if (mounts[i].status==Mount::ACTIVE) {
	activecount++;//totalcount=0;
	mounts[i].DeActive (Missile);
	if (lasttotal) {
	  totalcount=(i+1)%nummounts;
	  if (i==nummounts-1) {
	    sz = mounts[0].type.size;
	  }else {
	    sz =mounts[i+1].type.size;
	  }
	}
	lasttotal=false;
      } 
    }
  }
  if (Missile) {
    int i=totalcount;
    for (int j=0;j<2;j++) {
      for (;i<nummounts;i++) {
	if (mounts[i].type.size==sz) {
	  if ((mounts[i].type.type==weapon_info::PROJECTILE)) {
	    mounts[i].Activate(true);
	    return;
	  }else {
	    sz = mounts[(i+1)%nummounts].type.size;
	  }
	}
      }
      i=0;
    }
  }
  if (totalcount==activecount) {
    ActivateGuns (mounts[0].type.size,Missile);
  } else {
    if (lasttotal) {
      SelectAllWeapon(Missile);
    }else {
      ActivateGuns (sz,Missile);
    }
  }
}

///cycles through the loop twice turning on all matching to ms weapons of size or after size
void Unit::ActivateGuns (weapon_info::MOUNT_SIZE sz, bool ms) {
  for (int j=0;j<2;j++) {
    for (int i=0;i<nummounts;i++) {
      if (mounts[i].type.size==sz) {
	if (mounts[i].status<Mount::DESTROYED&&(mounts[i].type.type==weapon_info::PROJECTILE)==ms) {
	  mounts[i].Activate(ms);
	}else {
	  sz = mounts[(i+1)%nummounts].type.size;
	}
      }
    }
  }
}
void Unit::Mount::PhysicsAlignedUnfire() {
  //Stop Playing SOund?? No, that's done in the beam, must not be aligned
  if (processed==UNFIRED) {
  if (AUDIsPlaying (sound))
    AUDStopPlaying (sound);
    processed=PROCESSED;
  }
}
void Unit::Mount::UnFire () {
  processed = UNFIRED;
  if (status!=ACTIVE||ref.gun==NULL||type.type!=weapon_info::BEAM)
    return ;
  //  AUDStopPlaying (sound);
  ref.gun->Destabilize();
}
bool Unit::Mount::PhysicsAlignedFire(const Transformation &Cumulative, const float * m, const Vector & velocity, Unit * owner, Unit *target) {
  if (processed==FIRED) {
    processed = PROCESSED;
    Unit * temp;
    Transformation tmp = LocalPosition;
    tmp.Compose (Cumulative,m);
    Matrix mat;
    tmp.to_matrix (mat);
    switch (type.type) {
    case weapon_info::BEAM:
      break;
    case weapon_info::BOLT:
      new Bolt (type, mat, velocity, owner);//FIXME turrets! Velocity      
      break;
    case weapon_info::BALL:
      new Bolt (type,mat, velocity,  owner);//FIXME:turrets won't work      
      break;
    case weapon_info::PROJECTILE:
      temp = new Missile (type.file.c_str(),owner->faction,"",type.Damage,type.PhaseDamage,type.Range/type.Speed);
      if (target&&target!=owner) {
	temp->Target (target);
	temp->EnqueueAI (new AIScript ((type.file+".xai").c_str()));
      } else {
	temp->EnqueueAI (new Orders::MatchLinearVelocity(Vector (0,0,100000),true,false));
      }
      temp->SetOwner (owner);
      temp->Velocity = velocity;
      temp->curr_physical_state = temp->prev_physical_state= temp->cumulative_transformation = tmp;
      CopyMatrix (temp->cumulative_transformation_matrix,m);
      _Universe->activeStarSystem()->AddUnit(temp);
      break;
    }
    if (!AUDIsPlaying (sound)) {
      AUDPlay (sound,tmp.position,velocity,1);
    }else {
      AUDAdjustSound(sound,tmp.position,velocity);
    }
    return true;
  }
  return false;
}
bool Unit::Mount::Fire (Unit * owner, bool Missile) {
  if (processed==FIRED||status!=ACTIVE||(Missile!=(type.type==weapon_info::PROJECTILE))||ammo==0)
    return false;
  if (type.type==weapon_info::BEAM) {
    if (ref.gun==NULL) {
      if (ammo>0)
	ammo--;
      processed=FIRED;
      ref.gun = new Beam (LocalPosition,type,owner,sound);
      return true;
    } else {
      if (ref.gun->Ready()) {
	if (ammo>0)
	  ammo--;
	processed=FIRED;
	ref.gun->Init (LocalPosition,type,owner);
	return true;
      } else 
	return true;//can't fire an active beam
    }
  }else { 
    if (ref.refire>type.Refire) {
      ref.refire =0;
      if (ammo>0)
	ammo--;
      processed=FIRED;	

      if(owner==_Universe->AccessCockpit()->GetParent()){
	//printf("player has fired a bolt\n");
	forcefeedback->playLaser();
      };

      return true;
    }
  }
  return false;
}
Unit::Mount::Mount(const string& filename, short ammo): size(weapon_info::NOWEAP),ammo(ammo),type(weapon_info::BEAM),sound(-1){
  ref.gun = NULL;
  status=(UNCHOSEN);
  weapon_info * temp = getTemplate (filename);  
  if (temp==NULL) {
    status=UNCHOSEN;
  }else {
    type = *temp;
    status=ACTIVE;
  }
}
void Unit::TargetTurret (Unit * targ) {
  if (!SubUnits.empty()) {
    un_iter iter = getSubUnits();
    Unit * su;
    Vector localcoord;
    while ((su=iter.current())) {
      if (su->InRange (targ,localcoord)) {
	su->Target (targ);
	su->TargetTurret(targ);
      }
      iter.advance();
    }
  }
}
void Unit::Target (Unit *targ) {
  computer.target.SetUnit(targ);
}
void Unit::VelocityReference (Unit *targ) {
  computer.velocity_ref.SetUnit(targ);
}
void Unit::SetRecursiveOwner(Unit *target) {
  owner=target;
  if (!SubUnits.empty()) {
    UnitCollection::UnitIterator iter = getSubUnits();
    Unit * su;
    while ((su=iter.current())) {
      su->SetOwner (target);
      iter.advance();
    }
  }
}
void Unit::SetOwner(Unit *target) {
  owner=target;
}

void Unit::Cloak (bool loak) {
  if (loak) {
    if (image->cloakenergy<energy) {
      image->cloakrate =(image->cloakrate>=0)?image->cloakrate:-image->cloakrate; 
      if (cloaking==(short)32768) {
	cloaking=32767;
      } else {
       
      }
    }
  }else {
    image->cloakrate= (image->cloakrate>=0)?-image->cloakrate:image->cloakrate;
    if (cloaking==cloakmin)
      cloaking++;
  }
}
float Unit::CloakVisible() const {
  if (cloaking<0)
    return 1;
  return ((float)cloaking)/32767;
}
