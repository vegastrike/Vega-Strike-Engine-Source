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

struct GFXColor;
#include "gfx_transform_matrix.h"
#include "quaternion.h"
#include "gfx_lerp.h"
//#include <iostream>
#include <string>
#include "cmd_weapon_xml.h"
#include "cmd_collide.h"
#include "UnitContainer.h"
using std::string;
//using std::queue;
#include "xml_support.h"
class Beam;
class Animation;
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
class Halo;
class BSPTree;
class PlanetaryOrbit;
class UnitCollection;
//////OBSOLETE!!!!!! Vector MouseCoordinate (int x, int y, float zplane);
enum clsptr {
	UNITPTR,
	PLANETPTR,
	TEXTPTR,
	JUMPPTR
};
class Unit {
friend class PlanetaryOrbit;
 public:
  struct Computer {
    Vector NavPoint;
    UnitContainer target;//...and check it each frame    
    float set_speed;
    float max_speed;
    float max_ab_speed;
    float max_yaw;
    float max_pitch;
    float max_roll;
  };

 private:

  struct XML;
  XML *xml;

  
  void LoadXML(const char *filename);

  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

 protected:

  BSPTree *bspTree;
  int ucref;
  bool killed;
  bool invisible;
  //used for physics
  Transformation prev_physical_state;
  Transformation curr_physical_state;
  Matrix cumulative_transformation_matrix;
  Transformation cumulative_transformation;
  LineCollide CollideInfo;
  Animation **explosion;
  float timeexplode;  
  int nummesh;
  Mesh **meshdata;
  float shieldtight;//are shields tight to the hull.  zero means bubble
  int numsubunit;
  Unit **subunits; // the new children fun fun stuff
  int numhalos;
  Halo **halos;
  int nummounts;
  class Mount {
    Transformation LocalPosition;
  public:
    Beam *gun;//only beams are actually coming out of the gun at all times...bolts, balls, etc aren't
    enum {ACTIVE, INACTIVE, DESTROYED, UNCHOSEN} status;
    short size;
    weapon_info type;
    Mount():gun(NULL),status(UNCHOSEN),size(weapon_info::NOWEAP),type(weapon_info::BEAM){}
    Mount(const string& name);
    void Activate () {
      if (status==INACTIVE)
	status = ACTIVE;
    }
    void DeActive () {
      if (status==ACTIVE)
	status = INACTIVE;
    }
    void SetMountPosition (const Transformation &t) {LocalPosition = t;}
    void UnFire();
    bool Fire (const Transformation &Cumulative, const float * mat, Unit *owner);
  } *mounts;
  
  //static int refcount; for the inherited classes
  struct {
    unsigned short right, left, front, back;
  } armor;
  struct {
    int  number;
    int leak; 
    float recharge; 
    union {
      float fb[4];
      struct {
	unsigned short front, back, right, left;
	unsigned short frontmax, backmax, rightmax, leftmax;
      }fbrl;
      struct {
	unsigned short v[6];
	unsigned short fbmax,rltbmax;
      }fbrltb;
    };
  } shield;
  float hull;
  float DealDamageToShield (const Vector & pnt, float &Damage);
  float DealDamageToHull (const Vector &pnt, float Damage);
  //  bool active;
  
  //Vector pp, pq, pr, ppos;
  void BuildBSPTree (const char *filename, bool vplane=false, Mesh * hull=NULL);//if hull==NULL, then use meshdata **
  AI *aistate;
  float accel;
  float recharge;
  float energy;
  float mass;
  float fuel;
  float MomentOfInertia;
  Vector NetForce;
  Vector NetLocalForce;
  Vector NetTorque;
  Vector NetLocalTorque;
  Vector AngularVelocity;
  Vector Velocity;
  int slerp_direction;
  enum restr {YRESTR=1, PRESTR=2, RRESTR=4};
  char yprrestricted;
  float ymin, ymax, ycur;
  float pmin, pmax, pcur;
  float rmin, rmax, rcur;
  long fpos;
  // thrusting limits
  struct Limits {
    float yaw;
    float pitch;
    float roll;

    float lateral;
    float vertical;
    float forward;
    float retro;
    float afterburn;
  } limits;
  Computer computer;
  bool calculatePhysics; // physics have an effect on this object (set to false for planets)

  bool selected;
  float radial_size;
  Vector corner_min, corner_max; // corners of object
  void calculate_extent();
  Box *selectionBox;

public:
  void UpdateCollideQueue();
  Vector origin;
  string name;
  float rSize () {return radial_size;}
  //no default constructor; dymanically allocated arrays are evil, gotta do it java style to make it more sane

  Unit();
  Unit(const char *filename, bool xml=false);
  virtual ~Unit();
  virtual enum clsptr isUnit() {return UNITPTR;}
  static void ProcessDeleteQueue();
  void Init();
  bool Explode(bool draw=true);
  void Destroy();//explodes then deletes
  virtual void Kill();//deletes
  inline bool Killed() {return killed;}
  inline void Ref() {ucref++;}
  void UnRef();
  Unit *Target(){return computer.target.GetUnit();}
  void Target (Unit * targ) {computer.target.SetUnit(targ);}
  void Fire();
  void UnFire();
  Computer & GetComputerData () {return computer;}
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


  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix m = identity_matrix);
  virtual void ProcessDrawQueue();
  float getMinDis(const Vector &pnt);//for clicklist
  bool querySphere (const Vector &pnt, float err);//for weapons
  float querySphere (const Vector &start, const Vector & end);//for beams
  float querySphere (const Vector &st, const Vector &dir, float err);//for click list
  float queryBSP (const Vector &st, const Vector & end, Vector & normal);
  bool queryBSP (const Vector &pnt, float err, Vector & normal, float &dist);
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
  bool OneWayCollide (Unit *target, Vector & normal, float &dist);
  bool Collide(Unit * target);
  void CollideAll();//checks for collisions with all beams and other units roughly
  Vector Position(){return cumulative_transformation.position;};
  Vector LocalPosition(){return curr_physical_state.position;};
  void SetPosition(const Vector &pos) {/*prev_physical_state.position = curr_physical_state.position;*/
  prev_physical_state.position = curr_physical_state.position = pos;}
  void SetPosition(float x, float y, float z) {/*prev_physical_state.position = curr_physical_state.position;*/
  prev_physical_state.position = curr_physical_state.position = Vector(x,y,z);}
  void SetOrigin(const Vector &pos) {SetPosition(pos);origin=pos;}
  void SetCameraToCockpit();
  //  void Destroy(){active = false;};
  /*
  Unit *Update() {
    if(active) {
      return this;
    }
    else {
      delete this;
      return NULL;
    }
  }
  */
  void Rotate(const Vector &axis);
  void FireEngines (const Vector &Direction, /*unit vector... might default to "r"*/
					float FuelSpeed,
					float FMass);
  void ApplyForce(const Vector &Vforce); //applies a force for the whole gameturn upon the center of mass
  void ApplyLocalForce(const Vector &Vforce); //applies a force for the whole gameturn upon the center of mass, using local coordinates
  void Accelerate(const Vector &Vforce); // applies a force that is multipled by the mass of the ship
  void ApplyTorque (const Vector &Vforce, const Vector &Location);
  void ApplyLocalTorque (const Vector &Vforce, const Vector &Location);
  void ApplyBalancedLocalTorque (const Vector &Vforce, const Vector &Location); //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)

  // Help out AI creation
  void ApplyLocalTorque(const Vector &torque); //convenient shortcut
  void ApplyLocalDamage (const Vector &pnt, const Vector & normal, float amt, const GFXColor &);
  void ApplyDamage (const Vector & pnt, const Vector & normal, float amt, const GFXColor & );
  Vector ClampThrust(const Vector &thrust, bool afterburn);
  Vector MaxThrust(const Vector &thrust);
  void Thrust(const Vector &amt,bool afterburn = false);
  void LateralThrust(float amt);
  void VerticalThrust(float amt);
  void LongitudinalThrust(float amt);

  Vector ClampTorque(const Vector &torque);
  Vector MaxTorque(const Vector &torque);
  void YawTorque(float amt);
  void PitchTorque(float amt);
  void RollTorque(float amt);
  void UpdatePhysics (const Transformation &trans, const Matrix transmat, bool ResolveLast, UnitCollection *uc=NULL);
  void ResolveForces (const Transformation &, const Matrix);
  //  void ResolveLast(const Transformation &, const Matrix); // used for lerp
  void GetOrientation(Vector &p, Vector &q, Vector &r) const;
  Vector UpCoordinateLevel(const Vector &v) const;
  Vector ToLocalCoordinates(const Vector &v) const;
  Vector ToWorldCoordinates(const Vector &v) const;
  const Vector &GetAngularVelocity() const { return AngularVelocity; }
  const Vector &GetVelocity() const { return Velocity; }

  float GetMoment() const { return MomentOfInertia; }
  float GetMass() const { return mass; }
  float GetElasticity ();
  const Limits &Limits() const { return limits; }

  inline bool queryCalculatePhysics() { return calculatePhysics; }
  void ExecuteAI();
};
struct Unit::XML {
  vector<Halo*> halos;
  vector<Unit::Mount *> mountz;
  vector<Mesh*> meshes;
  Mesh * shieldmesh;
  Mesh * bspmesh;
  vector<Unit*> units;
  int unitlevel;
  bool hasBSP;
};
#endif
