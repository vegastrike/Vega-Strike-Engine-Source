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
  Vector axis_key;
 protected:
  void KeyboardUp (float val);
  void KeyboardRight (float val);
  void KeyboardRollRight (float val);
  float clamp_axis(float v);
  float reduce_axis (float v);
 public:
  static void ChangeCommStatus (int,KBSTATE);
  static void DownFreq (int,KBSTATE);
  static void UpFreq (int,KBSTATE);
  static void KSwitchFlightMode (int,KBSTATE);
  static void KThrustRight (int,KBSTATE);
  static void KThrustLeft (int,KBSTATE);
  static void KThrustUp (int,KBSTATE);
  static void KThrustDown (int,KBSTATE);
  static void KThrustFront (int,KBSTATE);
  static void KThrustBack (int,KBSTATE);
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
  static void SwitchCombatModeKey (int, KBSTATE);
  static void StopAutoKey (int, KBSTATE);
  static void SetVelocityRefKey (int, KBSTATE);
  static void SetNullVelocityRefKey (int, KBSTATE);
  int whichplayer;
  FlyByKeyboard (unsigned int whichplayer);
  virtual void Destroy();
  virtual ~FlyByKeyboard();
  void Execute (bool);//should we reset our ang velocity
  void Execute();
};

#endif
