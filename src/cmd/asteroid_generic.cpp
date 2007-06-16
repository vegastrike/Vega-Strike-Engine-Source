#include "cmd/asteroid_generic.h"
#include "cmd/collection.h"
#include "gfx/vec.h"
#include "cmd/script/flightgroup.h"
#include "cmd/unit_generic.h"

static void RecursiveSetSchedule(Unit *un)
{
	if (un) {
		if (un->SubUnits.empty())
			un->schedule_priority = Unit::scheduleRoid;
		else {
			un->schedule_priority = Unit::scheduleAField;
			un->do_subunit_scheduling = true;
			for (un_iter it=un->getSubUnits(); it.notDone(); ++it)
				RecursiveSetSchedule(*it);
		}
	}
}


void Asteroid::Init(float difficulty)
{
	asteroid_physics_offset=0;
	un_iter iter = getSubUnits();
	while(*iter) {
		float x=2*difficulty*((float)rand())/RAND_MAX-difficulty;
		float y=2*difficulty*((float)rand())/RAND_MAX-difficulty;
		float z=2*difficulty*((float)rand())/RAND_MAX-difficulty;
		(*iter)->SetAngularVelocity(Vector(x,y,z));
		++iter;
	}
	RecursiveSetSchedule(this);
}


void Asteroid::reactToCollision(Unit * smaller, const QVector& biglocation, const Vector& bignormal, const QVector& smalllocation, const Vector& smallnormal, float dist)
{
	switch (smaller->isUnit()) {
		case ASTEROIDPTR:
		case ENHANCEMENTPTR:
			break;
		case NEBULAPTR:
			smaller->reactToCollision(this,smalllocation,smallnormal,biglocation,bignormal,dist);
			break;
		default:
			/***** DOES THAT STILL WORK WITH UNIT:: ?????????? *******/
			Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
			break;
	}
}


Asteroid::Asteroid(const char * filename, int faction, Flightgroup* fg, int fg_snumber, float difficulty):Unit (filename,false, faction,string(""),fg,fg_snumber)
{
	Init( difficulty);
}
