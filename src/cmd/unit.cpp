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
void GameUnit<UnitType>::SetPlanetHackTransformation (Transformation *&ct,Matrix *&ctm) {
  static Transformation planet_temp_transformation;
  static Matrix planet_temp_matrix;
  if (this->planet) {
    if (this->planet->trans==_Universe->AccessCamera()->GetPlanetaryTransform()&&this->planet->trans!=NULL) {
      Matrix tmp;
      Vector p,q,r;
      QVector c;
      MatrixToVectors (this->cumulative_transformation_matrix,p,q,r,c);
      this->planet->trans->InvTransformBasis(tmp,p,q,r,c);
      MultMatrix (planet_temp_matrix,*_Universe->AccessCamera()->GetPlanetGFX(),tmp);
      planet_temp_transformation = Transformation::from_matrix (planet_temp_matrix);
      ct = &planet_temp_transformation;
      *ctm = planet_temp_matrix;
      ///warning: hack FIXME
      this->cumulative_transformation=*ct;
      CopyMatrix (this->cumulative_transformation_matrix,*ctm);
    }
  }  
}

template <class UnitType>
GameUnit<UnitType>::GameUnit<UnitType>( int /*dummy*/ ) {
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
GameUnit<UnitType>::GameUnit<UnitType> (std::vector <Mesh *>& meshes, bool SubU, int fact):UnitType( meshes, SubU, fact) {

}
extern void update_ani_cache();
template <class UnitType>
GameUnit<UnitType>::GameUnit<UnitType>(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber, string * netxml) {
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
  if (this->planet)
    delete this->planet;
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
  if (this->planet) {
    Transformation ct (linear_interpolate(this->prev_physical_state, this->curr_physical_state, interpolation_blend_factor));  
    ct.to_matrix (m);
    ctm=m;
  }
  Vector q (ctm.getQ());
  Vector r (ctm.getR());
  Vector tmp;
  CrossProduct(r,q, tmp);
  _Universe->AccessCamera(whichcam)->SetOrientation(tmp,q ,r);
  
  _Universe->AccessCamera(whichcam)->SetPosition (Transform (ctm,this->image->CockpitCenter.Cast()),this->GetWarpVelocity(),this->GetAngularVelocity());
}
extern bool flickerDamage (Unit * un, float hullpercent);   
extern int cloakVal (int cloakint, int cloakminint, int cloakrateint, bool cloakglass); //short fix?
template <class UnitType>
void GameUnit<UnitType>::DrawNow (const Matrix &mato, float lod) {
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
      float d = GFXSphereInFrustum(TransformedPosition,
				   this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
      if (d) {  //d can be used for level of detail 
	this->meshdata[i]->DrawNow(lod,false,mat,cloak);//cloakign and nebula
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
    if (enginescale>cmas)
      enginescale=cmas;
    Vector Scale (1,1,enginescale/(cmas));
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
             glEnable(GL_NORMALIZE);
             Transformation mountLocation(mahnt->GetMountOrientation(),mahnt->GetMountLocation().Cast());
             Matrix mountmat; mountLocation.to_matrix(mountmat);
             Matrix ct;
             MultMatrix(ct,mat,mountmat);
             ScaleMatrix(ct,Vector(mahnt->xyscale,mahnt->xyscale,mahnt->zscale));
             gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));		  
             gun->DrawNow(lod,0,ct,1,cloak);//cloakign and nebula
             if (mahnt->type->gun1){
               gun = mahnt->type->gun1;
               gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));		  

               gun->DrawNow(lod,0,ct,1,cloak);//cloakign and nebula			  
             }
             glDisable(GL_NORMALIZE);
           }
	 }
    }
  }
  if (halos.ShouldDraw (enginescale)) 
    halos.Draw(mat,Scale,cloak,0, this->GetHullPercent(),this->GetVelocity(),this->faction);
}
template <class UnitType>
void GameUnit<UnitType>::Draw(const Transformation &parent, const Matrix &parentMatrix)
{

  this->cumulative_transformation = linear_interpolate(this->prev_physical_state, this->curr_physical_state, interpolation_blend_factor);
  Matrix *ctm;
  Matrix invview;
  Transformation * ct;
  this->cumulative_transformation.Compose(parent, parentMatrix);
  ctm =&this->cumulative_transformation_matrix;
  ct = &this->cumulative_transformation;
  this->cumulative_transformation.to_matrix(this->cumulative_transformation_matrix);
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
  SetPlanetHackTransformation (ct,ctm);

#ifdef PERFRAMESOUND
  AUDAdjustSound (sound.engine,cumulative_transformation.position,GetVelocity());
#endif
  int cloak=this->cloaking;
  if (this->cloaking>this->cloakmin) {
    cloak = (int)(this->cloaking-interpolation_blend_factor*this->image->cloakrate*SIMULATION_ATOM);
    cloak = cloakVal ( cloak,this->cloakmin,this->image->cloakrate,this->image->cloakglass);
  }
  
  unsigned int i;
  if (this->hull <0) {
    Explode(true, GetElapsedTime());
  }
  float damagelevel=this->hull/this->maxhull;
  unsigned char chardamage=(char)(damagelevel*255);
  chardamage=255-chardamage;
  bool On_Screen=false;
  float minmeshradius = (_Universe->AccessCamera()->GetVelocity().Magnitude()+this->Velocity.Magnitude())*SIMULATION_ATOM;
  bool myparent = (this==_Universe->AccessCockpit()->GetParent());
  float numKeyFrames = this->graphicOptions.NumAnimationPoints;
  if ((!(this->invisible&UnitType::INVISUNIT))&&((!(this->invisible&UnitType::INVISCAMERA))||(!myparent))) {
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
      double d = GFXSphereInFrustum(TransformedPosition,
				   minmeshradius+this->meshdata[i]->clipRadialSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
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
			float currentprogress=floor(this->meshdata[i]->getCurrentFrame()*numKeyFrames/numAnimFrames);
			if (numKeyFrames&&
				floor(currentFrame*numKeyFrames/numAnimFrames)   !=
				currentprogress) {
				this->graphicOptions.Animating=0;
				this->meshdata[i]->setCurrentFrame(.1+currentprogress*numAnimFrames/numKeyFrames);
			}else if (!this->graphicOptions.Animating) {
				this->meshdata[i]->setCurrentFrame(currentFrame);//dont' budge
			}
		}		
	} else {

	}
      }
    }
    
    un_fiter iter =this->SubUnits.fastIterator();
    Unit * un;
    while ((un = iter.current())) {
      (un)->Draw (*ct,*ctm);
      iter.advance();
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
                  float d = (mat.p-_Universe->AccessCamera()->GetPosition()).Magnitude();
                  float lod =g_game.detaillevel*g_game.x_resolution*2*gun->rSize()/GFXGetZPerspective((d-gun->rSize()<g_game.znear)?g_game.znear:d-gun->rSize());
		  ScaleMatrix(mat,Vector(mahnt->xyscale,mahnt->xyscale,mahnt->zscale));
		  gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));		  
		  gun->Draw(lod,mat,1,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula
		  if (mahnt->type->gun1){
			  gun = mahnt->type->gun1;
			  gun->setCurrentFrame(this->mounts[i].ComputeAnimatedFrame(gun));		  
			  gun->Draw(lod,mat,1,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula			  
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
  if (On_Screen) {
    float enginescale = this->GetVelocity().MagnitudeSquared();
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas=this->computer.max_ab_speed()*this->computer.max_ab_speed();
    if (cmas==0)
      cmas =1;
    if (enginescale>cmas)
      enginescale=cmas;
    Vector Scale (1,1,enginescale/(cmas));
#endif
    if (halos.ShouldDraw (enginescale)) 
      halos.Draw(this->WarpMatrix(*ctm),Scale,cloak,(_Universe->AccessCamera()->GetNebula()==this->nebula&&this->nebula!=NULL)?-1:0,this->GetHull()>0?damagelevel:1.0,this->GetVelocity(),this->faction);
	int numm = this->nummesh();
	if (damagelevel<.99&&numm>0&&this->GetHull()>0) {
		unsigned int switcher=(damagelevel>.8)?1:
			(damagelevel>.6)?2:(damagelevel>.4)?3:(damagelevel>.2)?4:5;
		const unsigned long thus=(unsigned long)this;
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
