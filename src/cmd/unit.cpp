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
#include "vs_globals.h"
#include "vs_path.h"
#include "file_main.h"
#include "gfx/halo.h"
#include "beam.h"
#include "unit.h"

#include "gfx/sprite.h"
#include "lin_time.h"


#include "ai/order.h"
#include "ai/script.h"
#include "ai/flybywire.h"
#include "gfx/box.h"
#include "bolt.h"
#include "gfx/lerp.h"
#include "gfx/bsp.h"
#include "audiolib.h"
#include "gfx/cockpit.h"
#include "config_xml.h"
#include "images.h"
#include "gfx/planetary_transform.h"

#include "script/mission.h"
#include "collide/rapcol.h"
//if the PQR of the unit may be variable...for radius size computation
//#define VARIABLE_LENGTH_PQR





double interpolation_blend_factor;


#define PARANOIA .4
void Unit::Threaten (Unit * targ, float danger) {
  if (danger>PARANOIA&&danger>computer.threatlevel) {
    computer.threat.SetUnit(targ);
    computer.threatlevel = danger;
  }

}

void Unit::calculate_extent() {  
  int a;
  for(a=0; a<nummesh; a++) {
    corner_min = corner_min.Min(meshdata[a]->corner_min());
    corner_max = corner_max.Max(meshdata[a]->corner_max());
  }/* have subunits now in table
  for(a=0; a<numsubunit; a++) {
    corner_min = corner_min.Min(subunits[a]->corner_min);
    corner_max = corner_max.Max(subunits[a]->corner_max);
    }*/
  image->selectionBox = new Box(corner_min, corner_max);
  float tmp1 = corner_min.Magnitude();
  float tmp2 = corner_max.Magnitude();
  radial_size = tmp1>tmp2?tmp1:tmp2;
  if (!SubUnit) {
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

void Unit::Init()
{
  planet=NULL;
  image = new UnitImages;
  sound = new UnitSounds;
  limits.structurelimits=Vector(0,0,1);
  limits.limitmin=-1;
  cloaking=-1;
  image->cloakglass=false;
  cloakmin=image->cloakglass?1:0;
  image->cloakrate=100;
  image->cloakenergy=0;
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
  
  bspShield = bspTree = NULL;
  colTree = colShield = NULL;
  invisible=false;
  //origin.Set(0,0,0);
  corner_min.Set (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max.Set (-FLT_MAX,-FLT_MAX,-FLT_MAX);
  
  shieldtight=0;//sphere mesh by default
  energy=maxenergy=10000;
  recharge = 10;
  shield.recharge=shield.leak=0;
  shield.fb[0]=shield.fb[1]=shield.fb[2]=shield.fb[3]=armor.front=armor.back=armor.right=armor.left=0;
  hull=10;
  shield.number=2;
  
  image->explosion=NULL;
  image->timeexplode=0;
  killed=false;
  ucref=0;
  numhalos = nummounts= nummesh = numsubunit = 0;
  halos = NULL; mounts = NULL;meshdata = NULL;subunits = NULL;
  aistate = NULL;
  SetAI (new Order());
  Identity(cumulative_transformation_matrix);
  cumulative_transformation = identity_transformation;
  curr_physical_state = prev_physical_state = identity_transformation;
  mass = 1.5;
  fuel = 2000;

  /*
  yprrestricted=0;
  ymin = pmin = rmin = -PI;
  ymax = pmax = rmax = PI;
  ycur = pcur = rcur = 0;
  */
  MomentOfInertia = 1.5;
  AngularVelocity = Vector(0,0,0);
  Velocity = Vector(0,0,0);
  
  NetTorque =NetLocalTorque = Vector(0,0,0);
  NetForce = Vector(0,0,0);
  NetLocalForce=Vector(0,0,0);

  selected = false;
  image->selectionBox = NULL;

  limits.yaw = 25.5;
  limits.pitch = 25.5;
  limits.roll = 25.5;
	
  limits.lateral = 80;
  limits.vertical = 80;
  limits.forward = 200;
  limits.afterburn=500;
  limits.retro=200;
  Target(NULL);
  computer.threat.SetUnit (NULL);
  computer.set_speed=0;
  computer.max_speed=15;
  computer.max_ab_speed=40;
  computer.max_yaw=100;
  computer.max_pitch=100;
  computer.max_roll=100;
  computer.NavPoint=Vector(0,0,0);
  computer.itts = false;
  computer.radar.maxrange=10000;
  computer.radar.maxcone=-1;
  computer.radar.error=0;
  computer.radar.color=true;
  //  Fire();

  flightgroup=NULL;
  flightgroup_subnumber=0;
}

void Unit::SetVisible(bool invis) {
  invisible=!invis;
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
  calculate_extent();
}
Unit::Unit(const char *filename, bool xml, bool SubU, int faction,Flightgroup *flightgrp,int fg_subnumber) {
	Init();
	SubUnit = SubU;
	this->faction = faction;
	flightgroup=flightgrp;
	flightgroup_subnumber=fg_subnumber;

	vssetdir (GetSharedUnitPath().c_str());
	vschdir (filename);
	FILE *fp = fopen (filename,"r");
	if (!fp) {
	  vscdup();
	  const char *c;
	  if ((c=_Universe->GetFaction(faction)))
	    vschdir (c);
	  else
	    vschdir ("unknown");
	  vschdir (filename);
	} else {
	  fclose (fp);
	}
	name = filename;
	/*Insert file loading stuff here*/
	if(xml) {
	  LoadXML(filename);
	  calculate_extent();
	  ToggleWeapon(true);//change missiles to only fire 1
	  vscdup();
	  if (fp) 
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

		meshdata[meshcount] = new Mesh(meshfilename, false, faction);

		//		meshdata[meshcount]->SetPosition(Vector (x,y,z));
	}

	ReadInt(numsubunit);
	for(int unitcount = 0; unitcount < numsubunit; unitcount++)
	{
		char unitfilename[64];
		float x,y,z;
		int type;
		ReadUnit(unitfilename, type, x,y,z);
		switch(type)
		{
		default:
		  subunits[unitcount] = new Unit (unitfilename,false,true,faction,flightgroup,flightgroup_subnumber);
		}
		subunits[unitcount]->SetPosition(Vector(x,y,z));
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
	calculate_extent();
	ToggleWeapon(true);//change missiles to only fire 1
	vscdup();
	if (fp) 
	  vscdup();
	vsresetdir();

}

Unit::~Unit()
{
  if (sound->engine!=-1) {
    AUDDeleteSound (sound->engine);
  }
  if (sound->explode!=-1) {
    AUDDeleteSound (sound->explode);
  }
  if (sound->shield!=-1) {
    AUDDeleteSound (sound->shield);
  }
  if (sound->armor!=-1) {
    AUDDeleteSound (sound->armor);
  }
  if (sound->hull!=-1) {
    AUDDeleteSound (sound->hull);
  }
  if (sound->cloak!=-1) {
    AUDDeleteSound (sound->cloak);
  }

  if (planet)
    delete planet;
  if (image->hudImage )
    delete image->hudImage;
  delete image;
  delete sound;
  if (CollideInfo.object.u)
    KillCollideTable (&CollideInfo);
  if (bspTree)
    delete bspTree;
  if (bspShield)
    delete bspShield;
  if (colTree)
    delete colTree;
  if (colShield)
    delete colShield;
  for (int beamcount=0;beamcount<nummounts;beamcount++) {
    AUDDeleteSound(mounts[beamcount].sound);
    if (mounts[beamcount].ref.gun&&mounts[beamcount].type.type==weapon_info::BEAM)
      delete mounts[beamcount].ref.gun;//hope we're not killin' em twice...they don't go in gunqueue
  }
	if(meshdata&&nummesh>0)
	{
		for(int meshcount = 0; meshcount < nummesh; meshcount++)
			delete meshdata[meshcount];
		delete [] meshdata;
	}
	if(subunits)
	{
		for(int subcount = 0; subcount < numsubunit; subcount++)
			delete subunits[subcount];
		delete [] subunits;
	}
	if (halos) {
	  for (int hc=0;hc<numhalos;hc++) {
	    delete halos[hc];
	  }
	  delete [] halos;
	}
	if (mounts) {
	  delete []mounts;
	}
	if(aistate)
		delete aistate;
/*
	if(weapons)
	{
		for(int weapcount = 0; weapcount < numweap; subcount++)
			delete subsidiary[subcount];
		delete [] subsidiary;
	}
*/
}
void Unit::getAverageGunSpeed(float & speed, float &range) const {
   if (nummounts) {
     int nummt = nummounts;
     for (int i=0;i<nummounts;i++) {
       if (mounts[i].type.type!=weapon_info::PROJECTILE) {
	 range+=mounts[i].type.Range;
	 speed+=mounts[i].type.Speed;
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
Vector Unit::PositionITTS (const Vector & posit, float speed) const{
  Vector retval = Position()-posit;
  speed = retval.Magnitude()/speed;//FIXME DIV/0 POSSIBLE
  retval = Position()+Velocity*speed;
  return retval;
}
float Unit::cosAngleTo (Unit * targ, float &dist, float speed, float range) const{
   Vector Normal (cumulative_transformation_matrix[8],cumulative_transformation_matrix[9],cumulative_transformation_matrix[10]);
   //   if (range!=FLT_MAX) {
   //     getAverageGunSpeed(speed,range);
   //   }
   Vector totarget (targ->PositionITTS(cumulative_transformation.position, speed+((targ->Position()-Position()).Normalize().Dot (Velocity))));
   totarget = totarget-cumulative_transformation.position;
   float tmpcos = Normal.Dot (totarget);
   dist = totarget.Magnitude();
   if (tmpcos>0) {
      tmpcos = dist*dist - tmpcos*tmpcos;
      tmpcos = targ->rSize()/sqrtf(tmpcos);//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
   } else {
     tmpcos /= dist;
   }
   dist /= range;//WARNING POTENTIAL DIV/0
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
    Vector Normal (mat[8],mat[9],mat[10]);
    
    Vector totarget (targ->PositionITTS(finaltrans.position, mounts[i].type.Speed));
    
    tmpcos = Normal.Dot (totarget);
    tmpdist = totarget.Magnitude();
    if (tmpcos>0) {
      tmpcos = tmpdist*tmpdist - tmpcos*tmpcos;
      tmpcos = targ->rSize()/tmpcos;//one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
    } else {
      tmpcos /= tmpdist;
    }
    tmpdist /= mounts[i].type.Range;//UNLIKELY DIV/0
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
  
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->queryFrustum(frustum))
      return true;
  }
  return false;
}


float Unit::GetElasticity() {return .5;}
void Unit::UpdateHudMatrix() {
  Matrix m;
  float * ctm=cumulative_transformation_matrix;
  if (planet) {
    Transformation ct (linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor));  
    ct.to_matrix (m);
    ctm=m;
  }
  Vector q (ctm[4],	    ctm[5],	    ctm[6]);
  Vector r (ctm[8],	    ctm[9],	    ctm[10]);
  Vector tmp;
  CrossProduct(r,q, tmp);
  _Universe->AccessCamera()->SetOrientation(tmp,q ,r);
  
  _Universe->AccessCamera()->SetPosition (Transform (ctm,image->CockpitCenter));
}
   

void Unit::SetPlanetHackTransformation (Transformation *&ct,float *&ctm) {
  static Transformation planet_temp_transformation;
  static Matrix planet_temp_matrix;
  if (planet) {
    if (planet->trans==_Universe->AccessCamera()->GetPlanetaryTransform()) {
      Matrix tmp;
      Vector p,q,r,c;
      MatrixToVectors (cumulative_transformation_matrix,p,q,r,c);
      planet->trans->InvTransformBasis(tmp,p,q,r,c);
      MultMatrix (planet_temp_matrix,_Universe->AccessCamera()->GetPlanetGFX(),tmp);
      planet_temp_transformation = Transformation::from_matrix (planet_temp_matrix);
      ct = &planet_temp_transformation;
      ctm = planet_temp_matrix;
      ///warning: hack FIXME
      cumulative_transformation=*ct;
      CopyMatrix (cumulative_transformation_matrix,ctm);
    }
  }  
}
void Unit::Draw(const Transformation &parent, const Matrix parentMatrix)
{

  cumulative_transformation = linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor);
  float * ctm;
  Transformation * ct;
  cumulative_transformation.Compose(parent, parentMatrix);
  ctm =cumulative_transformation_matrix;
  ct = &cumulative_transformation;
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  SetPlanetHackTransformation (ct,ctm);

#ifdef PERFRAMESOUND
  AUDAdjustSound (sound.engine,cumulative_transformation.position,Velocity);
#endif
  short cloak=cloaking;
  if (cloaking>cloakmin) {
    cloak = (short)(cloaking-interpolation_blend_factor*image->cloakrate);
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
  if (!invisible) {
    for (i=0;i<=nummesh;i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
      if (meshdata[i]==NULL) 
		continue;
	  if (i==nummesh&&(meshdata[i]->numFX()==0||hull<0)) 
		continue;
      Vector TransformedPosition = Transform (ctm,
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
	  meshdata[i]->Draw(lod,ctm,d,cloak,(_Universe->AccessCamera()->GetNebula()!=nebula&&nebula!=NULL)?0:-1);//cloakign and nebula
	} else {

	}
      }
    }
    
    for(int subcount = 0; subcount < numsubunit; subcount++) {
      subunits[subcount]->Draw(*ct, ctm);
    }
    if(selected) {
      static bool doInputDFA=XMLSupport::parse_bool (vs_config->getVariable ("graphics","MouseCursor","false"));
      if (doInputDFA)
	image->selectionBox->Draw(g_game.x_resolution,ctm);
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
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].ref.gun) {
	mounts[i].ref.gun->Draw(*ct,ctm);
      }
    }
  }
  float haloalpha=1;
  if (cloak>=0) {
    haloalpha=((float)cloak)/32767;
  }
  for (i=0;i<numhalos;i++) {
    halos[i]->Draw(*ct,ctm,haloalpha);
  }
}
void Unit::PrimeOrders (int i) {
  subunits[i]->PrimeOrders();
}
void Unit::SetAI (Order *newAI, int i) {
  subunits[i]->SetAI (newAI);
}
void Unit::EnqueueAI(Order *newAI, int i) { 
  subunits[i]->EnqueueAI(newAI);
}
void Unit::PrimeOrders () {
  if (aistate) {
    aistate->ReplaceOrder (new Order);
  } else {
    aistate = new Order; //get 'er ready for enqueueing
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
void Unit::ExecuteAI() {
  if(aistate) aistate->Execute();
  for(int a=0; a<numsubunit; a++) {
    subunits[a]->ExecuteAI();//like dubya
  }
}
void Unit::Select() {
  selected = true;
}
void Unit::Deselect() {
  selected = false;
}
bool Unit::InRange (Unit *target, Vector &localcoord) const {
  localcoord =Vector(ToLocalCoordinates(target->Position()-Position()));
  float mm= localcoord.Magnitude();
  if (mm>computer.radar.maxrange||(localcoord.k/mm)<computer.radar.maxcone||target->CloakVisible()<.8) {
    return false;
  }
  return true;
}

const string Unit::getFgID()  {
    if(flightgroup!=NULL){
      char buffer[200];
      sprintf(buffer,"-%d",flightgroup_subnumber);
      return flightgroup->name+buffer;
    }
    else{
      return name;
    }
};
