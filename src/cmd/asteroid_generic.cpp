#include "cmd/asteroid_generic.h"

void Asteroid::Init(float difficulty)
{
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit *unit;
  while((unit = iter.current())!=NULL) {
    float x=2*difficulty*((float)rand())/RAND_MAX -difficulty;
    float y=2*difficulty*((float)rand())/RAND_MAX-difficulty;
    float z=2*difficulty*((float)rand())/RAND_MAX-difficulty;
    unit->SetAngularVelocity(Vector(x,y,z));
    iter.advance();
  }
}

void Asteroid::reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist)
{
  Unit * t=this;
  switch (smaller->isUnit()) {
  case ASTEROIDPTR:
  case ENHANCEMENTPTR:
    break;
  case NEBULAPTR:
    smaller->reactToCollision(t,smalllocation,smallnormal,biglocation,bignormal,dist);
    break;
  default:
	/***** DOES THAT STILL WORK WITH UNIT:: ?????????? *******/
    Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
    break;
  }
}
Asteroid::Asteroid(const char * filename, int faction, Flightgroup* fg, int fg_snumber, float difficulty):Unit (filename,false, faction,string(""),fg,fg_snumber) {
	this->Init( difficulty);
  }
