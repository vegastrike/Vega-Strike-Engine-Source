#include "script.h"
#include "cmd/unit.h"
#include "hard_coded_scripts.h"
#include "flybywire.h"
#include "navigation.h"
#include "tactics.h"
void AddOrd (AIScript *aisc, Unit * un, Order * ord) {
  ord->SetParent (un);
  aisc->EnqueueOrder (ord);
}

void AfterburnTurnTowards (AIScript * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,true),true,true,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);    
}
void AfterburnTurnTowardsITTS (AIScript * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,true),true,true,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTargetITTS(false, 3));
  AddOrd (aisc,un,ord);    
}

void Evade(AIScript * aisc, Unit * un) {
  Order * ord = new Orders::ChangeHeading (un->Position()+Vector(1,0,1),2);
  AddOrd (aisc,un,ord);
  ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Vector (-10000,0,10000),true),false,true,true);
  AddOrd (aisc,un,ord);
  ord = new Orders::ChangeHeading (un->Position()+Vector(-1,0,1),2);
  AddOrd (aisc,un,ord);
  ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Vector (10000,0,10000),true),false, true,true);  
  AddOrd (aisc,un,ord);
}
void MoveTo(AIScript * aisc, Unit * un) {
  Vector Targ (un->Position());
  Unit * untarg = un->Target();
  if (untarg) {
    Targ = untarg->Position();
  }
  Order * ord = new Orders::MoveTo(Targ,false,3);
  AddOrd (aisc,un,ord);
}
void Kickstop(AIScript * aisc, Unit * un) {
  Vector vec (0,0,-4);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,true);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);  
}

static void SetupVAndTargetV (Vector & targetv, Vector &targetpos, Unit* un) {
  Unit *targ;
  if ((targ = un->Target())) {
    targetv = targ->GetVelocity();
    targetpos = targ->Position();
  }  
}

void SheltonSlide(AIScript * aisc, Unit * un) {
  Vector def (un->Position()+Vector(1,0,0));
  Vector targetv(def);
  Vector targetpos(def);
  SetupVAndTargetV(targetpos,targetv,un);
  Vector difference = targetpos-un->Position();
  Vector perp = targetv.Cross (-difference);
  perp.Normalize();
  perp=perp*(targetv.Dot(difference*-1./(difference.Magnitude())));
  perp =(perp+difference)*10000;
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(perp,true),false,true,true);  
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);
}

void AfterburnerSlide(AIScript * aisc, Unit * un) {
  Vector def = un->Position()+Vector(1,0,0);
  Vector targetv (def);
  Vector targetpos(def);
  SetupVAndTargetV(targetpos,targetv,un);

  Vector difference = targetpos-un->Position();
  Vector perp = targetv.Cross (-difference);
  perp.Normalize();
  perp=perp*(targetv.Dot(difference*-1./(difference.Magnitude())));
  perp =(perp+difference)*1000;
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(perp,true),false,true,true);  
  AddOrd (aisc,un,ord);
  ord = new ExecuteFor (new Orders::ChangeHeading (perp+un->Position(),3),1.5);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);
}
void SkilledABSlide (AIScript * aisc, Unit * un) {
  Vector def = un->Position()+Vector(1,0,0);
  Vector targetv (def);
  Vector targetpos (def);
  SetupVAndTargetV(targetpos,targetv,un);

  Vector difference = targetpos-un->Position();
  Vector ndifference = difference;ndifference.Normalize();
  Vector Perp;
  ScaledCrossProduct (ndifference,targetv,Perp);
  Perp = Perp+.5*ndifference;
  Perp = Perp *10000;



  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Perp,true),false,true,true);  
  AddOrd (aisc,un,ord);
  ord = new ExecuteFor (new Orders::ChangeHeading (Perp+un->Position(),3),.5);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);
  
}
void Stop (AIScript * aisc, Unit * un) {
  Vector vec (0,0,0000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);//<!-- should we fini? -->
}
void TurnAway(AIScript * aisc, Unit * un) {
  Vector v(un->Position());
  Vector u(v);
  Unit * targ =un->Target();
  if (targ) {
    u=targ->Position();
  }
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(200*(v-u),true),false,true,false);
  AddOrd (aisc,un,ord);
  ord = new ExecuteFor (new Orders::ChangeHeading ((200*(v-u)) + v,3), 2);
  AddOrd (aisc,un,ord);
}
void TurnTowards(AIScript * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new ExecuteFor (new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false),4);
  AddOrd (aisc,un,ord);
  ord=new CloakFor(1,4);
  AddOrd (aisc,un,ord);
  ord =       new ExecuteFor(new Orders::FaceTarget(0, 3),2);
  AddOrd (aisc,un,ord);
}
void CloakForScript(AIScript * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTarget(0, 3));
  AddOrd (aisc,un,ord);
  
}
void TurnTowardsITTS(AIScript * aisc, Unit * un) {
  Vector vec (0,0,10000);
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,false);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTargetITTS(0, 3));
  AddOrd (aisc,un,ord);    

}
