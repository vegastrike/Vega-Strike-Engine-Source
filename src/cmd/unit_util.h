#ifndef __UNIT_UTIL_H__
#define __UNIT_UTIL_H__
#include <string>
#include "unit_generic.h"
#include "images.h"
using std::string;

namespace UnitUtil {
	string getFactionName (Unit *my_unit);
	int getFactionIndex (Unit *my_unit);
	void setFactionIndex (Unit *my_unit,int factionname);
	void setFactionName (Unit *my_unit,string factionname);
	string getName(Unit *my_unit);
	void setName(Unit *my_unit,string name);
	void SetHull(Unit *my_unit,float hull);
    string getFlightgroupName(Unit *my_unit);
	Unit *getFlightgroupLeader (Unit *my_unit);
	void orbit (Unit * my_unit, Unit * orbitee, float speed, QVector R, QVector S, QVector center); 
	bool setFlightgroupLeader (Unit *my_unit, Unit *un);
	string getFgDirective(Unit *my_unit);
	bool setFgDirective(Unit *my_unit,string inp);
	int getFgSubnumber(Unit *my_unit);
	int removeCargo(Unit *my_unit,string s, int quantity, bool erasezero);
        bool repair (Unit * my_unit);
	float upgrade(Unit *my_unit, string file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts);
	int addCargo (Unit *my_unit,Cargo carg);
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity);
	bool decrementCargo(Unit *my_unit,float percentagechange);
	float getDistance(Unit *my_unit,Unit *un);
	float getSignificantDistance (Unit *un, Unit *sig);
	int hasCargo (Unit * my_unit, std::string mycarg);
	Cargo GetCargoIndex (Unit * my_unit, int index);
	Cargo GetCargo (Unit *my_unit, std::string cargname);
	string getUnitSystemFile (Unit * my_unit);
	float getCredits(Unit *my_unit);
	void addCredits(Unit *my_unit,float credits);
	bool isSignificant(Unit *my_unit);
	bool isSun(Unit *my_unit);
	void switchFg(Unit *my_unit,string arg);
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood);
	bool commAnimation(Unit *my_unit,string anim);
	bool JumpTo (Unit * unit, string system);
	int isPlayerStarship (Unit * un);
	void setECM (Unit * un, short NewECM);
	short getECM (Unit * un);
	void setSpeed (Unit * un, float speed);
	Unit *owner (Unit *un);
	float maxSpeed (Unit *un);
	float maxAfterburnerSpeed (Unit *un);
	void performDockingOperations (Unit * un, Unit * unitToDockWith);
}

#endif
