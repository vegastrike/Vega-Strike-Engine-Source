#ifndef MISSILE_H_
#define MISSILE_H_

class MissileEffect {

  Vector pos;
  float damage;
  float phasedamage;
  float radius;
  float radialmultiplier;
 public:
  void ApplyDamage (Unit *);
  MissileEffect (const Vector & pos, float dam, float pdam, float radius, float radmult):pos(pos) {
    damage=dam;phasedamage=pdam;
    this->radius=radius;
    radialmultiplier=radmult;
  }
};
class Missile:public Unit {
 public:
 protected:
  virtual float ExplosionRadius();
  float time;
  float damage;
  float phasedamage;
  float radial_effect;
  float radial_multiplier;
  float detonation_radius;
  bool discharged;
 public:
  void Discharge();
  virtual enum clsptr isUnit() {return MISSILEPTR;}
  Missile (const char * filename, int faction, const string &modifications,const float damage,float phasedamage,  float time, float radialeffect, float radmult, float detonation_radius):Unit (filename,false,faction,modifications),time(time),damage(damage),phasedamage(phasedamage), radial_effect(radialeffect), radial_multiplier (radmult), detonation_radius(detonation_radius),discharged(false) { }
  virtual void Kill (bool eraseFromSave=true);
  virtual void reactToCollision (Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist);
  virtual void UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);

};


#endif
