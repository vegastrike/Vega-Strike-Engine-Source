#include "flykeyboard.h"

class FlyByJoystick: public FlyByKeyboard {
  int which_joystick;
  bool keyboard;
 public:
  FlyByJoystick(int which, bool kb);
  void Execute();
  virtual ~FlyByJoystick();
};
