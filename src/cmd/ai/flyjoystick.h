#include "flykeyboard.h"

class FlyByJoystick: public FlyByKeyboard {
  std::vector <int> whichjoystick;//which joysticks are bound to this
  bool keyboard;
 public:
  FlyByJoystick(unsigned int whichplayer);
  static void JAccelKey (KBSTATE,float,float,int);
  static void JDecelKey (KBSTATE,float,float,int);
  static void JShelt (KBSTATE,float,float,int);
  static void JAB (KBSTATE,float,float,int);
  
  void Execute();
  virtual ~FlyByJoystick();
};
