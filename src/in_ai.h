#ifndef _IN_AI_H_
#define _IN_AI_H_
#include "in.h"
#include <GL/glut.h>
#include "cmd_flybywire.h"


class FlyByKeyboard: public FlyByWire {

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
  FlyByKeyboard (float max_ab_spd,float max_spd,float maxyaw,float maxpitch,float maxroll);
  virtual ~FlyByKeyboard();
  AI * Execute();
};

#endif
