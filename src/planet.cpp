#include <math.h>
#include "physics.h"
#include "planet.h"
#include "gfxlib.h"
#include "gfx_sphere.h"

#include "UnitCollection.h"

AI *PlanetaryOrbit::Execute() {
  Vector x_offset = cos(theta) * x_size;
  Vector y_offset = sin(theta) * y_size;
  double radius =  sqrt((x_offset - focus).MagnitudeSquared() + (y_offset - focus).MagnitudeSquared());
  theta+=velocity/radius * SIMULATION_ATOM;
  
  parent->SetPosition(parent->origin - focus + 
		      x_offset + y_offset);
  return this;
}

void Planet::InitPlanet(FILE *fp) {
  InitUnit();
  envMap = FALSE;
  satellites = NULL;
  numSatellites = 0;
  calculatePhysics=false;

  float orbital_velocity, orbital_position;
  Vector x_axis;
  Vector y_axis;
  char texname[255];
  int a;

  fscanf(fp, "%s\n", texname);
  fscanf(fp, "%f\n", &radius);
  fscanf(fp, "%f\n", &gravity);
  fscanf(fp, "%f %f %f\n", &x_axis.i, &x_axis.j, &x_axis.k);
  fscanf(fp, "%f %f %f\n", &y_axis.i, &y_axis.j, &y_axis.k);
  fscanf(fp, "%f %f\n", &orbital_velocity, &orbital_position);
  SetAI(new PlanetaryOrbit(this, orbital_velocity, orbital_position, x_axis, y_axis));

  cerr << texname << " " << orbital_position << endl;

  fscanf(fp, "%d\n", &numSatellites);
  satellites = new Planet*[numSatellites];

  for(a=0; a<numSatellites; a++) {
    satellites[a] = new Planet();
    satellites[a]->InitPlanet(fp);
  }
  
  meshdata = new Mesh*[1];
  meshdata[0] = new SphereMesh(radius, 4, 4, texname);
  meshdata[0]->setEnvMap(FALSE);
  nummesh = 1;
  fpos = ftell(fp);
}

Planet::Planet()  : Unit(), radius(0.0f), origin(0,0,0), satellites(NULL), numSatellites(0) {
  InitUnit();

  SetAI(new AI()); // no behavior
}

Planet::Planet(char *filename) : Unit(), radius(0.0f), origin(0,0,0), satellites(NULL), numSatellites(0) {
  InitUnit();

  FILE *fp = fopen(filename, "r");
  InitPlanet(fp);
  SetAI(new AI()); // no behavior
  cerr << "\nPlanet: " << this << endl;
  fclose(fp);
}

Planet::~Planet() { }

void Planet::gravitate(UnitCollection *uc, Matrix matrix) {
  Matrix t;
  MultMatrix(t, matrix, transformation);

  if(gravity!=0.0) {
    Iterator *iterator = uc->createIterator();
    Unit *unit;
    Vector vec(0,0,0);
    
    while((unit = iterator->current())!=NULL) {
      if(unit->queryCalculatePhysics()) {
	Vector r = (unit->Position() - (vec.Transform(t)));
	//      cerr << "Unit (" << unit << "): " << endl;
	//      cerr << "Gravity source: " << vec.Transform(t) << "\nUnit position: " << unit->Position() << "\nDelta: " << r << endl;
	float _r_ = r.Magnitude();
	r = r * (1.0/_r_);
	r =  r * -(gravity/(_r_*_r_));
	//      cerr << "Distance: " << _r_ << "\nGravity force vector: " << r << endl;
      
	if(_r_ > radius) {
	  unit->Accelerate(r);
	}
      }
      iterator->advance();
    }
    delete iterator;
  }

  // fake gravity
  for(int a=0; a<numSatellites; a++) {
    satellites[a]->origin = origin + pos;
    satellites[a]->gravitate(uc, t);
  }
}

/*void Planet::Draw() {
  GFXMultMatrix(MODEL, tmat);
  Unit::Draw();
}
void Planet::Draw(Matrix tmatrix) {abort();}
void Planet::DrawStreak(const Vector &v) {abort();}
void Planet::Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos) {abort();}
*/

void Planet::gravitate(UnitCollection *uc) {
  Matrix t;
  Identity(t);
  gravitate(uc, t);
}
