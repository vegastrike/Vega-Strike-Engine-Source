/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _UNIT_H_
#define _UNIT_H_
#include <time.h>
#include "gfx_transform_matrix.h"
#include "quaternion.h"
#include "gfx_lerp.h"
#include <iostream>
#include <string>

using std::string;
using std::cerr;

#include "xml_support.h"

using namespace XMLSupport;
//#include "Gun.h"
//#include "Warhead.h"
/*EXPLANATION OF TERMS:
 * Weapon: the weapon as on a ship
 * Damager: the active weapon after launch/firing
 */
class Gun;
class Warhead;
class AI;
class Box;
class Mesh;
class Camera;

//////OBSOLETE!!!!!! Vector MouseCoordinate (int x, int y, float zplane);
enum Aggression{
	LOW, MEDIUM, HIGH, DISABLE, DESTROY
};
class Unit {
 private:
  struct XML {
    vector<Mesh*> meshes;
    vector<Unit*> units;
  } *xml;
  
  void LoadXML(const char *filename);

  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

protected:
  //used for physics
  Transformation prev_physical_state;
  Transformation curr_physical_state;
  Matrix cumulative_transformation_matrix;

  int nummesh;
  Mesh **meshdata;
  Unit **subunits; // the new children fun fun stuff
  //Gun **weapons; // the guns
  
  int numsubunit;
  
  //static int refcount; for the inherited classes
  Unit *target;
  
  Aggression aggression;
  
  bool active;
  
  //Vector pp, pq, pr, ppos;
  AI *aistate;
  long fpos;
  string name;
  
  float mass;
  float fuel;
  float MomentOfInertia;
  Vector NetForce;
  Vector NetTorque;
  Vector AngularVelocity;
  Vector Velocity;
  int slerp_direction;

  float ymin, ymax, ycur;
  bool yrestricted;
  float pmin, pmax, pcur;
  bool prestricted;
  float rmin, rmax, rcur;
  bool rrestricted;

  // thrusting limits
  struct Limits {
    float yaw;
    float pitch;
    float roll;

    float lateral;
    float vertical;
    float longitudinal;
  } limits;

  bool calculatePhysics; // physics have an effect on this object (set to false for planets)

  bool selected;
  Vector corner_min, corner_max; // corners of object
  void calculate_extent();
  Box *selectionBox;

public:
  //no default constructor; dymanically allocated arrays are evil, gotta do it java style to make it more sane
  Unit();
  Unit(const char *filename, bool xml=false);
  virtual ~Unit();
  void Init();
  
  Unit *&Target(){return target;}; // Accessor for target; probably shouldn't use it

	/*COMMAND*/
	/*
	virtual void ChangeTarget(Unit *target) = 0; // sent by the flight commander, supercommand AI, or player; tells it to switch to this new target HANDLETHIS BY REPLACE/ENQUEUE ORDER after having primed orders
	virtual void ChangeAggression(enum Aggression aggression) = 0; // same as above
	virtual void AddNav(const Vector &pos) = 0; // same as above; tells it to add this position to the navpoint list
	virtual void MoveTo(const Vector &pos) = 0; // same as above; clears the navpoint list and GOES there 
	*/

	/*INTERACTION*/
	/*
	virtual void LockOn(Unit *shooter) = 0; // sent by the shooter
	virtual void Launch(Unit *shooter, Warhead *warhead) = 0; // sent by the shooter
	virtual void HitTarget(Unit *target) = 0; // sent by the damager that hit the target
	virtual void Damaged(Unit *shooter) = 0; // sent by the damager that hit it
	*/

  void RestrictYaw(float min, float max);
  void RestrictPitch(float min, float max);
  void RestrictRoll(float min, float max);

  void UpdateHudMatrix();


  virtual void DrawStreak(const Vector &v);
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix m = identity_matrix);
  virtual void ProcessDrawQueue();
  float getMinDis(const Vector &pnt);
  bool querySphere (const Vector &pnt, float err);
  int querySphere (const Vector &st, const Vector &end, float err);
  bool queryFrustum (float frustum[6][4]);

  /**Queries the bounding box with a ray.  1 if ray hits in front... -1 if ray
   * hits behind.
   * 0 if ray misses */
  bool Unit::queryBoundingBox (const Vector &pnt, float err);
  int queryBoundingBox(const Vector &origin,const Vector &direction, float err);
  /**Queries the bounding sphere with a duo of mouse coordinates that project
   * to the center of a ship and compare with a sphere...pretty fast*/
  bool querySphere (int,int, float err, Camera *activeCam);

  void Select();
  void Deselect();

  void PrimeOrders();
  void SetAI(AI *newAI);
  void EnqueueAI(AI *newAI);

  Vector Position(){return curr_physical_state.position;};
  void SetPosition(const Vector &pos) {/*prev_physical_state.position = curr_physical_state.position;*/
  prev_physical_state.position = curr_physical_state.position = pos;}
  void SetPosition(float x, float y, float z) {/*prev_physical_state.position = curr_physical_state.position;*/
  prev_physical_state.position = curr_physical_state.position = Vector(x,y,z);}

  void Destroy(){active = false;};
  virtual void Fire(){};

  Unit *Update() {
    if(active) {
      return this;
    }
    else {
      delete this;
      return NULL;
    }
  }

  void Rotate(const Vector &axis);
  void FireEngines (const Vector &Direction, /*unit vector... might default to "r"*/
					float FuelSpeed,
					float FMass);
  void ApplyForce(const Vector &Vforce); //applies a force for the whole gameturn upon the center of mass
  void Accelerate(const Vector &Vforce); // applies a force that is multipled by the mass of the ship
  void ApplyTorque (const Vector &Vforce, const Vector &Location);
  void ApplyLocalTorque (const Vector &Vforce, const Vector &Location);
  void ApplyBalancedLocalTorque (const Vector &Vforce, const Vector &Location); //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)

  // Help out AI creation
  void ApplyLocalTorque(const Vector &torque); //convenient shortcut

  Vector ClampThrust(const Vector &thrust);
  Vector MaxThrust(const Vector &thrust);
  void Thrust(const Vector &amt);
  void LateralThrust(float amt);
  void VerticalThrust(float amt);
  void LongitudinalThrust(float amt);

  void YawThrust(float amt);
  void PitchThrust(float amt);
  void RollThrust(float amt);

  void ResolveForces ();
  void ResolveLast(); // used for lerp
  void GetOrientation(Vector &p, Vector &q, Vector &r) const;
  Vector ToLocalCoordinates(const Vector &v) const;
  const Vector &GetAngularVelocity() const { return AngularVelocity; }
  const Vector &GetVelocity() const { return Velocity; }
  const Vector &GetPosition() const { return curr_physical_state.position; }
  float GetMoment() const { return MomentOfInertia; }
  float GetMass() const { return mass; }
  const Limits &Limits() const { return limits; }

  inline bool queryCalculatePhysics() { return calculatePhysics; }
  void ExecuteAI();

  ostream &output(ostream& os) const;
};

ostream &operator<<(ostream &os, const Unit &);
#endif
