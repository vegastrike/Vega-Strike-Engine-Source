#include <math.h>
#include "physics.h"
#include "planet.h"
#include "gfxlib.h"
#include "gfx_sphere.h"

AI *PlanetaryOrbit::Execute() {
  theta += angular_delta;
  parent->SetPosition(radius * cos(theta), radius * sin(theta), 0);
  return this;
}

/* Move this into a Sphere class!!! */

Planet::Planet(FILE *fp) : Unit(), radius(0.0f) {
  InitUnit();

  double orbital_radius, orbital_velocity, orbital_position;
  char texname[255];
  int a;

  fscanf(fp, "%s\n", texname);
  fscanf(fp, "%f\n", &radius);
  fscanf(fp, "%lf %lf %lf\n", &orbital_radius, &orbital_velocity, &orbital_position);
  SetAI(new PlanetaryOrbit(orbital_radius, orbital_velocity, orbital_position));

  fscanf(fp, "%d\n", &numsubunit);
  subunits = new Unit*[numsubunit];

  for(a=0; a<numsubunit; a++) {
    subunits[a] = new Planet(fp);
  }
  
  meshdata = new Mesh*[1];
  meshdata[0] = new SphereMesh(radius, 8, 8, texname);
  nummesh = 1;
  fpos = ftell(fp);
}

Planet::Planet(char *filename) : Unit(), radius(0.0f) {
  InitUnit();

  FILE *fp = fopen(filename, "r");
  *this = *(new Planet(fp)); // Take this out!!!
  SetAI(new AI()); // no behavior
  cerr << "\nPlanet: " << this << endl;
  fclose(fp);
}

Planet::~Planet() { }
