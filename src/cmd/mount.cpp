#include "unit_generic.h"
#include "beam.h"
#include "bolt.h"
#include "weapon_xml.h"
#include "audiolib.h"
#include "unit_factory.h"
#include "ai/order.h"
#include "ai/fireall.h"
#include "ai/script.h"
#include "ai/navigation.h"
#include "ai/flybywire.h"
#include "configxml.h"
#include "gfx/cockpit_generic.h"
#include "force_feedback.h"
#include "networking/netclient.h"
#include "ai/aggressive.h"
#include "lin_time.h"
extern char SERVER;
extern bool isMissile(const weapon_info *);
Mount::Mount() {
	static weapon_info wi(weapon_info::BEAM);
        functionality=1;
        maxfunctionality=1;
	type=&wi; size=weapon_info::NOWEAP;
        functionality=maxfunctionality=1.0f;
	ammo=-1;
	status= UNCHOSEN;
	processed=Mount::PROCESSED;
	sound=-1;
	static float xyscalestat=XMLSupport::parse_float (vs_config->getVariable ("graphics","weapon_xyscale","1"));

	static float zscalestat=XMLSupport::parse_float (vs_config->getVariable ("graphics","weapon_zscale","1"));
	xyscale=xyscalestat;	
	zscale=zscalestat;
	serial = 0;
}
extern double interpolation_blend_factor;
void DestroyMount (Mount * mount) {
  mount->UnFire();
  AUDStopPlaying(mount->sound); 
  mount->status=Mount::DESTROYED;

}
float Mount::ComputeAnimatedFrame(Mesh * gun) {
	if (type->type==weapon_info::BEAM) {
		if (ref.gun) {
			if (ref.gun->Ready() ) {
				return getNewTime()+type->Refire-ref.gun->refireTime()-interpolation_blend_factor*SIMULATION_ATOM;
			}else {
				return getNewTime()*gun->getFramesPerSecond();				
			}
		}else {
			return 0;
		}
	}else {
		if (ref.refire<type->Refire) {
			return getNewTime()*gun->getFramesPerSecond();
		}else {
			return getNewTime()+type->Refire-ref.refire-interpolation_blend_factor*SIMULATION_ATOM;
		}
	}
}
Mount::Mount(const string& filename, int am, int vol, float xyscale, float zscale, float func,float maxfunc, bool banked):bank(banked){ //short fix
  functionality=func;
  maxfunctionality=maxfunc;
  static weapon_info wi(weapon_info::BEAM);
  size = weapon_info::NOWEAP;
  static float xyscalestat=XMLSupport::parse_float (vs_config->getVariable ("graphics","weapon_xyscale","1"));
  
  static float zscalestat=XMLSupport::parse_float (vs_config->getVariable ("graphics","weapon_zscale","1"));  
  if (xyscale==-1)
	  xyscale=xyscalestat;
  if (zscale==-1)
	  zscale=zscalestat;
  this->zscale=zscale;
  this->xyscale=xyscale;
  serial = 0;
  ammo = am;
  sound = -1;
  type = &wi;
  this->volume=vol;
  ref.gun = NULL;
  status=(UNCHOSEN);
  processed=Mount::PROCESSED;
  weapon_info * temp = getTemplate (filename);  
  if (temp==NULL) {
    status=UNCHOSEN;
    time_to_lock=0;
  }else {
    type = temp;
    status=ACTIVE;
    time_to_lock = temp->LockTime;
	if (type->type!=weapon_info::BEAM)
		ref.refire=type->Refire;
  }
}

extern bool AdjustMatrix (Matrix &mat,const Vector &velocity, Unit * target, float speed, bool lead, float cone);
void AdjustMatrixToTrackTarget (Matrix &mat,const Vector & velocity, Unit * target, float speed, bool lead, float cone) {
  AdjustMatrix (mat,velocity, target,speed,lead,cone);
}

void Mount::UnFire () {
  processed = UNFIRED;
  if (status!=ACTIVE||ref.gun==NULL||type->type!=weapon_info::BEAM)
    return ;
  //  AUDStopPlaying (sound);
  ref.gun->Destabilize();
}

void Mount::SwapMounts(Mount * other) {
	  int thisvol = volume; //short fix
	  int othervol = other->volume; //short fix
	  //short othersize = other->size;
	  int thissize = size;
          bool thisbank=this->bank;
          bool otherbank=other->bank;
	  Mount mnt = *this;
	  this->size=thissize;
	  *this=*other;
	  *other=mnt;
	  volume=thisvol;

	  other->volume=othervol;//volumes stay the same even if you swap out
	  Vector v =this->GetMountLocation();
	  Quaternion q = this->GetMountOrientation();
	  this->SetMountPosition(other->GetMountLocation());
	  this->SetMountOrientation(other->GetMountOrientation());
	  other->SetMountPosition (v);
	  other->SetMountOrientation (q);  
          other->bank=otherbank;
          this->bank=thisbank;
}
void Mount::ReplaceMounts (const Mount * other) {
	int thisvol = volume; //short fix
	int thissize = size;  //short fix
        float xyscale=this->xyscale;
        float zscale=this->zscale;
        bool thisbank=this->bank;
	Quaternion q =this->GetMountOrientation();
	Vector v = this->GetMountLocation();
	*this=*other;
	this->size=thissize;
	volume=thisvol;
	this->SetMountPosition(v);
	this->SetMountOrientation(q);	
        this->xyscale=xyscale;
        this->zscale=zscale;
        this->bank=thisbank;
	ref.gun=NULL;
	if (type->type!=weapon_info::BEAM)
		ref.refire=type->Refire;
	this->ReplaceSound();
	if (other->ammo==-1)
		ammo=-1;
	else if (other->ammo!=-1 && ammo==-1) {
		ammo=0;//zero ammo if other was not zero earlier.
	}
}
double Mount::Percentage (const Mount *newammo) const{
	  float percentage=1./1024;
	  int thingstocompare=0;
	  if (status==UNCHOSEN||status==DESTROYED)
            return percentage;
	  if (newammo->ammo==-1) {
		if (ammo!=-1) {
		  thingstocompare++;
		}else {
                  if (newammo->type->Range==type->Range&&newammo->type->Damage==type->Damage&&newammo->type->PhaseDamage==type->PhaseDamage)
                    return 1;
                  if (newammo->type->weapon_name==type->weapon_name)
                    return 1;             
                }
	  } else {
		if (newammo->ammo>0) {
		  percentage+=.25;
		  thingstocompare++;
                  if (ammo>0) {
                    if (newammo->type->Range==type->Range&&newammo->type->Damage==type->Damage&&newammo->type->PhaseDamage==type->PhaseDamage)
                      return 1;
                    if (newammo->type->weapon_name==type->weapon_name)
                      return 1;                                 
                  }
		}
	  }
	  if (newammo->type->Range) {
            if (type->Range>newammo->type->Range)percentage+= .25;
		thingstocompare++;
	  }
	  if (newammo->type->Damage+100*newammo->type->PhaseDamage) {
                if (type->Damage+100*type->PhaseDamage>newammo->type->Damage+100*newammo->type->PhaseDamage) percentage+=.75;
		thingstocompare++;
	  }
	  if (thingstocompare) {
		return percentage/thingstocompare;
	  }else {
		return 1./1024;
	  }
}
extern void GetMadAt(Unit* un, Unit * parent, int numhits=0);

//bool returns whether to refund the cost of firing
bool Mount::PhysicsAlignedFire(Unit * caller, const Transformation &Cumulative, const Matrix & m, const Vector & velocity, void * owner, Unit *target, signed char autotrack, float trackingcone,CollideMap::iterator &hint) {
	using namespace VSFileSystem;
  if (time_to_lock>0) {
    target=NULL;
  }
  static bool lock_disrupted_by_false_fire=XMLSupport::parse_bool(vs_config->getVariable("physics","out_of_arc_fire_disrupts_lock","false"));
  if (lock_disrupted_by_false_fire)
    time_to_lock = type->LockTime;
  if (processed==FIRED) {
    if (type->type!=weapon_info::BOLT && type->type!=weapon_info::BALL) {
      // Missiles and beams set to processed.
      processed = PROCESSED;
    } else {
      if (ref.refire<type->Refire || type->EnergyRate>caller->energy) {
        // Wait until refire has expired and reactor has produced enough energy for the next bolt.
        return true; // Not ready to refire yet.  But don't stop firing.
      }
    }
    Unit * temp;
    Transformation tmp (orient,pos.Cast());
    tmp.Compose (Cumulative,m);
    Matrix mat;
    tmp.to_matrix (mat);
    mat.p = Transform(mat,(type->offset+Vector(0,0,zscale)).Cast());
    static bool firemissingautotrackers = XMLSupport::parse_bool (vs_config->getVariable("physics","fire_missing_autotrackers","true"));
    if (autotrack&&NULL!=target) {
      if (!AdjustMatrix (mat,velocity,target,type->Speed,autotrack>=2,trackingcone)) {
        if (!firemissingautotrackers)
          return false;
      }
    }else if (this->size&weapon_info::AUTOTRACKING) {
      if (!firemissingautotrackers)
        return false;
    }
    if (type->type!=weapon_info::BEAM) {
      ref.refire =0;
      if (ammo>0)
        ammo--;
    }else {
      static bool reduce_beam_ammo = XMLSupport::parse_bool (vs_config->getVariable ("physics","reduce_beam_ammo","0"));      
      if (ammo>0&&reduce_beam_ammo) {
        ammo--;
      }
    }
    time_to_lock = type->LockTime;
			switch (type->type) {
			case weapon_info::BEAM:
				if (ref.gun)
					ref.gun->Init(Transformation(orient,pos.Cast()),*type,owner,caller);
			  break;
			case weapon_info::BOLT:
			case weapon_info::BALL:
              caller->energy-=type->EnergyRate;
			  hint=Bolt (type, mat, velocity, owner,hint).location;//FIXME turrets won't work! Velocity
			  break;
			case weapon_info::PROJECTILE:
			{
				static bool match_speed_with_target = XMLSupport::parse_float (vs_config->getVariable("physics","match_speed_with_target","true"));
				string skript =/*string("ai/script/")+*/type->file+string(".xai");
				VSError err = LookForFile( skript, AiFile);
                                if (err<=Ok) {
                                  temp = UnitFactory::createMissile (type->file.c_str(),caller->faction,"",type->Damage,type->PhaseDamage,type->Range/type->Speed,type->Radius,type->RadialSpeed,type->PulseSpeed/*detonation_radius*/);
                                  if (!match_speed_with_target) {
                                    temp->GetComputerData().max_combat_speed= type->Speed+velocity.Magnitude();
                                    temp->GetComputerData().max_combat_ab_speed= type->Speed+velocity.Magnitude();
                                  }
                                }else {
//						temp = UnitFactory::createUnit(type->file.c_str(),false,owner->faction);
                                          
                                  Flightgroup * testfg = caller->getFlightgroup();
                                  if (testfg == NULL) {
                                    static Flightgroup bas;
                                    bas.name="Base";
                                    testfg=&bas;

                                  }
                                  if (testfg->name == "Base")
                                  {
                                    int fgsnumber=0;
                                    Flightgroup * fg = Flightgroup::newFlightgroup ("Base_Patrol",
                                                                                    type->file,
                                                                                    FactionUtil::GetFactionName(caller->faction),
                                                                                    "deafult",
                                                                                    1,
                                                                                    1,
                                                                                    "",
                                                                                    "",
                                                                                    mission);
                                    if (fg!=NULL) {
                                      fg->target.SetUnit(caller->Target());
                                      fg->directive="a";
                                      fg->name = "Base_Patrol";   // this fixes base-spawned fighters becoming navpoints, which happens sometimes
                                      
                                      fgsnumber=fg->nr_ships;
                                      fg->nr_ships=1;
                                      fg->nr_ships_left=1;
                                    }
                                    
                                    temp = UnitFactory::createUnit (type->file.c_str(),false,caller->faction,"",fg,fgsnumber);
                                  }
                                  else
                                  {
                                    Flightgroup * fg = caller->getFlightgroup();
                                    int fgsnumber=0;
                                    if (fg!=NULL) {                                      
                                      fgsnumber=fg->nr_ships;
                                      fg->nr_ships++;
                                      fg->nr_ships_left++;
                                    }                                    
                                    temp = UnitFactory::createUnit (type->file.c_str(),false,caller->faction,"",fg,fgsnumber);
                                  }
                                  
                                  
//      fg->name += "_Patrol";



//	  temp = UnitFactory::createUnit (type->file.c_str(),false,caller->faction,"",fg,fgsnumber);
// this stuff happens farther down, no panic
//	  temp->PrimeOrdersLaunched();
//	  temp->SetTurretAI();
//	  temp->EnqueueAI (new Orders::AggressiveAI ("interceptor.agg.xml"));
//	  temp->Tar=caller->Target;
	  //he's alive!!!!!
					
					


					
					
					
					
					
					
					}
                                        Vector adder=Vector(mat.r[6],mat.r[7],mat.r[8])*type->Speed;
					temp->SetVelocity(caller->GetVelocity()+adder);
					
			  // Affect the stored mount serial to the new missile
			  temp->SetSerial( this->serial);
			  this->serial = 0;
			  if (target&&target!=owner) {
					temp->Target (target);
//void GetMadAt(Unit* un, Unit * parent, int numhits=0) {

					temp->TargetTurret(target);
					if (err<=Ok) {
						temp->EnqueueAI (new AIScript ((type->file+".xai").c_str()));
						temp->EnqueueAI (new Orders::FireAllYouGot);
						if (match_speed_with_target)
							temp->GetComputerData().velocity_ref.SetUnit(target);
					}else {
						temp->EnqueueAI(new Orders::AggressiveAI("default.agg.xml"));
						temp->SetTurretAI();
						temp->TurretFAW(); // turrets are for DEFENSE damnit!
						temp->owner = caller; // spawned wingmen act as cargo (owned) wingmen, not as hired wingmen
//						static float fudge = XMLSupport::parse_float(vs_config->getVariable("ai","launched_relation_mult","100.0"));
                                                float relat;
						relat = caller->getRelation(target);
						if (caller->isSubUnit()&&relat>=0)
						{

                                                  //relat = caller->owner->getRelation(target);
                                                  relat=-1;
                                                  temp->owner = caller->owner;
						}
						if (relat<0) {
                                                  int i=0;
                                                  while (relat < temp->getRelation(target)&&i++<100) {
                                                    GetMadAt(target, temp, 2);
                                                  }
                                                }
                                                // pissed off					getMadAt(target, 10); // how do I cause an attack here?
                                                
					}
			  } else {
					temp->EnqueueAI (new Orders::MatchLinearVelocity(Vector (0,0,100000),true,false));
					temp->EnqueueAI (new Orders::FireAllYouGot);
			  }
			  temp->SetOwner ((Unit*)owner);
			  temp->Velocity = velocity+adder;
			  temp->curr_physical_state = temp->prev_physical_state= temp->cumulative_transformation = tmp;
			  CopyMatrix (temp->cumulative_transformation_matrix,m);
			  _Universe->activeStarSystem()->AddUnit(temp);
                          temp->UpdateCollideQueue(_Universe->activeStarSystem(),hint);
			}
			break;
			}

    static bool use_separate_sound=XMLSupport::parse_bool (vs_config->getVariable ("audio","high_quality_weapon","true"));
    static bool ai_use_separate_sound=XMLSupport::parse_bool (vs_config->getVariable ("audio","ai_high_quality_weapon","false"));
	static bool ai_sound=XMLSupport::parse_bool (vs_config->getVariable ("audio","ai_sound","true"));
	Cockpit * cp;
	bool ips = ((cp=_Universe->isPlayerStarshipVoid(owner))!=NULL);
    if ((((!use_separate_sound)||type->type==weapon_info::BEAM)||((!ai_use_separate_sound)&&!ips))&&(isMissile(type)==false)) {
		if (ai_sound||(ips&&type->type==weapon_info::BEAM)) {
			if (!AUDIsPlaying (sound)) {
				AUDPlay (sound,tmp.position,velocity,1);
			}else {
				AUDAdjustSound(sound,tmp.position,velocity);
			}
		}
    }else {
		if (ai_sound||ips) {
			int snd =AUDCreateSound(sound,false);
                        if (ips&&cp!=NULL&&cp->GetView()<=CP_RIGHT) {
                          AUDAdjustSound(snd,Vector(0,0,0),velocity);
                          static float gain=XMLSupport::parse_float(vs_config->getVariable("audio","weapon_gain",".25"));
                          AUDSoundGain(snd,gain);
                        }else {
                          AUDAdjustSound(snd,tmp.position,velocity);
                        }
			AUDStartPlaying (snd);
			AUDDeleteSound(snd);
		}
    }
    return true;
  }
  return true;
}
bool Mount::NextMountCloser(Mount * nextmount, Unit * firer) {
  Unit * target;
  if (nextmount&&(target=firer->Target())) {
    Matrix mat;
    nextmount->orient.to_matrix(mat);
    Vector nextR=mat.getR();
    this->orient.to_matrix(mat);
    Vector diff = firer->LocalCoordinates(target);
    Vector nextmountnorm=diff-nextmount->pos;
    nextmountnorm.Normalize();
    Vector thismountnorm=diff-this->pos;
    thismountnorm.Normalize();
    return (nextR.Dot(nextmountnorm) > mat.getR().Dot(thismountnorm));
  }
  return false;
}
bool Mount::Fire (Unit * firer, void * owner,bool Missile, bool listen_to_owner) {
  if (ammo==0) {
    processed=UNFIRED;
  }

  if (processed==FIRED||status!=ACTIVE||(Missile!=(isMissile(type)))||ammo==0)
    return false;
  if (type->type==weapon_info::BEAM) {
    bool fireit=ref.gun==NULL;
    if (!fireit)
      fireit = ref.gun->Ready();
    else
      ref.gun = new Beam (Transformation(orient,pos.Cast()),*type,owner,firer,sound);
    if (fireit) {
      ref.gun->ListenToOwner(listen_to_owner);
      processed=FIRED;
    }
    return true;
  }else { 
    if (ref.refire>=type->Refire) {
      processed=FIRED;	
      if(owner==_Universe->AccessCockpit()->GetParent()){
		  forcefeedback->playLaser();
      };

      return true;
    }
  }
  return false;
}
void Mount::PhysicsAlignedUnfire() {
  //Stop Playing SOund?? No, that's done in the beam, must not be aligned
  if (processed==UNFIRED) {
  if (AUDIsPlaying (sound))
    AUDStopPlaying (sound);
    processed=PROCESSED;
  }
}

void Mount::ReplaceSound () {
  sound = AUDCreateSound (sound,type->type!=weapon_info::PROJECTILE);//copy constructor basically
}

