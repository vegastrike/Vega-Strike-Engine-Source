#include "cmd_order.h"
#include "in_kb.h"
#include <GL/glut.h>
class MatchLinearVelocity : public Order {
 protected:
  Vector desired_velocity;//werld space... generally r*speed;
  bool LocalVelocity;//specified in Local or World coordinates
 public:
  MatchLinearVelocity (const Vector &desired, bool Local):desired_velocity(desired),LocalVelocity(Local) {}
  AI * Execute ();
  void SetDesiredVelocity (const Vector &desired, bool Local) {desired_velocity=desired;LocalVelocity=Local;}
};
class MatchAngularVelocity : public Order {
 protected:
  Vector desired_ang_velocity;//werld space... generally r*speed;
  bool LocalAng;//specified in Local or World coordinates
 public:
  MatchAngularVelocity (const Vector &desired, bool Local):desired_ang_velocity(desired),LocalAng(Local) {}
  AI * Execute ();
  void SetDesiredAngularVelocity (const Vector &desired, bool Local) {desired_ang_velocity=desired;LocalAng=Local;}
};

class MatchVelocity : public MatchLinearVelocity {
 protected:
  Vector desired_ang_velocity;//werld space... generally r*speed;
  bool LocalAng;
 public:
  MatchVelocity (const Vector &desired,const Vector &desired_ang, bool Local):MatchLinearVelocity (desired,Local),desired_ang_velocity(desired_ang), LocalAng(Local) {}
  AI * Execute ();
  void SetDesiredAngularVelocity (const Vector &desired, bool Local) {desired_ang_velocity=desired;LocalAng=Local;}
};

class FlyByWire : public MatchVelocity {

  float set_speed;
  float dream_speed;
  float max_speed;
  float max_ab_speed;
  float max_yaw;
  float max_pitch;
  float max_roll;
 public:
  static void UpKey(int, KBSTATE);
  static void DownKey (int,KBSTATE);
  static void LeftKey (int, KBSTATE);
  static void RightKey (int,KBSTATE);
  static void ABKey (int, KBSTATE);
  static void AccelKey (int,KBSTATE);
  static void DecelKey (int,KBSTATE);
  static void RollLeftKey (int,KBSTATE);
  static void RollRightKey (int,KBSTATE);
  FlyByWire (float max_ab_spd,float max_spd,float maxyaw,float maxpitch,float maxroll);
  ~FlyByWire();
  void Right (float percentage);//pass in the percentage of the turn they were turnin right.  -%age indicates left
  void Up (float percentage);//pass in the percentage of the turn they were turning up
  void RollRight (float percentage);
  void Afterburn (float percentage);
  void Accel (float percentage);//negative is decel... 0 = nothing
  AI * Execute();
};
