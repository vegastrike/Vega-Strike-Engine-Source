#include "mount.h"
#include "beam.h"
#include "bolt.h"
#include "weapon_xml.h"
#include "audiolib.h"
#include "unit_factory.h"
#include "ai/order.h"
#include "ai/fire.h"
#include "ai/script.h"
#include "ai/navigation.h"
#include "ai/flybywire.h"
#include "configxml.h"
#include "gfx/cockpit_generic.h"
#include "force_feedback.h"

GameMount::GameMount (){static weapon_info wi(weapon_info::BEAM); type=&wi; size=weapon_info::NOWEAP; ammo=-1;status= UNCHOSEN; processed=GameMount::PROCESSED;ref.gun=NULL; sound=-1;}

void GameMount::ReplaceMounts (const Mount *other) {
  Mount::ReplaceMounts( other);
  sound = AUDCreateSound (sound,type->type!=weapon_info::PROJECTILE);//copy constructor basically
}

void GameMount::PhysicsAlignedUnfire() {
  //Stop Playing SOund?? No, that's done in the beam, must not be aligned
  if (processed==UNFIRED) {
  if (AUDIsPlaying (sound))
    AUDStopPlaying (sound);
    processed=PROCESSED;
  }
}

void GameMount::UnFire () {
  processed = UNFIRED;
  if (status!=ACTIVE||ref.gun==NULL||type->type!=weapon_info::BEAM)
    return ;
  //  AUDStopPlaying (sound);
  ref.gun->Destabilize();
}

extern void AdjustMatrix (Matrix &mat, Unit * target, float speed, bool lead, float cone);
void AdjustMatrixToTrackTarget (Matrix &mat,Unit * target, float speed, bool lead, float cone) {
  AdjustMatrix (mat,target,speed,lead,cone);
}
bool GameMount::PhysicsAlignedFire(const Transformation &Cumulative, const Matrix & m, const Vector & velocity, Unit * owner, Unit *target, signed char autotrack, float trackingcone) {
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
    static bool use_separate_sound=XMLSupport::parse_bool (vs_config->getVariable ("audio","high_quality_weapon","true"));
    if ((((!use_separate_sound)||type->type==weapon_info::BEAM)||(!_Universe->isPlayerStarship(owner)))&&(type->type!=weapon_info::PROJECTILE)) {
    if (!AUDIsPlaying (sound)) {
      AUDPlay (sound,tmp.position,velocity,1);
    }else {
      AUDAdjustSound(sound,tmp.position,velocity);
    }
    }else {
      int snd =AUDCreateSound(sound,false);
      AUDAdjustSound(snd,tmp.position,velocity);
      AUDStartPlaying (snd);
      AUDDeleteSound(snd);
    }
    return true;
  }
  return false;
}

bool GameMount::Fire (Unit * owner, bool Missile) {
  if (ammo==0) {
    processed=UNFIRED;
  }
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

GameMount::GameMount(const string& filename, short am,short vol){
  static weapon_info wi(weapon_info::BEAM);
  size = weapon_info::NOWEAP;
  ammo = am;
  sound = -1;
  type = &wi;
  this->volume=vol;
  ref.gun = NULL;
  status=(UNCHOSEN);
  processed=GameMount::PROCESSED;
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
