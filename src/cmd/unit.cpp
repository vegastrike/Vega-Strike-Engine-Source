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


#include "vs_path.h"
#include "vs_globals.h"
#include "file_main.h"
#include "gfx/halo.h"

//#include "unit.h"
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
	if (!BaseInterface::CurrentBase) {
		string basename = (::getCargoUnitName(baseun->getFullname().c_str()));
	  if (baseun->isUnit()!=PLANETPTR) {
	    basename = baseun->name;
	  }
	  BaseInterface *base=new BaseInterface (basename.c_str(),baseun,this);
	  base->InitCallbacks();
	  SetSoftwareMousePosition(0,0);
	}
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
		this->GameUnit::Init();
}

#define PARANOIA .4

template <class UnitType>
void GameUnit<UnitType>::calculate_extent(bool update_collide_queue) {  
  int a;
  corner_min=Vector (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max=Vector (-FLT_MAX,-FLT_MAX,-FLT_MAX);

  for(a=0; a<nummesh(); a++) {
    corner_min = corner_min.Min(meshdata[a]->corner_min());
    corner_max = corner_max.Max(meshdata[a]->corner_max());
  }/* have subunits now in table*/
  un_kiter iter =SubUnits.constIterator();
  const Unit * un;
  while ((un = iter.current())) {
    corner_min = corner_min.Min(un->LocalPosition().Cast()+un->corner_min);
    corner_max = corner_max.Max(un->LocalPosition().Cast()+un->corner_max);
    iter.advance();
  }

  if (corner_min.i==FLT_MAX||corner_max.i==-FLT_MAX||!FINITE (corner_min.i)||!FINITE(corner_max.i)) {
    radial_size=0;
    corner_min.Set (0,0,0);
    corner_max.Set (0,0,0);
  }else {
    float tmp1 = corner_min.Magnitude();
    float tmp2 = corner_max.Magnitude();
    radial_size = tmp1>tmp2?tmp1:tmp2;
    if (!SubUnit)
      image->selectionBox = new Box(corner_min, corner_max);
  }
  if (!SubUnit&&update_collide_queue) {
    UpdateCollideQueue();
  }
  if (isUnit()==PLANETPTR) {
    radial_size = corner_max.i;
  }
}

extern void UncheckUnit (Unit * un);
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
  /*
  yprrestricted=0;
  ymin = pmin = rmin = -PI;
  ymax = pmax = rmax = PI;
  ycur = pcur = rcur = 0;
  */
  static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
  //static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
  //static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics", "autotracking", ".93"));// DO NOT CHANGE see unit_customize.cpp
  //static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  //  Fire();
}

template <class UnitType>
Sprite * GameUnit<UnitType>::getHudImage () const{
	return image->hudImage;
}

template <class UnitType>
GameUnit<UnitType>::GameUnit<UnitType> (std::vector <Mesh *>& meshes, bool SubU, int fact) {
  Init ();
  this->faction = fact;
  SubUnit = SubU;
  meshdata = meshes;
  meshes.clear();
  meshdata.push_back(NULL);
  calculate_extent(false);
}
template <class UnitType>
vector <Mesh *> GameUnit<UnitType>::StealMeshes() {
  vector <Mesh *>ret;
  
  Mesh * shield = meshdata.empty()?NULL:meshdata.back();
  for (int i=0;i<nummesh();i++) {
    ret.push_back (meshdata[i]);
  }
  meshdata.clear();
  meshdata.push_back(shield);
  
  return ret;
}

extern void update_ani_cache();
template <class UnitType>
GameUnit<UnitType>::GameUnit<UnitType>(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber) {
	this->GameUnit::Init();
	update_ani_cache();
	//if (!SubU)
	//  _Universe->AccessCockpit()->savegame->AddUnitToSave(filename,UNITPTR,FactionUtil::GetFaction(faction),(long)this);
	SubUnit = SubU;
	this->faction = faction;
	SetFg (flightgrp,fg_subnumber);
	bool doubleup=false;
	char * my_directory=GetUnitDir(filename);
	vssetdir (GetSharedUnitPath().c_str());
	FILE * fp=NULL;
	if (!fp) {
	  const char *c;
	  if ((c=FactionUtil::GetFaction(faction)))
	    vschdir (c);
	  else
	    vschdir ("unknown");
	  doubleup=true;
	  vschdir (my_directory);
	} else {
	  fclose (fp);
	}
	if (filename[0])
    	    fp = fopen (filename,"r");
	if (!fp) {
	  vscdup();
	  vscdup();
	  doubleup=false;
	  vschdir (my_directory);
	  if (filename[0])
    	    fp = fopen (filename,"r");
	}

	if (!fp) {
	  if (doubleup) {
	    vscdup();
	  }
	  vscdup();
	  vschdir ("neutral");
	  faction=FactionUtil::GetFaction("neutral");//set it to neutral
	  doubleup=true;
	  vschdir (my_directory);
	  if (filename[0])
    	    fp = fopen (filename,"r");
	  if (fp) fclose (fp); 
	  else {
	    fprintf (stderr,"Warning: Cannot locate %s",filename);	  
	    meshdata.clear();
	    meshdata.push_back(NULL);
	    this->name=string("LOAD_FAILED");
	    //	    assert ("Unit Not Found"==NULL);
	  }
	}else {
	  fclose (fp);
	}
	free(my_directory);
	//Insert file loading stuff here
	if(1&&fp) {
	  name = filename;

	  Unit::LoadXML(filename,unitModifications.c_str());
	}
	if (1) {
	  calculate_extent(false);
	  ToggleWeapon(true);//change missiles to only fire 1
       	  vscdup();
	  if (doubleup) 
	    vscdup();
	  vsresetdir();
	  return;
	}
	LoadFile(filename);
	int nummesh;
	ReadInt(nummesh);
	meshdata.clear();
	for(int meshcount = 0; meshcount < nummesh; meshcount++)
	{
		int meshtype;
		ReadInt(meshtype);
		char meshfilename[64];
		float x,y,z;
		ReadMesh(meshfilename, x,y,z);
		meshdata.push_back(new Mesh(meshfilename, 1, faction,NULL));
		//		meshdata[meshcount]->SetPosition(Vector (x,y,z));
	}
	meshdata.push_back(NULL);
	int numsubunit;
	ReadInt(numsubunit);
	for(int unitcount = 0; unitcount < numsubunit; unitcount++)
	{
		char unitfilename[64];
		float x,y,z;
		int type;
		ReadUnit(unitfilename, type, x,y,z);
		Unit *un;
		switch(type)
		{
		default:
		  SubUnits.prepend (un=UnitFactory::createUnit (unitfilename,true,faction,unitModifications,flightgroup,flightgroup_subnumber));

		}
		un->SetPosition(QVector(x,y,z));
	}

	int restricted;
	float min, max;
	ReadInt(restricted); //turrets and weapons
	//ReadInt(restricted); // What's going on here? i hsould have 2, but that screws things up

	ReadRestriction(restricted, min, max);
	if(restricted) {
	  //RestrictYaw(min,max);
	}
	ReadRestriction(restricted, min, max);
	if(restricted) {
	  //RestrictPitch(min,max);
	}
	ReadRestriction(restricted, min, max);
	if(restricted) {
	  //RestrictRoll(min,max);
	}
	float maxspeed, maxaccel, mass;
	ReadFloat(maxspeed);
	ReadFloat(maxaccel);
	ReadFloat(mass);


	CloseFile();
	calculate_extent(false);
	ToggleWeapon(true);//change missiles to only fire 1
	vscdup();
	if (fp) 
	  vscdup();
	vsresetdir();

}
template <class UnitType>
GameUnit<UnitType>::~GameUnit<UnitType>()
{
  if (image->hudImage )
    delete image->hudImage;
  if (planet)
    delete planet;
  //  fprintf (stderr,"Freeing Unit %s\n",name.c_str());
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
    AUDStopPlaying(mounts[beamcount]->sound);
    AUDDeleteSound(mounts[beamcount]->sound);
    if (mounts[beamcount]->ref.gun&&mounts[beamcount]->type->type==weapon_info::BEAM)
      delete mounts[beamcount]->ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }
  for(unsigned int meshcount = 0; meshcount < meshdata.size(); meshcount++)
    if (meshdata[meshcount])
      delete meshdata[meshcount];
  meshdata.clear();
  
}
template <class UnitType>
StarSystem * GameUnit<UnitType>::getStarSystem () {

  if (activeStarSystem) {
    return activeStarSystem;
  }else {
    Cockpit * cp=_Universe->isPlayerStarship(this);
    if (cp) {
      if (cp->activeStarSystem)
	return cp->activeStarSystem;
    }
  }
  return _Universe->activeStarSystem();
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
  
  _Universe->AccessCamera(whichcam)->SetPosition (Transform (ctm,image->CockpitCenter.Cast()));
}
   
extern short cloakVal (short cloak, short cloakmin, short cloakrate, bool cloakglass);
template <class UnitType>
void GameUnit<UnitType>::DrawNow (const Matrix & mat, float lod) {
  unsigned int i;
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
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas=computer.max_ab_speed()*computer.max_ab_speed();
    if (cmas==0)
      cmas =1;
    Vector Scale (1,1,GetVelocity().MagnitudeSquared()/(cmas));
#endif
    halos.Draw(mat,Scale,cloak,0, GetHullPercent(),GetVelocity(),faction);
}
template <class UnitType>
void GameUnit<UnitType>::Draw(const Transformation &parent, const Matrix &parentMatrix)
{

  cumulative_transformation = linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor);
  Matrix *ctm;
  Transformation * ct;
  cumulative_transformation.Compose(parent, parentMatrix);
  ctm =&cumulative_transformation_matrix;
  ct = &cumulative_transformation;
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
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
  bool On_Screen=false;
  if (!invisible||(this!=_Universe->AccessCockpit()->GetParent())) {
    for (i=0;i<meshdata.size();i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
      if (meshdata[i]==NULL) 
		continue;
	  if ((int)i==nummesh()&&(meshdata[i]->numFX()==0||hull<0)) 
		continue;
	  QVector TransformedPosition = Transform (*ctm,
					      meshdata[i]->Position().Cast());
#if 0
      //This is a test of the box in frustum setup to be used with terrain
      GFXBoxInFrustumModel (ctm);
      int tmp = GFXBoxInFrustum (meshdata[i]->corner_min(),meshdata[i]->corner_max());
      if ((d==0)!=(tmp==0)) {
	fprintf (stderr,"Mismatch for %s with Box being %d", name.c_str(),tmp);
      }
#endif
      //      fprintf (stderr,"%s %d ",name.c_str(),i);
      float d = GFXSphereInFrustum(TransformedPosition,
				   meshdata[i]->clipRadialSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
      float lod;
      //      fprintf (stderr,"\n");
      if (d) {  //d can be used for level of detail shit
	d = (TransformedPosition-_Universe->AccessCamera()->GetPosition().Cast()).Magnitude();
	if ((lod =g_game.detaillevel*g_game.x_resolution*2*meshdata[i]->rSize()/GFXGetZPerspective((d-meshdata[i]->rSize()<g_game.znear)?g_game.znear:d-meshdata[i]->rSize()))>=g_game.detaillevel) {//if the radius is at least half a pixel (detaillevel is the scalar... so you gotta make sure it's above that
	  meshdata[i]->Draw(lod,*ctm,d,cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0);//cloakign and nebula
	  On_Screen=true;
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
        fprintf (stderr,"%s: <%f d: %f\n", curun->name.c_str(), tmpf, tmpdis);

      }
      tmpiter->advance();
    }
    delete tmpiter;
    **/
  }

  for (i=0;(int)i<GetNumMounts();i++) {
    static bool draw_mounts = XMLSupport::parse_bool (vs_config->getVariable ("graphics","draw_weapons","false"));

    if (draw_mounts&&On_Screen) {
      Mesh * gun = WeaponMeshCache::getCachedMutable (mounts[i]->type->weapon_name);
      if (gun) {
	Transformation mountLocation=mounts[i]->GetMountLocation();
	mountLocation.Compose (*ct,*ctm);
	Matrix mat;
	mountLocation.to_matrix(mat);
	gun->Draw(100,mat,1,cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0);//cloakign and nebula
		  
      }
    }
    if (mounts[i]->type->type==weapon_info::BEAM) {
      if (mounts[i]->ref.gun) {
	mounts[i]->ref.gun->Draw(*ct,*ctm,((mounts[i]->size&weapon_info::AUTOTRACKING)&&mounts[i]->time_to_lock<=0)? Unit::Target():NULL,computer.radar.trackingcone);
      }
    }
  }
  float haloalpha=1;
  if (cloak>=0) {
    haloalpha=((float)cloak)/32767;
  }
  if (On_Screen) {
#ifdef CAR_SIM
    Vector Scale (1,image->ecm,computer.set_speed);
#else
    float cmas = computer.max_ab_speed()*computer.max_ab_speed();
    if (cmas==0)
      cmas=1;
    Vector Scale (1,1,GetVelocity().MagnitudeSquared()/cmas);
#endif
    halos.Draw(*ctm,Scale,cloak,(_Universe->AccessCamera()->GetNebula()==nebula&&nebula!=NULL)?-1:0,GetHullPercent(),GetVelocity(),faction);
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
// MAYBE MOVE THAT TO UNIT TOO
// BUT USES GETMINDIS and GETMINDIS USES meshdata which is not member of Unit for now
template <class UnitType>
void GameUnit<UnitType>::scanSystem(){

  double nowtime=mission->getGametime();

  if(scanner.last_scantime==nowtime){
    return;
  }

    StarSystem *ssystem=_Universe->activeStarSystem();
    un_iter uiter(ssystem->getUnitList().createIterator());
    
    float min_enemy_dist=9999999.0;
    float min_friend_dist=9999999.0;
    float min_ship_dist=9999999.0;
    Unit * min_enemy=NULL;
    Unit * min_friend=NULL;
    Unit * min_ship=NULL;
    
    int leader_num=getFgSubnumber(); //my own subnumber
    Unit *my_leader=this; // say I'm the leader
    Flightgroup *my_fg=getFlightgroup();
    
    Unit *unit=uiter.current();
    while(unit!=NULL){
      
      if(this!=unit){
	// won;t scan ourselves
	
	QVector unit_pos=unit->Position();
	double dist=getMinDis(unit_pos);
	float relation=getRelation(unit);
	
	if(relation<0.0){
	  //we are enmies
	  if(dist<min_enemy_dist){
	    min_enemy_dist=dist;
	    min_enemy=unit;
	  }
	}
	if(relation>0.0){
	//we are friends
	  if(dist<min_friend_dist){
	    min_friend_dist=dist;
	  min_friend=unit;
	  }
	  // check for flightgroup leader
	  if(my_fg!=NULL && my_fg==unit->getFlightgroup()){
	    // it's a ship from our flightgroup
	    int fgnum=unit->getFgSubnumber();
	    if(fgnum<leader_num){
	      //set this to be our leader
	      my_leader=unit;
	      leader_num=fgnum;
	    }
	  }
	}
	// for all ships
	if(dist<min_ship_dist){
	  min_ship_dist=dist;
	  min_ship=unit;
	}
      }
      
      unit=++(uiter);
  }
    
    scanner.nearest_enemy_dist=min_enemy_dist;
    scanner.nearest_enemy=min_enemy;
    
    scanner.nearest_friend_dist=min_friend_dist;
    scanner.nearest_friend=min_friend;

    scanner.nearest_ship_dist=min_ship_dist;
    scanner.nearest_ship=min_ship;
    
    scanner.leader=my_leader;
}
