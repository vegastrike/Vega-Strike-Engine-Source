
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"
#include "in_joystick.h"
#include "cmd/unit.h"
#include "communication.h"
#include "gfx/cockpit.h"
#include "gfx/animation.h"
#include "audiolib.h"
#include "config_xml.h"
#include "cmd/images.h"
#include "cmd/planet.h"
#include "cmd/script/flightgroup.h"
FireKeyboard::FireKeyboard (unsigned int whichplayer, unsigned int whichjoystick): Order (WEAPON,0){
  this->whichjoystick = whichjoystick;
  this->whichplayer=whichplayer;
  gunspeed = gunrange = .0001;
  refresh_target=true;
  sex = XMLSupport::parse_int( vs_config->getVariable ("player","sex","0"));
}
const unsigned int NUMCOMMKEYS=10;
struct FIREKEYBOARDTYPE {
  FIREKEYBOARDTYPE() {
    commKeys[0]=commKeys[1]=commKeys[2]=commKeys[3]=commKeys[4]=commKeys[5]=commKeys[6]=commKeys[7]=commKeys[8]=commKeys[9]=turretaikey = UP;
    eject=ejectcargo=firekey=missilekey=jfirekey=rtargetkey=jtargetkey=jmissilekey=weapk=misk=cloakkey=neartargetkey=threattargetkey=picktargetkey=targetkey=nearturrettargetkey =threatturrettargetkey= pickturrettargetkey=turrettargetkey=UP;
    doc=und=req=0;
  }
 KBSTATE firekey;
 bool doc;
 bool und;
 bool req;
 KBSTATE rtargetkey;
 KBSTATE missilekey;
 KBSTATE jfirekey;
 KBSTATE jtargetkey;
 KBSTATE jmissilekey;
 KBSTATE weapk;
 KBSTATE misk;
 KBSTATE eject;
 KBSTATE ejectcargo;
 KBSTATE cloakkey;
 KBSTATE ECMkey;
 KBSTATE neartargetkey;
 KBSTATE threattargetkey;
 KBSTATE picktargetkey;
 KBSTATE targetkey;
 KBSTATE turretaikey;

  KBSTATE commKeys[NUMCOMMKEYS];
 KBSTATE nearturrettargetkey;
 KBSTATE threatturrettargetkey;
 KBSTATE pickturrettargetkey;
 KBSTATE turrettargetkey;
};
static std::vector <FIREKEYBOARDTYPE> vectorOfKeyboardInput;
static FIREKEYBOARDTYPE &g() {
  while (vectorOfKeyboardInput.size()<=(unsigned int)_Universe->CurrentCockpit()||vectorOfKeyboardInput.size()<=(unsigned int)MAX_JOYSTICKS) {
    vectorOfKeyboardInput.push_back(FIREKEYBOARDTYPE());
  }
  return vectorOfKeyboardInput [_Universe->CurrentCockpit()];
}
FIREKEYBOARDTYPE &FireKeyboard::f() {
  return vectorOfKeyboardInput[whichplayer];
}
FIREKEYBOARDTYPE &FireKeyboard::j() {
  return vectorOfKeyboardInput[whichjoystick];
}

void FireKeyboard::PressComm1Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[0]=PRESS;
  }
}
void FireKeyboard::PressComm2Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[1]=PRESS;
  }
}
void FireKeyboard::PressComm3Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[2]=PRESS;
  }
}
void FireKeyboard::PressComm4Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[3]=PRESS;
  }
}
void FireKeyboard::PressComm5Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[4]=PRESS;
  }
}
void FireKeyboard::PressComm6Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[5]=PRESS;
  }
}
void FireKeyboard::PressComm7Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[6]=PRESS;
  }
}
void FireKeyboard::PressComm8Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[7]=PRESS;
  }
}
void FireKeyboard::PressComm9Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[8]=PRESS;
  }
}
void FireKeyboard::PressComm10Key (int, KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[9]=PRESS;
  }
}


void FireKeyboard::RequestClearenceKey(int, KBSTATE k) {

    if (k==PRESS) {
      g().req=true;      
    }
    if (k==RELEASE) {
      g().req=false;      
    }
}
void FireKeyboard::DockKey(int, KBSTATE k) {

    if (k==PRESS) {
      g().doc=true;      
    }
    if (k==RELEASE) {
      g().doc=false;      
    }
}
void FireKeyboard::UnDockKey(int, KBSTATE k) {
    if (k==PRESS) {
      g().und=true;      
    }
    if (k==RELEASE) {
      g().und=false;      
    }
}
void FireKeyboard::EjectKey (int, KBSTATE k) {
  if (k==PRESS) {
    g().eject= k;
  }
}


void FireKeyboard::TurretAI (int, KBSTATE k) {
  if (k==PRESS) {
    if (g().turretaikey==UP) {

      g().turretaikey=PRESS;
    }else {

      g().turretaikey=RELEASE;
    }
  }
}


void FireKeyboard::EjectCargoKey (int, KBSTATE k) {
    if (k==PRESS) {
      g().ejectcargo = k;      
    }
  
}
void FireKeyboard::CloakKey(int, KBSTATE k) {

    if (k==PRESS) {
      g().cloakkey = k;      
    }
}
void FireKeyboard::ECMKey(int, KBSTATE k) {

    if (k==PRESS) {
      g().ECMkey = k;      
    }
    if (k==RELEASE) {
      g().ECMkey = k;
    }
}
void FireKeyboard::FireKey(int key, KBSTATE k) {
  if(g().firekey==DOWN && k==UP){
    return;
  }
  if (k==UP&&g().firekey==RELEASE) {

  } else {

    g().firekey = k;
    //    printf("firekey %d %d\n",k,key);
  }
}

void FireKeyboard::TargetKey(int, KBSTATE k) {
  if (g().targetkey!=PRESS)
    g().targetkey = k;
  if (k==RESET) {
    g().targetkey=PRESS;
  }
}
void FireKeyboard::ReverseTargetKey(int, KBSTATE k) {
  if (g().rtargetkey!=PRESS)
    g().rtargetkey = k;
  if (k==RESET) {
    g().rtargetkey=PRESS;
  }
}
void FireKeyboard::PickTargetKey(int, KBSTATE k) {
  if (g().picktargetkey!=PRESS)
    g().picktargetkey = k;
  if (k==RESET) {
    g().picktargetkey=PRESS;
  }
}

void FireKeyboard::NearestTargetKey(int, KBSTATE k) {
  if (g().neartargetkey!=PRESS)
    g().neartargetkey = k;

}
void DoSpeech (Unit * un, const string &speech) {
  string myname ("[Static]");
  if (un) {
    myname= un->name;
  }
  mission->msgcenter->add ("game","all",myname+string(": ")+speech);
}
void LeadMe (Unit * un, string directive, string speech) { 
  if (un!=NULL) {
    for (int i=0;i<_Universe->numPlayers();i++) {
      Unit * pun =_Universe->AccessCockpit(i)->GetParent();
      if (pun) {
	if (pun->getFlightgroup()==un->getFlightgroup()){
	  DoSpeech (un, speech);	
	}
      }
    }
    Flightgroup * fg = un->getFlightgroup();
    if (fg) {
      if (fg->leader.GetUnit()!=un) {
	fg->leader.SetUnit (un);
      }
      fg->directive = directive;
    }
  }
}
static void LeadMe (string directive, string speech) {
  Unit * un= _Universe->AccessCockpit()->GetParent();
  LeadMe (un,directive,speech);
  
}
extern Unit * GetThreat (Unit * par, Unit * leader);
void HelpOut (bool crit, std::string conv) {
  Unit * un = _Universe->AccessCockpit()->GetParent();
  if (un) {
    Unit * par=NULL;
    DoSpeech(  un,conv);
    for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator();
	 (par = (*ui));
	 ++ui) {
      if ((crit&&_Universe->GetRelation(par->faction,un->faction)>0)||par->faction==un->faction) {
	Unit * threat = GetThreat (par,un);
	CommunicationMessage c(par,un,NULL,0);
	if (threat) {
	  par->Target(threat);
	  c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
	} else {
	  c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	}
	un->getAIState()->Communicate (c);
      }
    }
  }
}
void FireKeyboard::JoinFg (int, KBSTATE k) {
  if (k==PRESS) {
    Unit * un = _Universe->AccessCockpit()->GetParent();
    if (un) {
      Unit * targ = un->Target();
      if (targ) {
	if (targ->faction==un->faction) {
	  Flightgroup * fg = targ->getFlightgroup();
	  if (fg) {
	    if (fg!=un->getFlightgroup()) {
	      if (un->getFlightgroup()) {
		un->getFlightgroup()->Decrement(un);
	      }
	      fg->nr_ships_left++;
	      fg->nr_ships++;
	      un->SetFg(fg,fg->nr_ships_left-1);
	    }
	  }
	}
      }

    }
  }

}

void FireKeyboard::AttackTarget (int, KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("a","Attack my target!");
  }
}
void FireKeyboard::HelpMeOut (int, KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("h","Help me out!");
  }
}
void FireKeyboard::HelpMeOutFaction (int, KBSTATE k) {
  if (k==PRESS) {
    HelpOut (false,"Help me out! I need critical assistance!");
  }
}
void FireKeyboard::HelpMeOutCrit (int, KBSTATE k) {
  if (k==PRESS) {
    HelpOut (true,"Help me out! Systems going critical!");
  }
}

void FireKeyboard::FormUp (int, KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("f","Form on my wing.");
  }
}
void FireKeyboard::BreakFormation(int, KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("b","Break formation and open fire!");
  }
}
void FireKeyboard::ThreatTargetKey(int, KBSTATE k) {
  if (g().threattargetkey!=PRESS)
    g().threattargetkey = k;
}


void FireKeyboard::TargetTurretKey(int, KBSTATE k) {
  if (g().turrettargetkey!=PRESS)
    g().turrettargetkey = k;
  if (k==RESET) {
    g().turrettargetkey=PRESS;
  }
}
void FireKeyboard::PickTargetTurretKey(int, KBSTATE k) {
  if (g().pickturrettargetkey!=PRESS)
    g().pickturrettargetkey = k;
  if (k==RESET) {
    g().pickturrettargetkey=PRESS;
  }
}

void FireKeyboard::NearestTargetTurretKey(int, KBSTATE k) {
  if (g().nearturrettargetkey!=PRESS)
    g().nearturrettargetkey = k;

}
void FireKeyboard::ThreatTargetTurretKey(int, KBSTATE k) {
  if (g().threatturrettargetkey!=PRESS)
    g().threatturrettargetkey = k;
}



void FireKeyboard::WeapSelKey(int, KBSTATE k) {
  if (g().weapk!=PRESS)
    g().weapk = k;
}
void FireKeyboard::MisSelKey(int, KBSTATE k) {
  if (g().misk!=PRESS)
    g().misk = k;
} 

void FireKeyboard::MissileKey(int, KBSTATE k) {
  if (g().missilekey!=PRESS)
   g().missilekey = k;
}
void FireKeyboard::ChooseNearTargets(bool turret) {
  UnitCollection::UnitIterator iter = _Universe->activeStarSystem()->getUnitList().createIterator();
  Unit * un;
  float range=FLT_MAX;
  while ((un = iter.current())) {
    Vector t;
    bool tmp = parent->InRange (un);
    t = parent->LocalCoordinates (un);
    if (tmp&&t.Dot(t)<range&&t.k>0&&_Universe->GetRelation(parent->faction,un->faction)<0) {
      range = t.Dot(t);
      if (turret)
	parent->TargetTurret(un);
      parent->Target (un);
    }
    iter.advance();
  }
#ifdef ORDERDEBUG
  fprintf (stderr,"i4%x",iter);
  fflush (stderr);
#endif

#ifdef ORDERDEBUG
  fprintf (stderr,"i4\n");
  fflush (stderr);
#endif

}
void FireKeyboard::ChooseThreatTargets(bool turret) {
  Unit * threat = parent->Threat();
  if (threat) 
    parent->Target(threat);
  if (turret&&threat)
    parent->TargetTurret(threat);
}

void FireKeyboard::PickTargets(bool Turrets){
  UnitCollection::UnitIterator uiter = _Universe->activeStarSystem()->getUnitList().createIterator();

  float smallest_angle=PI;

  Unit *other=uiter.current();
  Unit *found_unit=NULL;

  while(other){
    if(other!=parent && parent->InRange(other)){
      Vector p,q,r;
      QVector vectothem=QVector(other->Position() - parent->Position()).Normalize();
      parent->GetOrientation(p,q,r);
      double angle=acos( vectothem.Dot(r.Cast()) );
      
      if(angle<smallest_angle){
	found_unit=other;
	smallest_angle=angle;
      }
    }
    uiter.advance();
    other=uiter.current();
  }
  if (Turrets)
    parent->TargetTurret(found_unit);
  parent->Target(found_unit);

}
void FireKeyboard::ChooseRTargets (bool turret) {
  Unit * targ =parent->Target();
  Unit * oldun=NULL;
  Unit * un=NULL;
  un_iter i;
  for (i=_Universe->activeStarSystem()->getUnitList().createIterator();
       (un=*i)!=NULL;
       ++i) {
    if (un==targ) {
      break;
    }else {
      if (un!=parent) {
	oldun=un;
      }
    }
  }
  if (oldun==NULL) {//get us the last
    for (i=_Universe->activeStarSystem()->getUnitList().createIterator();
	 (un=*i)!=NULL;
	 ++i) {
      if (un!=parent) {
	oldun=un;
      }
    }
  }
  parent->Target (oldun);
  if (turret) {
	parent->TargetTurret (un);
  }
}

void FireKeyboard::ChooseTargets (bool turret) {
  UnitCollection::UnitIterator iter = _Universe->activeStarSystem()->getUnitList().createIterator();
  Unit * un ;
  bool found=false;
  bool find=false;
  while ((un = iter.current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    Vector t;

    if (un==parent->Target()) {
      iter.advance();
      found=true;
      continue;
    }
    if (!parent->InRange(un)) {
      iter.advance();
      continue;
    }
    iter.advance();
    if (found) {
      find=true;
      if (turret)
	parent->TargetTurret (un);
      parent->Target (un);
      break;
    }
  }
  //  if ((un = iter->current())) {


    //  }
#ifdef ORDERDEBUG
  fprintf (stderr,"i5%x",iter);
  fflush (stderr);
#endif

  
#ifdef ORDERDEBUG
  fprintf (stderr,"i5\n");
  fflush (stderr);
#endif
  if (!find) {
    iter = _Universe->activeStarSystem()->getUnitList().createIterator();
    while ((un = iter.current())) {
      //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
      Vector t;
      if (un==parent->Target()){
	iter.advance();
	continue;
      }
      if (!parent->InRange(un)) {
	iter.advance();
	continue;
      }
      if (turret)
	parent->TargetTurret(un);
      parent->Target(un);
      break;
    }
  }
}
FireKeyboard::~FireKeyboard () {
#ifdef ORDERDEBUG
  fprintf (stderr,"fkb%x",this);
  fflush (stderr);
#endif

}
bool FireKeyboard::ShouldFire(Unit * targ) {
  float dist;
  if (gunspeed==.0001) {
    parent->getAverageGunSpeed (gunspeed,gunrange);
  }
  float angle = parent->cosAngleTo (targ, dist,gunspeed,gunrange);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<.8*agg&&angle>1/agg);
}
static bool UnDockNow (Unit* me, Unit * targ) {
  bool ret=false;
  Unit * un;
  for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
       (un = *i)!=NULL;
       ++i) {
    if (un->isDocked (me)) {
      if (me->UnDock (un)) {
	ret=true;
      }
    }
  }
  return ret;
}
static Unit * getAtmospheric (Unit * targ) {
  if (targ) {
    Unit * un;
    for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
	 (un=*i)!=NULL;
	 ++i) {
      if (un->isUnit()==PLANETPTR) {
	if ((targ->Position()-un->Position()).Magnitude()<targ->rSize()*.5) {
	  if (!(((Planet *)un)->isAtmospheric())) {
	    return un;
	  }
	}
      }
    }
  }
  return NULL;
  
}
static void DoDockingOps (Unit * parent, Unit * targ,unsigned char playa, unsigned char sex) {
  static bool nodockwithclear = XMLSupport::parse_bool (vs_config->getVariable ("physics","dock_with_clear_planets","true"));
    if (vectorOfKeyboardInput[playa].req) {
      if (targ->isUnit()==PLANETPTR) {

	if (((Planet * )targ)->isAtmospheric()&&nodockwithclear) {
	  targ = getAtmospheric (targ);
	  if (!targ) {
	    mission->msgcenter->add("game","all","[Computer] Cannot dock with insubstantial object, target another object and retry.");
	    return;
	  }
	}
      }
      //      fprintf (stderr,"request %d", targ->RequestClearance (parent));
      CommunicationMessage c(parent,targ,NULL,sex);
      c.SetCurrentState(c.fsm->GetRequestLandNode(),NULL,sex);
      targ->getAIState()->Communicate (c);
      vectorOfKeyboardInput[playa].req=false;
    }
    if (vectorOfKeyboardInput[playa].doc) {
      if (targ->isUnit()==PLANETPTR) {
      if (((Planet * )targ)->isAtmospheric()&&nodockwithclear) {
	targ = getAtmospheric (targ);
	if (!targ) {
	  mission->msgcenter->add("game","all","[Computer] Cannot dock with insubstantial object, target another object and retry.");
	  return;
	}
      }
      }
      CommunicationMessage c(targ,parent,NULL,0);

      bool hasDock = parent->Dock(targ);
      if (hasDock) {
	  c.SetCurrentState (c.fsm->GetDockNode(),NULL,0);
      }else {
        if (UnDockNow(parent,targ)) {
	  c.SetCurrentState (c.fsm->GetUnDockNode(),NULL,0);
        }else {
          //docking is unsuccess
	  c.SetCurrentState (c.fsm->GetFailDockNode(),NULL,0);
        }
      }
      parent->getAIState()->Communicate (c);
      vectorOfKeyboardInput[playa].doc=false;
    }
    if (vectorOfKeyboardInput[playa].und) {
      CommunicationMessage c(targ,parent,NULL,0);
      if (UnDockNow(parent,targ)) {
	c.SetCurrentState (c.fsm->GetUnDockNode(),NULL,0);
      }else {
	c.SetCurrentState (c.fsm->GetFailDockNode(),NULL,0);
      }
      parent->getAIState()->Communicate (c);
      vectorOfKeyboardInput[playa].und=0;
    }
}
using std::list;

void FireKeyboard::ProcessCommMessage (class CommunicationMessage&c){

  Unit * un = c.sender.GetUnit();
  if (!AUDIsPlaying (c.getCurrentState()->GetSound(c.sex))) {
    AUDStartPlaying(c.getCurrentState()->GetSound(c.sex));
  }
  if (un) {
    for (list<CommunicationMessage>::iterator i=resp.begin();i!=resp.end();i++) {
      if ((*i).sender.GetUnit()==un) {
	i = resp.erase (i);
      }
    }
    resp.push_back(c);
    AdjustRelationTo(un,c.getCurrentState()->messagedelta);
    DoSpeech (un,c.getCurrentState()->message);
    if (parent==_Universe->AccessCockpit()->GetParent()) {
      _Universe->AccessCockpit()->SetCommAnimation (c.ani);
    }
    refresh_target=true;
  }else {
    DoSpeech (NULL,c.getCurrentState()->message);
    if (parent==_Universe->AccessCockpit()->GetParent()) {
      static Animation Statuc ("static.ani");
      _Universe->AccessCockpit()->SetCommAnimation (&Statuc);
    }

  }
}
using std::list;

static CommunicationMessage * GetTargetMessageQueue (Unit * targ,std::list <CommunicationMessage> &messagequeue) {
      CommunicationMessage * mymsg=NULL;
      for (list<CommunicationMessage>::iterator i=messagequeue.begin();i!=messagequeue.end();i++) {
	if ((*i).sender.GetUnit()==targ) {
	  mymsg = &(*i);
	  break;
	}
      }
      return mymsg;
}


void FireKeyboard::Execute () {
  while (vectorOfKeyboardInput.size()<=whichplayer||vectorOfKeyboardInput.size()<=whichjoystick) {
    vectorOfKeyboardInput.push_back(FIREKEYBOARDTYPE());
  }
  ProcessCommunicationMessages(SIMULATION_ATOM,true);
  Unit * targ;
  if ((targ = parent->Target())) {
    ShouldFire (targ);
    DoDockingOps(parent,targ,whichplayer,sex);
  } else {
    ChooseTargets(false);
    refresh_target=true;
  }
  if (f().firekey==PRESS||f().jfirekey==PRESS||j().firekey==DOWN||j().jfirekey==DOWN) 
    parent->Fire(false);
  if (f().missilekey==DOWN||f().missilekey==PRESS||j().jmissilekey==PRESS||j().jmissilekey==DOWN) {
    parent->Fire(true);
    if (f().missilekey==PRESS)
      f().missilekey = DOWN;
    if (j().jmissilekey==PRESS)
      j().jmissilekey=DOWN;
  }
  else if (f().firekey==RELEASE||j().jfirekey==RELEASE) {
    f().firekey=UP;
    j().jfirekey=UP;
    parent->UnFire();
  }
  if (f().cloakkey==PRESS) {
    static bool toggle=true;
    f().cloakkey=DOWN;
    parent->Cloak(toggle);
    toggle=!toggle;
  }
  if (f().ECMkey==PRESS) {
    parent->GetImageInformation().ecm=-parent->GetImageInformation().ecm;
    
  }
  if (f().targetkey==PRESS||j().jtargetkey==PRESS) {
    f().targetkey=DOWN;
    j().jtargetkey=DOWN;
    ChooseTargets(false);
    refresh_target=true;
  }
  if (f().rtargetkey==PRESS) {
    f().rtargetkey=DOWN;
    ChooseRTargets(false);
    refresh_target=true;
  }
  if(f().picktargetkey==PRESS){
    f().picktargetkey=DOWN;
    PickTargets(false);
    refresh_target=true;
  }
  if (f().neartargetkey==PRESS) {
    ChooseNearTargets (false);
    f().neartargetkey=DOWN;
    refresh_target=true;
  }
  if (f().threattargetkey==PRESS) {
    ChooseThreatTargets (false);
    f().threattargetkey=DOWN;
    refresh_target=true;
  }

  if (f().turretaikey == PRESS) {
      parent->DisableTurretAI();
      f().turretaikey = DOWN;
  }
  if (f().turretaikey==RELEASE) {
    f().turretaikey = UP;
    parent->SetTurretAI();
    parent->TargetTurret(parent->Target());
  }
  if (f().turrettargetkey==PRESS) {
    f().turrettargetkey=DOWN;
    //    ChooseTargets(true);
    parent->TargetTurret(parent->Target());
    refresh_target=true;
  }
  if(f().pickturrettargetkey==PRESS){
    f().pickturrettargetkey=DOWN;
    parent->TargetTurret(parent->Target());
    refresh_target=true;
  }
  if (f().nearturrettargetkey==PRESS) {
    parent->TargetTurret(parent->Target());
    f().nearturrettargetkey=DOWN;
    refresh_target=true;
  }
  if (f().threatturrettargetkey==PRESS) {
    parent->TargetTurret(parent->Target());
    f().threatturrettargetkey=DOWN;
    refresh_target=true;
  }



  if (f().weapk==PRESS) {
    f().weapk=DOWN;
    parent->ToggleWeapon (false);
  }
  if (f().misk==PRESS) {
    f().misk=DOWN;
    parent->ToggleWeapon(true);
  }
  for (unsigned int i=0;i<NUMCOMMKEYS;i++) {
    if (f().commKeys[i]==PRESS) {
      f().commKeys[i]=RELEASE;
      Unit * targ=parent->Target();
      if (targ) {
	CommunicationMessage * mymsg = GetTargetMessageQueue(targ,resp);       

	if (mymsg==NULL) {
	  CommunicationMessage c(parent,targ,i,NULL,sex);
	  DoSpeech (parent,c.getCurrentState()->message);
	  if (!AUDIsPlaying (c.getCurrentState()->GetSound(c.sex))) {
	    AUDStartPlaying(c.getCurrentState()->GetSound(c.sex));
	  }
	  targ->getAIState ()->Communicate (c);
	}else {
	  FSM::Node * n = mymsg->getCurrentState();
	  if (i<n->edges.size()) {
	    CommunicationMessage c(parent,targ,*mymsg,i,NULL,sex);
	    DoSpeech (parent,c.getCurrentState()->message);
	    if (!AUDIsPlaying (c.getCurrentState()->GetSound(c.sex))) {
	      AUDStartPlaying(c.getCurrentState()->GetSound(c.sex));
	    }
	    targ->getAIState ()->Communicate (c);
	  }
	}
      }
    }
  }
  if (refresh_target) {
    Unit * targ;
    if ((targ =parent->Target())) {
      CommunicationMessage *mymsg = GetTargetMessageQueue(targ,resp);
      if (mymsg==NULL) {
	FSM *fsm =_Universe->GetConversation (parent->faction,targ->faction);
	_Universe->AccessCockpit()->communication_choices=fsm->GetEdgesString(fsm->getDefaultState(parent->getRelation(targ)));
      }else {
       _Universe->AccessCockpit()->communication_choices=mymsg->fsm->GetEdgesString(mymsg->curstate);
      }
    } else {
      _Universe->AccessCockpit()->communication_choices="\nNo Communication\nLink\nEstablished";
    }
  }
  if (f().ejectcargo==PRESS) {
    int offset = _Universe->AccessCockpit()->getScrollOffset (VDU::MANIFEST);
    if (offset<3) {
      offset=0;
    }else {
      offset-=3;
    }
    parent->EjectCargo(offset);
    f().ejectcargo=DOWN;
  }
  if (f().eject==PRESS) {
    f().eject=DOWN;
    Cockpit * cp;
    if ((cp=_Universe->isPlayerStarship (parent))) {
      cp->Eject();
    }
  }
}
