#include "cmd_order.h"
#include "in_kb.h"
#include <GL/glut.h>
class MatchVelocity : public Order {
  Vector desired_velocity;//werld space... generally r*speed;
  bool LocalVelocity;//specified in Local or World coordinates
  MatchVelocity (const Vector desired, bool Local):desired_velocity(desired),LocalVelocity(Local) {}
  AI * Execute ();
};


class FlyByWire : public Order {
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
