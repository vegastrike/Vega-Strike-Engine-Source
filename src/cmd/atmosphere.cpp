#include "gfx/mesh.h"
#include "atmosphere.h"
#include "vegastrike.h"
#include "star_system.h"
#include "gfx/sphere.h"
#include "gfx/matrix.h"
#include "cmd/unit.h"
#include "cmd/planet.h"
#include "gfxlib.h"

int divisions = 64;
int l0,l1,l2;

void setArray(float c0[4], const GFXColor&c1) {
	c0[0]=c1.r;
	c0[1]=c1.g;
	c0[2]=c1.b;
	c0[3]=c1.a;
}
void setArray1(float c0[3], const GFXColor&c1) {
	c0[0]=c1.r;
	c0[1]=c1.g;
	c0[2]=c1.b;
}

Atmosphere::Atmosphere(const Parameters &params) : user_params(params) {
	dome = new SphereMesh(params.radius, divisions, divisions, "white.bmp", 
					NULL,true,ONE,ZERO,false,
					0,M_PI/2);
}

Atmosphere::~Atmosphere() {
	int a;

	for(a=0; a<sunboxes.size(); a++) {
		delete sunboxes[a];
	}
}

const Atmosphere::Parameters &Atmosphere::parameters()
{
	return user_params;
}

void Atmosphere::SetParameters(const Parameters &params)
{
	user_params = params;
}

void Atmosphere::Update(const Vector &position, const Matrix tmatrix)
{
	int a;
	Planet *currPlanet;
	StarSystem *system = _Universe->activeStarSystem();

	for(a=0; a<(int)sunboxes.size(); a++) {
		delete sunboxes[a];
	}
	sunboxes.clear();
	Vector localDir;
	float rho1;
	for(a=0;a<system->numprimaries;a++) {
		if(system->primaries[a]->isUnit()==PLANETPTR && 
			(currPlanet = (Planet*)system->primaries[a])->hasLights()) {
			const std::vector <int> & lights = currPlanet->activeLights();
			/* for now just assume all planets with lights are really bright */
			Vector direction = (currPlanet->Position()-position);
			direction.Normalize();
			float rho = direction * InvTransformNormal(tmatrix,Vector(0,1,0));
			if(rho > 0) { /* above the horizon */
				Vector localDirection = InvTransformNormal(tmatrix,direction);
				
				
				localDir = localDirection; /* bad */
				rho1=rho;

				/* need a function for the sunbox size. for now, say it takes up a quarter
				   of the screen */
				/* drop the z value and find the theta */
				Vector lprime = localDirection;
				lprime.k = 0;
				lprime.Normalize();
				float theta = atan2(lprime.i,lprime.j);
				float size = .125;
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
		Vector r;
		ScaledCrossProduct(Vector(0,1,0),localDir,r);
		Rotate(m,r,-80*(PI/180));
		r = Transform(m,Vector(0,0,1));
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
void Atmosphere::SetMatricesAndDraw(const Vector &pos, const Matrix mat) {
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
	Matrix rot = { 1, 0, 0, 0,
		0, 0, -1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1};
	Matrix rot1;
	MultMatrix(rot1,tmatrix,rot);
	CopyMatrix (rot1,tmatrix);

	Vector tmp(rot1[8],rot1[9],rot1[10]);
	Vector tmp2(rot1[4],rot1[5],rot1[6]);

	rot1[8]=-tmp.i;
	rot1[9]=-tmp.j;
	rot1[10]=-tmp.k;

	rot1[4]=-tmp2.i;
	rot1[5]=-tmp2.j;
	rot1[6]=-tmp2.k;

	GFXMaterial a = {0,0,0,0,
					1,1,1,1,
					0,0,0,0,
					0,0,0,0,
					0};
	dome->SetMaterial(a);
    GFXLoadMatrix(MODEL,rot1);
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
