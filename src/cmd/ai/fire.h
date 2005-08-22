#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "comm_ai.h"
#include "event_xml.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
bool RequestClearence(class Unit *parent, class Unit *targ, unsigned char sex);
Unit * getAtmospheric (Unit * targ);
namespace Orders {

class FireAt: public CommunicatingAI {

protected:
  bool ShouldFire(Unit * targ,bool &missilelock);
  float missileprobability;
  float lastmissiletime;
  float rxntime;
  float delay;
  float agg;
  float distance;
  float gunspeed;
  float gunrange;
  float missilerange;
  float lastchangedtarg;
  bool had_target;
  void FireWeapons (bool shouldfire,bool lockmissile);
  //  bool DealWithMultipleTargets();
  virtual void ChooseTargets(int num, bool force=false);//chooses n targets and puts the best to attack in unit's target container
  bool isJumpablePlanet(Unit *);
  void ReInit (float rxntime, float agglevel);
  virtual void SignalChosenTarget();
public:
  //Other new Order functions that can be called from Python.
  virtual void ChooseTarget () {
    ChooseTargets (1,true);
  }
  void PossiblySwitchTarget(bool istargetjumpableplanet);
  virtual void getAverageGunSpeed (float & speed, float & range, float &missilerange) const;
  virtual bool PursueTarget(Unit *, bool leader);
  void AddReplaceLastOrder (bool replace);
  void ExecuteLastScriptFor(float time);
  void FaceTarget (bool end);
  void FaceTargetITTS (bool end);
  void MatchLinearVelocity(bool terminate, Vector vec, bool afterburn, bool local);
  void MatchAngularVelocity(bool terminate, Vector vec, bool local);
  void ChangeHeading(QVector vec);
  void ChangeLocalDirection(Vector vec);
  void MoveTo(QVector, bool afterburn);
  void MatchVelocity(bool terminate, Vector vec, Vector angvel, bool afterburn, bool local);
  void Cloak(bool enable,float seconds);
  void FormUp(QVector pos);
  void FormUpToOwner(QVector pos);
  void FaceDirection (float distToMatchFacing, bool finish);
  void XMLScript (string script);
  void LastPythonScript();

  virtual void SetParent (Unit * parent) {
	  CommunicatingAI::SetParent (parent);
  }
  Unit * GetParent() {return CommunicatingAI::GetParent();}
  FireAt (float reaction_time, float aggressivitylevel);//weapon prefs?
  FireAt();
  virtual void Execute();
  virtual std::string Pickle() {return std::string();}//these are to serialize this AI
  virtual void UnPickle(std::string) {}
  virtual ~FireAt();
};



}
#endif


