#include "unit.h"
#include "unit_factory.h"
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
#include "cmd/ai/fire.h"
#include "gfx/cockpit.h"
#include "config_xml.h"
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
    if (mounts[i].type->type==weapon_info::BEAM) {
      if (mounts[i].type->EnergyRate*SIMULATION_ATOM>energy) {
	mounts[i].UnFire();
	continue;
      }
    }else{ 
      if (mounts[i].type->EnergyRate>energy) 
	continue;
    }
    
    if (mounts[i].Fire(owner==NULL?this:owner,Missile)) {
      energy -=apply_float_to_short( mounts[i].type->type==weapon_info::BEAM?mounts[i].type->EnergyRate*SIMULATION_ATOM:mounts[i].type->EnergyRate);
    }
  }
}


void Unit::Mount::Activate (bool Missile) {
  if ((type->type==weapon_info::PROJECTILE)==Missile) {
    if (status==INACTIVE)
      status = ACTIVE;
  }
}
///Sets this gun to inactive, unless unchosen or destroyed
void Unit::Mount::DeActive (bool Missile) {
  if ((type->type==weapon_info::PROJECTILE)==Missile) {
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
  sz = mounts[0].type->size;
  for (int i=0;i<nummounts;i++) {
    if ((mounts[i].type->type==weapon_info::PROJECTILE)==Missile&&!Missile&&mounts[i].status<Mount::DESTROYED) {
      totalcount++;
      lasttotal=false;
      if (mounts[i].status==Mount::ACTIVE) {
	activecount++;
	lasttotal=true;
	mounts[i].DeActive (Missile);
	if (i==nummounts-1) {
	  sz=mounts[0].type->size;
	}else {
	  sz =mounts[i+1].type->size;
	}
      }
    }
    if ((mounts[i].type->type==weapon_info::PROJECTILE)==Missile&&Missile&&mounts[i].status<Mount::DESTROYED) {
      if (mounts[i].status==Mount::ACTIVE) {
	activecount++;//totalcount=0;
	mounts[i].DeActive (Missile);
	if (lasttotal) {
	  totalcount=(i+1)%nummounts;
	  if (i==nummounts-1) {
	    sz = mounts[0].type->size;
	  }else {
	    sz =mounts[i+1].type->size;
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
	if (mounts[i].type->size==sz) {
	  if ((mounts[i].type->type==weapon_info::PROJECTILE)) {
	    mounts[i].Activate(true);
	    return;
	  }else {
	    sz = mounts[(i+1)%nummounts].type->size;
	  }
	}
      }
      i=0;
    }
  }
  if (totalcount==activecount) {
    ActivateGuns (mounts[0].type->size,Missile);
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
      if (mounts[i].type->size==sz) {
	if (mounts[i].status<Mount::DESTROYED&&(mounts[i].type->type==weapon_info::PROJECTILE)==ms) {
	  mounts[i].Activate(ms);
	}else {
	  sz = mounts[(i+1)%nummounts].type->size;
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
  if (status!=ACTIVE||ref.gun==NULL||type->type!=weapon_info::BEAM)
    return ;
  //  AUDStopPlaying (sound);
  ref.gun->Destabilize();
}
static void AdjustMatrix (Matrix mat, Unit * target, float speed, bool lead, float cone) {
  if (target) {
    QVector pos (mat.p);
    Vector R (mat.getR());
    QVector targpos (lead?target->PositionITTS (pos,speed):target->Position());

    Vector dir =( targpos-pos).Cast();
    dir.Normalize();
    if (dir.Dot (R)>=cone) {
      Vector Q(mat.getQ());
      Vector P;
      ScaledCrossProduct (Q,dir,P);
      ScaledCrossProduct (dir,P,Q);
      VectorAndPositionToMatrix (mat,P,Q,dir,pos);
    }
  }
}
bool Unit::Mount::PhysicsAlignedFire(const Transformation &Cumulative, const Matrix & m, const Vector & velocity, Unit * owner, Unit *target, signed char autotrack, float trackingcone) {
  if (time_to_lock>0) {
    target=NULL;
  }
  time_to_lock = type->LockTime;
  if (processed==FIRED) {
    processed = PROCESSED;
    Unit * temp;
    Transformation tmp = LocalPosition;
    tmp.Compose (Cumulative,m);
    Matrix mat;
    tmp.to_matrix (mat);
    if (autotrack&&NULL!=target) {
      AdjustMatrix (mat,target,type->Speed,autotrack>=2,trackingcone);
    }
    switch (type->type) {
    case weapon_info::BEAM:
      break;
    case weapon_info::BOLT:
      new Bolt (*type, mat, velocity, owner);//FIXME turrets! Velocity      
      break;
    case weapon_info::BALL:
      new Bolt (*type,mat, velocity,  owner);//FIXME:turrets won't work      
      break;
    case weapon_info::PROJECTILE:
      temp = UnitFactory::createMissile (type->file.c_str(),owner->faction,"",type->Damage,type->PhaseDamage,type->Range/type->Speed,type->Radius,type->RadialSpeed,type->PulseSpeed/*detonation_radius*/);
      if (target&&target!=owner) {
	temp->Target (target);
	temp->EnqueueAI (new AIScript ((type->file+".xai").c_str()));
	temp->EnqueueAI (new Orders::FireAt (.2,1));
      } else {
	temp->EnqueueAI (new Orders::MatchLinearVelocity(Vector (0,0,100000),true,false));
	temp->EnqueueAI (new Orders::FireAt(.2,1));
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
  if (processed==FIRED||status!=ACTIVE||(Missile!=(type->type==weapon_info::PROJECTILE))||ammo==0)
    return false;
  if (type->type==weapon_info::BEAM) {
    if (ref.gun==NULL) {
      if (ammo>0)
	ammo--;
      processed=FIRED;
      ref.gun = new Beam (LocalPosition,*type,owner,sound);
      return true;
    } else {
      if (ref.gun->Ready()) {
	if (ammo>0)
	  ammo--;
	processed=FIRED;
	ref.gun->Init (LocalPosition,*type,owner);
	return true;
      } else 
	return true;//can't fire an active beam
    }
  }else { 
    if (ref.refire>type->Refire) {
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
Unit::Mount::Mount (){static weapon_info wi(weapon_info::BEAM); type=&wi; size=weapon_info::NOWEAP; ammo=-1;status= UNCHOSEN; ref.gun=NULL; sound=-1;}
Unit::Mount::Mount(const string& filename, short ammo,short volume): size(weapon_info::NOWEAP),ammo(ammo),sound(-1){
  static weapon_info wi(weapon_info::BEAM);
  type = &wi;
  this->volume=volume;
  ref.gun = NULL;
  status=(UNCHOSEN);
  weapon_info * temp = getTemplate (filename);  
  if (temp==NULL) {
    status=UNCHOSEN;
    time_to_lock=0;
  }else {
    type = temp;
    status=ACTIVE;
    time_to_lock = temp->LockTime;
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
  if (targ==this) {
    return;
  }
  if (!(activeStarSystem==NULL||activeStarSystem==_Universe->activeStarSystem())) {
    computer.target.SetUnit(NULL);
    return;
    fprintf (stderr,"bad target system");
    const int BADTARGETSYSTEM=0;
    assert (BADTARGETSYSTEM);
  }
  if (targ) {
    if (targ->activeStarSystem==_Universe->activeStarSystem()||targ->activeStarSystem==NULL) {
      computer.target.SetUnit(targ);
    }else {
      if (jump.drive!=-1) {
	un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
	Unit * u;
	for (;(u=*i)!=NULL;i++) {
	  if (!u->GetDestinations().empty()) {
	    if (std::find (u->GetDestinations().begin(),u->GetDestinations().end(),targ->activeStarSystem->getFileName())!=u->GetDestinations().end()) {
	      Target (u);
	      ActivateJumpDrive(0);
	    }
	  }
	}
      }else {
	computer.target.SetUnit(NULL);
      }
    }
  }else {
    computer.target.SetUnit(NULL);
  }
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
