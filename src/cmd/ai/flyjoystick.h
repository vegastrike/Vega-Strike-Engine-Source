#include "flykeyboard.h"

class FlyByJoystick: public FlyByKeyboard {
  int which_joystick;
 public:
  FlyByJoystick(int which);
  void Execute();
  virtual ~FlyByJoystick();
};
