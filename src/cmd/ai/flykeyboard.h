#ifndef _IN_AI_H_
#define _IN_AI_H_
#include "in.h"
#if defined(__APPLE__) || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include "flybywire.h"

class FlyByKeyboard: public FlyByWire {
  Order * autopilot;
 public:
  static void SheltonKey (int,KBSTATE);
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
  static void MatchSpeedKey (int,KBSTATE);
  static void JumpKey (int, KBSTATE);
  static void AutoKey (int, KBSTATE);
  static void StopAutoKey (int, KBSTATE);
  static void SetVelocityRefKey (int, KBSTATE);
  static void SetNullVelocityRefKey (int, KBSTATE);
  int whichplayer;
  FlyByKeyboard (unsigned int whichplayer);
  virtual ~FlyByKeyboard();
  void Execute (bool);//should we reset our ang velocity
  void Execute();
};

#endif
