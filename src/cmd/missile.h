#ifndef MISSILE_H_
#define MISSILE_H_
class Missile:public Unit {
  float time;
  float damage;

 public:
  virtual enum clsptr isUnit() {return MISSILEPTR;}
  Missile (const char * filename, int faction, const string &modifications,const float damage, float time):Unit (filename,false,faction,modifications),time(time),damage(damage) { }
  virtual void reactToCollision (Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist) {
    smaller->ApplyDamage (biglocation,bignormal,damage,smaller,GFXColor(1,1,1,1));
    Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
    if (!killed)
      DealDamageToHull (smalllocation,hull+1);

  }
  virtual void UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL){
    Unit::UpdatePhysics (trans, transmat, CumulativeVelocity, ResolveLast, uc);
    this->time-=SIMULATION_ATOM;
    if (time<0)
      DealDamageToHull (Vector(.1,.1,.1),hull+1);
  }

};


#endif
