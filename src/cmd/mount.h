#ifndef __MOUNT_H
#define __MOUNT_H

#include "unit_generic.h"

/********************************************/
/**** GameMount class                       */
/********************************************/

  /** 
   * Contains information about a particular Mount on a unit.
   * And the weapons it has, where it is, where it's aimed, 
   * The ammo and the weapon type. As well as the possible weapons it may fit
   * Warning: type has a string inside... cannot be memcpy'd
   */

class GameMount: public Mount {
	///Where is it
	//Transformation LocalPosition;
  public:
	  void ReplaceMounts (const Mount * othermount);

	GameMount();
	// Requires weapon_xml.cpp stuff so Beam stuff so GFX and AUD stuff
	GameMount(const std::string& name, short int am=-1, short int vol=-1);
	///Turns off a firing beam (upon key release for example)
	void UnFire();
	/**
	 *  Fires a beam when the firing unit is at the Cumulative location/transformation 
	 * owner (won't crash into)  as owner and target as missile target. bool Missile indicates if it is a missile
	 * should it fire
	 */ 
	// Uses Sound Forcefeedback and other stuff
	void PhysicsAlignedUnfire();
	bool PhysicsAlignedFire (const Transformation &Cumulative, const Matrix & mat, const Vector & Velocity, Unit *owner,  Unit *target, signed char autotrack, float trackingcone);//0 is no track...1 is target 2 is target + lead
	bool Fire (Unit *owner, bool Missile=false, bool beams_target_owner=false);
};

#endif
