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
  if (planet) {
    if (planet->trans==_Universe->AccessCamera()->GetPlanetaryTransform()&&planet->trans!=NULL) {
      Matrix tmp;
      Vector p,q,r;
      QVector c;
      MatrixToVectors (cumulative_transformation_matrix,p,q,r,c);
      planet->trans->InvTransformBasis(tmp,p,q,r,c);
      MultMatrix (planet_temp_matrix,*_Universe->AccessCamera()->GetPlanetGFX(),tmp);
      planet_temp_transformation = Transformation::from_matrix (planet_temp_matrix);
      ct = &planet_temp_transformation;
      *ctm = planet_temp_matrix;
      ///warning: hack FIXME
      cumulative_transformation=*ct;
      CopyMatrix (cumulative_transformation_matrix,*ctm);
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
Sprite * GameUnit<UnitType>::getHudImage () const{
	return image->hudImage;
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
GameUnit<UnitType>::~GameUnit<UnitType>()
{
  if (image->hudImage )
    delete image->hudImage;
  if (image->explosion){
    delete image->explosion;
    image->explosion=NULL;
  }
  if (planet)
    delete planet;
  //  VSFileSystem::vs_fprintf (stderr,"Freeing Unit %s\n",name.c_str());
  if (sound->engine!=-1) {
    AUDStopPlaying (sound->engine);
    AUDDeleteSound (sound->engine);
  }
  if (sound->explode!=-1) {
    AUDStopPlaying (sound->explode);
    AUDDeleteSound (sound->explode);
  }
  if (sound->shield!=-1) {
    AUDStopPlaying (sound->shield);
    AUDDeleteSound (sound->shield);
  }
  if (sound->armor!=-1) {
    AUDStopPlaying (sound->armor);
    AUDDeleteSound (sound->armor);
  }
  if (sound->hull!=-1) {
    AUDStopPlaying (sound->hull);
    AUDDeleteSound (sound->hull);
  }
  if (sound->cloak!=-1) {
    AUDStopPlaying (sound->cloak);
    AUDDeleteSound (sound->cloak);
  }
  for (int beamcount=0;beamcount<GetNumMounts();beamcount++) {
    AUDStopPlaying(mounts[beamcount].sound);
    AUDDeleteSound(mounts[beamcount].sound);
    if (mounts[beamcount].ref.gun&&mounts[beamcount].type->type==weapon_info::BEAM)
      delete mounts[beamcount].ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }
  for(unsigned int meshcount = 0; meshcount < meshdata.size(); meshcount++)
    if (meshdata[meshcount])
      delete meshdata[meshcount];
  meshdata.clear();
  
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
        TargetPoint = Transform(cumulative_transformation_matrix,meshdata[i]->Position());
	
	if (GFXSphereInFrustum (frustum, 
				TargetPoint,
				meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				*SizeScaleFactor
#endif
				)){
	  return true;
	}
  }	
  un_fkiter iter =SubUnits.constFastIterator();
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
  Matrix ctm=cumulative_transformation_matrix;
  if (planet) {
    Transformation ct (linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor));  
    ct.to_matrix (m);
    ctm=m;
  }
  Vector q (ctm.getQ());
  Vector r (ctm.getR());
  Vector tmp;
  CrossProduct(r,q, tmp);
  _Universe->AccessCamera(whichcam)->SetOrientation(tmp,q ,r);
  
  _Universe->AccessCamera(whichcam)->SetPosition (Transform (ctm,image->CockpitCenter.Cast()),GetVelocity(),GetAngularVelocity());
}
extern bool flickerDamage (Unit * un, float hullpercent);   
extern short cloakVal (short cloak, short cloakmin, short cloakrate, bool cloakglass);
template <class UnitType>
void GameUnit<UnitType>::DrawNow (const Matrix &mato, float lod) {
  unsigned int i;
  Matrix mat(mato);
  if (graphicOptions.FaceCamera){
	  Vector p,q,r;
	  QVector pos (mato.p);
	  float wid,hei;
	  CalculateOrientation(pos,p,q,r,wid,hei,0,false,&mat);
	  pos=mato.p;
	  VectorAndPositionToMatrix(mat,p,q,r,pos);
  }
  short cloak=cloaking;
  if (cloaking>cloakmin) {
    cloak = cloakVal (cloak,cloakmin,image->cloakrate, image->cloakglass);
  }
  for (i=0;(int)i<nummesh();i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
    if (meshdata[i]==NULL) 
      continue;
    Vector TransformedPosition = Transform (mat,
					    meshdata[i]->Position());
      float d = GFXSphereInFrustum(TransformedPosition,
				   meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
      if (d) {  //d can be used for level of detail 
	meshdata[i]->DrawNow(lod,false,mat,cloak);//cloakign and nebula
      }
    }
    un_fiter iter =SubUnits.fastIterator();
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
      haloalpha=((float)cloak)/32767;
    }
    float enginescale = GetVelocity().MagnitudeSquared();
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas=computer.max_ab_speed()*computer.max_ab_speed();
    if (cmas==0)
      cmas =1;
    if (enginescale>cmas)
      enginescale=cmas;
    Vector Scale (1,1,enginescale/(cmas));
#endif
    if (halos.ShouldDraw (enginescale)) 
      halos.Draw(mat,Scale,cloak,0, GetHullPercent(),GetVelocity(),faction);
}
template <class UnitType>
void GameUnit<UnitType>::Draw(const Transformation &parent, const Matrix &parentMatrix)
{

  cumulative_transformation = linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor);
  Matrix *ctm;
  Matrix invview;
  Transformation * ct;
  cumulative_transformation.Compose(parent, parentMatrix);
  ctm =&cumulative_transformation_matrix;
  ct = &cumulative_transformation;
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  if (graphicOptions.FaceCamera==1) {
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
  short cloak=cloaking;
  if (cloaking>cloakmin) {
    cloak = (short)(cloaking-interpolation_blend_factor*image->cloakrate*SIMULATION_ATOM);
    cloak = cloakVal ( cloak,cloakmin,image->cloakrate,image->cloakglass);
  }
  
  unsigned int i;
  if (hull <0) {
    Explode(true, GetElapsedTime());
  }
  float damagelevel=hull/maxhull;
  unsigned char chardamage=(char)(damagelevel*255);
  chardamage=255-chardamage;
  bool On_Screen=false;
  float minmeshradius = (_Universe->AccessCamera()->GetVelocity().Magnitude()+Velocity.Magnitude())*SIMULATION_ATOM;
  bool myparent = (this==_Universe->AccessCockpit()->GetParent());
  float numKeyFrames = graphicOptions.NumAnimationPoints;
  if ((!(invisible&INVISUNIT))&&((!(invisible&INVISCAMERA))||(!myparent))) {
    for (i=0;i<meshdata.size();i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
      if (meshdata[i]==NULL) 
		continue;
	  if ((int)i==nummesh()&&(meshdata[i]->numFX()==0||hull<0)) 
		continue;
	  if (meshdata[i]->getBlendDst()==ONE) {
		  if ((invisible&INVISGLOW)!=0)
			  continue;

		  if (damagelevel<.9)
			  if (flickerDamage (this,damagelevel))
				  continue;
	  }
	  QVector TransformedPosition = Transform (*ctm,
					      meshdata[i]->Position().Cast());
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
				   minmeshradius+meshdata[i]->clipRadialSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
      double lod;
      //      VSFileSystem::vs_fprintf (stderr,"\n");
      if (d) {  //d can be used for level of detail shit
	d = (TransformedPosition-_Universe->AccessCamera()->GetPosition()).Magnitude();
	if ((lod =g_game.detaillevel*g_game.x_resolution*2*meshdata[i]->rSize()/GFXGetZPerspective((d-meshdata[i]->rSize()<g_game.znear)?g_game.znear:d-meshdata[i]->rSize()))>=g_game.detaillevel) {//if the radius is at least half a pixel (detaillevel is the scalar... so you gotta make sure it's above that
		float currentFrame = meshdata[i]->getCurrentFrame();
		meshdata[i]->Draw(lod,this->WarpMatrix(*ctm),d,i==meshdata.size()-1?-1:cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0,chardamage);//cloakign and nebula		
		On_Screen=true;
		if (meshdata[i]->getFramesPerSecond()) {
			float currentprogress=floor(meshdata[i]->getCurrentFrame()*numKeyFrames/meshdata[i]->getNumLOD());
			if (numKeyFrames&&
				floor(currentFrame*numKeyFrames/meshdata[i]->getNumLOD())   !=
				currentprogress) {
				graphicOptions.Animating=0;
				meshdata[i]->setCurrentFrame(.1+currentprogress*meshdata[i]->getNumLOD()/numKeyFrames);
			}else if (!graphicOptions.Animating) {
				meshdata[i]->setCurrentFrame(currentFrame);//dont' budge
			}
		}		
	} else {

	}
      }
    }
    
    un_fiter iter =SubUnits.fastIterator();
    Unit * un;
    while ((un = iter.current())) {
      (un)->Draw (*ct,*ctm);
      iter.advance();
    }
  
    if(selected) {
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
  int nummounts= GetNumMounts();
  for (i=0;(int)i<nummounts;i++) {
    static bool draw_mounts = XMLSupport::parse_bool (vs_config->getVariable ("graphics","draw_weapons","false"));
	Mount * mahnt = &mounts[i];
    if (draw_mounts&&On_Screen) {
		
//      Mesh * gun = WeaponMeshCache::getCachedMutable (mounts[i]->type->weapon_name);
	 if (mahnt->xyscale!=0&&mahnt->zscale!=0) {		
      Mesh * gun = mahnt->type->gun;	  
      if (gun) {
		  Transformation mountLocation(mahnt->GetMountOrientation(),mahnt->GetMountLocation().Cast());
		  mountLocation.Compose (*ct,this->WarpMatrix(*ctm));
		  Matrix mat;
		  mountLocation.to_matrix(mat);
		  ScaleMatrix(mat,Vector(mahnt->xyscale,mahnt->xyscale,mahnt->zscale));
		  gun->setCurrentFrame(mounts[i].ComputeAnimatedFrame(gun));		  
		  gun->Draw(100,mat,1,cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula
		  if (mahnt->type->gun1){
			  gun = mahnt->type->gun1;
			  gun->setCurrentFrame(mounts[i].ComputeAnimatedFrame(gun));		  
			  gun->Draw(100,mat,1,cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0,chardamage,true);//cloakign and nebula			  
		  }
      }
	 }
	}
    if (mounts[i].type->type==weapon_info::BEAM) {
      if (mounts[i].ref.gun) {
	mounts[i].ref.gun->Draw(*ct,this->WarpMatrix(*ctm),((mounts[i].size&weapon_info::AUTOTRACKING)&&mounts[i].time_to_lock<=0)? Unit::Target():NULL,computer.radar.trackingcone);
      }
    }
  }
  float haloalpha=1;
  if (cloak>=0) {
    haloalpha=((float)cloak)/32767;
  }
  if (On_Screen) {
    float enginescale = GetVelocity().MagnitudeSquared();
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas=computer.max_ab_speed()*computer.max_ab_speed();
    if (cmas==0)
      cmas =1;
    if (enginescale>cmas)
      enginescale=cmas;
    Vector Scale (1,1,enginescale/(cmas));
#endif
    if (halos.ShouldDraw (enginescale)) 
      halos.Draw(this->WarpMatrix(*ctm),Scale,cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0,GetHull()>0?damagelevel:1.0,GetVelocity(),faction);
	int numm = nummesh();
	if (damagelevel<.99&&numm>0&&GetHull()>0) {
		unsigned int switcher=(damagelevel>.8)?1:
			(damagelevel>.6)?2:(damagelevel>.4)?3:(damagelevel>.2)?4:5;
		const unsigned long thus=(unsigned long)this;
		Mesh * tmp;
		switch (switcher) {
		case 5:
			tmp=meshdata[(thus+5)%(numm)];
			if (tmp)
				if (tmp->getBlendDst()==ONE)
					tmp=meshdata[(thus+5+17)%(numm)];
			LaunchOneParticle(*ctm,GetVelocity(),((long)this)+165,tmp,damagelevel,faction);
		case 4:
			tmp=meshdata[(thus+47)%(numm)];
			if (tmp)
				if (tmp->getBlendDst()==ONE)
					tmp=meshdata[(thus+47+17)%(numm)];			
			LaunchOneParticle(*ctm,GetVelocity(),((long)this)+47,tmp,damagelevel,faction);
		case 3:
			tmp=meshdata[(thus+61)%(numm)];
			if (tmp)
				if (tmp->getBlendDst()==ONE)
					tmp=meshdata[(thus+47+61)%(numm)];						
			LaunchOneParticle(*ctm,GetVelocity(),((long)this)+61,tmp,damagelevel,faction);
		case 2:
			tmp=meshdata[(thus+2)%(numm)];
			if (tmp)
				if (tmp->getBlendDst()==ONE)
					tmp=meshdata[(thus+1)%(numm)];						
			LaunchOneParticle(*ctm,GetVelocity(),((long)this)+65537,tmp,damagelevel,faction);			
		default:
			tmp=meshdata[(thus)%(numm)];
			if (tmp)
				if (tmp->getBlendDst()==ONE)
					tmp=meshdata[(thus+129)%(numm)];
			LaunchOneParticle(*ctm,GetVelocity(),((long)this)+257,tmp,damagelevel,faction);
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
    //halos[i]->GetDimensions (x,y);
    //halos[i]->SetDimensions (x/(1024),y/(1024));
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
