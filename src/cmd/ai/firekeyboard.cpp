
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
FireKeyboard::FireKeyboard (unsigned int whichplayer, unsigned int whichjoystick): Order (WEAPON){
  this->whichjoystick = whichjoystick;
  this->whichplayer=whichplayer;
  gunspeed = gunrange = .0001;
  refresh_target=true;
  sex = XMLSupport::parse_int( vs_config->getVariable ("player","sex","0"));
}
const unsigned int NUMCOMMKEYS=10;
struct FIREKEYBOARDTYPE {
  FIREKEYBOARDTYPE() {
    commKeys[0]=commKeys[1]=commKeys[2]=commKeys[3]=commKeys[4]=commKeys[5]=commKeys[6]=commKeys[7]=commKeys[8]=commKeys[9]=UP;
    eject=ejectcargo=firekey=missilekey=jfirekey=jtargetkey=jmissilekey=weapk=misk=cloakkey=neartargetkey=threattargetkey=picktargetkey=targetkey=nearturrettargetkey =threatturrettargetkey= pickturrettargetkey=turrettargetkey=UP;
    doc=und=req=0;
  }
 KBSTATE firekey;
 bool doc;
 bool und;
 bool req;
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
    bool tmp = parent->InRange (un,t);
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
    Vector t;
    if(other!=parent && parent->InRange(other,t)){
      Vector p,q,r;
      Vector vectothem=Vector(other->Position() - parent->Position()).Normalize();
      parent->GetOrientation(p,q,r);
      float angle=acos( vectothem.Dot(r) );
      
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
    if (!parent->InRange(un,t)) {
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
      if (!parent->InRange(un,t)) {
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

static void DoDockingOps (Unit * parent, Unit * targ,unsigned char playa, unsigned char sex) {
    if (vectorOfKeyboardInput[playa].req) {
      //      fprintf (stderr,"request %d", targ->RequestClearance (parent));
      CommunicationMessage c(parent,targ,NULL,sex);
      c.SetCurrentState(c.fsm->GetRequestLandNode(),NULL,sex);
      targ->getAIState()->Communicate (c);
      vectorOfKeyboardInput[playa].req=false;
    }
    if (vectorOfKeyboardInput[playa].doc) {
      if (!parent->Dock(targ))
	parent->UnDock(targ);
      vectorOfKeyboardInput[playa].doc=false;
    }
    if (vectorOfKeyboardInput[playa].und) {
      fprintf (stderr,"udock %d", parent->UnDock(targ));
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
    mission->msgcenter->add ("game","all",un->name+string(": ")+c.getCurrentState()->message);
    if (parent==_Universe->AccessCockpit()->GetParent()) {
      _Universe->AccessCockpit()->SetCommAnimation (c.ani);
    }
    refresh_target=true;
  }else {
    mission->msgcenter->add ("game","all",string("[static]: ")+c.getCurrentState()->message);
    if (parent==_Universe->AccessCockpit()->GetParent()) {
      Animation Statuc ("static.ani");
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
  if (f().eject==PRESS) {
    parent->EjectCargo((unsigned int )-1);
    f().eject=DOWN;
  }
  if (f().ejectcargo==PRESS) {
    int offset = _Universe->AccessCockpit()->getScrollOffset (VDU::MANIFEST);
    if (offset<2) {
      offset=0;
    }else {
      offset-=2;
    }
    parent->EjectCargo(offset);
    f().eject=DOWN;
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



  if (f().turrettargetkey==PRESS) {
    f().turrettargetkey=DOWN;
    ChooseTargets(true);
    refresh_target=true;
  }
  if(f().pickturrettargetkey==PRESS){
    f().pickturrettargetkey=DOWN;
    PickTargets(true);
    refresh_target=true;
  }
  if (f().nearturrettargetkey==PRESS) {
    ChooseNearTargets (true);
    f().nearturrettargetkey=DOWN;
    refresh_target=true;
  }
  if (f().threatturrettargetkey==PRESS) {
    ChooseThreatTargets (true);
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
	  mission->msgcenter->add ("game","all",string("[Outgoing]")+string(": ")+c.getCurrentState()->message);
	  if (!AUDIsPlaying (c.getCurrentState()->GetSound(c.sex))) {
	    AUDStartPlaying(c.getCurrentState()->GetSound(c.sex));
	  }
	  targ->getAIState ()->Communicate (c);
	}else {
	  FSM::Node * n = mymsg->getCurrentState();
	  if (i<n->edges.size()) {
	    CommunicationMessage c(parent,targ,*mymsg,i,NULL,sex);
	    mission->msgcenter->add ("game","all",string("[Outgoing]")+string(": ")+c.getCurrentState()->message);
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
	_Universe->AccessCockpit()->communication_choices=fsm->GetEdgesString(fsm->getDefaultState(_Universe->GetRelation(parent->faction,targ->faction)));
      }else {
       _Universe->AccessCockpit()->communication_choices=mymsg->fsm->GetEdgesString(mymsg->curstate);
      }
    } else {
      _Universe->AccessCockpit()->communication_choices="\nNo Communication\nLink\nEstablished";
    }
  }
}
