/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include "file.h"
#include "gfx_halo.h"
#include "cmd_beam.h"
#include "cmd_unit.h"
#include "cmd_gun.h"
#include "gfx_sprite.h"
#include "lin_time.h"
#include "gfx_hud.h"

#include "cmd_ai.h"
#include "cmd_order.h"
#include "gfx_box.h"
#include "gfx_animation.h"
#include "gfx_lerp.h"
#include "gfx_bsp.h"
//if the PQR of the unit may be variable...for radius size computation
//#define VARIABLE_LENGTH_PQR





double interpolation_blend_factor;


static list<Unit*> Unitdeletequeue;

void Unit::calculate_extent() {  
  int a;
  for(a=0; a<nummesh; a++) {
    corner_min = corner_min.Min(meshdata[a]->corner_min());
    corner_max = corner_max.Max(meshdata[a]->corner_max());
  }
  for(a=0; a<numsubunit; a++) {
    corner_min = corner_min.Min(subunits[a]->corner_min);
    corner_max = corner_max.Max(subunits[a]->corner_max);
  }
  selectionBox = new Box(corner_min, corner_max);
  float tmp1 = corner_min.Magnitude();
  float tmp2 = corner_max.Magnitude();
  radial_size = tmp1>tmp2?tmp1:tmp2;
}

void Unit::Init()
{
  CollideInfo.object = NULL;
  CollideInfo.type = LineCollide::UNIT;
  bspTree = NULL;
  invisible=false;
  origin.Set(0,0,0);
  corner_min.Set (FLT_MAX,FLT_MAX,FLT_MAX);
  corner_max.Set (-FLT_MAX,-FLT_MAX,-FLT_MAX);
  numhalos=0;
  halos=NULL;
  nummounts=0;
  nummesh=0;
  mounts=NULL;
  shieldtight=0;//sphere mesh by default
  //    nummounts=1;
    
  //    mounts = new Mount [1];
  //    mounts[0].Activate();
    //  mounts[0].Fire(identity_transformation,identity_matrix,this);
  energy=10000;
  explosion=NULL;
  timeexplode=0;
  killed=false;
  ucref=0;
  meshdata = NULL;
  subunits = NULL;
  aistate = NULL;
  //weapons = NULL;
  numsubunit = 0;


  Identity(cumulative_transformation_matrix);
  cumulative_transformation = identity_transformation;
  curr_physical_state = prev_physical_state = identity_transformation;
  fpos = 0;
  mass = 1;
  fuel = 0;

  //  yrestricted = prestricted = rrestricted = FALSE;
  yprrestricted=0;
  ymin = pmin = rmin = -PI;
  ymax = pmax = rmax = PI;
  ycur = pcur = rcur = 0;

  MomentOfInertia = 1;
  AngularVelocity = Vector(0,0,0);
  Velocity = Vector(0,0,0);
  
  NetTorque =NetLocalTorque = Vector(0,0,0);
  NetForce = Vector(0,0,0);
  NetLocalForce=Vector(0,0,0);

  calculatePhysics = true;
  selected = false;
  selectionBox = NULL;

  limits.yaw = 25.5;
  limits.pitch = 25.5;
  limits.roll = 25.5;
	
  limits.lateral = 10;
  limits.vertical = 10;
  limits.forward = 10;
  limits.afterburn=20;
  limits.retro=1;
  Target(NULL);
  computer.set_speed=0;
  computer.max_speed=10;
  computer.max_ab_speed=30;
  computer.max_yaw=4;
  computer.max_pitch=4;
  computer.max_roll=4;
  computer.NavPoint=Vector(0,0,0);
  //  Fire();
}

void Unit::SetCameraToCockpit() {
  invisible=true;
  /*  cumulative_transformation = linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor);
  cumulative_transformation.Compose(parent, parentMatrix);
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  _Universe->AccessCamera()->SetOrientation(Vector (cumulative_transformation_matrix[0],
						    cumulative_transformation_matrix[1],
						    cumulative_transformation_matrix[2]),
					    Vector (cumulative_transformation_matrix[4],
						    cumulative_transformation_matrix[5],
						    cumulative_transformation_matrix[6]),
					    Vector (cumulative_transformation_matrix[8],
						    cumulative_transformation_matrix[9],
						    cumulative_transformation_matrix[10]));
						    _Universe->AccessCamera()->SetPosition (cumulative_transformation.position);*/
}
void Unit::UnRef() {
  ucref--;
  if (killed&&ucref==0) {
    Unitdeletequeue.push_back(this);//delete
  }
}
void Unit::Kill() {
  killed = true;
  if (CollideInfo.object) {
    KillCollideTable (&CollideInfo);
    CollideInfo.object=NULL;
  }
  Target((Unit *)NULL);
  if (ucref==0)
    Unitdeletequeue.push_back(this);
}
void Unit::ProcessDeleteQueue() {
  while (Unitdeletequeue.size()) {
    delete Unitdeletequeue.back();
    Unitdeletequeue.pop_back();
  }
}

Unit::Unit() {
	Init();
}
Unit::Unit(const char *filename, bool xml) {
	Init();

	name = filename + string(" - Unit");
	/*Insert file loading stuff here*/
	if(xml) {
	  LoadXML(filename);
	  calculate_extent();
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

		meshdata[meshcount] = new Mesh(meshfilename);

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
		  subunits[unitcount] = new Unit (unitfilename);
		}
		subunits[unitcount]->SetPosition(Vector(x,y,z));
	}

	int restricted;
	float min, max;
	ReadInt(restricted); //turrets and weapons
	//ReadInt(restricted); // What's going on here? i hsould have 2, but that screws things up

	ReadRestriction(restricted, min, max);
	if(restricted)
		RestrictYaw(min,max);

	ReadRestriction(restricted, min, max);
	if(restricted)
		RestrictPitch(min,max);

	ReadRestriction(restricted, min, max);
	if(restricted)
		RestrictRoll(min,max);

	float maxspeed, maxaccel, mass;
	ReadFloat(maxspeed);
	ReadFloat(maxaccel);
	ReadFloat(mass);

	fpos = ::GetPosition();

	CloseFile();
	calculate_extent();
}

Unit::~Unit()
{
  if (CollideInfo.object)
    KillCollideTable (&CollideInfo);
  if (bspTree)
    delete bspTree;
  for (int beamcount=0;beamcount<nummounts;beamcount++) {
    if (mounts[beamcount].gun)
      delete mounts[beamcount].gun;//hope we're not killin' em twice...they don't go in gunqueue
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
	if (halos&&numhalos)
	  delete [] halos;
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







bool Unit::querySphere (const Vector &pnt, float err) {
  int i;
  float * tmpo = cumulative_transformation_matrix;
  
  Vector TargetPoint (tmpo[0],tmpo[1],tmpo[2]);
#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));//adjust the ship radius by the scale of local coordinates
#endif
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform (tmpo,meshdata[i]->Position())-pnt;
    if (TargetPoint.Dot (TargetPoint)< 
	err*err+
	meshdata[i]->rSize()*meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
	*SizeScaleFactor*SizeScaleFactor
#endif
	+
#ifdef VARIABLE_LENGTH_PQR
	SizeScaleFactor*
#endif
	2*err*meshdata[i]->rSize()
	)
      return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->querySphere (pnt,err))
      return true;
  }
  return false;
}


// dir must be normalized

float Unit::querySphere (const Vector &start, const Vector &end) {
  int i;
  float t=0;
  float tmp;
  Vector st,dir;
  for (i=0;i<nummesh;i++) {
    float a, b,c;
    st = start - Transform (cumulative_transformation_matrix,meshdata[i]->Position());	
    dir = end-start;//now start and end are based on mesh's position
    // v.Dot(v) = r*r; //equation for sphere
    // (x0 + (x1 - x0) *t) * (x0 + (x1 - x0) *t) = r*r
    c = st.Dot (st) - meshdata[i]->rSize()*meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
      *SizeScaleFactor*SizeScaleFactor
#endif
      ;
    b = 2 * (dir.Dot (st));
    a = dir.Dot(dir);
    //b^2-4ac
    c = b*b - 4*a*c;
    if (c<0)
      continue;
    a *=2;
    tmp = (-b + sqrtf (c))/a;
    c = (-b - sqrtf (c))/a;
    if (tmp>0&&tmp<=1) {
      return (c>0&&c<tmp) ? c : tmp;
    } else if (c>0&&c<=1) {
	return c;
    }
  }
  for (i=0;i<numsubunit;i++) {
    if ((tmp = subunits[i]->querySphere (start,end))!=0) {
      return tmp;
    }
  }
  return 0;
}

void Unit::Destroy() {
  if (!killed)
    if (!Explode())
      Kill();
}

bool Unit::Explode () {
  int i;
  if (explosion==NULL&&timeexplode==0&&nummesh) {	//no explosion in unit data file && explosions haven't started yet
    explosion = new Animation * [nummesh];
    timeexplode=0;
    for (i=0;i<nummesh;i++){
      explosion[i]= new Animation ("explosion_orange.ani",false,.1,BILINEAR,false);
    }    
  }
  float tmp[16];
  
  float tmp2[16];
  bool alldone =false;
  if (explosion) {
    GFXDisable(DEPTHWRITE);
    for (i=0;i<nummesh;i++) {
      if (!explosion[i])
	continue;
      timeexplode+=GetElapsedTime();
      Translate (tmp,meshdata[i]->Position());
      MultMatrix (tmp2,cumulative_transformation_matrix,tmp);
      explosion[i]->SetPosition(tmp2[12],tmp2[13],tmp2[14]);
      if (timeexplode>i*.5){
	explosion[i]->Draw();
      }
      if (explosion[i]->Done()) {
	delete explosion[i];	
	explosion[i]=NULL;
      }else {
	alldone=true;
      }
      GFXEnable (DEPTHWRITE);
    }
    if (!alldone){
      delete [] explosion;
      explosion = NULL;
    }
  }
  for (i=0;i<numsubunit;i++) {
    alldone |=subunits[i]->Explode();
  }
  return alldone;
}

bool Unit::queryBSP (const Vector &pt, float err, Vector & norm, float &dist) {
  int i;
  Vector st (InvTransform (cumulative_transformation_matrix,pt));
  for (i=0;i<numsubunit;i++) {
    if ((subunits[i]->queryBSP(pt,err, norm,dist)))
      return true;
  }
  bool temp=false;
  for (i=0;i<nummesh&&!temp;i++) {
    temp|=meshdata[i]->queryBoundingBox (st,err);
  }
  if (!temp)
    return false;
  if (!bspTree)
      return true;
  if (bspTree->intersects (st,err,norm,dist)) {
    norm = ToWorldCoordinates (norm);
    return true;
  }
  return false;
}

float Unit::queryBSP (const Vector &start, const Vector & end, Vector & norm) {
  int i;
  float tmp;

  for (i=0;i<numsubunit;i++) {
    if (tmp = subunits[i]->queryBSP(start,end,norm))
      return tmp;
  }
  if (!bspTree) {
    tmp = querySphere (start,end);
    norm = (tmp * (start-end));
    tmp = norm.Magnitude();
    norm +=start;
    norm.Normalize();//normal points out from center
    return tmp;
  }
  Vector st (InvTransform (cumulative_transformation_matrix,start));
  Vector ed (InvTransform (cumulative_transformation_matrix,end));
  bool temp=false;
  for (i=0;i<nummesh&&!temp;i++) {
    temp = (1==meshdata[i]->queryBoundingBox (st,ed,0));
  }
  if (!temp)
    return false;
  if (tmp = bspTree->intersects (st,ed,norm)) {
    norm = ToWorldCoordinates (norm);
    return tmp;
  }
  return 0;
}


bool Unit::queryFrustum(float frustum [6][4]) {
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
  //FIXME
  Matrix tmatrix;
  Vector camp,camq,camr;
  _Universe->AccessCamera()->GetPQR(camp,camq,camr);
  
	//GFXIdentity(MODEL);
	//Identity (tmatrix);
	//	Translate (tmatrix,_Universe->AccessCamera()->GetPosition());
	//	GFXLoadMatrix(MODEL,tmatrix);
  //VectorAndPositionToMatrix (tmatrix,-camp,camq,camr,_Universe->AccessCamera()->GetPosition()+1.23*camr);  
  VectorAndPositionToMatrix (tmatrix,camp,camq,camr,_Universe->AccessCamera()->GetPosition());

}

void Unit::Draw(const Transformation &parent, const Matrix parentMatrix)
{
  //Matrix cumulative_transformation_matrix;
  /*Transformation*/ cumulative_transformation = linear_interpolate(prev_physical_state, curr_physical_state, interpolation_blend_factor);
  cumulative_transformation.Compose(parent, parentMatrix);
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
#ifdef VARIABLE_LENGTH_PQR
        Vector MeshCenter (cumulative_transformation_matrix[0],cumulative_transformation_matrix[1],cumulative_transformation_matrix[2]); 
        float SizeScaleFactor=sqrtf (MeshCenter.Dot(MeshCenter));
#else
        Vector MeshCenter;
#endif
  int i;
  if (!invisible) {
    for (i=0;i<=nummesh;i++) {//NOTE LESS THAN OR EQUALS...to cover shield mesh
      if (meshdata[i]==NULL) 
		continue;
	  if (i==nummesh&&meshdata[i]->numFX()==0) 
		continue;
      float d = GFXSphereInFrustum(Transform (cumulative_transformation_matrix,
					      meshdata[i]->Position()),
				   meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				   *SizeScaleFactor
#endif 
				   );
      if (d) {  //d can be used for level of detail shit
	meshdata[i]->Draw(cumulative_transformation, cumulative_transformation_matrix);
      }
    }
    
    for(int subcount = 0; subcount < numsubunit; subcount++) {
      subunits[subcount]->Draw(cumulative_transformation, cumulative_transformation_matrix);
    }
    if(selected) {
      selectionBox->Draw(cumulative_transformation, cumulative_transformation_matrix);
    }
  } else {
	Vector q (cumulative_transformation_matrix[4],
			  cumulative_transformation_matrix[5],
			  cumulative_transformation_matrix[6]);
    Vector r (cumulative_transformation_matrix[8],
			  cumulative_transformation_matrix[9],
			  cumulative_transformation_matrix[10]);
	Vector tmp;
	CrossProduct(r,q, tmp);
    _Universe->AccessCamera()->SetOrientation(tmp,q ,r);
	_Universe->AccessCamera()->SetPosition (cumulative_transformation.position);
  }
  for (i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].gun) {
	mounts[i].gun->Draw(cumulative_transformation,cumulative_transformation_matrix);
      }
    }
  }
  for (i=0;i<numhalos;i++) {
        halos[i]->Draw(cumulative_transformation,cumulative_transformation_matrix);
  }
}


void Unit::ProcessDrawQueue() {
  int a;	
  for(a=0; a<nummesh; a++) {
    meshdata[a]->ProcessDrawQueue();
  }
  for(a=0; a<numsubunit; a++) {
    subunits[a]->ProcessDrawQueue();
  }
}

void Unit::PrimeOrders () {
  if (aistate) {
    aistate = aistate->ReplaceOrder (new Order);
  } else {
    aistate = new Order; //get 'er ready for enqueueing
  }
}
void Unit::SetAI(AI *newAI)
{
  newAI->SetParent(this);
  if (aistate) {
    aistate = aistate->ReplaceOrder (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::EnqueueAI(AI *newAI) {
  newAI->SetParent(this);
  if (aistate) {
    aistate = aistate->EnqueueOrder (newAI);
  }else {
    aistate = newAI;
  }
}
void Unit::ExecuteAI() {
  if(aistate) aistate = aistate->Execute();
  for(int a=0; a<numsubunit; a++) {
    subunits[a]->ExecuteAI();//like dubya
  }
}
void Unit::UnFire () {
  for (int i=0;i<nummounts;i++) {
    mounts[i].UnFire();//turns off beams;
  }
}
void Unit::Fire () {
  for (int i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].type.EnergyRate*SIMULATION_ATOM>energy)
	continue;
    }else{ 
      if (mounts[i].type.EnergyConsumption>energy) 
	continue;
    }
    if (mounts[i].Fire(cumulative_transformation,cumulative_transformation_matrix,this)) {
    energy -= mounts[i].type.type==weapon_info::BEAM?mounts[i].type.EnergyRate*SIMULATION_ATOM:mounts[i].type.EnergyConsumption;
    }//unfortunately cumulative transformation not generated in physics atom
  }
}
void Unit::Mount::UnFire () {
  if (status!=ACTIVE||gun==NULL||type.type!=weapon_info::BEAM)
    return;
  gun->Destabilize();
}
bool Unit::Mount::Fire (const Transformation &Cumulative, const float * m, Unit * owner) {
  if (status!=ACTIVE) 
    return false;
  if (type.type==weapon_info::BEAM) {
    if (gun==NULL)
      gun = new Beam (LocalPosition,type,owner);
    else
      if (gun->Ready())
	gun->Init (LocalPosition,type,owner);
      else 
	return false;//can't fire an active beam
  }else { 
    Transformation tmp = LocalPosition;
    tmp.Compose (Cumulative,m);
    switch (type.type) {
    case weapon_info::BALL:
      gun = NULL;
      //new Ball (tmp, type, owner);
      break;
    case weapon_info::BOLT:
      gun=NULL;
      //new Bolt (tmp, type, owner);
      break;
    case weapon_info::PROJECTILE:
      gun=NULL;
      //new Missile (tmp, type, owner);
      break;
    default: 
      break;
    }
  }
  return true;
}
Unit::Mount::Mount(const string& filename) :gun(NULL),status(UNCHOSEN),size(weapon_info::NOWEAP),type(weapon_info::BEAM){
  weapon_info * temp = getTemplate (filename);  
  if (temp==NULL) {
    status=UNCHOSEN;
  }else {
    type = *temp;
    status=ACTIVE;
  }
}
void Unit::Select() {
  selected = true;
}
void Unit::Deselect() {
  selected = false;
}
void Unit::RestrictYaw(float min, float max) {
  ymin = min; ymax = max;
}
void Unit::RestrictPitch(float min, float max) {
  pmin = min, pmax = max;
}
void Unit::RestrictRoll(float min, float max) {
  rmin = min, rmax = max;
}


