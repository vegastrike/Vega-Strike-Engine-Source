#ifndef __UNIT_UTIL_H__
#define __UNIT_UTIL_H__
#include <string>
#include "images.h"
using std::string;

class Unit;
namespace UnitUtil {
	string getFactionName (Unit *my_unit);
	int getFactionIndex (Unit *my_unit);
	void setFactionIndex (Unit *my_unit,int factionname);
	void setFactionName (Unit *my_unit,string factionname);
	string getName(Unit *my_unit);
	float getCredits(Unit *my_unit);
	void addCredits(Unit *my_unit,float credits);
    string getFlightgroupName(Unit *my_unit);
	Unit *getFlightgroupLeader (Unit *my_unit);
	bool setFlightgroupLeader (Unit *my_unit, Unit *un);
	string getFgDirective(Unit *my_unit);
	bool setFgDirective(Unit *my_unit,string inp);
	int getFgSubnumber(Unit *my_unit);
	bool isSignificant(Unit *my_unit);
	bool isSun(Unit *my_unit);
	void switchFg(Unit *my_unit,string arg);
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood);
	bool commAnimation(Unit *my_unit,string anim);
	int removeCargo(Unit *my_unit,string s, int quantity, bool erasezero);
	float upgrade(Unit *my_unit, string file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts);
	int addCargo (Unit *my_unit,Cargo carg);
	bool JumpTo (Unit * unit, string system);
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity);
	bool decrementCargo(Unit *my_unit,float percentagechange);
	float getDistance(Unit *my_unit,Unit *un);
	float getPlanetRadiusPercent ();
	float getSignificantDistance (Unit *un, Unit *sig);
	int isPlayerStarship (Unit * un);
	int hasCargo (Unit * my_unit, std::string mycarg);

}



#endif
