#include "flykeyboard.h"

class FlyByJoystick: public FlyByKeyboard {
  int which_joystick;
  bool keyboard;
 public:
  FlyByJoystick(int which, const char * configfile);
  void Execute();
  virtual ~FlyByJoystick();
};
