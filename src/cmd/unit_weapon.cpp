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

extern unsigned short apply_float_to_short (float tmp);

void GameUnit::GameMount::PhysicsAlignedUnfire() {
  //Stop Playing SOund?? No, that's done in the beam, must not be aligned
  if (processed==UNFIRED) {
  if (AUDIsPlaying (sound))
    AUDStopPlaying (sound);
    processed=PROCESSED;
  }
}

void GameUnit::GameMount::UnFire () {
  processed = UNFIRED;
  if (status!=ACTIVE||ref.gun==NULL||type->type!=weapon_info::BEAM)
    return ;
  //  AUDStopPlaying (sound);
  ref.gun->Destabilize();
}

static void AdjustMatrix (Matrix &mat, Unit * target, float speed, bool lead, float cone) {
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
void AdjustMatrixToTrackTarget (Matrix &mat,Unit * target, float speed, bool lead, float cone) {
  AdjustMatrix (mat,target,speed,lead,cone);
}
bool GameUnit::GameMount::PhysicsAlignedFire(const Transformation &Cumulative, const Matrix & m, const Vector & velocity, Unit * owner, Unit *target, signed char autotrack, float trackingcone) {
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

void GameUnit::Fire (bool Missile) {
  if (cloaking>=0)
    return;
  for (int i=0;i<GetNumMounts();i++) {
    if (mounts[i]->type->type==weapon_info::BEAM) {
      if (mounts[i]->type->EnergyRate*SIMULATION_ATOM>energy) {
	mounts[i]->UnFire();
	continue;
      }
    }else{ 
      if (mounts[i]->type->EnergyRate>energy) 
	continue;
    }
    
    if (mounts[i]->Fire(owner==NULL?this:owner,Missile)) {
      energy -=apply_float_to_short( mounts[i]->type->type==weapon_info::BEAM?mounts[i]->type->EnergyRate*SIMULATION_ATOM:mounts[i]->type->EnergyRate);
    }
  }
}

bool GameUnit::GameMount::Fire (Unit * owner, bool Missile) {
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
GameUnit::GameMount::GameMount(const string& filename, short am,short vol){
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

void GameUnit::TargetTurret (Unit * targ) {
	if (!SubUnits.empty()) {
		un_iter iter = getSubUnits();
		Unit * su;
		bool inrange = (targ!=NULL)?InRange(targ):true;
                if (inrange) {
                while ((su=iter.current())) {
			su->Target (targ);
			su->TargetTurret(targ);
			iter.advance();
		}
                }
                

	}

}
void GameUnit::Target (Unit *targ) {
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
		if (targ!=Unit::Target()) {
        for (int i=0;i<GetNumMounts();i++){ 
  	  mounts[i]->time_to_lock = mounts[i]->type->LockTime;
        }
        computer.target.SetUnit(targ);
	LockTarget(false);
      }
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
