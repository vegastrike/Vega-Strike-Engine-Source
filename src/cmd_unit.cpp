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
#include "cmd_unit.h"
#include "cmd_gun.h"
#include "gfx_sprite.h"
#include "lin_time.h"
#include "cmd_unitenum.h"
#include "gfx_hud.h"


//if the PQR of the unit may be variable...for radius size computation
#define VARIABLE_LENGTH_PQR





void Unit::Init()
{
	meshdata = NULL;
	subunits = NULL;
	aistate = NULL;
	//weapons = NULL;
	numsubunit = 0;
	time = 0;
	active = TRUE;

	Identity(tmatrix);
	ResetVectors(pp,pq,pr);
	ppos = Vector(0,0,0);
	fpos = 0;
	mass = 1;
	fuel = 0;
	
	MomentOfInertia = 1;
	AngularVelocity = Vector(0,0,0);
	Velocity = Vector(0,0,0);

	NetTorque = Vector(0,0,0);
	NetForce = Vector(0,0,0);

	calculatePhysics = true;
}

Unit::Unit()
{
	Init();
}
Unit::Unit(char *filename):Mesh()
{
	Init();

	debugName = strdup((filename + string(" - Unit")).c_str());
	/*Insert file loading stuff here*/
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
		case GUN:
			subunits[unitcount] = new Gun(unitfilename);
			break;
		default:
		  printf ("unit type not supported");
		}
		subunits[unitcount]->SetPosition(x,y,z);
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
}

Unit::~Unit()
{
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
  UpdateMatrix();
  float minsofar=1e+10;
  float tmpvar;
  int i;
  Vector TargetPoint (transformation[0],transformation[1],transformation[2]);

#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint)); //the scale factor of the current UNIT
#endif
  for (i=0;i<nummesh;i++) {

    TargetPoint = Transform(transformation,meshdata[i]->Position())-pnt;
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
    tmpvar = subunits[i]->getMinDis (transformation,pnt);
    if (tmpvar<minsofar) {
      minsofar=tmpvar;
    }			       
  }
  return minsofar;
}

float Unit::getMinDis (Matrix t,const Vector &pnt) {
  UpdateMatrix();
  int i;
  Matrix tmpo;
  float minsofar=1e+10;
  float tmpvar;
  MultMatrix (tmpo,t,transformation);
  
  Vector TargetPoint (tmpo[0],tmpo[1],tmpo[2]);
#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));//adjust the ship radius by the scale of local coordinates
#endif
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform (tmpo,meshdata[i]->Position())-pnt;
    tmpvar= sqrtf (TargetPoint.Dot (TargetPoint))-meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
      *SizeScaleFactor
#endif
      ;
    if (tmpvar<minsofar) {
      minsofar=tmpvar;
    }
  }
  for (i=0;i<numsubunit;i++) {
    tmpvar = subunits[i]->getMinDis (tmpo,pnt);
    if (tmpvar<minsofar) {
      minsofar=tmpvar;
    }	
  }
  return minsofar;
}




bool Unit::querySphere (const Vector &pnt, float err) {
  UpdateMatrix();
  int i;
  Vector TargetPoint (transformation[0],transformation[1],transformation[2]);

#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint)); //the scale factor of the current UNIT
#endif
  for (i=0;i<nummesh;i++) {

    TargetPoint = Transform(transformation,meshdata[i]->Position())-pnt;
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
    if (subunits[i]->querySphere (transformation,pnt,err))
      return true;
  }
  return false;
}

bool Unit::querySphere (Matrix t,const Vector &pnt, float err) {
  UpdateMatrix();
  int i;
  Matrix tmpo;
  MultMatrix (tmpo,t,transformation);
  
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
    if (subunits[i]->querySphere (tmpo,pnt,err))
      return true;
  }
  return false;
}

bool Unit::queryBoundingBox (const Vector &pnt, float err) {
  UpdateMatrix();
  int i;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (transformation);
    if (bbox->Within(pnt,err)) {
      delete bbox;
      return true;
    }
    delete bbox;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->subunits[i]->queryBoundingBox (transformation,pnt,err)) 
      return true;
  }
  return false;
}

bool Unit::queryBoundingBox (Matrix t,const Vector &pnt, float err) {
  int i;
  Matrix tmpo;
  MultMatrix (tmpo,t, transformation);
  BoundingBox *bbox=0;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (tmpo);
    if (bbox->Within(pnt,err)){
      delete bbox;
      return true;
    }
    delete bbox;
  }
  if (numsubunit>0) {
    for (i=0;i<numsubunit;i++) {
      if (subunits[i]->queryBoundingBox (tmpo,pnt,err))
	return true;
    }
  }
  return false;

}

int Unit::queryBoundingBox (const Vector &origin, const Vector &direction, float err) {
  UpdateMatrix();
  int i;
  int retval=0;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (transformation);
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
    switch (subunits[i]->queryBoundingBox (transformation,origin,direction,err)) {
    case 1: return 1;
    case -1: retval= -1;
      break;
    case 0: break;
    }
  }
  return retval;
}

int Unit::queryBoundingBox (Matrix t,const Vector &eye, const Vector &pnt, float err) {
  int i;
  int retval=0;
  Matrix tmpo;
  MultMatrix (tmpo,t, transformation);
  BoundingBox *bbox=0;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (tmpo);
    switch (bbox->Intersect(eye,pnt,err)){
    case 1: 
      delete bbox;
      return 1;
    case -1: 
      delete bbox;
      retval= -1;
      break;
    case 0: 
      delete bbox;
      break;
    default:
      delete bbox;
    }
  }
  for (i=0;i<numsubunit;i++) {
    switch (subunits[i]->queryBoundingBox (tmpo,eye,pnt,err)) {
    case 1:return 1;
    case -1:retval=-1;
      break;
    case 0: break;
    }
  }	
  return retval;

}

bool Unit::queryFrustum(float frustum [6][4]) {
  int i;
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (transformation[0],transformation[1],transformation[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif
  for (i=0;i<nummesh;i++) {
        TargetPoint = Transform(transformation,meshdata[i]->Position());
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
    if (subunits[i]->queryFrustum(transformation,frustum))
      return true;
  }
  return false;
}


bool Unit::queryFrustum(Matrix t, float frustum [6][4]) {

  Matrix tmpo;
  MultMatrix (tmpo,t,transformation);
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (tmpo[0],tmpo[1],tmpo[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif
  int i;
  for (i=0;i<nummesh;i++) {
        TargetPoint = Transform(tmpo,meshdata[i]->Position());
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
    if (subunits[i]->queryFrustum(tmpo,frustum))
      return true;
  }
  return false;
}

Vector MouseCoordinate (int x, int y, float zplane) {

  Vector xyz = Vector (0,0,zplane);
  //first get xyz in camera space
  xyz.i=zplane*(2.*x/g_game.x_resolution-1) /*  *g_game.MouseSensitivityX*/  *GFXGetXInvPerspective();
  xyz.j=zplane*(-2.*y/g_game.y_resolution+1) /*  *g_game.MouseSensitivityY*/  *GFXGetYInvPerspective();
  return xyz;
}

bool Unit::querySphere (int mouseX, int mouseY, float err, Camera * activeCam) {
  UpdateMatrix();
  int i;
  Matrix vw;
  GFXGetMatrix (VIEW,vw);
  Vector mousePoint;
#ifdef VARIABLE_LENGTH_PQR
  Vector TargetPoint (transformation[0],transformation[1],transformation[2]);
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#else
  Vector TargetPoint;
#endif

  Vector CamP,CamQ,CamR;
  for (i=0;i<nummesh;i++) {
 
    TargetPoint = Transform(transformation,meshdata[i]->Position());
    mousePoint = Transform (vw,TargetPoint);
    if (mousePoint.k>0) { //z coordinate reversed  -  is in front of camera
      continue;
    }
    mousePoint = MouseCoordinate (mouseX,mouseY,-mousePoint.k);
    
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
    if (subunits[i]->querySphere (transformation,mouseX,mouseY,err,activeCam,vw))
      return true;
  }
  return false;
}

bool Unit::querySphere (Matrix t,int mouseX, int mouseY, float err, Camera * activeCam, Matrix vw) {
  UpdateMatrix();
  int i;
  Matrix tmpo;
  MultMatrix (tmpo,t,transformation);
  Vector mousePoint;
  Vector TargetPoint(tmpo[0],tmpo[1],tmpo[2]);//adjusts the mesh size by the relative size of the current unit
#ifdef VARIABLE_LENGTH_PQR
  float SizeScaleFactor = sqrtf(TargetPoint.Dot(TargetPoint));
#endif
  Vector CamP,CamQ,CamR;
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform (tmpo,meshdata[i]->Position());
    mousePoint = Transform (vw,TargetPoint);
    if (mousePoint.k>0)
      continue;
    mousePoint = MouseCoordinate (mouseX,mouseY,-mousePoint.k);
    activeCam->GetPQR(CamP,CamQ,CamR);
    mousePoint = Transform (CamP,CamQ,CamR,mousePoint);	
    activeCam->GetPosition(CamP);    
    mousePoint +=CamP; 

    TargetPoint = TargetPoint-mousePoint;
    if (TargetPoint.Dot (TargetPoint)< err*err+meshdata[i]->rSize()*meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
	*SizeScaleFactor*SizeScaleFactor 
#endif
	+ 
#ifdef VARIABLE_LENGTH_PQR
	SizeScaleFactor* 
#endif
	err*2*meshdata[i]->rSize()
	)
      return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->querySphere (tmpo,mouseX,mouseY,err,activeCam,vw))
      return true;
  }
  return false;
}

void Unit::Draw()
{
	//glMatrixMode(GL_MODELVIEW);
	
	/*
	if(changed)
		MultMatrix(transformation, translation, orientation);
	glMultMatrixf(transformation);
	*/
	time += GetElapsedTime();
	UpdateMatrix();
	Vector np = Transform(pp,pq,pr,p), 
		nq = Transform(pp,pq,pr,q),
		nr = Transform(pp,pq,pr,r),
		npos = ppos+pos;

	Matrix currentMatrix;
	GFXGetMatrix(MODEL, currentMatrix);
	  
#ifdef VARIABLE_LENGTH_PQR
	  Vector MeshCenter (currentMatrix[0],currentMatrix[1],currentMatrix[2]); 
	  float SizeScaleFactor=sqrtf (MeshCenter.Dot(MeshCenter));
#else
	  Vector MeshCenter;
#endif

	for (int i=0;i<nummesh;i++) {
	  //meshdata[i]->Draw(np, nq, nr, npos);
	  /////////already completed if the camera was changedGFXCalculateFrustum(); Moved into update camera
	  ;
	  float d = GFXSphereInFrustum(Transform (currentMatrix,
						  meshdata[i]->Position()),
				       meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				       *SizeScaleFactor
#endif 
				       );
	  if (d) {  //d can be used for level of detail shit
	    meshdata[i]->Draw();
	  }else {

	  }
	  GFXLoadMatrix(MODEL, currentMatrix); // not a problem with overhead if the mesh count is kept down
	}
	for(int subcount = 0; subcount < numsubunit; subcount++) {
		subunits[subcount]->Draw(tmatrix, np, nq, nr, npos);
	  GFXLoadMatrix(MODEL, currentMatrix); // not a problem with overhead if the mesh count is kept down
	}
}

void Unit::DrawStreak(const Vector &v)
{
	Vector v1 = v;
	int steps = (int)v.Magnitude()*10;
	v1 = v1 * (1.0/steps);
	Vector opos = pos;
	GFXColor(0.5, 0.5, 0.5, 0.5);
	for(int a = 0; a < steps; a++) {
		Draw();
		pos+=v1;
	}
	GFXColor(1.0, 1.0, 1.0, 1.0);
	pos = opos;
}

void Unit::Draw(Matrix tmatrix)
{
	CopyMatrix(this->tmatrix, tmatrix);
	Draw();
}

void Unit::Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos)
{
	//Matrix orientation;
	//Matrix translation;
	//Matrix tmatrix;

	this->pp = pp;
	this->pq = pq;
	this->pr = pr;
	this->ppos = ppos;

	/*
	  VectorToMatrix(orientation, pp,pq,pr);
	  Translate(translation, ppos.i,ppos.j,ppos.k);
	  
	  MultMatrix(tmatrix, translation, orientation);
	
	  CopyMatrix(this->tmatrix, tmatrix);
	*/
	Draw();
}

void Unit::SetAI(AI *newAI)
{
	if(aistate)
		delete aistate;
	aistate = newAI;
	aistate->SetParent(this);
}

void Unit::ExecuteAI() {
  if(aistate) aistate->Execute();
  for(int a=0; a<numsubunit; a++) {
    subunits[a]->ExecuteAI();
  }
}
