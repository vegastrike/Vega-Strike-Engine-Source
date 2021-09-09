/**
* unit_util.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __UNIT_UTIL_H__
#define __UNIT_UTIL_H__
#include <string>
#include "unit_generic.h"
#include "images.h"
using std::string;

namespace UnitUtil {
	void setMissionRelevant(Unit* my_unit);
	void unsetMissionRelevant(Unit* my_unit);
	string getFactionName (const Unit *my_unit);
	int getFactionIndex (const Unit *my_unit);
	void setFactionIndex (Unit *my_unit,int factionname);
	void RecomputeUnitUpgrades(Unit *my_unit);
	void setFactionName (Unit *my_unit,string factionname);
	float getFactionRelation (const Unit *my_unit, const Unit *their_unit);
	float getRelationToFaction (const Unit *my_unit, int other_faction);
	float getRelationFromFaction (const Unit *their_unit, int my_faction);
	string getName(const Unit *my_unit);
	void setName(Unit *my_unit,string name);
	void SetHull(Unit *my_unit,float hull);
    string getFlightgroupName(const Unit *my_unit);
	const string& getFlightgroupNameCR(const Unit *my_unit);
	Unit *getFlightgroupLeader (Unit *my_unit);
	void orbit (Unit * my_unit, Unit * orbitee, float speed, QVector R, QVector S, QVector center); 
	bool setFlightgroupLeader (Unit *my_unit, Unit *un);
	string getFgDirective(const Unit *my_unit);
	bool setFgDirective(Unit *my_unit,string inp);
	int getPhysicsPriority(Unit * un);
	int getFgSubnumber(const Unit *my_unit);
	int removeCargo(Unit *my_unit,string s, int quantity, bool erasezero);
        bool repair (Unit * my_unit);
        int removeWeapon(Unit *my_unit, string weapon,int mountoffset,bool loop_through_mounts); // -1 tells no weapon removed
	float upgrade(Unit *my_unit, string file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts);
	int addCargo (Unit *my_unit,Cargo carg);
	int forceAddCargo (Unit *my_unit,Cargo carg);
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity);
	bool decrementCargo(Unit *my_unit,float percentagechange);
	float getDistance(const Unit *my_unit,const Unit *un);
	float getSignificantDistance (const Unit *un, const Unit *sig);
	int hasCargo (const Unit * my_unit, std::string mycarg);
	Cargo GetCargoIndex (const Unit * my_unit, int index);
	Cargo GetCargo (const Unit *my_unit, std::string cargname);
	string getUnitSystemFile (const Unit * my_unit);
	float getCredits(const Unit *my_unit);
	void addCredits(const Unit *my_unit,float credits);
	bool isSignificant(const Unit *my_unit);
        bool isCloseEnoughToDock(const Unit *my_unit, const Unit *un);
	bool isCapitalShip(const Unit *my_unit);
	bool isDockableUnit(const Unit *my_unit);
	bool isAsteroid(const Unit *my_unit);
	bool isSun(const Unit *my_unit);
	void switchFg(Unit *my_unit,string arg);
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood);
	bool commAnimation(Unit *my_unit,string anim);
	bool JumpTo (Unit * unit, string system);
	int isPlayerStarship (const Unit * un);
	void setECM (Unit * un, int NewECM);  //short fix
	int getECM (const Unit * un); //short fix
	void setSpeed (Unit * un, float speed);
	Unit *owner (const Unit *un);
	float maxSpeed (const Unit *un);
	float maxAfterburnerSpeed (const Unit *un);
	void performDockingOperations (Unit * un, Unit * unitToDockWith,int actuallyDockP);
        float PercentOperational(Unit * un, string,string category,bool countHullAndArmorAsFull);
}

#endif
