#include "sprite.h"
class Cockpit;
class Gauge: public Sprite {

 public:
  enum DIRECTION {GAUGE_UP,GAUGE_DOWN,GAUGE_RIGHT, GAUGE_LEFT};
private:  enum DIRECTION dir;
 public:
  Gauge (const char *file, DIRECTION up);
  void Draw (float percentage);
};
