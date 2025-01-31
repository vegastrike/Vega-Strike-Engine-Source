/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
// NO HEADER GUARD

#ifndef WHICH_UNIT_EXPORT_NUMBER
#define WHICH_UNIT_EXPORT_NUMBER 0
#endif //WHICH_UNIT_EXPORT_NUMBER

#if ((WHICH_UNIT_EXPORT_NUMBER == 3) || (WHICH_UNIT_EXPORT_NUMBER == 2))
#else //((WHICH_UNIT_EXPORT_NUMBER == 3) || (WHICH_UNIT_EXPORT_NUMBER == 2))
////////////////////////////AUTOMATIC WRAP//////
WRAPPED2(bool, AutoPilotTo, UnitWrapper, un, bool, ignore_energy_requirement,
false )
voidWRAPPED0( SetTurretAI )
voidWRAPPED0( DisableTurretAI )
voidWRAPPED3( leach,
float, XshieldPercent, float, YrechargePercent, float, ZenergyPercent )
WRAPPED0(int, getFgSubnumber,
-1 )
WRAPPED0( std::string, getFgID, std::string("")
)
voidWRAPPED1( setFullname, std::string, name
)
WRAPPED0( std::string, getFullname, std::string()
)
WRAPPED0( std::string, getFullAIDescription, std::string()
)
voidWRAPPED3( setTargetFg, std::string, primary, std::string, secondary, std::string, tertiary
)
voidWRAPPED1( ReTargetFg,
int, which_target )
WRAPPED0(bool, isStarShip,
false )
WRAPPED0(bool, isPlanet,
false )
WRAPPED0(bool, isJumppoint,
false )
WRAPPED1(bool, isEnemy, UnitWrapper, other,
false )
WRAPPED1(bool, isFriend, UnitWrapper, other,
false )
WRAPPED1(bool, isNeutral, UnitWrapper, other,
false )
WRAPPED1(float, getRelation, UnitWrapper, other,
0 )
voidWRAPPED1( ToggleWeapon,
bool, Missile )
voidWRAPPED1( SelectAllWeapon,
bool, Missile )
voidWRAPPED1( Split,
int, level )

voidWRAPPED0( Init )
voidWRAPPED1( ActivateJumpDrive,
int, destination )
voidWRAPPED0( DeactivateJumpDrive )
voidWRAPPED0( Destroy )
WRAPPED1( Vector, LocalCoordinates, UnitWrapper, un, Vector(0, 0, 0)
)
WRAPPED3(bool, InRange, UnitWrapper, target, bool, cone, bool, cap,
false )
WRAPPED0(float, CloakVisible,
false )
voidWRAPPED1( ActivateCloak,
bool, cloak )
voidWRAPPED0( RemoveFromSystem )
WRAPPED4(QVector,
        PositionITTS,
        QVector,
        local_posit,
        Vector,
        local_velocity,
float,
speed,
bool,
steadyITTS,
QVector(0, 0, 0))
WRAPPED0( QVector, Position, QVector(0, 0, 0)
)
WRAPPED0( QVector, LocalPosition, QVector(0, 0, 0)
)

WRAPPED0( UnitWrapper, Threat, UnitWrapper()
)
voidWRAPPED1( TargetTurret, UnitWrapper, targ
)
voidWRAPPED2( Threaten, UnitWrapper, targ,
float, danger )
voidWRAPPED0( ResetThreatLevel )
voidWRAPPED2( Fire,
unsigned int, Missile, bool, OnlyCollideWithTarget )
voidWRAPPED0( UnFire )
WRAPPED0(float, computeLockingPercent,
0 )  //how locked are we

#endif // ((WHICH_UNIT_EXPORT_NUMBER == 3) || (WHICH_UNIT_EXPORT_NUMBER == 2))

#if (WHICH_UNIT_EXPORT_NUMBER == 1) || (WHICH_UNIT_EXPORT_NUMBER == 3)
#else // (WHICH_UNIT_EXPORT_NUMBER == 1) || (WHICH_UNIT_EXPORT_NUMBER == 3)
WRAPPED1(double, getMinDis, QVector, pnt,
0 )  //for clicklist
WRAPPED3(float, querySphere, QVector, start, QVector, end, float, my_unit_radius,
0 )
voidWRAPPED0( PrimeOrders )
voidWRAPPED1( LoadAIScript, std::string, aiscript
)
WRAPPED0(bool, LoadLastPythonAIScript,
false )
WRAPPED0(bool, EnqueueLastPythonAIScript,
false )
voidWRAPPED1( SetPosition, QVector, pos
)
voidWRAPPED1( SetCurPosition, QVector, pos
)
voidWRAPPED1( SetPosAndCumPos, QVector, pos
)
voidWRAPPED1( Rotate, Vector, axis
)
voidWRAPPED1( ApplyForce, Vector, Vforce
)
voidWRAPPED1( ApplyLocalForce, Vector, Vforce
)
voidWRAPPED1( Accelerate, Vector, Vforce
)
voidWRAPPED2( ApplyTorque, Vector, Vforce, QVector, Location
)
voidWRAPPED2( ApplyBalancedLocalTorque, Vector, Vforce, Vector, Location
)
voidWRAPPED1( ApplyLocalTorque, Vector, torque
)
WRAPPED2(float, DealDamageToHull, Vector, pnt, float, Damage,
0 )
WRAPPED2( Vector, ClampThrust, Vector, thrust,
bool, afterburn, Vector( 0, 0, 0 ))
voidWRAPPED2( Thrust, Vector, amt,
bool, afterburn )
voidWRAPPED1( LateralThrust,
float, amt )
voidWRAPPED1( VerticalThrust,
float, amt )
voidWRAPPED1( LongitudinalThrust,
float, amt )
WRAPPED2( Vector, ClampVelocity, Vector, velocity,
bool, afterburn, Vector( 0, 0, 0 ))
WRAPPED1( Vector, ClampAngVel, Vector, vel, Vector(0, 0, 0)
)
WRAPPED1( Vector, ClampTorque, Vector, torque, Vector(0, 0, 0)
)
voidWRAPPED2( SetOrientation, QVector, q, QVector, r
)
WRAPPED1( Vector, UpCoordinateLevel, Vector, v, Vector(0, 0, 0)
)
WRAPPED1( Vector, DownCoordinateLevel, Vector, v, Vector(0, 0, 0)
)
WRAPPED1( Vector, ToLocalCoordinates, Vector, v, Vector(0, 0, 0)
)
WRAPPED1( Vector, ToWorldCoordinates, Vector, v, Vector(0, 0, 0)
)
WRAPPED0( Vector, GetAngularVelocity, Vector(0, 0, 0)
)
WRAPPED0( Vector, GetVelocity, Vector(0, 0, 0)
)
voidWRAPPED1( SetVelocity, Vector, v
)
voidWRAPPED1( SetAngularVelocity, Vector, v
)
#endif // (WHICH_UNIT_EXPORT_NUMBER == 1) || (WHICH_UNIT_EXPORT_NUMBER == 3)

#if (WHICH_UNIT_EXPORT_NUMBER == 1) || (WHICH_UNIT_EXPORT_NUMBER == 2)
#else // (WHICH_UNIT_EXPORT_NUMBER == 1) || (WHICH_UNIT_EXPORT_NUMBER == 2)
WRAPPED0(float, GetMoment,
0 )
WRAPPED0(float, GetMass,
0 )
WRAPPED0(int, LockMissile,
0 )
voidWRAPPED1( EjectCargo,
int, index )
WRAPPED1(float, PriceCargo, std::string, s,
0 )
WRAPPED0(int, numCargo,
0 )
WRAPPED1(bool, IsCleared, UnitWrapper, dockingunit,
false )
voidWRAPPED5( ImportPartList, std::string, category,
float, price, float, pricedev, float, quantity, float, quantdev )
WRAPPED1(bool, RequestClearance, UnitWrapper, dockingunit,
false )
WRAPPED1(bool, isDocked, UnitWrapper, dockingUnit,
false )
WRAPPED1(bool, Dock, UnitWrapper, unitToDockWith,
false )

voidWRAPPED1( setCombatRole, std::string, role
)

WRAPPED0( std::string, getCombatRole,
"INERT" )  //legacy function returns getUnitType

voidWRAPPED1( setAttackPreference, std::string, role
)

WRAPPED0( std::string, getAttackPreference,
"INERT" )

voidWRAPPED1( setUnitRole, std::string, role
)

WRAPPED0( std::string, getUnitRole,
"INERT" )

WRAPPED1(bool, UnDock, UnitWrapper, unitToDockWith,
false )
WRAPPED0(int, DockedOrDocking,
0 )
WRAPPED0(int, getNumMounts,
0 )
EXPORT_UTIL0( UnitWrapper, owner
)
voidEXPORT_UTIL2( performDockingOperations, UnitWrapper, unitToDockWith,
int, acootuallydock )

EXPORT_UTIL0( std::string, getFactionName
)
EXPORT_UTIL0(int, getFactionIndex)
voidEXPORT_UTIL1( setFactionIndex,
int, factionname )
voidEXPORT_UTIL1( setFactionName, std::string, factionname
)
EXPORT_UTIL1(float, getFactionRelation, UnitWrapper, their_unit)
EXPORT_UTIL1(float, getRelationToFaction, int, their_faction)
EXPORT_UTIL1(float, getRelationFromFaction, int, my_faction)
EXPORT_UTIL0( std::string, getName
)
voidEXPORT_UTIL1( setName, std::string, name
)
voidEXPORT_UTIL1( SetHull,
float, hull )
EXPORT_UTIL0( std::string, getFlightgroupName
)
EXPORT_UTIL0( UnitWrapper, getFlightgroupLeader
)
EXPORT_UTIL1(bool, setFlightgroupLeader, UnitWrapper, un)
EXPORT_UTIL0( std::string, getFgDirective
)
voidEXPORT_UTIL0( RecomputeUnitUpgrades )
EXPORT_UTIL1(bool, setFgDirective, std::string, inp)
//EXPORT_UTIL0(int,getFgSubnumber)
EXPORT_UTIL3(int, removeCargo, std::string, s, int, quantity, bool, erasezero)
EXPORT_UTIL3(int, removeWeapon, std::string, name, int, offset, bool, loop)
EXPORT_UTIL5(float,
        upgrade,
        std::string,
        file,
        int,
        mountoffset,
        int,
        subunitoffset,
        bool,
        force,
        bool,
        loop_through_mounts)
EXPORT_UTIL1(int, addCargo, Cargo, carg)
EXPORT_UTIL1(int, forceAddCargo, Cargo, carg)
EXPORT_UTIL2(bool, incrementCargo, float, percentagechange, int, quantity)
EXPORT_UTIL1(bool, decrementCargo, float, percentagechange)
EXPORT_UTIL1(float, getDistance, UnitWrapper, un)
//EXPORT_UTIL0(float, getPlanetRadiusPercent)
EXPORT_UTIL1(float, getSignificantDistance, UnitWrapper, sig)
EXPORT_UTIL1(int, hasCargo, std::string, mycarg)
EXPORT_UTIL1( Cargo, GetCargoIndex,
int, index );
EXPORT_UTIL1( Cargo, GetCargo, std::string, cargname
)
EXPORT_UTIL0( std::string, getUnitSystemFile
)
EXPORT_UTIL0(float, getCredits)
voidEXPORT_UTIL1( addCredits,
float, credits )
EXPORT_UTIL0(bool, isSignificant)
EXPORT_UTIL0(bool, isCapitalShip)
EXPORT_UTIL0(bool, isDockableUnit)
EXPORT_UTIL0(bool, isSun)
EXPORT_UTIL0(bool, isAsteroid)
voidEXPORT_UTIL1( switchFg, std::string, arg
)
EXPORT_UTIL2(int, communicateTo, UnitWrapper, other_unit, float, mood)
EXPORT_UTIL1(bool, commAnimation, std::string, anim)
EXPORT_UTIL1(bool, JumpTo, std::string, system)
EXPORT_UTIL0(int, isPlayerStarship)
voidEXPORT_UTIL1( setECM,
int, NewECM )         //short fix
EXPORT_UTIL0(int, getECM)         //short fix
voidEXPORT_UTIL1( setSpeed,
float, speed )
EXPORT_UTIL0(float, maxSpeed)
EXPORT_UTIL0(float, maxAfterburnerSpeed)
voidEXPORT_UTIL0( setMissionRelevant )
voidEXPORT_UTIL0( unsetMissionRelevant )

voidEXPORT_UTIL5( orbit, UnitWrapper, orbitee,
float, speed, QVector, R, QVector, S, QVector, center )
WRAPPED0(int, RepairCost,
0 )
WRAPPED0(int, RepairUpgrade,
0 )
EXPORT_UTIL3(float, PercentOperational, std::string, content, std::string, category, bool, countHullAndArmorAsFull)
WRAPPED1( Vector, MountPercentOperational,
int, whichMount, Vector( -1.0f, -1.0f, -1.0f ))
#endif // (WHICH_UNIT_EXPORT_NUMBER == 1) || (WHICH_UNIT_EXPORT_NUMBER == 2)

