#include <math.h>
#include "physics.h"
#include "planet.h"
#include "gfxlib.h"
#include "gfx_sphere.h"

#include "UnitCollection.h"

AI *PlanetaryOrbit::Execute() {
  theta += angular_delta;
  parent->SetPosition(radius * cos(theta), radius * sin(theta), 0);
  return this;
}

void Planet::InitPlanet(FILE *fp) {
  InitUnit();
  satellites = NULL;
  numSatellites = 0;
  calculatePhysics=false;

  double orbital_radius, orbital_velocity, orbital_position;
  char texname[255];
  int a;

  fscanf(fp, "%s\n", texname);
  fscanf(fp, "%f\n", &radius);
  fscanf(fp, "%f\n", &gravity);
  fscanf(fp, "%lf %lf %lf\n", &orbital_radius, &orbital_velocity, &orbital_position);
  SetAI(new PlanetaryOrbit(orbital_radius, orbital_velocity, orbital_position));

  fscanf(fp, "%d\n", &numSatellites);
  satellites = new Planet*[numSatellites];

  for(a=0; a<numSatellites; a++) {
    satellites[a] = new Planet();
    satellites[a]->InitPlanet(fp);
  }
  
  meshdata = new Mesh*[1];
  meshdata[0] = new SphereMesh(radius, 8, 8, texname);
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

  Iterator *iterator = uc->createIterator();
  Unit *unit;
  Vector vec(0,0,0);
  while((unit = iterator->current())!=NULL) {
    if(unit->queryCalculatePhysics()) {
    Vector r = (unit->Position() - (vec.Transform(t)));
    //cerr << "Gravity source: " << vec.Transform(t) << "\nUnit position: " << unit->Position() << "\nDelta: " << r << endl;
    float _r_ = r.Magnitude();
    r = r * (1.0/_r_);
    r =  r * -(gravity/(_r_*_r_));
    //cerr << "Distance: " << _r_ << "\nGravity force vector: " << r << endl;

    if(_r_ > radius) {
      unit->Accelerate(r);
    }
    }
    iterator->advance();
  }
  delete iterator;

  for(int a=0; a<numSatellites; a++) {
    satellites[a]->gravitate(uc, t);
    satellites[a]->origin = origin + pos;
  }
}

void Planet::Draw() {
  Matrix tmat;
  Translate(tmat, origin);

  GFXMultMatrix(MODEL, tmat);
  Unit::Draw();
}

void Planet::Draw(Matrix tmatrix) {abort();}
void Planet::DrawStreak(const Vector &v) {abort();}
void Planet::Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos) {abort();}

void Planet::gravitate(UnitCollection *uc) {
  Matrix t;
  Identity(t);
  gravitate(uc, t);
}
