#include "script.h"
#include "cmd/unit.h"
#include "hard_coded_scripts.h"
#include "flybywire.h"
#include "navigation.h"
#include "tactics.h"
void AddOrd (Order *aisc, Unit * un, Order * ord) {
  ord->SetParent (un);
  aisc->EnqueueOrder (ord);
}

void AfterburnTurnTowards (Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,true),true,true,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);    
}
void AfterburnTurnTowardsITTS (Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,true),true,true,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTargetITTS(false, 3));
  AddOrd (aisc,un,ord);    
}

void Evade(Order * aisc, Unit * un) {
  Order * ord = new Orders::ChangeHeading (un->Position()+QVector(1,0,1),2);
  AddOrd (aisc,un,ord);
  ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Vector (-10000,0,10000),true),false,true,true);
  AddOrd (aisc,un,ord);
  ord = new Orders::ChangeHeading (un->Position()+QVector(-1,0,1),2);
  AddOrd (aisc,un,ord);
  ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Vector (10000,0,10000),true),false, true,true);  
  AddOrd (aisc,un,ord);
}
void MoveTo(Order * aisc, Unit * un) {
  QVector Targ (un->Position());
  Unit * untarg = un->Target();
  if (untarg) {
    Targ = untarg->Position();
  }
  Order * ord = new Orders::MoveTo(Targ,false,3);
  AddOrd (aisc,un,ord);
}
void Kickstop(Order * aisc, Unit * un) {
  Vector vec (0,0,-4);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,true);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);  
}

static void SetupVAndTargetV (QVector & targetv, QVector &targetpos, Unit* un) {
  Unit *targ;
  if ((targ = un->Target())) {
    targetv = targ->GetVelocity().Cast();
    targetpos = targ->Position();
  }  
}

void SheltonSlide(Order * aisc, Unit * un) {
  QVector def (un->Position()+QVector(1,0,0));
  QVector targetv(def);
  QVector targetpos(def);
  SetupVAndTargetV(targetpos,targetv,un);
  QVector difference = targetpos-un->Position();
  QVector perp = targetv.Cross (-difference);
  perp.Normalize();
  perp=perp*(targetv.Dot(difference*-1./(difference.Magnitude())));
  perp =(perp+difference)*10000.;
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(perp.Cast(),true),false,true,true);  
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);
}

void AfterburnerSlide(Order * aisc, Unit * un) {
  QVector def = un->Position()+QVector(1,0,0);
  QVector targetv (def);
  QVector targetpos(def);
  SetupVAndTargetV(targetpos,targetv,un);

  QVector difference = targetpos-un->Position();
  QVector perp = targetv.Cross (-difference);
  perp.Normalize();
  perp=perp*(targetv.Dot(difference*-1./(difference.Magnitude())));
  perp =(perp+difference)*1000;
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(perp.Cast(),true),false,true,true);  
  AddOrd (aisc,un,ord);
  ord = new ExecuteFor (new Orders::ChangeHeading (perp+un->Position(),3),1.5);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);
}
void SkilledABSlide (Order * aisc, Unit * un) {
  QVector def = un->Position()+QVector(1,0,0);
  QVector targetv (def);
  QVector targetpos (def);
  SetupVAndTargetV(targetpos,targetv,un);

  QVector difference = targetpos-un->Position();
  QVector ndifference = difference;ndifference.Normalize();
  QVector Perp;
  ScaledCrossProduct (ndifference,targetv,Perp);
  Perp = Perp+.5*ndifference;
  Perp = Perp *10000;



  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Perp.Cast(),true),false,true,true);  
  AddOrd (aisc,un,ord);
  ord = new ExecuteFor (new Orders::ChangeHeading (Perp+un->Position(),3),.5);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);
  
}
void Stop (Order * aisc, Unit * un) {
  Vector vec (0,0,0000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);//<!-- should we fini? -->
}
void TurnAway(Order * aisc, Unit * un) {
  QVector v(un->Position());
  QVector u(v);
  Unit * targ =un->Target();
  if (targ) {
    u=targ->Position();
  }
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(200*(v-u).Cast(),true),false,true,false);
  AddOrd (aisc,un,ord);
  ord = new Orders::ChangeHeading ((200*(v-u)) + v,3);
  AddOrd (aisc,un,ord);
}
void TurnTowards(Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);

  ord = new Orders::FaceTarget(0, 3);
    AddOrd (aisc,un,ord);
}
void FlyStraight(Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchVelocity(un->ClampVelocity(vec,false),Vector(0,0,0),true,false,false);
  AddOrd (aisc,un,ord);
}
void FlyStraightAfterburner(Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchVelocity(un->ClampVelocity(vec,false),Vector(0,0,0),true,true,false);
  AddOrd (aisc,un,ord);
}
void CloakForScript(Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(0, 3));
  AddOrd (aisc,un,ord);
  ord=new ExecuteFor(new CloakFor(1,8),32);
  AddOrd(aisc,un,ord);
}
void TurnTowardsITTS(Order * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTargetITTS(0, 3));
  AddOrd (aisc,un,ord);    

}
