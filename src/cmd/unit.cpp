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

#include "unit.h"
#include "unit_factory.h"

#include "gfx/sprite.h"
#include "lin_time.h"


#include "ai/navigation.h"
#include "ai/fire.h"
#include "ai/script.h"
#include "ai/flybywire.h"
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
//if the PQR of the unit may be variable...for radius size computation
//#define VARIABLE_LENGTH_PQR

//#define DESTRUCTDEBUG
#include "beam.h"


double interpolation_blend_factor;


#define PARANOIA .4
void Unit::Threaten (Unit * targ, float danger) {
  if (!targ) {
    computer.threatlevel=danger;
    computer.threat.SetUnit (NULL);
  }else {
    if (targ->owner!=this&&this->owner!=targ&&danger>PARANOIA&&danger>computer.threatlevel) {
      computer.threat.SetUnit(targ);
      computer.threatlevel = danger;
    }
  }
}

void Unit::calculate_extent(bool update_collide_queue) {  
  int a;
  corner_min=Vector (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max=Vector (-FLT_MAX,-FLT_MAX,-FLT_MAX);

  for(a=0; a<nummesh; a++) {
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

//FIXME Daughter units should be able to be turrets (have y/p/r)
void Unit::SetResolveForces (bool ys) {
  resolveforces = ys;
  /*
  for (int i=0;i<numsubunit;i++) {
    subunits[i]->SetResolveForces (ys);
  }
  */
}


void Unit::ActivateJumpDrive (int destination) {
  const int jumpfuelratio=1;

  if (((docked&(DOCKED|DOCKED_INSIDE))==0)&&jump.drive!=-2&&(energy>jump.energy&&(jump.energy>=0||fuel>-jump.energy))) {
    jump.drive = destination;
    if (jump.energy>0)
      energy-=jump.energy;
    else
      fuel += jump.energy*mass*jumpfuelratio;
  }
}
void Unit::DeactivateJumpDrive () {
  if (jump.drive>=0) {
    jump.drive=-1;
  }
}
float capship_size=500;
void Unit::Init()
{
  static float capsize = XMLSupport::parse_float(vs_config->getVariable("physics","capship_size","500"));
  capship_size=capsize;
  activeStarSystem=NULL;
  xml=NULL;
  docked=NOT_DOCKED;
  SubUnit =0;
  jump.energy = 100;
  jump.delay=5;
  jump.damage=0;
  jump.drive=-2;// disabled
  afterburnenergy=0;
  planet=NULL;
  nebula=NULL;
  image = new UnitImages;
 int numg= 1+MAXVDUS+Cockpit::NUMGAUGES;
  image->cockpit_damage=(float*)malloc((numg)*sizeof(float));
  for (unsigned int damageiterator=0;damageiterator<numg;damageiterator++) {
	image->cockpit_damage[damageiterator]=1;
  }
  sound = new UnitSounds;
  limits.structurelimits=Vector(0,0,1);
  limits.limitmin=-1;
  cloaking=-1;
  image->repair_droid=0;
  image->ecm=0;
  image->cloakglass=false;
  image->cargo_volume=0;
  image->unitwriter=NULL;
  cloakmin=image->cloakglass?1:0;
  image->cloakrate=100;
  image->cloakenergy=0;
  image->forcejump=false;
  sound->engine=-1;  sound->armor=-1;  sound->shield=-1;  sound->hull=-1; sound->explode=-1;
  sound->cloak=-1;
  image->hudImage=NULL;
  owner = NULL;
  faction =0;
  resolveforces=true;
  CollideInfo.object.u = NULL;
  CollideInfo.type = LineCollide::UNIT;
  CollideInfo.Mini.Set (0,0,0);
  CollideInfo.Maxi.Set (0,0,0);
  colTrees=NULL;
  invisible=false;
  //origin.Set(0,0,0);
  corner_min.Set (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max.Set (-FLT_MAX,-FLT_MAX,-FLT_MAX);
  
  shieldtight=0;//sphere mesh by default
  energy=maxenergy=1;
  recharge = 1;
  shield.recharge=shield.leak=0;
  shield.fb[0]=shield.fb[1]=shield.fb[2]=shield.fb[3]=armor.front=armor.back=armor.right=armor.left=0;
  hull=10;
  shield.number=2;
  
  image->explosion=NULL;
  image->timeexplode=0;
  killed=false;
  ucref=0;
  nummounts= nummesh=0;
  mounts = NULL;meshdata = NULL;
  aistate = NULL;
  SetAI (new Order());
  Identity(cumulative_transformation_matrix);
  cumulative_transformation = identity_transformation;
  curr_physical_state = prev_physical_state = identity_transformation;
  mass = .01;
  fuel = 000;

  /*
  yprrestricted=0;
  ymin = pmin = rmin = -PI;
  ymax = pmax = rmax = PI;
  ycur = pcur = rcur = 0;
  */
  MomentOfInertia = .01;
  static Vector myang(XMLSupport::parse_float (vs_config->getVariable ("general","pitch","0")),XMLSupport::parse_float (vs_config->getVariable ("general","yaw","0")),XMLSupport::parse_float (vs_config->getVariable ("general","roll","0")));
  AngularVelocity = myang;
  Velocity = Vector(0,0,0);
  
  NetTorque =NetLocalTorque = Vector(0,0,0);
  NetForce = Vector(0,0,0);
  NetLocalForce=Vector(0,0,0);

  selected = false;
  image->selectionBox = NULL;

  limits.yaw = 2.55;
  limits.pitch = 2.55;
  limits.roll = 2.55;
	
  limits.lateral = 2;
  limits.vertical = 8;
  limits.forward = 2;
  limits.afterburn=5;
  limits.retro=2;
  Target(NULL);
  VelocityReference(NULL);
  computer.threat.SetUnit (NULL);
  computer.slide_start=computer.slide_end=0;
  computer.set_speed=0;
  computer.max_speed=1;
  computer.max_ab_speed=1;
  computer.max_yaw=1;
  computer.max_pitch=1;
  computer.max_roll=1;
  computer.NavPoint=Vector(0,0,0);
  computer.itts = false;
  static float rr = XMLSupport::parse_float (vs_config->getVariable ("graphics","hud","radarRange","20000"));
  computer.radar.maxrange=rr;
  computer.radar.maxcone=-1;
  static float minTrackingNum = XMLSupport::parse_float (vs_config->getVariable("physics",
										  "autotracking",
										".93"));// DO NOT CHANGE see unit_customize.cpp
    
  computer.radar.trackingcone = minTrackingNum;
  static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  computer.radar.lockcone=lc;
  computer.radar.mintargetsize=0;
  computer.radar.color=true;
  //  Fire();

  flightgroup=NULL;
  flightgroup_subnumber=0;

  scanner.last_scantime=0.0;
}

void Unit::SetVisible(bool invis) {
  invisible=!invis;
}

Unit::Unit( int /*dummy*/ ) {
	Init();
}
Unit::Unit() {
	Init();
}
Sprite * Unit::getHudImage () const{
	return image->hudImage;
}
std::string Unit::getCockpit () const{
	return image->cockpitImage;
}


Unit::Unit (Mesh ** meshes, int num, bool SubU, int faction) {
  Init ();
  this->faction = faction;
  SubUnit = SubU;
  meshdata = (Mesh **)malloc ((1+num)*sizeof (Mesh *));
  memcpy (meshdata,meshes,(num)*sizeof (Mesh *));
  nummesh = num;
  meshdata[nummesh]=NULL;//turn off shield
  calculate_extent(false);
}
char * GetUnitDir (const char * filename) {
  char * retval=strdup (filename);
  if (retval[0]=='\0')
    return retval;
  if (retval[1]=='\0')
    return retval;
  for (int i=0;retval[i]!=0;i++) {
    if (retval[i]=='.') {
      retval[i]='\0';
      break;
    }
  }
  return retval;
}
void Unit::GetCargoCat (const std::string &cat, vector <Cargo> &cats) {
  unsigned int max = numCargo();
  for (unsigned int i=0;i<max;i++) {
    if (GetCargo(i).category==cat) {
      cats.push_back (GetCargo(i));
    }
  }
}
vector <Mesh *> Unit::StealMeshes() {
  vector <Mesh *>ret;
  Mesh * shield = meshdata[nummesh];
  for (int i=0;i<nummesh;i++) {
    ret.push_back (meshdata[i]);
  }
  delete []meshdata;
  meshdata = new Mesh *[1];
  meshdata[0]= shield;
  nummesh=0;
  return ret;
}
#if 0
Unit * _1800GetGod () {
  return UnitFactory::get_static_1800God( );
}
#endif

void Unit::SetFg(Flightgroup * fg, int fg_subnumber) {
  flightgroup=fg;
  flightgroup_subnumber=fg_subnumber;
}
extern void update_ani_cache();
Unit::Unit(const char *filename, bool SubU, int faction,std::string unitModifications, Flightgroup *flightgrp,int fg_subnumber) {


	Init();
	update_ani_cache();
	if (!SubU)
	  _Universe->AccessCockpit()->savegame->AddUnitToSave(filename,UNITPTR,_Universe->GetFaction(faction),(int)this);
	SubUnit = SubU;
	this->faction = faction;
	SetFg (flightgrp,fg_subnumber);
	bool doubleup=false;
	char * my_directory=GetUnitDir(filename);
	vssetdir (GetSharedUnitPath().c_str());
	FILE * fp=NULL;
	if (!fp) {
	  const char *c;
	  if ((c=_Universe->GetFaction(faction)))
	    vschdir (c);
	  else
	    vschdir ("unknown");
	  doubleup=true;
	  vschdir (my_directory);
	} else {
	  fclose (fp);
	}
	fp = fopen (filename,"r");
	if (!fp) {
	  vscdup();
	  vscdup();
	  doubleup=false;
	  vschdir (my_directory);
	  fp = fopen (filename,"r");
	}

	if (!fp) {
	  if (doubleup) {
	    vscdup();
	  }
	  vscdup();
	  vschdir ("neutral");
	  faction=_Universe->GetFaction("neutral");//set it to neutral
	  doubleup=true;
	  vschdir (my_directory);
	  fp = fopen (filename,"r");
	  if (fp) fclose (fp); 
	  else {
	    fprintf (stderr,"Warning: Cannot locate %s",filename);	  
	    meshdata = new Mesh * [1];
	    meshdata[0]=NULL;
	    nummesh=0;
	    this->name=string("LOAD_FAILED");
	    //	    assert ("Unit Not Found"==NULL);
	  }
	}else {
	  fclose (fp);
	}
	free(my_directory);
	/*Insert file loading stuff here*/
	if(1&&fp) {
	  name = filename;

	  LoadXML(filename,unitModifications.c_str());
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
	ReadInt(nummesh);
	meshdata = new Mesh*[nummesh+1];
	meshdata[nummesh]=0;
	for(int meshcount = 0; meshcount < nummesh; meshcount++)
	{
		int meshtype;
		ReadInt(meshtype);
		char meshfilename[64];
		float x,y,z;
		ReadMesh(meshfilename, x,y,z);

		meshdata[meshcount] = new Mesh(meshfilename, 1, faction,NULL);

		//		meshdata[meshcount]->SetPosition(Vector (x,y,z));
	}
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
const std::vector <char *>& Unit::GetDestinations () const{
  return image->destination;
}
void Unit::AddDestination (const char * dest) {
  image->destination.push_back (strdup (dest));
}

Unit::~Unit()
{
	free(image->cockpit_damage);
  if ((!killed)) {
    fprintf (stderr,"Assumed exit on unit %s(if not quitting, report error)\n",name.c_str());
  }
  if (ucref) {
    fprintf (stderr,"DISASTER AREA!!!!");
  }
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"stage %d %x %d\n", 0,this,ucref);
  fflush (stderr);
#endif
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
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 1,planet);
  fflush (stderr);
#endif
  if (planet)
    delete planet;
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x\n", 2,image->hudImage);
  fflush (stderr);
#endif
  if (image->hudImage )
    delete image->hudImage;
  if (image->unitwriter)
    delete image->unitwriter;
  unsigned int i;
  for (i=0;i<image->destination.size();i++) {
    delete [] image->destination[i];
  }

#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x", 3,image);
  fflush (stderr);
#endif
  delete image;
  delete sound;
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x %x", 4,bspTree, bspShield);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d", 5);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x", 6,mounts);
  fflush (stderr);
#endif
  for (int beamcount=0;beamcount<nummounts;beamcount++) {
    AUDStopPlaying(mounts[beamcount].sound);
    AUDDeleteSound(mounts[beamcount].sound);
    if (mounts[beamcount].ref.gun&&mounts[beamcount].type->type==weapon_info::BEAM)
      delete mounts[beamcount].ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 7,meshdata);
  fflush (stderr);
#endif
  if(meshdata&&nummesh>0) {
    for(int meshcount = 0; meshcount < nummesh; meshcount++)
      delete meshdata[meshcount];
    delete [] meshdata;
  }

#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 9, halos);
  fflush (stderr);
#endif
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d %x ", 1,mounts);
  fflush (stderr);
#endif
  if (mounts) {
    delete []mounts;
  }
#ifdef DESTRUCTDEBUG
  fprintf (stderr,"%d", 0);
  fflush (stderr);
#endif
}
void Unit::getAverageGunSpeed(float & speed, float &range) const {
   if (nummounts) {
     int nummt = nummounts;
     for (int i=0;i<nummounts;i++) {
       if (mounts[i].type->type!=weapon_info::PROJECTILE) {
	 range+=mounts[i].type->Range;
	 speed+=mounts[i].type->Speed;
       } else {
	 nummt--;
       }
     }
     if (nummt) {
       range/=nummt;
       speed/=nummt;
     }
   }
  
}

float Unit::TrackingGuns(bool &missilelock) {
  float trackingcone = 0;
  missilelock=false;
  for (int i=0;i<nummounts;i++) {
    if (mounts[i].status==Mount::ACTIVE&&(mounts[i].size&weapon_info::AUTOTRACKING)) {
      trackingcone= computer.radar.trackingcone;
    }
    if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime>0&&mounts[i].time_to_lock<=0) {
      missilelock=true;
    }
  }
  return trackingcone;
}
int Unit::LockMissile() {
  bool missilelock=false;
  bool dumblock=false;
  for (int i=0;i<nummounts;i++) {
    if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime>0&&mounts[i].time_to_lock<=0&&mounts[i].type->type==weapon_info::PROJECTILE) {
      missilelock=true;
    }else {
      if (mounts[i].status==Mount::ACTIVE&&mounts[i].type->LockTime==0&&mounts[i].type->type==weapon_info::PROJECTILE&&mounts[i].time_to_lock<=0) {
	dumblock=true;
      }
    }    
  }
  return (missilelock?1:(dumblock?-1:0));
}
QVector Unit::PositionITTS (const QVector & posit, float speed) const{
  QVector retval = Position()-posit;
  speed = retval.Magnitude()/speed;//FIXME DIV/0 POSSIBLE
  retval = Position()+Velocity.Cast().Scale(speed);
  return retval;
}
float Unit::cosAngleTo (Unit * targ, float &dist, float speed, float range) const{
  Vector Normal (cumulative_transformation_matrix.getR());
   //   if (range!=FLT_MAX) {
   //     getAverageGunSpeed(speed,range);
   //   }
   QVector totarget (targ->PositionITTS(cumulative_transformation.position, speed+((targ->Position()-Position()).Normalize().Dot (Velocity.Cast()))));
   totarget = totarget-cumulative_transformation.position;
   double tmpcos = Normal.Cast().Dot (totarget);
   dist = totarget.Magnitude();
   if (tmpcos>0) {
      tmpcos = dist*dist - tmpcos*tmpcos;
      tmpcos = targ->rSize()/sqrtf(tmpcos);//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
   } else {
     tmpcos /= dist;
   }
   float rsize = targ->rSize()+rSize();
   if ((!targ->GetDestinations().empty()&&jump.drive>=0)||(targ->faction==faction)) {
     rsize=0;//HACK so missions work well
   }
   dist = (dist-rsize)/range;//WARNING POTENTIAL DIV/0
   if (!FINITE(dist)||dist<0) {
     dist=0;
   }
   return tmpcos;
}
float Unit::cosAngleFromMountTo (Unit * targ, float & dist) const{
  float retval = -1;
  dist = FLT_MAX;
  float tmpcos;
  Matrix mat;
  for (int i=0;i<nummounts;i++) {
    float tmpdist = .001;
    Transformation finaltrans (mounts[i].GetMountLocation());
    finaltrans.Compose (cumulative_transformation, cumulative_transformation_matrix);
    finaltrans.to_matrix (mat);
    Vector Normal (mat.getR());
    
    QVector totarget (targ->PositionITTS(finaltrans.position, mounts[i].type->Speed));
    
    tmpcos = Normal.Dot (totarget.Cast());
    tmpdist = totarget.Magnitude();
    if (tmpcos>0) {
      tmpcos = tmpdist*tmpdist - tmpcos*tmpcos;
      tmpcos = targ->rSize()/tmpcos;//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
    } else {
      tmpcos /= tmpdist;
    }
    tmpdist /= mounts[i].type->Range;//UNLIKELY DIV/0
    if (tmpdist < 1||tmpdist<dist) {
      if (tmpcos-tmpdist/2 > retval-dist/2) {
	dist = tmpdist;
	retval = tmpcos;
      }      
    }
  }
  return retval;
}





bool Unit::queryFrustum(float frustum [6][4]) const{
  int i;
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (cumulative_transformation_matrix[0],cumulative_transformation_matrix[1],cumulative_transformation_matrix[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif
  for (i=0;i<nummesh;i++) {
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
    if (un->queryFrustum (frustum)) {
      return true;
    }
    iter.advance();
  }
  return false;
}


float Unit::GetElasticity() {return .5;}
void Unit::UpdateHudMatrix(int whichcam) {
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
   
void Unit::SetFaction (int faction) {
  this->faction=faction;
  for (un_iter ui=getSubUnits();(*ui)!=NULL;++ui) {
    (*ui)->SetFaction(faction);
  }
}
void Unit::SetPlanetHackTransformation (Transformation *&ct,Matrix *&ctm) {
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
void Unit::Draw(const Transformation &parent, const Matrix &parentMatrix)
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
  AUDAdjustSound (sound.engine,cumulative_transformation.position,Velocity);
#endif
  short cloak=cloaking;
  if (cloaking>cloakmin) {
    cloak = (short)(cloaking-interpolation_blend_factor*image->cloakrate*SIMULATION_ATOM);
    if (cloak<0&&image->cloakrate<0) {
      cloak=(unsigned short)32768;//intended warning should be -32768 :-) leave it be
    }
    if ((cloak&0x1)&&!image->cloakglass) {
      cloak-=1;
    }
    if ((cloak&0x1)==0&&image->cloakglass) {
      cloak+=1;
    }
    if (cloak<cloakmin&&image->cloakrate>0)
      cloak=cloakmin;


  }
  
  int i;
  if (hull <0) {
    Explode(true, GetElapsedTime());
  }
  bool On_Screen=false;
  if (!invisible||(this!=_Universe->AccessCockpit()->GetParent())) {
    for (i=0;i<=nummesh;i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
      if (meshdata[i]==NULL) 
		continue;
	  if (i==nummesh&&(meshdata[i]->numFX()==0||hull<0)) 
		continue;
      Vector TransformedPosition = Transform (*ctm,
					      meshdata[i]->Position());
#if 0
      //This is a test of the box in frustum setup to be used with terrain
      GFXBoxInFrustumModel (ctm);
      int tmp = GFXBoxInFrustum (meshdata[i]->corner_min(),meshdata[i]->corner_max());
      if ((d==0)!=(tmp==0)) {
	fprintf (stderr,"Mismatch for %s with Box being %d", name.c_str(),tmp);
      }
#endif

      float d = GFXSphereInFrustum(TransformedPosition,
				   meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
      float lod;

      if (d) {  //d can be used for level of detail shit
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
      un->Draw (*ct,*ctm);
      iter.advance();
    }
  
    if(selected) {
      static bool doInputDFA=XMLSupport::parse_bool (vs_config->getVariable ("graphics","MouseCursor","false"));
      if (doInputDFA)
	image->selectionBox->Draw(g_game.x_resolution,*ctm);
    }
  } else {
	  _Universe->AccessCockpit()->SetupViewPort();///this is the final, smoothly calculated cam
    //        UpdateHudMatrix();
    /***DEBUGGING cosAngleFromMountTo
    UnitCollection *dL = _Universe->activeStarSystem()->getUnitList();
    UnitCollection::UnitIterator *tmpiter = dL->createIterator();
    Unit * curun;
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
  for (i=0;i<nummounts;i++) {
    if (mounts[i].type->type==weapon_info::BEAM) {
      if (mounts[i].ref.gun) {
	mounts[i].ref.gun->Draw(*ct,*ctm);
      }
    }
  }
  float haloalpha=1;
  if (cloak>=0) {
    haloalpha=((float)cloak)/32767;
  }
  if (On_Screen) {
    Vector Scale (1,1,Velocity.MagnitudeSquared()/(computer.max_ab_speed*computer.max_ab_speed));
    halos.Draw(*ctm,Scale,haloalpha);
  }
}
void Unit::PrimeOrders () {
  if (aistate) {
    delete aistate;
  }
  aistate = new Order; //get 'er ready for enqueueing
  aistate->SetParent (this);
}
#if 0
void Unit::SwapOutHalos() {
  for (int i=0;i<numhalos;i++) {
    // float x,y;
    //halos[i]->GetDimensions (x,y);
    //halos[i]->SetDimensions (x/(1024),y/(1024));
    halos[i]->Draw (cumulative_transformation,cumulative_transformation_matrix,0);
  }
}
void Unit::SwapInHalos() {
  for (int i=0;i<numhalos;i++) {
    // float x,y;
    //halos[i]->GetDimensions (x,y);
    //halos[i]->SetDimensions (x*(1024),y*(1024));
  }
}
#endif
void Unit::SetTurretAI () {
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit * un;
  while (NULL!=(un=iter.current())) {
    un->EnqueueAIFirst (new Orders::FireAt(.2,15));
    un->EnqueueAIFirst (new Orders::FaceTarget (false,3));
    un->SetTurretAI ();
    iter.advance();
  }
}
void Unit::DisableTurretAI () {
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit * un;
  while (NULL!=(un=iter.current())) {
    if (un->aistate) {
      delete un->aistate;
    }
    un->aistate = new Order; //get 'er ready for enqueueing
    un->aistate->SetParent (this);
    un->DisableTurretAI ();
    iter.advance();
  }
}

void Unit::SetAI(Order *newAI)
{
  newAI->SetParent(this);
  if (aistate) {
    aistate->ReplaceOrder (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::EnqueueAI(Order *newAI) {
  newAI->SetParent(this);
  if (aistate) {
    aistate->EnqueueOrder (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::EnqueueAIFirst(Order *newAI) {
  newAI->SetParent(this);
  if (aistate) {
    aistate->EnqueueOrderFirst (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::ExecuteAI() {
  if (flightgroup) {
      Unit * leader = flightgroup->leader.GetUnit();
      if (leader?(flightgroup->leader_decision>-1)&&(leader->getFgSubnumber()>=getFgSubnumber()):true) {//no heirarchy in flight group
	if (!leader) {
	  flightgroup->leader_decision = flightgroup->nr_ships;
	}
	flightgroup->leader.SetUnit(this);
      }
      flightgroup->leader_decision--;
  
  }
  if(aistate) aistate->Execute();
  if (!SubUnits.empty()) {
    un_iter iter =getSubUnits();
    Unit * un;
    while ((un = iter.current())) {
      un->ExecuteAI();//like dubya
      iter.advance();
    }
  }
}
void Unit::Select() {
  selected = true;
}
void Unit::Deselect() {
  selected = false;
}

un_iter Unit::getSubUnits () {
  return SubUnits.createIterator();
}
un_kiter Unit::viewSubUnits() const{
  return SubUnits.constIterator();
}

const string Unit::getFgID()  {
    if(flightgroup!=NULL){
      char buffer[200];
      sprintf(buffer,"-%d",flightgroup_subnumber);
      return flightgroup->name+buffer;
    }
    else{
      return fullname;
    }
};

string Unit::getFullAIDescription(){
  return getFgID()+":"+getAIState()->createFullOrderDescription(0).c_str();
}

void Unit::setTargetFg(string primary,string secondary,string tertiary){
  target_fgid[0]=primary;
  target_fgid[1]=secondary;
  target_fgid[2]=tertiary;

  ReTargetFg(0);
}

void Unit::ReTargetFg(int which_target){
#if 0
      StarSystem *ssystem=_Universe->activeStarSystem();
      UnitCollection *unitlist=ssystem->getUnitList();
      Iterator uiter=unitlist->createIterator();

      Unit *other_unit=uiter.current();
      Unit *found_target=NULL;
      int found_attackers=1000;

      while(other_unit!=NULL){
	string other_fgid=other_unit->getFgID();
	if(other_unit->matchesFg(target_fgid[which_target])){
	  // the other unit matches our primary target

	  int num_attackers=other_unit->getNumAttackers();
	  if(num_attackers<found_attackers){
	    // there's less ships attacking this target than the previous one
	    found_target=other_unit;
	    found_attackers=num_attackers;
	    setTarget(found_target);
	  }
	}

	other_unit=uiter.advance();
      }

      if(found_target==NULL){
	// we haven't found a target yet, search again
	if(which_target<=1){
	  ReTargetFg(which_target+1);
	}
	else{
	  // we can't find any target
	  setTarget(NULL);
	}
      }
#endif
}

void Unit::scanSystem(){

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
