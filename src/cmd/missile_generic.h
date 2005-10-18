#ifndef MISSILEGENERIC_H_
#define MISSILEGENERIC_H_
#include "unit_generic.h"

class MissileEffect {
  Vector pos;
  float damage;
  float phasedamage;
  float radius;
  float radialmultiplier;
  void * ownerDoNotDereference;
 public:
  void ApplyDamage (Unit *);
  MissileEffect (const Vector & pos, float dam, float pdam, float radius, float radmult, void * owner):pos(pos) {
    damage=dam;phasedamage=pdam;
    this->radius=radius;
    radialmultiplier=radmult;
    this->ownerDoNotDereference=owner;
  }
  float GetRadius() const { return radius; };
  const Vector& GetCenter() const { return pos; };
};
class Missile:public Unit {
 public:
 protected:
	Missile (std::vector <Mesh *> m,bool b,int i):Unit (m,b,i){}
  virtual float ExplosionRadius();
  float time;
  float damage;
  float phasedamage;
  float radial_effect;
  float radial_multiplier;
  float detonation_radius;
  bool discharged;
  signed char retarget;
 public:
  void Discharge();
  virtual enum clsptr isUnit()const {return MISSILEPTR;}

protected:
    /// constructor only to be called by UnitFactory
    Missile( const char * filename,
             int faction,
	     const string &modifications,
	     const float damage,
	     float phasedamage,
	     float time,
	     float radialeffect,
	     float radmult,
	     float detonation_radius)
        : Unit (filename,false,faction,modifications)
        , time(time)
        , damage(damage)
        , phasedamage(phasedamage)
        , radial_effect(radialeffect)
        , radial_multiplier (radmult)
        , detonation_radius(detonation_radius)
        , discharged(false)
        , retarget (-1)
    {maxhull*=10; }

  void InitMissile( float ptime, const float pdamage, float pphasedamage, float pradial_effect,
					float pradial_multiplier, float pdetonation_radius, bool pdischarged, signed char pretarget)
  {
	  time=ptime;
	  damage=pdamage;
	  phasedamage=pphasedamage;
	  radial_effect=pradial_effect;
	  radial_multiplier=pradial_multiplier;
	  detonation_radius=pdetonation_radius;
	  discharged=pdischarged;
	  retarget=pretarget;
  }
  friend class UnitFactory;

public:
  virtual void Kill (bool eraseFromSave=true);
  virtual void reactToCollision (Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  virtual void UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);	
	

protected:
    /// default constructor forbidden
    Missile( ) {}
    /// copy constructor forbidden
    //Missile( const Missile& );
    /// assignment operator forbidden
    //Missile& operator=( const Missile& );
};


#endif
