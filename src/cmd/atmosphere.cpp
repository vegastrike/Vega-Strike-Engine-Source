#include "gfx/mesh.h"
#include "atmosphere.h"
#include "vegastrike.h"
#include "star_system.h"
#include "gfx/sphere.h"
#include "gfx/matrix.h"
#include "cmd/unit.h"
#include "cmd/planet.h"
#include "gfxlib.h"

int divisions = 16;
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
			float rho = direction * TransformNormal(tmatrix,Vector(0,1,0));
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
		float rho=acos(rho1)/(PI/2)-0.1;
		float radius = user_params.radius;
		/* index 0 is the top color, index 1 is the bottom color */
		GFXLight light0 = GFXLight();
		setArray(light0.ambient, rho*user_params.high_ambient_color[0] + (1-rho)*user_params.low_ambient_color[0]);
		setArray(light0.diffuse, rho*user_params.high_color[0] + (1-rho)*user_params.low_color[0]);
		setArray(light0.attenuate, GFXColor(0,0.5,0));
		setArray(light0.vect, GFXColor(0,1.1*radius,0,1));

		/* do a linear interpolation between this and the next one */
		
		GFXLight light1 = GFXLight();
		setArray(light1.ambient, (1-rho)*user_params.high_ambient_color[1] + rho*user_params.low_ambient_color[1]);
		setArray(light1.diffuse, (1-rho)*user_params.high_color[1] + rho*user_params.low_color[1]);
		setArray(light1.attenuate, GFXColor(0,0,0.5));
		setArray(light1.vect, GFXColor(0,-1.1*radius,0,1));

		/* Note!! make sure that this light never goes too far around the sphere */
		GFXLight light2 = light1; /* -80 degree declination from sun position */
		Matrix m;
		Vector r;
		ScaledCrossProduct(Vector(0,1,0),localDir,r);
		Rotate(m,r,-80*(PI/180));
		r = Transform(m,Vector(0,0,1));
		float sradius = 1.1 * radius;
		setArray(light2.vect, GFXColor(sradius * r.i,sradius * r.j,sradius * r.k,1));

		//GFXCreateLight(l0,light0,true);
		//GFXCreateLight(l1,light1,true);
		//GFXCreateLight(l2,light2,true);
	}
}

void Atmosphere::Draw(const Vector &position, const Matrix tmatrix)
{
  GFXDisable (TEXTURE1);
	Update(position,tmatrix);
	GFXLoadMatrix(MODEL,tmatrix);

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
	GFXMaterial a = {0,0,0,0,
					.01,.01,.01,1,
					0,0,0,0,
					0.5,0.5,0.5,1,
					0};
	GFXDisable(DEPTHWRITE);
	dome->DrawNow(10,GFXTRUE,identity_transformation,rot1);
	//GFXEnable(DEPTHWRITE);

	//GFXDeleteLight(l0);
	//GFXDeleteLight(l1);
	//GFXDeleteLight(l2);
}

void Atmosphere::DrawAtmospheres()
{
	abort();
}
