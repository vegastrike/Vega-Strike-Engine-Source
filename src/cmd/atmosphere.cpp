#include "gfx/mesh.h"
#include "atmosphere.h"
#include "vegastrike.h"
#include "star_system.h"

#include "gfx/matrix.h"
#include "cmd/unit.h"
#include "cmd/planet.h"
#include "gfxlib.h"
#include "gfx/sphere.h"
int l0,l1,l2;

void Atmosphere::Update(const QVector &position, const Matrix &tmatrix)
{
	int a;
	Planet *currPlanet;
	StarSystem *system = _Universe->activeStarSystem();

	for(a=0; a<(int)sunboxes.size(); a++) {
		delete sunboxes[a];
	}
	sunboxes.clear();
	QVector localDir;
	float rho1;
	UnitCollection::UnitIterator iter (system->getUnitList().createIterator());
	Unit * primary;

	for(;NULL!=(primary=iter.current());iter.advance()) {
		if(primary->isUnit()==PLANETPTR && 
			(currPlanet = (GamePlanet*)primary)->hasLights()) {
			//const std::vector <int> & lights = currPlanet->activeLights();
			/* for now just assume all planets with lights are really bright */
			QVector direction = (currPlanet->Position()-position);
			direction.Normalize();
			double rho = direction * InvTransformNormal(tmatrix,QVector(0,1,0));
			if(rho > 0) { /* above the horizon */
				QVector localDirection = InvTransformNormal(tmatrix,direction);
				
				
				localDir = localDirection; /* bad */
				rho1=rho;

				/* need a function for the sunbox size. for now, say it takes up a quarter
				   of the screen */
				/* drop the z value and find the theta */
				QVector lprime = localDirection;
				lprime.k = 0;
				lprime.Normalize();
				//float theta = atan2(lprime.i,lprime.j);
				//float size = .125;
				sunboxes.push_back(new SunBox(NULL /*
					new SphereMesh(user_params.radius, divisions, divisions, "", 
					NULL,true,ONE,ZERO,false,
					rho-size,rho+size,theta-size,theta+size)*/));
				break;
			}
		}
	}
	if(!sunboxes.empty()) {
		float rho=acos(rho1)/(PI/2);
		float radius = user_params.radius;
		/* index 0 is the top color, index 1 is the bottom color */
		GFXLight light0 = GFXLight();
		light0.SetProperties(AMBIENT,rho*user_params.high_ambient_color[0] + (1-rho)*user_params.low_ambient_color[0]);
		light0.SetProperties(DIFFUSE,rho*user_params.high_color[0] + (1-rho)*user_params.low_color[0]);
		light0.SetProperties(ATTENUATE,0.5*GFXColor(1,0.25/radius,0));
		light0.SetProperties(POSITION,GFXColor(0,1.1*radius,0,1));

		/* do a linear interpolation between this and the next one */
		
		GFXLight light1 = GFXLight();
		light1.SetProperties(AMBIENT, (1-rho)*user_params.high_ambient_color[1] + rho*user_params.low_ambient_color[1]);
		light1.SetProperties(DIFFUSE, (1-rho)*user_params.high_color[1] + rho*user_params.low_color[1]);
		light1.SetProperties(ATTENUATE, 0.5*GFXColor(1,0.75/radius,0));
		light1.SetProperties(POSITION, GFXColor(0,-1.1*radius,0,1));

		/* Note!! make sure that this light never goes too far around the sphere */
		GFXLight light2 = light1; /* -80 degree declination from sun position */
		Matrix m;
		QVector r;
		ScaledCrossProduct(QVector(0,1,0),localDir,r);
		Rotate(m,r.Cast(),-80*(PI/180));
		r = Transform(m,QVector(0,0,1));
		float sradius = 1.1 * radius;
		light2.SetProperties(POSITION,GFXColor(sradius * r.i,sradius * r.j,sradius * r.k,1));

		GFXCreateLight(l0,light0,true);
		GFXCreateLight(l1,light1,true);
		//GFXCreateLight(l2,light2,true);
		GFXEnableLight(l0);
		GFXEnableLight(l1);
		//GFXEnableLight(l2);
	}
}
static std::vector <Atmosphere *> draw_queue;
void Atmosphere::SetMatricesAndDraw(const QVector &pos, const Matrix mat) {
  CopyMatrix (tmatrix,mat);
  position =pos;
  draw_queue.push_back (this);
}

void Atmosphere::ProcessDrawQueue () {
  GFXEnable (LIGHTING);
  GFXDisable (TEXTURE1);
  GFXDisable (TEXTURE0);
  GFXDisable (DEPTHWRITE);
  
  while (!draw_queue.empty()) {
    draw_queue.back()->Draw();
    draw_queue.pop_back();
  }
}
void Atmosphere::Draw()
{
  GFXDisable (TEXTURE1);
	/*
[ 1 0 0 0 ]
[ 0 0 1 0 ]
[ 0 -1 0 0 ]
*/
	Matrix rot( 1, 0, 0,
		0, 0, -1,
		0, 1, 0,
		QVector (0, 0, 0));
	Matrix rot1;
	MultMatrix(rot1,tmatrix,rot);
	CopyMatrix (rot1,tmatrix);

 	Vector tmp(rot1.getR());
	Vector tmp2(rot1.getQ());

	rot1.r[6]=-tmp.i;
	rot1.r[7]=-tmp.j;
	rot1.r[8]=-tmp.k;

	rot1.r[3]=-tmp2.i;
	rot1.r[4]=-tmp2.j;
	rot1.r[5]=-tmp2.k;

	GFXMaterial a = {0,0,0,0,
					1,1,1,1,
					0,0,0,0,
					0,0,0,0,
					0};
	dome->SetMaterial(a);
    GFXLoadMatrixModel (rot1);
	Update(position,rot1);

	GFXDisable(DEPTHWRITE);
	dome->DrawNow(100000,GFXFALSE,rot1);
	GFXDisableLight (l0);
	GFXDisableLight (l1);
	GFXDeleteLight(l0);
	GFXDeleteLight(l1);
	//GFXDeleteLight(l2);
}

void Atmosphere::DrawAtmospheres()
{
	abort();
}
