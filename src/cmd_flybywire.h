#include "cmd_order.h"
class FlyByWire : public Order {
  float max_speed;
  
 public:
  FlyByWire (float max_spd);
  AI * Execute();
};
