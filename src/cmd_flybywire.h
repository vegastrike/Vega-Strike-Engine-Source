#include "cmd_order.h"
#include "in_kb.h"
class FlyByWire : public Order {
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
  
  FlyByWire (float max_ab_spd,float max_spd,float maxyaw,float maxpitch,float maxroll);
  AI * Execute();
};
