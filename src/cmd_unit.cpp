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

#include "cmd_beam.h"
#include "cmd_unit.h"
#include "cmd_gun.h"
#include "gfx_sprite.h"
#include "lin_time.h"
#include "cmd_unitenum.h"
#include "gfx_hud.h"
#include "gfx_bounding_box.h"
#include "cmd_ai.h"
#include "cmd_order.h"
#include "gfx_box.h"
#include "gfx_animation.h"
#include "gfx_lerp.h"
//if the PQR of the unit may be variable...for radius size computation
//#define VARIABLE_LENGTH_PQR

extern Vector mouseline;
extern vector<Vector> perplines;
Vector MouseCoordinate (int mouseX, int mouseY);

double interpolation_blend_factor;


static list<Unit*> Unitdeletequeue;

void Unit::calculate_extent() {  
	int a;
	for(a=0; a<nummesh; a++) {
    corner_min = corner_min.Min(meshdata[a]->corner_min() + meshdata[a]->Position());
    corner_max = corner_max.Max(meshdata[a]->corner_max() + meshdata[a]->Position());
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
  nummounts=0;
  mounts=NULL;
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

  yrestricted = prestricted = rrestricted = FALSE;
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

  limits.yaw = 2.55;
  limits.pitch = 2.55;
  limits.roll = 2.5;
	
  limits.lateral = 0.1;
  limits.vertical = 0.1;
  limits.longitudinal = 1;
  //  Fire();
}
void Unit::UnRef() {
  ucref--;
  if (killed&&ucref==0) {
    Unitdeletequeue.push_back(this);//delete
  }
}
void Unit::Kill() {
  killed = true;
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
	meshdata = new Mesh*[nummesh];

	for(int meshcount = 0; meshcount < nummesh; meshcount++)
	{
		int meshtype;
		ReadInt(meshtype);
		char meshfilename[64];
		float x,y,z;
		ReadMesh(meshfilename, x,y,z);

		if(meshtype == 0)
			meshdata[meshcount] = new Mesh(meshfilename);
		else
			meshdata[meshcount] = new Sprite(meshfilename);

		meshdata[meshcount]->SetPosition(x,y,z);
	}

	ReadInt(numsubunit);
	for(int unitcount = 0; unitcount < numsubunit; unitcount++)
	{
		char unitfilename[64];
		float x,y,z;
		int type;
		ReadUnit(unitfilename, type, x,y,z);
		switch((UnitType)type)
		{
		case TEXTPLANE:
			subunits[unitcount] = new TextPlane(unitfilename);
			break;
		case LIGHT:
			subunits[unitcount] = new Unit(unitfilename);	//how to group turrets like fighters??? maybes stick in special cases
														//Translation: This ROYALLY screws up the idea of having every object as a class...
														//perhaps use a switch statement? that would get rather big, although it would be straightforward
														//maybe have a behavior variable in each turret that specifies its stats, but that sucks too
														//or, make a special exception to the copy constructor rule... *frumple* this sucks
			break;
		default:
		  printf ("unit type not supported");
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







float Unit::getMinDis (const Vector &pnt) {
  float minsofar=1e+10;
  float tmpvar;
  int i;
  Vector TargetPoint (cumulative_transformation_matrix[0],cumulative_transformation_matrix[1],cumulative_transformation_matrix[2]);

#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint)); //the scale factor of the current UNIT
#endif
  for (i=0;i<nummesh;i++) {

    TargetPoint = Transform(cumulative_transformation_matrix,meshdata[i]->Position())-pnt;
    tmpvar = sqrtf (TargetPoint.Dot (TargetPoint))-meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
	*SizeScaleFactor
#endif
      ;
    if (tmpvar<minsofar) {
      minsofar = tmpvar;
    }
  }
  for (i=0;i<numsubunit;i++) {
    tmpvar = subunits[i]->getMinDis (pnt);
    if (tmpvar<minsofar) {
      minsofar=tmpvar;
    }			       
  }
  return minsofar;
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
float Unit::querySphere (const Vector &st, const Vector &dir, float err) {
  int i;
  float retval=0;
  float * tmpo = cumulative_transformation_matrix;

  Vector TargetPoint (tmpo[0],tmpo[1],tmpo[2]);
#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));//adjust the ship radius by the scale of local coordinates
#endif
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform (tmpo,meshdata[i]->Position());
    Vector origPoint = TargetPoint;

    perplines.push_back(TargetPoint);
    //find distance away from the line now :-)
    //find scale factor of end on start to get line.
    Vector tst = TargetPoint-st;
    //Vector tst = TargetPoint;
    float k = tst.Dot (dir);
    TargetPoint = tst - k*(dir);
    /*
    cerr << origPoint << "-" << st << " = " << tst << " projected length " << k << " along direction " << dir << endl;
    cerr << "projected line " << st << " - " << st + k*dir << endl;
    cerr << "length of orthogonal projection " << TargetPoint.Magnitude() << ", " << "radius " << meshdata[i]->rSize() << endl;
    */
    perplines.push_back(origPoint-TargetPoint);
    
    ///      fprintf (stderr, "i%f,j%f,k%f end %f,%f,%f>, k %f distance %f, rSize %f\n", st.i,st.j,st.k,end.i,end.j,end.k,k,TargetPoint.Dot(TargetPoint), meshdata[i]->rSize());    
    
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
      {
	if (retval==0) {
	  retval = k;
	}else {
	  if (retval>0&&k<retval)
	    retval = k;
	  if (retval<0&&k>retval)
	    retval = k;
	}
    }
  }
  for (i=0;i<numsubunit;i++) {
    float tmp = (subunits[i]->querySphere (st,dir,err));
    if (tmp==0) continue;
    if (retval==0) {
      retval = tmp;
    }else{
	  if (retval>0&&tmp<retval)
	    retval = tmp;
	  if (retval<0&&tmp>retval)
	    retval = tmp;
    }
  }
  return retval;
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

bool Unit::queryBoundingBox (const Vector &pnt, float err) {
  int i;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (cumulative_transformation_matrix);
    if (bbox->Within(pnt,err)) {
      delete bbox;
      return true;
    }
    delete bbox;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->queryBoundingBox (pnt,err)) 
      return true;
  }
  return false;
}

int Unit::queryBoundingBox (const Vector &origin, const Vector &direction, float err) {
  int i;
  int retval=0;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (cumulative_transformation_matrix);
    switch (bbox->Intersect(origin,direction,err)) {
    case 1:delete bbox;
      return 1;
    case -1:delete bbox;
      retval =-1;
      break;
    case 0: delete bbox;
      break;
    }
  }
  for (i=0;i<numsubunit;i++) {
    switch (subunits[i]->queryBoundingBox (origin,direction,err)) {
    case 1: return 1;
    case -1: retval= -1;
      break;
    case 0: break;
    }
  }
  return retval;
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


bool Unit::querySphere (int mouseX, int mouseY, float err, Camera * activeCam) {
  int i;
  Matrix vw;
  _GFX->AccessCamera()->GetView (vw);
  Vector mousePoint;
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (cumulative_transformation_matrix[0],cumulative_transformation_matrix[1],cumulative_transformation_matrix[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif

  Vector CamP,CamQ,CamR;
  for (i=0;i<nummesh;i++) {
    //cerr << "pretransform position: " << meshdata[i]->Position() << endl;
    TargetPoint = Transform(cumulative_transformation_matrix,meshdata[i]->Position());
    
    mousePoint = Transform (vw,TargetPoint);
    if (mousePoint.k>0) { //z coordinate reversed  -  is in front of camera
      continue;
    }
    mousePoint = MouseCoordinate (mouseX,mouseY);
    
    activeCam->GetPQR(CamP,CamQ,CamR);
    mousePoint = Transform (CamP,CamQ,CamR,mousePoint);	
    activeCam->GetPosition(CamP);    
    mousePoint +=CamP; 
    
    
    
    TargetPoint =TargetPoint-mousePoint;
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
    if (subunits[i]->querySphere (mouseX,mouseY,err,activeCam))
      return true;
  }
  return false;
}

void Unit::UpdateHudMatrix() {
  //FIXME
  Matrix tmatrix;
  Vector camp,camq,camr;
  _GFX->AccessCamera()->GetPQR(camp,camq,camr);
  
	//GFXIdentity(MODEL);
	//Identity (tmatrix);
	//	Translate (tmatrix,_GFX->AccessCamera()->GetPosition());
	//	GFXLoadMatrix(MODEL,tmatrix);
  //VectorAndPositionToMatrix (tmatrix,-camp,camq,camr,_GFX->AccessCamera()->GetPosition()+1.23*camr);  
  VectorAndPositionToMatrix (tmatrix,camp,camq,camr,_GFX->AccessCamera()->GetPosition());

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
  for (i=0;i<nummesh;i++) {
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
  for (i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].gun) {
	mounts[i].gun->Draw(cumulative_transformation,cumulative_transformation_matrix);
      }
    }
  }
}

void Unit::DrawStreak(const Vector &v)
{
  /*
	Vector v1 = v;
	int steps = (int)v.Magnitude()*10;
	v1 = v1 * (1.0/steps);
	Vector opos = local_transformation.position;
	GFXColor(0.5, 0.5, 0.5, 0.5);
	for(int a = 0; a < steps; a++) {
		Draw();
		local_transformation.position+=v1;
	}
	GFXColor(1.0, 1.0, 1.0, 1.0);
	local_transformation.position = opos;
  */
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
    aistate = aistate->ReplaceOrder (newAI);
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
void Unit::Fire () {
  for (int i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].type.EnergyRate*SIMULATION_ATOM>energy)
	continue;
    }else{ 
      if (mounts[i].type.EnergyConsumption>energy) 
	continue;
    }
    ///FIXME!!    if (mounts[i].Fire(cumulative_transformation,cumulative_transformation_matrix,this)) {
    ///      energy -= mounts[i].type==BEAM?mounts[i].type.EnergyRate*SIMULATION_ATOM:mounts[i].type.EnergyConsumption;
    ///}//unfortunately cumulative transformation not generated in physics atom
    mounts[i].gun = new Beam(identity_transformation,mounts[i].type,this);
  }
  
}
bool Unit::Mount::Fire (const Transformation &Cumulative, const float * m,  Unit * owner) {
  if (status!=ACTIVE) 
    return false;
  if (type.type==weapon_info::BEAM) {
    if (gun==NULL)
      gun = new Beam (LocalPosition,type,owner);
    else
      if (gun->Dissolved())
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
/*
ostream &Unit::output(ostream& os) const {
  return os << name;
}
*/
/*
ostream &operator<<(ostream &os, const Unit &u) {
  return u.output(os);
} 
*/
void Unit::RestrictYaw(float min, float max) {
  ymin = min; ymax = max;
}
void Unit::RestrictPitch(float min, float max) {
  pmin = min, pmax = max;
}
void Unit::RestrictRoll(float min, float max) {
  rmin = min, rmax = max;
}

/*

void Mesh::XSlide(float factor)
{
	pos += p * factor;
	changed = TRUE;
}
void Mesh::YSlide(float factor)
{
	pos += q * factor;
	changed = TRUE;
}
void Mesh::ZSlide(float factor)
{
	pos += r * factor;
	changed = TRUE;
}

*/
