/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vsfilesystem.h"
#include "vs_globals.h"
#include "file_main.h"
#include "gfx/halo.h"


#include "unit_factory.h"

#include "gfx/sprite.h"
#include "lin_time.h"

#include "gfx/vsbox.h"
#include "bolt.h"
#include "gfx/lerp.h"
#include "gfx/bsp.h"
#include "audiolib.h"
#include "gfx/cockpit.h"
#include "config_xml.h"
#include "images.h"
#include "gfx/planetary_transform.h"
///for saving features
#include "main_loop.h"
#include "script/mission.h"
#include "script/flightgroup.h"
#include "collide/rapcol.h"
#include "savegame.h"
#include "xml_serializer.h"
#include "python/python_class.h"
#include "cmd/ai/missionscript.h"
#include "gfx/particle.h"
#include "cmd/ai/aggressive.h"
#include "cmd/base.h"
//#include "unit_template.h"
//#include "gfx/animation.h"
#include "gfx/point_to_cam.h"
#include "unit_jump.cpp"
#include "unit_customize.cpp"
#include "unit_damage.cpp"
#include "unit_physics.cpp"
#include "unit_bsp.h"
#include "unit_click.cpp"

//if the PQR of the unit may be variable...for radius size computation
//#define VARIABLE_LENGTH_PQR

//#define DESTRUCTDEBUG
#include "beam.h"
#include "python/init.h"
#include "unit_const_cache.h"
extern double interpolation_blend_factor;
extern double saved_interpolation_blend_factor;
extern bool cam_setup_phase;

/**** MOVED FROM BASE_INTERFACE.CPP ****/
extern string getCargoUnitName (const char *name);

template<class UnitType>
void GameUnit<UnitType>::UpgradeInterface(Unit * baseun) {
	if (BaseInterface::CurrentBase) {
		BaseInterface::CurrentBase->Terminate();
	}
	  string basename = (::getCargoUnitName(baseun->getFullname().c_str()));
	  if (baseun->isUnit()!=PLANETPTR) {
	    basename = baseun->name;
	  }
	  BaseInterface *base=new BaseInterface (basename.c_str(),baseun,this);
	  base->InitCallbacks();
//	  SetSoftwareMousePosition(0,0);
}

template <class UnitType>
GameUnit<UnitType>::GameUnit( int /*dummy*/ ) : sparkle_accum(0) {
		this->Unit::Init();
}

#define PARANOIA .4

extern void UncheckUnit (Unit * un);

/*
template <class UnitType>
void GameUnit<UnitType>::Init()
{
  this->Unit::Init();
  //  unsigned int numg= 1+MAXVDUS+UnitImages::NUMGAUGES;
  //  image->cockpit_damage=(float*)malloc((numg)*sizeof(float));
  //  for (unsigned int damageiterator=0;damageiterator<numg;damageiterator++) {
  //	image->cockpit_damage[damageiterator]=1;
  //  }
  CollideInfo.object.u = NULL;
  CollideInfo.type = LineCollide::UNIT;
  CollideInfo.Mini.Set (0,0,0);
  CollideInfo.Maxi.Set (0,0,0);
  SetAI (new Order());
  //yprrestricted=0;
  //ymin = pmin = rmin = -PI;
  //ymax = pmax = rmax = PI;
  //ycur = pcur = rcur = 0;
  //static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
  //static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
  //static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics", "autotracking", ".93"));// DO NOT CHANGE see unit_customize.cpp
  //static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  //  Fire();
}
*/

template <class UnitType>
VSSprite * GameUnit<UnitType>::getHudImage () const{
	return this->image->hudImage;
}

template <class UnitType>
GameUnit<UnitType>::GameUnit (std::vector <Mesh *>& meshes, bool SubU, int fact):UnitType( meshes, SubU, fact),sparkle_accum(0) {

}
extern void update_ani_cache();
template <class UnitType>
GameUnit<UnitType>::GameUnit(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber, string * netxml) : sparkle_accum(0) {
	Unit::Init( filename, SubU, faction, unitModifications, flightgrp, fg_subnumber, netxml);
}

template <class UnitType>
GameUnit<UnitType>::~GameUnit()
{
  if (this->image->hudImage )
    delete this->image->hudImage;
  if (this->image->explosion){
    delete this->image->explosion;
    this->image->explosion=NULL;
  }
  //  VSFileSystem::vs_fprintf (stderr,"Freeing Unit %s\n",name.c_str());
  for(unsigned int meshcount = 0; meshcount < this->meshdata.size(); meshcount++)
    if (this->meshdata[meshcount])
      delete this->meshdata[meshcount];
  this->meshdata.clear();
  
}

template <class UnitType>
bool GameUnit<UnitType>::queryFrustum(double frustum [6][4]) const{
  int i;
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (cumulative_transformation_matrix[0],cumulative_transformation_matrix[1],cumulative_transformation_matrix[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif
  for (i=0;i<nummesh();i++) {
        TargetPoint = Transform(this->cumulative_transformation_matrix,this->meshdata[i]->Position());
	
	if (GFXSphereInFrustum (frustum, 
				TargetPoint,
				this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				*SizeScaleFactor
#endif
				)){
	  return true;
	}
  }
  un_fkiter iter =this->SubUnits.constFastIterator();
  const Unit * un;
  while ((un = iter.current())) {
    if (((GameUnit<UnitType>*)un)->queryFrustum (frustum)) {
      return true;
    }
    iter.advance();
  }
  return false;
}


template <class UnitType>
void GameUnit<UnitType>::UpdateHudMatrix(int whichcam) {
  Matrix m;
  Matrix ctm=this->cumulative_transformation_matrix;
  /*if (this->planet) {
    Transformation ct (linear_interpolate(this->prev_physical_state, this->curr_physical_state, interpolation_blend_factor));  
    ct.to_matrix (m);
    ctm=m;
  }*/
  Vector q (ctm.getQ());
  Vector r (ctm.getR());
  Vector tmp;
  CrossProduct(r,q, tmp);
  _Universe->AccessCamera(whichcam)->SetOrientation(tmp,q ,r);
  
  _Universe->AccessCamera(whichcam)->SetPosition (Transform (ctm,this->image->CockpitCenter.Cast()),this->GetWarpVelocity(),this->GetAngularVelocity(),this->GetAcceleration());
}
extern bool flickerDamage (Unit * un, float hullpercent);   
extern int cloakVal (int cloakint, int cloakminint, int cloakrateint, bool cloakglass); //short fix?
template <class UnitType>
void GameUnit<UnitType>::DrawNow (const Matrix &mato, float lod) {
  static const void* rootunit=NULL;

  if (rootunit==NULL) rootunit=(const void*)this;

  float damagelevel;
  unsigned char chardamage;
  damagelevel=this->hull/this->maxhull;
  chardamage=(255 - (unsigned char)(damagelevel*255));

#ifdef VARIABLE_LENGTH_PQR
  const float vlpqrScaleFactor = SizeScaleFactor;
#else
  const float vlpqrScaleFactor = 1.f;
#endif

  unsigned int i;
  Matrix mat(mato);
  if (this->graphicOptions.FaceCamera){
	  Vector p,q,r;
	  QVector pos (mato.p);
	  float wid,hei;
	  CalculateOrientation(pos,p,q,r,wid,hei,0,false,&mat);
	  pos=mato.p;
	  VectorAndPositionToMatrix(mat,p,q,r,pos);
  }

  int cloak=this->cloaking;
  if (this->cloaking>this->cloakmin) {
    cloak = cloakVal (cloak,this->cloakmin,this->image->cloakrate, this->image->cloakglass);
  }
  for (i=0;(int)i<this->nummesh();i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
    if (this->meshdata[i]==NULL) 
      continue;
    Vector TransformedPosition = Transform (mat,
					    this->meshdata[i]->Position());
      float d = GFXSphereInFrustum(TransformedPosition,this->meshdata[i]->rSize()*vlpqrScaleFactor);
      if (d) {  //d can be used for level of detail
          //this->meshdata[i]->DrawNow(lod,false,mat,cloak);//cloakign and nebula
          this->meshdata[i]->Draw(lod,mat,d,cloak);
      }
    }
    un_fiter iter =this->SubUnits.fastIterator();
    Unit * un;
    while ((un = iter.current())) {
      Matrix temp;
      un->curr_physical_state.to_matrix (temp);
      Matrix submat;
      MultMatrix (submat,mat,temp);
      (un)->DrawNow (submat,lod);
      iter.advance();
    }
    float haloalpha=1;
    if (cloak>=0) {
      haloalpha=((float)cloak)/2147483647;
    }
    float enginescale = this->GetVelocity().MagnitudeSquared();
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas=this->computer.max_ab_speed()*this->computer.max_ab_speed();
    if (cmas==0)
      cmas =1;
    /*if (enginescale>cmas)
      enginescale=cmas;
    Vector Scale (1,1,enginescale/(cmas));*/
    Vector Scale (1,1,1); //Now HaloSystem should handle it
#endif

  int nummounts= this->GetNumMounts();
  for (i=0;(int)i<nummounts;i++) {
    static bool draw_mounts = XMLSupport::parse_bool (vs_config->getVariable ("graphics","draw_weapons","false"));
    Mount * mahnt = &this->mounts[i];
    if (draw_mounts) {

//      Mesh * gun = WeaponMeshCache::getCachedMutable (mounts[i]->type->weapon_name);
	 if (mahnt->xyscale!=0&&mahnt->zscale!=0) {
      Mesh * gun = mahnt->type->gun;
      if (gun&&mahnt->status!=Mount::UNCHOSEN) {
		  Transformation mountLocation(mahnt->GetMountOrientation(),mahnt->GetMountLocation().Cast());
          mountLocation.Compose (Transformation::from_matrix(mat),this->WarpMatrix(mat));
		  Matrix mmat;
		  mountLocation.to_matrix(mmat);
          if (GFXSphereInFrustum(mountLocation.position,gun->rSize()*vlpqrScaleFactor) > 0) {
              float d = (mountLocation.position-_Universe->AccessCamera()->GetPosition()).Magnitude();
              float lod = g_game.detaillevel*g_game.x_resolution*2*gun->rSize()/GFXGetZPerspective((d-gun->rSize()<g_game.znear)?g_game.znear:d-gun->rSize());
		      ScaleMatrix(mmat,Vector(mahnt->xyscale,mahnt->xyscale,mahnt->zscale));
		      gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));
		      gun->Draw(lod,mmat,d,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula
		      if (mahnt->type->gun1){
			      gun = mahnt->type->gun1;
			      gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));
			      gun->Draw(lod,mmat,d,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula			  
		      }
          }
      }
	 }
    }
  }

  Vector accel = this->GetAcceleration();
  float maxaccel = this->GetMaxAccelerationInDirectionOf(mat.getR(),true);
  Vector velocity = this->GetVelocity();
  if (halos.ShouldDraw (mat,velocity,accel,maxaccel,cmas) && !(this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE)))
    halos.Draw(mat,Scale,cloak,0, this->GetHullPercent(),velocity,accel,maxaccel,cmas,this->faction);

  if (rootunit==(const void*)this) {
      Mesh::ProcessZFarMeshes();
      Mesh::ProcessUndrawnMeshes();
      rootunit=NULL;
  }
}

extern double calc_blend_factor(double frac, int priority, int when_it_will_be_simulated, int cur_simulation_frame);
template <class UnitType>
void GameUnit<UnitType>::Draw(const Transformation &parent, const Matrix &parentMatrix)
{
  //Quick shortcut for camera setup phase 
  bool myparent = (this==_Universe->AccessCockpit()->GetParent());
  /*bool ormygrampa = myparent||(_Universe->AccessCockpit()->GetParent()&&(this==_Universe->AccessCockpit()->GetParent()->owner));
  bool topparent = _Universe->AccessCockpit()->GetParent()&&(_Universe->AccessCockpit()->GetParent()->owner == NULL);
  if (cam_setup_phase&&!ormygrampa&&(topparent||UnitType::SubUnits.empty()))
      return;*/

#ifdef VARIABLE_LENGTH_PQR
  const float vlpqrScaleFactor = SizeScaleFactor;
#else
  const float vlpqrScaleFactor = 1.f;
#endif

  Matrix *ctm;
  Matrix invview;
  Transformation * ct;
  if (cam_setup_phase) {
      this->cumulative_transformation = linear_interpolate(this->prev_physical_state, this->curr_physical_state, interpolation_blend_factor);
      this->cumulative_transformation.Compose(parent, parentMatrix);
      this->cumulative_transformation.to_matrix(this->cumulative_transformation_matrix);
  }
  ctm =&this->cumulative_transformation_matrix;
  ct = &this->cumulative_transformation;
  if (this->graphicOptions.FaceCamera==1) {
	  Vector p,q,r;
	  QVector pos (ctm->p);
	  float wid,hei;
	  float magr = parentMatrix.getR().Magnitude();
	  float magp = parentMatrix.getP().Magnitude();
	  float magq = parentMatrix.getQ().Magnitude();
	  CalculateOrientation(pos,p,q,r,wid,hei,0,false,ctm);
	  VectorAndPositionToMatrix(invview,p*magp,q*magq,r*magr,ctm->p);
//	  _Universe->AccessCamera()->GetView(invview);

	  ctm = &invview;
  }

#ifdef PERFRAMESOUND
  AUDAdjustSound (sound.engine,cumulative_transformation.position,GetVelocity());
#endif
  int cloak=this->cloaking;
  if (this->cloaking>this->cloakmin) {
    cloak = (int)(this->cloaking-interpolation_blend_factor*this->image->cloakrate*SIMULATION_ATOM);
    cloak = cloakVal ( cloak,this->cloakmin,this->image->cloakrate,this->image->cloakglass);
  }
  
  unsigned int i;
  if ((this->hull <0)&&(!cam_setup_phase)) {
    Explode(true, GetElapsedTime());
  }
  float damagelevel;
  unsigned char chardamage;
  if (!cam_setup_phase) {
      damagelevel=this->hull/this->maxhull;
      chardamage=(255 - (unsigned char)(damagelevel*255));
  }
  bool On_Screen=false;
  if ((!(this->invisible&UnitType::INVISUNIT))&&((!(this->invisible&UnitType::INVISCAMERA))||(!myparent))) {
      if (!cam_setup_phase) {
          float minmeshradius = (_Universe->AccessCamera()->GetVelocity().Magnitude()+this->Velocity.Magnitude())*SIMULATION_ATOM;
          unsigned int numKeyFrames = this->graphicOptions.NumAnimationPoints;
          for (i=0;i<this->meshdata.size();i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
              if (this->meshdata[i]==NULL) 
                  continue;
              if ((int)i==this->nummesh()&&(this->meshdata[i]->numFX()==0||this->hull<0)) 
                  continue;
              if (this->meshdata[i]->getBlendDst()==ONE) {
                  if ((this->invisible&UnitType::INVISGLOW)!=0)
                      continue;
                  
                  if (damagelevel<.9)
                      if (flickerDamage (this,damagelevel))
                          continue;
              }
              QVector TransformedPosition = Transform (*ctm,
                  this->meshdata[i]->Position().Cast());
#if 0
              //This is a test of the box in frustum setup to be used with terrain
              GFXBoxInFrustumModel (ctm);
              int tmp = GFXBoxInFrustum (meshdata[i]->corner_min(),meshdata[i]->corner_max());
              if ((d==0)!=(tmp==0)) {
                  VSFileSystem::vs_fprintf (stderr,"Mismatch for %s with Box being %d", name.c_str(),tmp);
              }
#endif
              
              //      VSFileSystem::vs_fprintf (stderr,"%s %d ",name.c_str(),i);
              double d = GFXSphereInFrustum(TransformedPosition,minmeshradius+this->meshdata[i]->clipRadialSize()*vlpqrScaleFactor);
              double lod;
              //      VSFileSystem::vs_fprintf (stderr,"\n");
              if (d) {  //d can be used for level of detail shit
                  d = (TransformedPosition-_Universe->AccessCamera()->GetPosition()).Magnitude();
                  if ((lod =g_game.detaillevel*g_game.x_resolution*2*this->meshdata[i]->rSize()/GFXGetZPerspective((d-this->meshdata[i]->rSize()<g_game.znear)?g_game.znear:d-this->meshdata[i]->rSize()))>=g_game.detaillevel) {//if the radius is at least half a pixel (detaillevel is the scalar... so you gotta make sure it's above that
                      float currentFrame = this->meshdata[i]->getCurrentFrame();
                      this->meshdata[i]->Draw(lod,this->WarpMatrix(*ctm),d,i==this->meshdata.size()-1?-1:cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage);//cloakign and nebula		
                      On_Screen=true;
                      unsigned int numAnimFrames=0;
                      if (this->meshdata[i]->getFramesPerSecond()&&
                          (numAnimFrames=this->meshdata[i]->getNumAnimationFrames(""))) {
                          float currentprogress=floor(this->meshdata[i]->getCurrentFrame()*numKeyFrames/(float)numAnimFrames);
                          if (numKeyFrames&&
                              floor(currentFrame*numKeyFrames/(float)numAnimFrames)   !=
                              currentprogress) {
                              this->graphicOptions.Animating=0;
                              this->meshdata[i]->setCurrentFrame(.1+currentprogress*numAnimFrames/(float)numKeyFrames);
                          }else if (!this->graphicOptions.Animating) {
                              this->meshdata[i]->setCurrentFrame(currentFrame);//dont' budge
                          }
                      }		
                  } else {
                      
                  }
              }
          }
      }
      
      {
          un_fiter iter =this->SubUnits.fastIterator();
          Unit * un;
          double backup = interpolation_blend_factor;
          int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
          while ((un = iter.current())) {
              float backup=SIMULATION_ATOM;
              if (this->sim_atom_multiplier&&un->sim_atom_multiplier)
                  SIMULATION_ATOM = SIMULATION_ATOM*un->sim_atom_multiplier/this->sim_atom_multiplier;

              interpolation_blend_factor=calc_blend_factor(saved_interpolation_blend_factor,un->sim_atom_multiplier,un->cur_sim_queue_slot,cur_sim_frame);
              (un)->Draw (*ct,*ctm);
              iter.advance();

              SIMULATION_ATOM = backup;
          }
          interpolation_blend_factor = backup;
      }
  
      if(this->selected) {
          //      static bool doInputDFA=XMLSupport::parse_bool (vs_config->getVariable ("graphics","MouseCursor","false"));
          //      if (doInputDFA)
          //	image->selectionBox->Draw(g_game.x_resolution,*ctm);
      }
  } else {
	  _Universe->AccessCockpit()->SetupViewPort();///this is the final, smoothly calculated cam
    //        UpdateHudMatrix();
    /***DEBUGGING cosAngleFromMountTo
    UnitCollection *dL = _Universe->activeStarSystem()->getUnitList();
    UnitCollection::UnitIterator *tmpiter = dL->createIterator();
    GameUnit<UnitType> * curun;
    while (curun = tmpiter->current()) {
      if (curun->selected) {
	float tmpdis;
	float tmpf = cosAngleFromMountTo (curun, tmpdis);
        VSFileSystem::vs_fprintf (stderr,"%s: <%f d: %f\n", curun->name.c_str(), tmpf, tmpdis);

      }
      tmpiter->advance();
    }
    delete tmpiter;
    **/
  }

  if (cam_setup_phase) return;

  int nummounts= this->GetNumMounts();
  for (i=0;(int)i<nummounts;i++) {
    static bool draw_mounts = XMLSupport::parse_bool (vs_config->getVariable ("graphics","draw_weapons","false"));
	Mount * mahnt = &this->mounts[i];
    if (draw_mounts&&On_Screen) {
		
//      Mesh * gun = WeaponMeshCache::getCachedMutable (mounts[i]->type->weapon_name);
	 if (mahnt->xyscale!=0&&mahnt->zscale!=0) {		
      Mesh * gun = mahnt->type->gun;	  
      if (gun&&mahnt->status!=Mount::UNCHOSEN) {
		  Transformation mountLocation(mahnt->GetMountOrientation(),mahnt->GetMountLocation().Cast());
		  mountLocation.Compose (*ct,this->WarpMatrix(*ctm));
		  Matrix mat;
		  mountLocation.to_matrix(mat);
          if (GFXSphereInFrustum(mountLocation.position,gun->rSize()*vlpqrScaleFactor) > 0) {
              float d = (mountLocation.position-_Universe->AccessCamera()->GetPosition()).Magnitude();
              float lod = g_game.detaillevel*g_game.x_resolution*2*gun->rSize()/GFXGetZPerspective((d-gun->rSize()<g_game.znear)?g_game.znear:d-gun->rSize());
		      ScaleMatrix(mat,Vector(mahnt->xyscale,mahnt->xyscale,mahnt->zscale));
		      gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));
		      gun->Draw(lod,mat,d,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula
		      if (mahnt->type->gun1){
			      gun = mahnt->type->gun1;
			      gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));
			      gun->Draw(lod,mat,d,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula
		      }
          }
      }
	 }
	}
    if (this->mounts[i].type->type==weapon_info::BEAM) {
      if (this->mounts[i].ref.gun) {
	this->mounts[i].ref.gun->Draw(*ct,this->WarpMatrix(*ctm),((this->mounts[i].size&weapon_info::AUTOTRACKING)&&this->mounts[i].time_to_lock<=0)? Unit::Target():NULL,this->computer.radar.trackingcone);
      }
    }
  }
  float haloalpha=1;
  if (cloak>=0) {
    haloalpha=((float)cloak)/2147483647;
  }
  if (On_Screen&&(halos.NumHalos()>0)&&!(this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE))) {
    Vector accel = this->GetAcceleration();
    float maxaccel = this->GetMaxAccelerationInDirectionOf(this->WarpMatrix(*ctm).getR(),true);
    Vector velocity = this->GetVelocity();
    //float enginescale = this->GetVelocity().MagnitudeSquared();
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas=this->computer.max_ab_speed()*this->computer.max_ab_speed();
    if (cmas==0)
      cmas =1;
    /*if (enginescale>cmas)
      enginescale=cmas;
    Vector Scale (1,1,enginescale/(cmas));*/
    Vector Scale (1,1,1);// Now, HaloSystem handles that
#endif
    //WARNING: cmas is not a valid maximum speed for the upcoming multi-direction thrusters, 
    //  nor is maxaccel. Instead, each halo should have its own limits specified in units.csv
    if (halos.ShouldDraw(this->WarpMatrix(*ctm),velocity,accel,maxaccel,cmas)) 
      halos.Draw(this->WarpMatrix(*ctm),Scale,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,this->GetHull()>0?damagelevel:1.0,velocity,accel,maxaccel,cmas,this->faction);
  }
  if (On_Screen&&!UnitType::graphicOptions.NoDamageParticles&&!(this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE))) {
	int numm = this->nummesh();
	if (damagelevel<.99&&numm>0&&this->GetHull()>0) {
		unsigned int switcher=(damagelevel>.8)?1:
			(damagelevel>.6)?2:(damagelevel>.4)?3:(damagelevel>.2)?4:5;
		const unsigned long thus=(unsigned long)this;

        static float sparklerate = XMLSupport::parse_float ( vs_config->getVariable("graphics","sparklerate","5") );
        sparkle_accum += GetElapsedTime()*sparklerate;
        int spawn=(int)(sparkle_accum);
        sparkle_accum -= spawn;
        while (spawn-- > 0) {
		    switch (switcher) {
		    case 5:
			    LaunchOneParticle(*ctm,this->GetVelocity(),((long)this)+165,this,damagelevel,this->faction);
		    case 4:
			    LaunchOneParticle(*ctm,this->GetVelocity(),((long)this)+47,this,damagelevel,this->faction);
		    case 3:
			    LaunchOneParticle(*ctm,this->GetVelocity(),((long)this)+61,this,damagelevel,this->faction);
		    case 2:
			    LaunchOneParticle(*ctm,this->GetVelocity(),((long)this)+65537,this,damagelevel,this->faction);			
		    default:
			    LaunchOneParticle(*ctm,this->GetVelocity(),((long)this)+257,this,damagelevel,this->faction);
		    }
        }
	}
  }
}
using Orders::FireAt;



 

#if 0
template <class UnitType>
void GameUnit<UnitType>::SwapOutHalos() {
  for (int i=0;i<numhalos;i++) {
    // float x,y;
    //halos[i]->GetDimensions (x,y);    //halos[i]->SetDimensions (x/(1024),y/(1024));
    halos[i]->Draw (cumulative_transformation,cumulative_transformation_matrix,0);
  }
}
template <class UnitType>
void GameUnit<UnitType>::SwapInHalos() {
  for (int i=0;i<numhalos;i++) {
    // float x,y;
    //halos[i]->GetDimensions (x,y);
    //halos[i]->SetDimensions (x*(1024),y*(1024));
  }
}
#endif
