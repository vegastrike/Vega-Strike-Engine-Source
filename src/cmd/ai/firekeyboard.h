#ifndef _CMD_KEYBOARD_AI_H_
#define _CMD_KEYBOARD_AI_H_
#include "in.h"
#include "order.h"
#include "event_xml.h"
#include "communication.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
#define NUMSAVEDTARGETS 10
class FireKeyboard: public Order {
  unsigned char sex;
  bool itts;
  bool cloaktoggle;
  bool refresh_target;
  float gunspeed;
  float gunrange;
  float rxntime;
  float delay;
  bool ShouldFire(Unit * targ);
  std::list <CommunicationMessage> resp;
 public:
#ifdef CAR_SIM
  static void BlinkLeftKey (const std::string&,KBSTATE);
  static void BlinkRightKey (const std::string&,KBSTATE);
  static void HeadlightKey (const std::string&,KBSTATE);
  static void SirenKey (const std::string&,KBSTATE);
#endif
  static void SetShieldsOneThird (const std::string&,KBSTATE);
  static void SetShieldsTwoThird (const std::string&,KBSTATE);
  static void ToggleGlow (const std::string&,KBSTATE);
  static void ToggleWarpDrive (const std::string&,KBSTATE);
  static void ToggleAnimation (const std::string&,KBSTATE);	
  static void PressComm1Key (const std::string&,KBSTATE);
  static void PressComm2Key (const std::string&,KBSTATE);
  static void PressComm3Key (const std::string&,KBSTATE);
  static void PressComm4Key (const std::string&,KBSTATE);
  static void PressComm5Key (const std::string&,KBSTATE);
  static void PressComm6Key (const std::string&,KBSTATE);
  static void PressComm7Key (const std::string&,KBSTATE);
  static void PressComm8Key (const std::string&,KBSTATE);
  static void PressComm9Key (const std::string&,KBSTATE);
  static void PressComm10Key (const std::string&,KBSTATE);
  static void SaveTarget1Key (const std::string&,KBSTATE);
  static void SaveTarget2Key (const std::string&,KBSTATE);
  static void SaveTarget3Key (const std::string&,KBSTATE);
  static void SaveTarget4Key (const std::string&,KBSTATE);
  static void SaveTarget5Key (const std::string&,KBSTATE);
  static void SaveTarget6Key (const std::string&,KBSTATE);
  static void SaveTarget7Key (const std::string&,KBSTATE);
  static void SaveTarget8Key (const std::string&,KBSTATE);
  static void SaveTarget9Key (const std::string&,KBSTATE);
  static void SaveTarget10Key (const std::string&,KBSTATE);
  static void RestoreTarget1Key (const std::string&,KBSTATE);
  static void RestoreTarget2Key (const std::string&,KBSTATE);
  static void RestoreTarget3Key (const std::string&,KBSTATE);
  static void RestoreTarget4Key (const std::string&,KBSTATE);
  static void RestoreTarget5Key (const std::string&,KBSTATE);
  static void RestoreTarget6Key (const std::string&,KBSTATE);
  static void RestoreTarget7Key (const std::string&,KBSTATE);
  static void RestoreTarget8Key (const std::string&,KBSTATE);
  static void RestoreTarget9Key (const std::string&,KBSTATE);
  static void RestoreTarget10Key (const std::string&,KBSTATE);
  static void RequestClearenceKey(const std::string&,KBSTATE);
  static void UnDockKey(const std::string&,KBSTATE);
  static void EjectKey(const std::string&,KBSTATE);
  static void EjectCargoKey(const std::string&,KBSTATE);
  static void DockKey(const std::string&,KBSTATE);
  static void FireKey(const std::string&,KBSTATE);
  static void MissileKey(const std::string&,KBSTATE);
  static void TargetKey(const std::string&,KBSTATE k);
  static void PickTargetKey(const std::string&,KBSTATE k);
  static void NearestTargetKey(const std::string&,KBSTATE k);
  static void SubUnitTargetKey(const std::string&,KBSTATE k);
  static void ThreatTargetKey(const std::string&,KBSTATE k);
  static void UnitTargetKey(const std::string&,KBSTATE k);
  static void SigTargetKey(const std::string&,KBSTATE k);
  static void ReverseTargetKey(const std::string&,KBSTATE k);
  static void ReversePickTargetKey(const std::string&,KBSTATE k);
  static void ReverseNearestTargetKey(const std::string&,KBSTATE k);
  static void ReverseThreatTargetKey(const std::string&,KBSTATE k);
  static void ReverseUnitTargetKey(const std::string&,KBSTATE k);
  static void ReverseSigTargetKey(const std::string&,KBSTATE k);
  static void NearestTargetTurretKey (const std::string&,KBSTATE);
  static void ThreatTargetTurretKey (const std::string&,KBSTATE);
  static void TargetTurretKey(const std::string&,KBSTATE);
  static void PickTargetTurretKey(const std::string&,KBSTATE);
  static void JFireKey(KBSTATE,float,float,int);  
  static void JMissileKey(KBSTATE,float,float,int);  
  static void JTargetKey(KBSTATE,float,float,int);  
  static void WeapSelKey (const std::string&,KBSTATE);
  static void MisSelKey (const std::string&,KBSTATE);
  static void ReverseWeapSelKey (const std::string&,KBSTATE);
  static void ReverseMisSelKey (const std::string&,KBSTATE);
  static void CloakKey (const std::string&, KBSTATE);
  static void LockKey (const std::string&, KBSTATE);
  static void ECMKey (const std::string&,KBSTATE);
  static void HelpMeOut (const std::string&,KBSTATE);
  static void HelpMeOutFaction (const std::string&,KBSTATE);

  static void HelpMeOutCrit (const std::string&,KBSTATE);
  static void JoinFg (const std::string&,KBSTATE);
  static void BreakFormation (const std::string&,KBSTATE);
  static void FormUp (const std::string&,KBSTATE);
  static void AttackTarget (const std::string&,KBSTATE);
  static void TurretAI (const std::string&,KBSTATE);
protected:
  void * savedTargets[NUMSAVEDTARGETS];
  float distance;
#if 0
  void ChooseTargets(bool targetturrets);//chooses n targets and puts the best to attack in unit's target container
  void ChooseRTargets(bool targetturrets);//chooses n targets and puts the best to attack in unit's target container
  void ChooseNearTargets(bool targetturrets);//chooses n targets and puts the best to attack in unit's target container
  void ChooseThreatTargets(bool targetturrets);//chooses n targets and puts the best to attack in unit's target container
  void PickTargets(bool targetturrets); // chooses the target which is nearest to the center of the screen
#endif
  unsigned int whichplayer;
  unsigned int whichjoystick;
  struct FIREKEYBOARDTYPE &f();
  struct FIREKEYBOARDTYPE &j();
  unsigned int DoSpeechAndAni(Unit * un, Unit * parent, class CommunicationMessage&c);
public:
  virtual void ProcessCommMessage (class CommunicationMessage&c);
  FireKeyboard (unsigned int whichjoystick, unsigned int whichplayer);//weapon prefs?
  virtual void Execute();
  virtual ~FireKeyboard();
};




#endif


