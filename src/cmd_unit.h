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
#include "gfx.h"
#include "gfx_transform_matrix.h"
#include <iostream.h>
//#include "Gun.h"
//#include "Warhead.h"
/*EXPLANATION OF TERMS:
 * Weapon: the weapon as on a ship
 * Damager: the active weapon after launch/firing
 */
class Gun;
class Warhead;
class AI;
/*
class MeshGroup{
	Mesh **meshes; // "siblings"
	int nummesh;

	Matrix tmatrix;

	Vector pp, pq, pr, ppos;

public:
	MeshGroup(char *filename); // file containing file names of all the shits to load
	~MeshGroup();

	void Draw();
	void Draw(Matrix tmatrix);
	void Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos);
};
*/
Vector MouseCoordinate (int x, int y, float zplane);
enum Aggression{
	LOW, MEDIUM, HIGH, DISABLE, DESTROY
};
class Unit:public Mesh{
 private:
  struct XML {
    vector<Mesh*> meshes;
    vector<Unit*> units;
  } *xml;
  
  void LoadXML(const char *filename);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

protected:
  int nummesh;
  Mesh **meshdata;
  Unit **subunits; // the new children fun fun stuff
  //Gun **weapons; // the guns
  
  int numsubunit;
  
  //static int refcount; for the inherited classes
  Unit *target;
  
  Aggression aggression;
  
  Matrix tmatrix;
  
  float time;
  BOOL active;
  
  //Vector pp, pq, pr, ppos;
  AI *aistate;
  long fpos;
  
  float mass;
  float fuel;
  float MomentOfInertia;
  Vector AngularVelocity;
  Vector NetForce;
  Vector NetTorque;
  Vector Velocity;
  
  bool calculatePhysics; // physics have an effect on this object (set to false for planetse)

  bool selected;
  Vector corner_min, corner_max; // corners of object
  void calculate_extent();

public:
  //no default constructor; dymanically allocated arrays are evil, gotta do it java style to make it more sane
  Unit();
  Unit(const char *filename, bool xml=false);
  virtual ~Unit();
  void Init();
  
  Unit *&Target(){return target;}; // Accessor for target; probably shouldn't use it

	/*COMMAND*/
	/*
	virtual void ChangeTarget(Unit *target) = 0; // sent by the flight commander, supercommand AI, or player; tells it to switch to this new target
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

  virtual void Draw();
  virtual void DrawStreak(const Vector &v);
  virtual void TDraw(){GFXLoadIdentity(MODEL); Draw();};
  virtual void Draw(Matrix tmatrix);
  virtual void Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos);
  float getMinDis(const Vector &pnt);
  float getMinDis(Matrix t, const Vector &pnt);
  bool querySphere (const Vector &pnt, float err);
  bool querySphere (const Vector &st, const Vector &end, float err);
  bool querySphere (Matrix,const Vector &st, const Vector &end, float err);
  bool querySphere (Matrix,const Vector &pnt, float err);
  bool queryBoundingBox(const Vector &pnt, float err);
  bool queryBoundingBox(Matrix,const Vector &, float);
  bool queryFrustum (float frustum[6][4]);
  bool queryFrustum (Matrix, float [6][4]);

  /**Queries the bounding box with a ray.  1 if ray hits in front... -1 if ray
   * hits behind.
   * 0 if ray misses */
  int queryBoundingBox(const Vector &origin,const Vector &direction, float err);
  int queryBoundingBox(Matrix,const Vector &, const Vector &pnt, float err);
  /**Queries the bounding sphere with a duo of mouse coordinates that project
   * to the center of a ship and compare with a sphere...pretty fast*/
  bool querySphere (int,int, float err, Camera *activeCam);
  bool querySphere (Matrix,int,int, float err,Camera *,Matrix);
  
  void SetAI(AI *newAI);
  Vector &Position(){return pos;};
  Vector &Nose(){return pr;};
  float GetTime(){return time;};
  
  void Destroy(){active = FALSE;};
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

  void FireEngines (Vector Direction, /*unit vector... might default to "r"*/
					float FuelSpeed,
					float FMass);
  void ApplyForce(Vector Vforce); //applies a force for the whole gameturn upon the center of mass
  void Accelerate(Vector Vforce); // applies a force that is multipled by the mass of the ship
  void ApplyTorque (Vector Vforce, Vector Location);
  void ApplyLocalTorque (Vector Vforce, Vector Location);
  void ApplyBalancedLocalTorque (Vector Vforce, Vector Location); //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
  void ResolveForces ();
  Vector &P(){return p;};
  Vector &Q(){return q;};
  Vector &R(){return r;};

  inline bool queryCalculatePhysics() { return calculatePhysics; }
  void ExecuteAI();

  void Select();
  void Deselect();

  ostream &output(ostream& os) const;
};

ostream &operator<<(ostream &os, const Unit &);
#endif
