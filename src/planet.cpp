#include <math.h>
#include "physics.h"
#include "planet.h"
#include "gfxlib.h"
#include "gfx_sphere.h"
#include "cmd_ai.h"
#include "UnitCollection.h"

AI *PlanetaryOrbit::Execute() {
  Vector x_offset = cos(theta) * x_size;
  Vector y_offset = sin(theta) * y_size;
  double radius =  sqrt((x_offset - focus).MagnitudeSquared() + (y_offset - focus).MagnitudeSquared());
  theta+=velocity/(radius?radius:1) * SIMULATION_ATOM;
  
  parent->prev_physical_state = parent->curr_physical_state;
  parent->curr_physical_state.position = (parent->origin - focus + 
		      x_offset + y_offset);
  return this;
}

/*void Planet::InitPlanet(FILE *fp) {
  Init();
  name = "Planet - ";
  satellites = NULL;
  numSatellites = 0;
  calculatePhysics=false;
  float orbital_velocity, orbital_position;
  Vector x_axis;
  Vector y_axis;
  char texname[255];
  int a;

  fscanf(fp, "%s\n", texname);
  name +=  texname;
  fscanf(fp, "%f\n", &radius);
  fscanf(fp, "%f\n", &gravity);
  fscanf(fp, "%f %f %f\n", &x_axis.i, &x_axis.j, &x_axis.k);
  fscanf(fp, "%f %f %f\n", &y_axis.i, &y_axis.j, &y_axis.k);
  fscanf(fp, "%f %f\n", &orbital_velocity, &orbital_position);
  SetAI(new PlanetaryOrbit(this, orbital_velocity, orbital_position, x_axis, y_axis));

  //  cerr << texname << " " << orbital_position << endl;

  fscanf(fp, "%d\n", &numSatellites);
  satellites = new Planet*[numSatellites];

  for(a=0; a<numSatellites; a++) {
    satellites[a] = new Planet();
    satellites[a]->InitPlanet(fp);
  }
  
  meshdata = new Mesh*[1];
  meshdata[0] = new SphereMesh(radius, 16, 16, texname);
  meshdata[0]->setEnvMap(GFXFALSE);
  nummesh = 1;
  fpos = ftell(fp);

  calculate_extent();
}
*/
void Planet::endElement() {  
}

void Planet::beginElement(Vector x,Vector y,float vely,float pos,float gravity,float radius,char * filename,int level,bool isunit){
  if (level>2) {
	  assert(numSatellites!=0);
	  if (satellites[numSatellites-1]->isUnit()==PLANETPTR) {
		((Planet *)satellites[numSatellites-1])->beginElement(x,y,vely,pos,gravity,radius,filename,level-1,isunit);
	  } else {
		  isUnit();
//		  ((Unit *)satellites[numSatellites-1])->Planet::beginElement(x,y,vely,pos,gravity,radius,filename,level-1,isunit);
	  }
	  return;	  
  }
  numSatellites++;
  if (numSatellites==1) {
	satellites = (Unit **) malloc (sizeof (Unit *));
  } else {
		satellites = (Unit **) realloc (satellites, numSatellites*sizeof (Unit *));
  }
  if (isunit==true) {
	satellites[numSatellites-1]=new Unit (filename, true);
	satellites[numSatellites-1]->SetAI (new PlanetaryOrbit (satellites[numSatellites-1],vely,pos,x,y)) ;
  }else
	satellites [numSatellites-1]=new Planet(x,y,vely,pos,gravity,radius,filename);
}

Planet::Planet()  : Unit(), radius(0.0f), satellites(NULL), numSatellites(0) {
  Init();

  SetAI(new AI()); // no behavior
}

Planet::Planet(Vector x,Vector y,float vely, float pos,float gravity,float radius,char * textname) : Unit(), radius(0.0f),  satellites(NULL), numSatellites(0) {
  numSatellites = 0;
  satellites = NULL;
  calculatePhysics=false;

  Init();
  killed=false;
  name = "Planet - ";
  name += textname;
  this->radius=radius;
  this->gravity=gravity;
//  FILE *fp = fopen(filename, "r");
//  InitPlanet(fp);
  SetAI(new PlanetaryOrbit(this, vely, pos, x, y)); // behavior
  //cerr << "\nPlanet: " << this << endl;
//  fclose(fp);
  meshdata = new Mesh*[1];
  meshdata[0] = new SphereMesh(radius, 16, 16, textname);
  meshdata[0]->setEnvMap(GFXFALSE);
  nummesh = 1;
  calculate_extent();

}

Planet::~Planet() { 
	for (int i=0;i<numSatellites;i++) {
		delete satellites[i];
	}
	if (numSatellites)
		free (satellites);
}

void Planet::gravitate(UnitCollection *uc) {
  float *t = cumulative_transformation_matrix;
  /*
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
    }*/

  // fake gravity
  for(int a=0; a<numSatellites; a++) {
    satellites[a]->origin =  curr_physical_state.position;
	if (satellites[a]->isUnit()==PLANETPTR) 
	    ((Planet *)satellites[a])->gravitate(uc);//FIXME 071201
  }
}
