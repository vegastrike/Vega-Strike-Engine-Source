#include "script.h"
#include "cmd/unit_generic.h"
#include "hard_coded_scripts.h"
#include "flybywire.h"
#include "navigation.h"
#include "tactics.h"
#include "fire.h"
#include "order.h"
#include "python/python_class.h"
using Orders::FireAt;

BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE

#ifdef USE_BOOST_129
#else
extern PyObject *to_python (Unit *x);
#endif

BOOST_PYTHON_END_CONVERSION_NAMESPACE

void AddOrd (Order *aisc, Unit * un, Order * ord) {
  ord->SetParent (un);
  aisc->EnqueueOrder (ord);
}
void ReplaceOrd (Order *aisc, Unit * un, Order * ord) {
  ord->SetParent (un);
  aisc->ReplaceOrder (ord);
}
static Order * lastOrder=NULL;
void FireAt::AddReplaceLastOrder (bool replace) {
	if (lastOrder) {
		if (replace) {
			ReplaceOrd (this,parent,lastOrder);
		}else {
			AddOrd (this,parent,lastOrder);
		}
		lastOrder=NULL;
	}
}
void FireAt::ExecuteLastScriptFor(float time) {
	if (lastOrder) {
		lastOrder = new ExecuteFor(lastOrder,time);
	}
}
void FireAt::FaceTarget (bool end) {
	lastOrder = new Orders::FaceTarget(end,4);
}
void FireAt::FaceTargetITTS (bool end) {
	lastOrder  = new Orders::FaceTargetITTS(end,4);
}
void FireAt::MatchLinearVelocity(bool terminate, Vector vec, bool afterburn, bool local) {
	lastOrder  = new Orders::MatchLinearVelocity(parent->ClampVelocity(vec,afterburn),local,afterburn,terminate);
}
void FireAt::MatchAngularVelocity(bool terminate, Vector vec, bool local) {
	lastOrder  = new Orders::MatchAngularVelocity(parent->ClampAngVel(vec),local,terminate);
}
void FireAt::ChangeHeading(QVector vec) {
	lastOrder  = new Orders::ChangeHeading (vec,3);
}
void FireAt::ChangeLocalDirection(Vector vec) {
	lastOrder  = new Orders::ChangeHeading (((parent->Position().Cast())+parent->ToWorldCoordinates(vec)).Cast(),3);
}
void FireAt::MoveTo(QVector vec, bool afterburn) {
	lastOrder  = new Orders::MoveTo(vec,afterburn,3);
}
void FireAt::MatchVelocity(bool terminate, Vector vec, Vector angvel, bool afterburn, bool local) {
	lastOrder  = new Orders::MatchVelocity(parent->ClampVelocity(vec,afterburn),parent->ClampAngVel(angvel),local,afterburn,terminate);
}
void FireAt::Cloak(bool enable,float seconds) {
	lastOrder  = new CloakFor(enable,seconds);
}
void FireAt::FormUp(QVector pos) {
	lastOrder  = new Orders::FormUp(pos);
}
void FireAt::FaceDirection (float distToMatchFacing, bool finish) {
	lastOrder  = new Orders::FaceDirection (distToMatchFacing, finish, 3);
}
void FireAt::XMLScript (string script) {
	lastOrder  = new AIScript (script.c_str());
}
void FireAt::LastPythonScript () {
	lastOrder = PythonAI<Orders::FireAt>::LastPythonClass();
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
  QVector v(un->Position());
  QVector u(v);
  Unit * targ =un->Target();
  if (targ) {
    u=targ->Position();
  }
  Order *ord = new Orders::ChangeHeading ((200*(v-u)) + v,3);
  AddOrd (aisc,un,ord);
  ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Vector (-10000,0,10000),true),false,true,true);
  AddOrd (aisc,un,ord);
  ord = new Orders::FaceTargetITTS(false,3);
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

void KickstopBase(Order * aisc, Unit * un, bool match) {
  Vector vec (0,0,0);
  if (match&&un->Target())
    vec=un->Target()->GetVelocity();
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,true);
  AddOrd (aisc,un,ord);
  ord =       (new Orders::FaceTargetITTS(false, 3));
  AddOrd (aisc,un,ord);  
}
void Kickstop(Order * aisc, Unit * un) {
  KickstopBase(aisc,un,false);
}
void MatchVelocity(Order * aisc, Unit * un) {
  KickstopBase(aisc,un,false);
}
Vector VectorThrustHelper(Order * aisc, Unit * un) {
  Vector vec (0,0,0);
  Vector retval(0,0,0);
  if (un->Target()) {
    Vector tpos=un->Target()->Position().Cast();
    Vector relpos=tpos-un->Position().Cast();
    CrossProduct(relpos,Vector(1,0,0),vec);
    retval+=tpos;
  }
  Order * ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec,false),true,false,true);
  AddOrd (aisc,un,ord);
  return retval;
}
void VeerAway(Order * aisc, Unit * un) {
  VectorThrustHelper (aisc,un);
  Order *ord =       (new Orders::FaceTarget(false, 3));
  AddOrd (aisc,un,ord);  
}
void VeerAwayITTS(Order * aisc, Unit * un) {
  VectorThrustHelper (aisc,un);
  Order *ord =       (new Orders::FaceTargetITTS(false, 3));
  AddOrd (aisc,un,ord);  
}
void VeerAndTurnAway(Order * aisc, Unit * un) {
  Vector retval=VectorThrustHelper (aisc,un);
  Order *ord = new Orders::ChangeHeading(retval,3,1);
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
  ord =       (new Orders::FaceTargetITTS(false, 3));
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
  ord =       (new Orders::FaceTargetITTS(false, 3));
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
  ord =       (new Orders::FaceTargetITTS(false, 3));
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
  ord =       (new Orders::FaceTargetITTS(0, 3));
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
