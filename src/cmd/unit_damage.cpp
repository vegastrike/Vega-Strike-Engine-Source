//#include "unit.h"
//#include "unit_template.h"
#include "unit_factory.h"
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
#include "missile.h"
#include "cmd/ai/communication.h"
#include "cmd/script/flightgroup.h"
#include "music.h"
//#define DESTRUCTDEBUG
#include "base.h"
extern unsigned short apply_float_to_short (float tmp);

static list<Unit*> Unitdeletequeue;
extern std::vector <Mesh *> MakeMesh(unsigned int mysize);

template<class UnitType>
void GameUnit<UnitType>::Kill(bool eraseFromSave)
{
  if (this->colTrees)
    this->colTrees->Dec();//might delete
  this->colTrees=NULL;
  for (int beamcount=0;beamcount<GetNumMounts();beamcount++) {
    AUDStopPlaying(mounts[beamcount].sound);
    AUDDeleteSound(mounts[beamcount].sound);
    if (mounts[beamcount].ref.gun&&mounts[beamcount].type->type==weapon_info::BEAM)
      delete mounts[beamcount].ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }
  UnitType::Kill(eraseFromSave);
}

template<class UnitType>
void GameUnit<UnitType>::Split (int level) {
  Vector PlaneNorm;
  int i;
  for (i=0;i<nummesh();) {
	  if (meshdata[i]){
		  if (meshdata[i]->getBlendDst()==ONE){
			  delete meshdata[i];
			  meshdata.erase(meshdata.begin()+i);
		  }else{i++;}
	  }else{meshdata.erase(meshdata.begin()+i);}
  }
  int nm = nummesh();
  if (nm<=0) {
    return;
  }
  
  std::vector <Mesh *> old = meshdata;

  for (int split=0;split<level;split++) {
    std::vector<Mesh *> nw= MakeMesh(nm*2+1);
    nw[nm*2]=old[nm];//copy shield
    for (i=0;i<nm;i++) {
      PlaneNorm.Set (rand()-RAND_MAX/2,rand()-RAND_MAX/2,rand()-RAND_MAX/2+.5);
      PlaneNorm.Normalize();  
      old[i]->Fork (nw[i*2], nw[i*2+1],PlaneNorm.i,PlaneNorm.j,PlaneNorm.k,-PlaneNorm.Dot(old[i]->Position()));//splits somehow right down the middle.
      if (nw[i*2]&&nw[i*2+1]) {
	delete old[i];
	old[i]=NULL;
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
    old = nw;
  }
  if (old[nm])
    delete old[nm];
  old[nm]=NULL;
  for (i=0;i<nm;i++) {
    Unit * splitsub;
    std::vector<Mesh *> tempmeshes;
    tempmeshes.push_back (old[i]);
    SubUnits.prepend(splitsub = UnitFactory::createUnit (tempmeshes,true,faction));
    splitsub->hull = 1000;
    splitsub->mass = mass/level;
    splitsub->image->timeexplode=.1;
    if (splitsub->meshdata[0]) {
      Vector loc = splitsub->meshdata[0]->Position();
      static float explosion_force = XMLSupport::parse_float (vs_config->getVariable ("graphics","explosionforce",".5"));//10 seconds for auto to kick in;
	  float locm=loc.Magnitude();
	  if (locm<.0001)
		  locm=1;
      splitsub->ApplyForce(splitsub->meshdata[0]->rSize()*explosion_force*10*splitsub->GetMass()*loc/locm);
      loc.Set (rand(),rand(),rand()+.1);
      loc.Normalize();
      static float explosion_torque = XMLSupport::parse_float (vs_config->getVariable ("graphics","explosiontorque",".001"));//10 seconds for auto to kick in;
      splitsub->ApplyLocalTorque(loc*splitsub->GetMoment()*explosion_torque*(1+rand()%(int)(1+rSize())));
    }
  }
  old.clear();
  meshdata.clear();
  meshdata.push_back(NULL);//the shield
  //FIXME...how the heck can they go spinning out of control!
}

extern Music *muzak;

extern float rand01 ();

template <class UnitType>
void GameUnit<UnitType>::ArmorDamageSound( const Vector &pnt)
{
	if (!AUDIsPlaying (sound->armor))
      AUDPlay (sound->armor,ToWorldCoordinates(pnt).Cast()+cumulative_transformation.position,Velocity,1);
    else
      AUDAdjustSound (sound->armor,ToWorldCoordinates(pnt).Cast()+cumulative_transformation.position,Velocity);
}

template <class UnitType>
void GameUnit<UnitType>::HullDamageSound( const Vector &pnt)
{
   if (!AUDIsPlaying (sound->hull))
     AUDPlay (sound->hull,ToWorldCoordinates(pnt).Cast()+cumulative_transformation.position,Velocity,1);
   else
     AUDAdjustSound (sound->hull,ToWorldCoordinates(pnt).Cast()+cumulative_transformation.position,Velocity);
}

template <class UnitType>
float GameUnit<UnitType>::DealDamageToShield (const Vector &pnt, float &damage) {
  float percent = UnitType::DealDamageToShield( pnt, damage);
  if (percent&&!AUDIsPlaying (sound->shield))
	AUDPlay (sound->shield,ToWorldCoordinates(pnt).Cast()+cumulative_transformation.position,Velocity,1);
  else
	AUDAdjustSound (sound->shield,ToWorldCoordinates(pnt).Cast()+cumulative_transformation.position,Velocity);

  return percent;
}

template <class UnitType>
float GameUnit<UnitType>::ApplyLocalDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit,const GFXColor &color, float phasedamage) {
  static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
  //  #ifdef REALLY_EASY
  float absamt= amt>=0?amt:-amt;
  Cockpit * cpt;
  if ((cpt=_Universe->isPlayerStarship(this))!=NULL) {
    if (color.a!=2) {
      //    ApplyDamage (amt);
      phasedamage*= (g_game.difficulty);
      amt*=(g_game.difficulty);
      cpt->Shake (absamt);
    }
  }
  //  #endif
  float percentage=0;
  percentage = UnitType::ApplyLocalDamage( pnt, normal,amt, affectedUnit, color, phasedamage);
  float leakamt = phasedamage+amt*.01*shield.leak;
  if( percentage==-1)
	return -1;
  if (GetNebula()==NULL||(nebshields>0)) {
    percentage = DealDamageToShield (pnt,absamt);
	amt = amt>=0?absamt:-absamt;
    if (meshdata.back()&&percentage>0&&amt==0) {//shields are up
      /*      meshdata[nummesh]->LocalFX.push_back (GFXLight (true,
	      GFXColor(pnt.i+normal.i,pnt.j+normal.j,pnt.k+normal.k),
	      GFXColor (.3,.3,.3), GFXColor (0,0,0,1), 
	      GFXColor (.5,.5,.5),GFXColor (1,0,.01)));*/
      //calculate percentage
      if (GetNebula()==NULL) 
	meshdata.back()->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector(0,0,0),percentage,color);
    }
  }
  if (shield.leak>0||!meshdata.back()||percentage==0||absamt>0||phasedamage) {
    percentage = DealDamageToHull (pnt, leakamt+amt);
    if (percentage!=-1) {//returns -1 on death--could delete
      for (int i=0;i<nummesh();i++) {
	if (percentage)
	  meshdata[i]->AddDamageFX(pnt,shieldtight?shieldtight*normal:Vector (0,0,0),percentage,color);
      }
    }
  }
  return 1;
}

extern void ScoreKill (Cockpit * cp, Unit * un, int faction);

template <class UnitType>
void GameUnit<UnitType>::ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedUnit, const GFXColor & color, Unit * ownerDoNotDereference, float phasedamage) {
  Cockpit * cp = _Universe->isPlayerStarship (ownerDoNotDereference);

  if (cp) {
      //now we can dereference it because we checked it against the parent
      CommunicationMessage c(ownerDoNotDereference,this,NULL,0);
      c.SetCurrentState(c.fsm->GetHitNode(),NULL,0);
      if (this->getAIState()) this->getAIState()->Communicate (c);      
      Threaten (ownerDoNotDereference,10);//the dark danger is real!
  }
  bool mykilled = hull<0;
  Vector localpnt (InvTransform(cumulative_transformation_matrix,pnt));
  Vector localnorm (ToLocalCoordinates (normal));
  ApplyLocalDamage(localpnt, localnorm, amt,affectedUnit,color,phasedamage);
  if (hull<0&&(!mykilled)) {
    if (cp) {
      ScoreKill (cp,ownerDoNotDereference,faction);
      
    }
  }
}
extern Animation * GetVolatileAni (unsigned int);
extern unsigned int AddAnimation (const QVector &, const float, bool, const std::string &, float percentgrow);


extern Animation * getRandomCachedAni() ;
extern std::string getRandomCachedAniString() ;
extern void disableSubUnits(Unit * un);
template <class UnitType>
bool GameUnit<UnitType>::Explode (bool drawit, float timeit) {

  if (image->explosion==NULL&&image->timeexplode==0) {	//no explosion in unit data file && explosions haven't started yet

  // notify the director that a ship got destroyed
    mission->DirectorShipDestroyed(this);
    disableSubUnits(this);
    image->timeexplode=0;
    static std::string expani = vs_config->getVariable ("graphics","explosion_animation","explosion_orange.ani");

    string bleh=image->explosion_type;
    if (bleh.empty()) {
      FactionUtil::getRandAnimation(faction,bleh);
    }
    if (bleh.empty()) {
      static Animation cache(expani.c_str(),false,.1,BILINEAR,false);
      bleh = getRandomCachedAniString();
      if (bleh.size()==0) {
	bleh = expani;
      }
    }
    image->explosion= new Animation (bleh.c_str(),false,.1,BILINEAR,false);
    image->explosion->SetDimensions(ExplosionRadius(),ExplosionRadius());
    if (isUnit()!=MISSILEPTR) {
      static float expdamagecenter=XMLSupport::parse_float(vs_config->getVariable ("physics","explosion_damage_center","1"));
      static float damageedge=XMLSupport::parse_float(vs_config->getVariable ("graphics","explosion_damage_edge",".125"));
      _Universe->activeStarSystem()->AddMissileToQueue (new MissileEffect (Position().Cast(),MaxShieldVal(),0,ExplosionRadius()*expdamagecenter,ExplosionRadius()*expdamagecenter*damageedge));
    }
	if (!SubUnit){
		QVector exploc = cumulative_transformation.position;
		Unit * un;
		if (NULL!=(un=_Universe->AccessCockpit(0)->GetParent())) {
			exploc = un->Position();						
		}
	    AUDPlay (sound->explode,exploc,Velocity,1);

	  un=_Universe->AccessCockpit()->GetParent();
	  if (isUnit()==UNITPTR) {
	    static float percentage_shock=XMLSupport::parse_float(vs_config->getVariable ("graphics","percent_shockwave",".5"));
	    if (rand () < RAND_MAX*percentage_shock&&(!SubUnit)) {
	      static float shockwavegrowth=XMLSupport::parse_float(vs_config->getVariable ("graphics","shockwave_growth","1.05"));
	      static string shockani (vs_config->getVariable ("graphics","shockwave_animation","explosion_wave.ani"));
	      
	      static Animation * __shock__ani = new Animation (shockani.c_str(),true,.1,MIPMAP,false);
	      __shock__ani->SetFaceCam(false);
	      unsigned int which = AddAnimation (Position(),ExplosionRadius(),true,shockani,shockwavegrowth);
	      Animation * ani = GetVolatileAni (which);
	      if (ani) {
		ani->SetFaceCam(false);
		Vector p,q,r;
		GetOrientation(p,q,r);
		int tmp = rand();

		if (tmp < RAND_MAX/24) {
		  ani->SetOrientation (Vector(0,0,1),Vector(1,0,0),Vector(0,1,0));
		}else if (tmp< RAND_MAX/16) {
		  ani->SetOrientation (Vector(0,1,0),Vector(0,0,1),Vector(1,0,0));
		}else if (tmp < RAND_MAX/8) {
		  ani->SetOrientation (Vector(1,0,0),Vector(0,1,0),Vector(0,0,1));
		}else {
		  ani->SetOrientation (p,q,r);
		}
	      }
	    }
		  if (un ) {
			static float badrel=XMLSupport::parse_float(vs_config->getVariable("sound","loss_relationship","-.1"));
			static float goodrel=XMLSupport::parse_float(vs_config->getVariable("sound","victory_relationship",".5"));
			float rel=un->getRelation(this);
			if (!BaseInterface::CurrentBase)
			  if (rel>goodrel) {
			    muzak->SkipRandSong(Music::LOSSLIST);
			  } else if (rel < badrel) {
			    muzak->SkipRandSong(Music::VICTORYLIST);
			  }
		  } else {
			muzak->SkipRandSong(Music::LOSSLIST);
		  }
	  }
	}
  }
  static float timebeforeexplodedone = XMLSupport::parse_float (vs_config->getVariable ("physics","debris_time","500"));
  bool timealldone =(image->timeexplode>timebeforeexplodedone||isUnit()==MISSILEPTR||_Universe->AccessCockpit()->GetParent()==this||this->SubUnits.empty());
  if (image->explosion) {
      image->timeexplode+=timeit;
      //Translate (tmp,meshdata[i]->Position());
      //MultMatrix (tmp2,cumulative_transformation_matrix,tmp);
      image->explosion->SetPosition(Position());
      Vector p,q,r;
      GetOrientation (p,q,r);
      image->explosion->SetOrientation(p,q,r);
      if (image->explosion->Done()&&timealldone) {
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
      bool temp = su->Explode(drawit,timeit);
      if (su->GetImageInformation().explosion)
	alldone |=temp;
      ui.advance();
    }
  }
  return alldone||(!timealldone);
}
