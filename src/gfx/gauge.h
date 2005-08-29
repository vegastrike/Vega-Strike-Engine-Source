#include "sprite.h"
class Cockpit;
class Gauge: public VSSprite {

 public:
  enum DIRECTION {GAUGE_UP,GAUGE_DOWN,GAUGE_RIGHT,GAUGE_LEFT,GAUGE_TIME };
private:  enum DIRECTION dir;
 public:
  Gauge (const char *file, DIRECTION up);
  void Draw (float percentage);
  void SetSize (float x, float y);
  void GetSize (float &x, float &y);
};
