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
  static void SwitchWebcam(const std::string&,KBSTATE);
  static void SwitchSecured(const std::string&,KBSTATE);
  static void ChangeCommStatus (const std::string&,KBSTATE);
  static void DownFreq (const std::string&,KBSTATE);
  static void UpFreq (const std::string&,KBSTATE);
  static void KSwitchFlightMode (const std::string&,KBSTATE);
  static void KThrustRight (const std::string&,KBSTATE);
  static void KThrustLeft (const std::string&,KBSTATE);
  static void KThrustUp (const std::string&,KBSTATE);
  static void KThrustDown (const std::string&,KBSTATE);
  static void KThrustFront (const std::string&,KBSTATE);
  static void KThrustBack (const std::string&,KBSTATE);
  static void SheltonKey (const std::string&,KBSTATE);
  static void StartKey (const std::string&,KBSTATE);
  static void StopKey (const std::string&,KBSTATE);
  static void UpKey(const std::string&,KBSTATE);
  static void DownKey (const std::string&,KBSTATE);
  static void LeftKey (const std::string&,KBSTATE);
  static void RightKey (const std::string&,KBSTATE);
  static void ABKey (const std::string&,KBSTATE);
  static void AccelKey (const std::string&,KBSTATE);
  static void DecelKey (const std::string&,KBSTATE);
  static void RollLeftKey (const std::string&,KBSTATE);
  static void RollRightKey (const std::string&,KBSTATE);
  static void MatchSpeedKey (const std::string&,KBSTATE);
  static void JumpKey (const std::string&,KBSTATE);
  static void AutoKey (const std::string&,KBSTATE);
  static void SwitchCombatModeKey (const std::string&,KBSTATE);
  static void StopAutoKey (const std::string&,KBSTATE);
  static void SetVelocityRefKey (const std::string&,KBSTATE);
  static void SetNullVelocityRefKey (const std::string&,KBSTATE);
  int whichplayer;
  FlyByKeyboard (unsigned int whichplayer);
  virtual void Destroy();
  virtual ~FlyByKeyboard();
  void Execute (bool);//should we reset our ang velocity
  void Execute();
};

#endif
