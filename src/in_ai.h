#ifndef _IN_AI_H_
#define _IN_AI_H_
#include "in.h"
#include <GL/glut.h>
#include "cmd_flybywire.h"


class FlyByKeyboard: public FlyByWire {

 public:
  static void StartKey (int,KBSTATE);
  static void StopKey (int, KBSTATE);
  static void UpKey(int, KBSTATE);
  static void DownKey (int,KBSTATE);
  static void LeftKey (int, KBSTATE);
  static void RightKey (int,KBSTATE);
  static void ABKey (int, KBSTATE);
  static void AccelKey (int,KBSTATE);
  static void DecelKey (int,KBSTATE);
  static void RollLeftKey (int,KBSTATE);
  static void RollRightKey (int,KBSTATE);
  FlyByKeyboard ();
  virtual ~FlyByKeyboard();
  void Execute();
};

#endif
