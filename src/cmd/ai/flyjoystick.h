#include "flykeyboard.h"

class FlyByJoystick: public FlyByKeyboard {
  int which_joystick;
  bool keyboard;
 public:
  FlyByJoystick(int which, const char * configfile);
  void JAccelKey (KBSTATE,float,float,int);
  void JDecelKey (KBSTATE,float,float,int);
  
  void Execute();
  virtual ~FlyByJoystick();
};
