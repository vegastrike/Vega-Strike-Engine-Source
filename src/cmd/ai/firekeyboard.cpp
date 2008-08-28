#include <set>
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"
#include "in_joystick.h"
#include "cmd/unit_generic.h"
#include "communication.h"
#include "gfx/cockpit.h"
#include "gfx/animation.h"
#include "audiolib.h"
#include "config_xml.h"
#include "cmd/images.h"
#include "cmd/planet.h"
#include "cmd/script/flightgroup.h"
#include "cmd/script/mission.h"
#include "vs_globals.h"
#include "gfx/car_assist.h"
#include "cmd/unit_util.h"
#include <algorithm>
#include "fire.h"
#include "docking.h"
#include "cmd/pilot.h"
//for getatmospheric
#include "cmd/role_bitmask.h"
FireKeyboard::FireKeyboard (unsigned int whichplayer, unsigned int whichjoystick): Order (WEAPON,0){
  memset (savedTargets,0,sizeof(void*)*NUMSAVEDTARGETS);
  this->autotrackingtoggle=1;
  this->cloaktoggle=true;
  this->whichjoystick = whichjoystick;
  this->whichplayer=whichplayer;
  gunspeed = gunrange = .0001;
  refresh_target=true;
}
const unsigned int NUMCOMMKEYS=10;

struct FIREKEYBOARDTYPE {
  FIREKEYBOARDTYPE() {
    toggleautotracking=togglewarpdrive=toggleglow=toggleanimation=lockkey=ECMkey=commKeys[0]=commKeys[1]=commKeys[2]=commKeys[3]=commKeys[4]=commKeys[5]=commKeys[6]=commKeys[7]=commKeys[8]=commKeys[9]=turretaikey = turretoffkey=turretfaw=saveTargetKeys[0]=saveTargetKeys[1]=saveTargetKeys[2]=saveTargetKeys[3]=saveTargetKeys[4]=saveTargetKeys[5]=saveTargetKeys[6]=saveTargetKeys[7]=saveTargetKeys[8]=saveTargetKeys[9]=turretaikey = restoreTargetKeys[0]=restoreTargetKeys[1]=restoreTargetKeys[2]=restoreTargetKeys[3]=restoreTargetKeys[4]=restoreTargetKeys[5]=restoreTargetKeys[6]=restoreTargetKeys[7]=restoreTargetKeys[8]=restoreTargetKeys[9]=turretaikey = UP;

    missiontargetkey=rmissiontargetkey=ejectdock=eject=ejectcargo=ejectnonmissioncargo=firekey=missilekey=jfirekey=jtargetkey=jmissilekey=weapk=misk=rweapk=rmisk=cloakkey=
      neartargetkey=targetskey=targetukey=threattargetkey=picktargetkey=subtargetkey=targetkey=
      rneartargetkey=rtargetskey=rtargetukey=rthreattargetkey=rpicktargetkey=rtargetkey=
      nearturrettargetkey =threatturrettargetkey= pickturrettargetkey=turrettargetkey=enslave=
	  freeslave=incomingmissiletargetkey=rincomingmissiletargetkey=nearesthostilekey=nearestdangeroushostilekey=missiletargetkey=rmissiletargetkey=
	  nearestfriendlykey=nearestbasekey=nearestplanetkey=nearestjumpkey=UP;

      shieldpowerstate=1;
#ifdef CAR_SIM
    blinkleftkey=blinkrightkey=headlightkey=sirenkey=UP;
#endif
    doc=und=req=0;
  }
 KBSTATE firekey;
 float shieldpowerstate;
 bool doc;
 bool und;
 bool req;
#ifdef CAR_SIM
  KBSTATE blinkleftkey;
  KBSTATE blinkrightkey;
  KBSTATE headlightkey;
  KBSTATE sirenkey;
#endif
 KBSTATE missiletargetkey;
 KBSTATE incomingmissiletargetkey;
 KBSTATE rmissiletargetkey;
 KBSTATE rincomingmissiletargetkey;
 KBSTATE rneartargetkey;
 KBSTATE rthreattargetkey;
 KBSTATE rpicktargetkey;
 KBSTATE rtargetkey;
 KBSTATE rtargetskey;
 KBSTATE rtargetukey;
 KBSTATE missilekey;
 KBSTATE missiontargetkey;
 KBSTATE rmissiontargetkey;
 KBSTATE jfirekey;
 KBSTATE jtargetkey;
 KBSTATE jmissilekey;
 KBSTATE weapk;
 KBSTATE misk;
 KBSTATE rweapk;
 KBSTATE rmisk;
 KBSTATE eject;
 KBSTATE ejectdock;
 KBSTATE lockkey;
 KBSTATE ejectcargo;
 KBSTATE ejectnonmissioncargo;
 KBSTATE cloakkey;
 KBSTATE ECMkey;
 KBSTATE neartargetkey;
 KBSTATE threattargetkey;
 KBSTATE picktargetkey;
 KBSTATE subtargetkey;
 KBSTATE targetkey;
 KBSTATE targetskey;
 KBSTATE targetukey;
 KBSTATE turretaikey;
 KBSTATE turretoffkey;
 KBSTATE turretfaw;
 KBSTATE toggleglow;
 KBSTATE toggleautotracking;
 KBSTATE togglewarpdrive;
 KBSTATE toggleanimation;	
 KBSTATE commKeys[NUMCOMMKEYS];
 KBSTATE saveTargetKeys[NUMSAVEDTARGETS];
 KBSTATE restoreTargetKeys[NUMSAVEDTARGETS];
 KBSTATE nearturrettargetkey;
 KBSTATE threatturrettargetkey;
 KBSTATE pickturrettargetkey;
 KBSTATE turrettargetkey;
 KBSTATE enslave;
 KBSTATE freeslave;
 //Added for nearest targets keys --ch
 KBSTATE nearesthostilekey; 
 KBSTATE nearestdangeroushostilekey;
 KBSTATE nearestfriendlykey;
 KBSTATE nearestbasekey;
 KBSTATE nearestplanetkey;
 KBSTATE nearestjumpkey;

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
void FireKeyboard::SetShieldsOneThird(const KBData&,KBSTATE k) {
  if (k==PRESS) {
    float pow=1./3;
    static soundContainer sc;
    if (sc.sound<0) {
      
      static string str=vs_config->getVariable("cockpitaudio","shield","vdu_d");
      sc.loadsound(str);
    }
    sc.playsound();

    if (g().shieldpowerstate!=pow)
      g().shieldpowerstate=pow;
    else
      g().shieldpowerstate=1;
  }
}


void FireKeyboard::SetShieldsOff(const KBData&,KBSTATE k) {
  if (k==PRESS) {
    float pow=0;
    static soundContainer sc;
    if (sc.sound<0) {
      
      static string str=vs_config->getVariable("cockpitaudio","shield","vdu_d");
      sc.loadsound(str);
    }
    sc.playsound();

    if (g().shieldpowerstate!=pow)
      g().shieldpowerstate=pow;
    else
      g().shieldpowerstate=1;
  }
}

void FireKeyboard::SetShieldsTwoThird(const KBData&,KBSTATE k) {
  if (k==PRESS) {
    float pow=2./3;
    static soundContainer sc;
    if (sc.sound<0) {
      
      static string str=vs_config->getVariable("cockpitaudio","shield","vdu_d");
      sc.loadsound(str);
    }
    sc.playsound();

    if (g().shieldpowerstate!=pow)
      g().shieldpowerstate=pow;
    else
      g().shieldpowerstate=1;
  }
}
void FireKeyboard::ToggleGlow (const KBData&,KBSTATE k) {
	if (k==PRESS) {
		g().toggleglow=PRESS;
	}
}
void FireKeyboard::ToggleAutotracking (const KBData&,KBSTATE k) {
	if (k==PRESS) {
		g().toggleautotracking=PRESS;
	}
}

void FireKeyboard::ToggleWarpDrive (const KBData&,KBSTATE k) {
	if (k==PRESS) {
		g().togglewarpdrive=PRESS;
	}
}

void FireKeyboard::ToggleAnimation (const KBData&,KBSTATE k) {
	if (k==PRESS) {
		g().toggleanimation=PRESS;
	}
}
void FireKeyboard::PressComm1Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[0]=PRESS;
  }
}
void FireKeyboard::PressComm2Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[1]=PRESS;
  }
}
void FireKeyboard::PressComm3Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[2]=PRESS;
  }
}
void FireKeyboard::PressComm4Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[3]=PRESS;
  }
}
void FireKeyboard::PressComm5Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[4]=PRESS;
  }
}
void FireKeyboard::PressComm6Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[5]=PRESS;
  }
}
void FireKeyboard::PressComm7Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[6]=PRESS;
  }
}
void FireKeyboard::PressComm8Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[7]=PRESS;
  }
}
void FireKeyboard::PressComm9Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[8]=PRESS;
  }
}
void FireKeyboard::PressComm10Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().commKeys[9]=PRESS;
  }
}
void FireKeyboard::SaveTarget1Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[0]=PRESS;
  }
}
void FireKeyboard::SaveTarget2Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[1]=PRESS;
  }
}
void FireKeyboard::SaveTarget3Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[2]=PRESS;
  }
}
void FireKeyboard::SaveTarget4Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[3]=PRESS;
  }
}
void FireKeyboard::SaveTarget5Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[4]=PRESS;
  }
}
void FireKeyboard::SaveTarget6Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[5]=PRESS;
  }
}
void FireKeyboard::SaveTarget7Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[6]=PRESS;
  }
}
void FireKeyboard::SaveTarget8Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[7]=PRESS;
  }
}
void FireKeyboard::SaveTarget9Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[8]=PRESS;
  }
}
void FireKeyboard::SaveTarget10Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().saveTargetKeys[9]=PRESS;
  }
}
void FireKeyboard::RestoreTarget1Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[0]=PRESS;
  }
}
void FireKeyboard::RestoreTarget2Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[1]=PRESS;
  }
}
void FireKeyboard::RestoreTarget3Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[2]=PRESS;
  }
}
void FireKeyboard::RestoreTarget4Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[3]=PRESS;
  }
}
void FireKeyboard::RestoreTarget5Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[4]=PRESS;
  }
}
void FireKeyboard::RestoreTarget6Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[5]=PRESS;
  }
}
void FireKeyboard::RestoreTarget7Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[6]=PRESS;
  }
}
void FireKeyboard::RestoreTarget8Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[7]=PRESS;
  }
}
void FireKeyboard::RestoreTarget9Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[8]=PRESS;
  }
}
void FireKeyboard::RestoreTarget10Key (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().restoreTargetKeys[9]=PRESS;
  }
}


extern void LeadMe (Unit * un, string directive, string speech, bool changetarget);

static void LeadMe (string directive, string speech, bool changetarget) {

  Unit * un= _Universe->AccessCockpit()->GetParent();
  if (un) LeadMe (un,directive,speech,changetarget);
}

void FireKeyboard::RequestClearenceKey(const KBData&,KBSTATE k) {

    if (k==PRESS) {
      g().req=true;      
    }
    if (k==RELEASE) {
      g().req=false;      
    }
}
void FireKeyboard::DockKey(const KBData&,KBSTATE k) {
    Unit *u = _Universe->AccessCockpit()->GetParent();
    if (k==PRESS && u && (u->isSubUnit() == false)) {
      g().doc=true;      
    }
    if (k==RELEASE && u && (u->isSubUnit() == false)) {
      g().doc=false;      
    }
}
void FireKeyboard::UnDockKey(const KBData&,KBSTATE k) {
    Unit *u = _Universe->AccessCockpit()->GetParent();
    if (k==PRESS && u && (u->isSubUnit() == false)) {
      g().und=true;      
    }
    if (k==RELEASE && u && (u->isSubUnit() == false)) {
      g().und=false;      
    }
}
void FireKeyboard::EjectKey (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("","I am ejecting! Record the current location of my ship.", false); // used to clear group target

    LeadMe ("e","Then get over here and pick me up!", false);

    g().eject= k;
  }
}
void FireKeyboard::EjectDockKey (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().ejectdock= k;
    g().doc=true;
  }
    if (k==RELEASE) 
      g().doc=false;      
}


void FireKeyboard::TurretAIOn (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    g().turretaikey=k;
  }
}
void FireKeyboard::TurretAIOff (const KBData&,KBSTATE k) {
  if (k==PRESS) {    
    g().turretoffkey=k;
  }
}
void FireKeyboard::TurretFireAtWill (const KBData&,KBSTATE k) {
  if (k==PRESS) {    
    g().turretfaw=k;
  }
}


void FireKeyboard::MissionTargetKey (const KBData&,KBSTATE k) {
	if (k==PRESS) {
		g().missiontargetkey=PRESS;
	}
}
void FireKeyboard::ReverseMissionTargetKey (const KBData&,KBSTATE k) {
	if (k==PRESS) {
		g().rmissiontargetkey=PRESS;
	}
}
void FireKeyboard::EjectCargoKey (const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().ejectcargo = k;      
    }
  
}
void FireKeyboard::EnslaveKey (const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().enslave = k;      
    }
  
}
void FireKeyboard::FreeSlaveKey (const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().freeslave = k;      
    }
  
}
void FireKeyboard::EjectNonMissionCargoKey (const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().ejectnonmissioncargo = k;      
    }
  
}
void FireKeyboard::CloakKey(const KBData&,KBSTATE k) {

    if (k==PRESS) {
      g().cloakkey = k;      
    }
}
void FireKeyboard::LockKey(const KBData&,KBSTATE k) {

    if (k==PRESS) {
      g().lockkey = k;      
    }
}
void FireKeyboard::ECMKey(const KBData&,KBSTATE k) {

    if (k==PRESS) {
      g().ECMkey = k;      
    }
    if (k==RELEASE) {
      g().ECMkey = k;
    }
}
void FireKeyboard::FireKey(const KBData&, KBSTATE k) {
  if(g().firekey==DOWN && k==UP){
    return;
  }
  if (k==UP&&g().firekey==RELEASE) {

  } else {

    g().firekey = k;
    //    printf("firekey %d %d\n",k,key);
  }
}
void ExamineWhenTargetKey();
void FireKeyboard::TargetKey(const KBData&,KBSTATE k) {
  if (g().targetkey!=PRESS)
    g().targetkey = k;
  if (k==RESET) {
    g().targetkey=PRESS;
  }
  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::PickTargetKey(const KBData&,KBSTATE k) {
  if (g().picktargetkey!=PRESS)
    g().picktargetkey = k;
  if (k==RESET) {
    g().picktargetkey=PRESS;
  }
  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::MissileTargetKey(const KBData&,KBSTATE k) {
  if (g().missiletargetkey!=PRESS)
    g().missiletargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
 
}

void FireKeyboard::IncomingMissileTargetKey(const KBData&,KBSTATE k) {
  if (g().incomingmissiletargetkey!=PRESS)
    g().incomingmissiletargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
 
}
void FireKeyboard::ReverseMissileTargetKey(const KBData&,KBSTATE k) {
  if (g().rmissiletargetkey!=PRESS)
    g().rmissiletargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
 
}

void FireKeyboard::ReverseIncomingMissileTargetKey(const KBData&,KBSTATE k) {
  if (g().rincomingmissiletargetkey!=PRESS)
    g().rincomingmissiletargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
 
}
void FireKeyboard::NearestTargetKey(const KBData&,KBSTATE k) {
  if (g().neartargetkey!=PRESS)
    g().neartargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();

}
void FireKeyboard::SubUnitTargetKey(const KBData&,KBSTATE k) {
  if (g().subtargetkey!=PRESS)
    g().subtargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();

}
void FireKeyboard::ThreatTargetKey(const KBData&,KBSTATE k) {
  if (g().threattargetkey!=PRESS)
    g().threattargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}

void FireKeyboard::UnitTargetKey(const KBData&,KBSTATE k) {
  if (g().targetukey!=PRESS)
    g().targetukey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}

void FireKeyboard::SigTargetKey(const KBData&,KBSTATE k) {
  if (g().targetskey!=PRESS)
    g().targetskey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}

void FireKeyboard::ReverseTargetKey(const KBData&,KBSTATE k) {
  if (g().rtargetkey!=PRESS)
    g().rtargetkey = k;
  if (k==RESET) {
    g().rtargetkey=PRESS;
  }
  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::ReversePickTargetKey(const KBData&,KBSTATE k) {
  if (g().rpicktargetkey!=PRESS)
    g().rpicktargetkey = k;
  if (k==RESET) {
    g().rpicktargetkey=PRESS;
  }
  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::ReverseNearestTargetKey(const KBData&,KBSTATE k) {
  if (g().rneartargetkey!=PRESS)
    g().rneartargetkey = k;

  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::ReverseThreatTargetKey(const KBData&,KBSTATE k) {
  if (g().rthreattargetkey!=PRESS)
    g().rthreattargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}

void FireKeyboard::ReverseUnitTargetKey(const KBData&,KBSTATE k) {
  if (g().rtargetukey!=PRESS)
    g().rtargetukey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}

void FireKeyboard::ReverseSigTargetKey(const KBData&,KBSTATE k) {
  if (g().rtargetskey!=PRESS)
    g().rtargetskey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::NearestHostileTargetKey(const KBData&,KBSTATE k) {	
	if (g().nearesthostilekey!=PRESS)
		g().nearesthostilekey = k;
	if (k==PRESS)
		ExamineWhenTargetKey();
}
void FireKeyboard::NearestDangerousHostileKey(const KBData&,KBSTATE k) {	
	if (g().nearestdangeroushostilekey!=PRESS)
		g().nearestdangeroushostilekey = k;
	if (k==PRESS)
		ExamineWhenTargetKey();
}
void FireKeyboard::NearestFriendlyKey(const KBData&,KBSTATE k) {	
	if (g().nearestfriendlykey!=PRESS)
		g().nearestfriendlykey = k;
	if (k==PRESS)
		ExamineWhenTargetKey();
}
void FireKeyboard::NearestBaseKey(const KBData&,KBSTATE k) {	
	if (g().nearestbasekey!=PRESS)
		g().nearestbasekey = k;
	if (k==PRESS)
		ExamineWhenTargetKey();
}
void FireKeyboard::NearestPlanetKey(const KBData&,KBSTATE k) {	
	if (g().nearestplanetkey!=PRESS)
		g().nearestplanetkey = k;
	if (k==PRESS)
		ExamineWhenTargetKey();
}
void FireKeyboard::NearestJumpKey(const KBData&,KBSTATE k) {	
	if (g().nearestjumpkey!=PRESS)
		g().nearestjumpkey = k;
	if (k==PRESS)
		ExamineWhenTargetKey();
}
#ifdef CAR_SIM
void FireKeyboard::BlinkLeftKey(const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().blinkleftkey = k;      
    }
    if (k==RELEASE) {
      g().blinkleftkey = k;
    }

}
void FireKeyboard::BlinkRightKey(const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().blinkrightkey = k;      
    }
    if (k==RELEASE) {
      g().blinkrightkey = k;
    }

}
void FireKeyboard::SirenKey(const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().sirenkey = k;      
    }
    if (k==RELEASE) {
      g().sirenkey = k;
    }
}
void FireKeyboard::HeadlightKey(const KBData&,KBSTATE k) {
    if (k==PRESS) {
      g().headlightkey = k;      
    }
    if (k==RELEASE) {
      g().headlightkey = k;
    }
}
#endif
extern unsigned int DoSpeech (Unit * un, Unit *player_un, const FSM::Node &convNode);
extern Unit * GetThreat (Unit * par, Unit * leader);
void HelpOut (bool crit, std::string conv) {
  Unit * un = _Universe->AccessCockpit()->GetParent();
  if (un) {
    Unit * par=NULL;
    DoSpeech(  un,NULL, FSM::Node::MakeNode(conv,.1));
    for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator();
	 (par = (*ui));
	 ++ui) {
      if ((crit&&UnitUtil::getFactionRelation(par,un)>0)||par->faction==un->faction) {
	Unit * threat = GetThreat (par,un);
	CommunicationMessage c(par,un,NULL,0);
	if (threat) {
	  par->Target(threat);
	  c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
	} else {
	  c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	}
	Order * o = un->getAIState();
	if (o)
		o->Communicate (c);
      }
    }
  }
}
void FireKeyboard::JoinFg (const KBData&,KBSTATE k) {
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

void FireKeyboard::AttackTarget (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("k","Attack my target!", true);

  }
}

void FireKeyboard::HelpMeOut (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("h","Help me out!", false);

  }
}
void FireKeyboard::HelpMeOutFaction (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    HelpOut (false,"Help me out! I need critical assistance!");
  }
}
void FireKeyboard::HelpMeOutCrit (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    HelpOut (true,"Help me out! Systems going critical!");
  }
}

void FireKeyboard::DockWithMe (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("l","Get in front of me and prepare to be tractored in.", false);

  }
}
void FireKeyboard::DefendTarget (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("p","Defend my target!", true);

  }
}
void FireKeyboard::FormUp (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("f","Form on my wing.", false);

  }
}
void FireKeyboard::BreakFormation(const KBData&,KBSTATE k) {
  if (k==PRESS) {
    LeadMe ("","Break formation!", false); // used to clear group target

    LeadMe ("b","Pick a target and open fire!", false);

  }
}


void FireKeyboard::TargetTurretKey(const KBData&,KBSTATE k) {
  if (g().turrettargetkey!=PRESS)
    g().turrettargetkey = k;
  if (k==RESET) {
    g().turrettargetkey=PRESS;
  }
  if (k==PRESS)
    ExamineWhenTargetKey();
}
void FireKeyboard::PickTargetTurretKey(const KBData&,KBSTATE k) {
  if (g().pickturrettargetkey!=PRESS)
    g().pickturrettargetkey = k;
  if (k==RESET) {
    g().pickturrettargetkey=PRESS;
  }
  if (k==PRESS)
    ExamineWhenTargetKey();
}

void FireKeyboard::NearestTargetTurretKey(const KBData&,KBSTATE k) {
  if (g().nearturrettargetkey!=PRESS)
    g().nearturrettargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();

}
void FireKeyboard::ThreatTargetTurretKey(const KBData&,KBSTATE k) {
  if (g().threatturrettargetkey!=PRESS)
    g().threatturrettargetkey = k;
  if (k==PRESS)
    ExamineWhenTargetKey();
}



void FireKeyboard::ReverseWeapSelKey(const KBData&,KBSTATE k) {
  if (g().rweapk!=PRESS)
    g().rweapk = k;
}
void FireKeyboard::ReverseMisSelKey(const KBData&,KBSTATE k) {
  if (g().rmisk!=PRESS)
    g().rmisk = k;
} 

void FireKeyboard::WeapSelKey(const KBData&,KBSTATE k) {
  if (g().weapk!=PRESS)
    g().weapk = k;
}
void FireKeyboard::MisSelKey(const KBData&,KBSTATE k) {
  if (g().misk!=PRESS)
    g().misk = k;
} 

void FireKeyboard::MissileKey(const KBData&,KBSTATE k) {
  if (k==PRESS)
   g().missilekey = k;
}
#if 0
void FireKeyboard::ChooseNearTargets(bool turret,bool reverse) {
  UnitCollection::UnitIterator iter = _Universe->activeStarSystem()->getUnitList().createIterator();
  Unit * un;
  float range=FLT_MAX;
  while ((un = iter.current())) {
    Vector t;
    bool tmp = parent->InRange (un);
    t = parent->LocalCoordinates (un);
    if (tmp&&t.Dot(t)<range&&t.k>0&&parent->isEnemy(un)) {
      range = t.Dot(t);
      if (turret)
	parent->TargetTurret(un);
      parent->Target (un);
    }
    iter.advance();
  }
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"i4%x",iter);
  fflush (stderr);
#endif

#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"i4\n");
  fflush (stderr);
#endif

}
void FireKeyboard::ChooseThreatTargets(bool turret,bool reverse) {
	static int index=0;
	UnitCollection * drawlist = &_Universe->activeStarSystem()->getUnitList();
	un_iter iter = drawlist->createIterator();
	Unit *target;
	std::vector <Unit *> vec; //not saving across frames
	Unit *threat=parent->Threat();
	if (target) {
		vec.push_back(threat);
	}
	while ((target = iter.current())!=NULL) {
		if (target->Target()==parent&&target!=threat) {
			vec.push_back(target);
		}
		iter.advance();
	}
	if (!vec.empty()) {
		if (reverse) {
			index--;
			if (index<0) {
				index=vec.size()-index;
				if (index<0) {
					index=0;
				}
			}
		} else {
			index++;
		}
		target=vec[index%vec.size()];
		parent->Target(target);
		if (turret)
			parent->TargetTurret(target);
	}
}

void FireKeyboard::PickTargets(bool Turrets,bool reverse){
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
void FireKeyboard::ChooseRTargets (bool turret,bool significant) {
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
      if (un!=parent&&parent->InRange(un,true,significant)) {
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

void FireKeyboard::ChooseTargets (bool turret,bool significant,bool reverse) {
  parent->LockTarget(false);
  if (reverse) {
    ChooseRTargets(turret,significant);
    return;
  }
  UnitCollection::UnitIterator iter = _Universe->activeStarSystem()->getUnitList().createIterator();
  Unit * un ;
  bool found=false;
  bool find=false;
  while ((un = iter.current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    //Vector t;

    if (un==parent->Target()) {
      iter.advance();
      found=true;
      continue;
    }
    if (!parent->InRange(un,true,significant)) {
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
  VSFileSystem::vs_fprintf (stderr,"i5%x",iter);
  fflush (stderr);
#endif

  
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"i5\n");
  fflush (stderr);
#endif
  if (!find) {
    iter = _Universe->activeStarSystem()->getUnitList().createIterator();
    while ((un = iter.current())) {
      //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
      //Vector t;
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
#endif
static bool isNotTurretOwner(Unit * parent, Unit * un) {
  return parent->isSubUnit()==false||un!=parent->owner;
}
bool TargMission(Unit *me, Unit*target) {
	for (unsigned int i=0;i<active_missions.size();++i) {
		if (active_missions[i]->runtime.pymissions) {
			vector<UnitContainer*> *relevant=&active_missions[i]->runtime.pymissions->relevant_units;
			vector<UnitContainer*>::iterator ir=relevant->begin();
			vector<UnitContainer*>::iterator ie=relevant->end();
			for (;ir!=ie;++ir) {
				if (**ir==target) return true;
			}
		}
	}
	return false;
}
bool TargAll (Unit *me,Unit *target) {
  static bool can_target_sun=XMLSupport::parse_bool(vs_config->getVariable("graphics","can_target_sun","false"));
	return (me->InRange(target,true,false)||me->InRange(target,true,true))&&(can_target_sun||!UnitUtil::isSun(target))&&isNotTurretOwner(me,target);
}
bool TargSig (Unit *me,Unit *target) {
  static bool can_target_asteroid=XMLSupport::parse_bool(vs_config->getVariable("graphics","can_target_asteroid","true"));
  
  bool ret=me->InRange(target,false,true)&&(UnitUtil::isSignificant(target)||TargMission(me,target))&&isNotTurretOwner(me,target);

  if (can_target_asteroid==false) {
    if (target->isUnit()==ASTEROIDPTR||target->name.get().find("Asteroid")==0){
      ret=false;
    }
  }
  return ret;
}
extern Unit*getTopLevelOwner();

bool TargUn (Unit *me,Unit *target) {
  static bool can_target_cargo=XMLSupport::parse_bool(vs_config->getVariable("graphics","can_target_cargo","false"));
  int up=FactionUtil::GetUpgradeFaction();
  return me->InRange(target,true,false)&&(target->isUnit()==UNITPTR||target->isUnit()==ENHANCEMENTPTR)&&getTopLevelOwner()!=target->owner&&(can_target_cargo||target->faction!=up)&&isNotTurretOwner(me,target);
}


bool TargMissile(Unit *me,Unit *target) {
  return me->InRange(target,true,false)&&(target->isUnit()==MISSILEPTR)&&isNotTurretOwner(me,target);
}

bool TargIncomingMissile(Unit *me,Unit *target) {
  Unit *tt=target->Target();
  return TargMissile(me,target)&&(tt==me||(me->isSubUnit()&&tt==_Universe->AccessCockpit()->GetSaveParent()));
}


 
bool TargFront (Unit *me,Unit *target) {
	/*
	float dist;
	if (me->cosAngleTo(target,dist)>.6) {
		return true;
	}
	return false;
	*/
	if (!TargAll(me,target))
		return false;
	QVector delta( target->Position()-me->Position());
	double mm = delta.Magnitude();
	double tempmm =mm-target->rSize();
	if (tempmm>0.0001) {
		if ((me->ToLocalCoordinates (Vector(delta.i,delta.j,delta.k)).k/tempmm)>.995) {
			return true;
		}
	}
	return false;
}
bool TargThreat (Unit *me,Unit *target) {
	if (!TargAll(me,target))
		return false;
	if (target->isUnit()==MISSILEPTR) {
		return false;
	}
	if (target->Target()==me) {
		return true;
	}
	if (me->Threat()==target) {
		return true;
	}
	return false;
}
bool TargNear (Unit *me,Unit *target) {
  static bool can_target_sun=XMLSupport::parse_bool(vs_config->getVariable("graphics","can_target_sun","false"));
	return (me->getRelation(target)<0||TargThreat(me,target)||target->getRelation(me)<0)&&TargAll(me,target)&&target->isUnit()!=MISSILEPTR&&(can_target_sun||!UnitUtil::isSun(target))&&isNotTurretOwner(me,target);
}

//Target the nearest unit of a specified type
//Possible types are:
//0 = hostile
//1 = hostile targetting me
//2 = friendly
//3 = base
//4 = planet
//5 = jump point
bool getNearestTargetUnit (Unit *me, int iType) {
  QVector pos (me->Position());
  Unit * un=NULL;
  Unit * targ=NULL;
  double minrange=FLT_MAX;
  for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator(); (un=(*i)); ++i) 
  {
	  if (un==me)
		  continue;   
	  if (un->hull<0)
		  continue;
	  if (!(me->InRange(un,true,false))||
		  !(me->InRange(un,true,true)))
		  continue;
	  
	  if ((iType == 0) &&
		  ((un->isUnit()!=UNITPTR) ||
		   !me->isEnemy(un)))
		  continue;

	  if ((iType == 1) &&
		  ((un->isUnit()!=UNITPTR) || 
		   (!me->isEnemy(un) &&
			(un->Target() != me))))
		  continue;

	  if ((iType == 2) &&
		  ((un->isUnit()!=UNITPTR) ||
		  me->isEnemy(un) ||
		  (UnitUtil::getFlightgroupName(un) == "Base")))
		  continue;

	  if ((iType == 3) &&
		  (UnitUtil::getFlightgroupName(un) != "Base"))
		  continue;

	  if ((iType == 4) &&
		  ((!un->isPlanet()) ||
		  (un->isJumppoint())))
		  continue;

	  if ((iType == 5) &&
		  (!un->isJumppoint()))
		  continue;

		  
	  
	  double temp = (un->Position() - pos).Magnitude();
	  if (targ==NULL) 
	  {
		  targ = un;
		  minrange = temp;
	  }
	  else 
	  {
		  if (temp<minrange) {
			  targ = un;
			  minrange = temp;
		  }
	  }
  }

  if (targ == NULL)
	  return false;

  me->Target(targ);

  if (Network!=NULL) {
    int player = _Universe->whichPlayerStarship( me);
    if (player>=0) 
      Network[player].targetRequest(targ);
  }
  return true;

}
bool ChooseTargets(Unit * me, bool (*typeofunit)(Unit *,Unit *), bool reverse) {
	UnitCollection * drawlist = &_Universe->activeStarSystem()->getUnitList();
	vector <Unit *> vec;
	Unit *target;
	for(un_iter iter = drawlist->createIterator();target = *iter;++iter){
		vec.push_back(target);
	}
        if (vec.size()==0)
          return false;
	if (reverse) {
		std::reverse (vec.begin(),vec.end());
	}
	std::vector <Unit *>::const_iterator veciter=std::find(vec.begin(),vec.end(),me->Target());
	if (veciter!=vec.end())
		++veciter;
	int cur=0;
	while (1) {
		while (veciter!=vec.end()) {
			if (((*veciter)!=me)&&((*veciter)->GetHull()>=0)&&typeofunit(me,(*veciter))) {
				me->Target(*veciter);
				if (Network!=NULL) {
					int player = _Universe->whichPlayerStarship( me);
					if (player>=0) {
						Network[player].targetRequest(*veciter);
					}
				}
				if ((*veciter)!=NULL) {
					if (reverse) {
						static soundContainer foosound;
						if (foosound.sound<0) {
							static string str=vs_config->getVariable("cockpitaudio","target","vdu_b");
							foosound.loadsound(str);
						}
						foosound.playsound();
					} else {
						static soundContainer foobersound;
						if (foobersound.sound<0) {
							static string str=vs_config->getVariable("cockpitaudio","target_reverse","vdu_a");
							foobersound.loadsound(str);
						}
						foobersound.playsound();
					}
				}
				return true;
			}
			++veciter;
		}
		++cur;
		if (cur>=2)
			break;
		veciter=vec.begin();
	}
        return true;
}

void ChooseSubTargets(Unit * me) {
	Unit *parent=UnitUtil::owner(me->Target());
	if (!parent) {
		return;
	}
	un_iter uniter=parent->getSubUnits();
	if (parent==me->Target()) {
		if (!(*uniter)) {
			return;
		}
		me->Target(*uniter);
		return;
	}
	Unit *tUnit;
	for(;tUnit = *uniter;++uniter){
		if(tUnit == me->Target()){
			++uniter;
                        tUnit=*uniter;
			break;
		}
	}
	if(tUnit)
		me->Target(tUnit);
	else
		me->Target(parent);
}


FireKeyboard::~FireKeyboard () {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"fkb%x",this);
  fflush (stderr);
#endif

}
bool FireKeyboard::ShouldFire(Unit * targ) {
  float dist=FLT_MAX;
  float mrange;
  if (gunspeed==.0001) {
    parent->getAverageGunSpeed (gunspeed,gunrange,mrange);
  }
  float angle = parent->cosAngleTo (targ, dist,gunspeed,gunrange);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<.8&&angle>1);
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
//#include <cmd/music.h>
void Enslave (Unit*, bool);
void abletodock(int dock) {
//	char dumb[2]={'\0'};
//	dumb[0]=(dock+'0');
//	muzak->GotoSong (string("Dockingsound #")+dumb);
  static bool play_anim=XMLSupport::parse_bool(vs_config->getVariable("graphics","docking_comm_anim","false"));
	switch (dock) {
	case 5:{
		static soundContainer reqsound;
		if (reqsound.sound==-2) {
			static string str=vs_config->getVariable("cockpitaudio","undocking_complete","undocking_complete");
			reqsound.loadsound(str);
		}
		reqsound.playsound();}
		break;
	case 4:{
		static soundContainer reqsound;
		if (reqsound.sound==-2) {
			static string str=vs_config->getVariable("cockpitaudio","undocking_failed","undocking_failed");
			reqsound.loadsound(str);
		}
		reqsound.playsound();}
		break;
	case 3:{
		static soundContainer reqsound;
                static string otherstr = vs_config->getVariable("audio","automatic_docking_zone","automatic_landing_zone.wav");
                if (otherstr!=""&&rand()<RAND_MAX/2) {
                  static int s = AUDCreateSoundWAV(otherstr,false);
                  AUDPlay(s,QVector(0,0,0),Vector(0,0,0),1);
                  if (play_anim) {
                    //Unit * un=_Universe->AccessCockpit()->GetParent();
                    //UnitUtil::commAnimation(un,"com_neutral_female_01.ani");
                  }
                }else {
                  if (reqsound.sound==-2) {
                    static string str=vs_config->getVariable("cockpitaudio","docking_complete","docking_complete");
                    reqsound.loadsound(str);
                  }
                  reqsound.playsound();}
                  if (play_anim) {
                    //Unit * un=_Universe->AccessCockpit()->GetParent();
                    //if (rand()%2) {
                    //  UnitUtil::commAnimation(un,"com_neutral_male_01.ani");
                    //}else {
                    //  UnitUtil::commAnimation(un,"com_neutral_male_02.ani");
                    //}
                  }
          }
          break;
	case 2:{
		static soundContainer reqsound;
		if (reqsound.sound==-2) {
			static string str=vs_config->getVariable("cockpitaudio","docking_failed","docking_failed");
			reqsound.loadsound(str);
		}
		reqsound.playsound();}
		break;
	case 1:{
		static soundContainer reqsound;
		if (reqsound.sound==-2) {
			static string str=vs_config->getVariable("cockpitaudio","docking_granted","request_granted");
			reqsound.loadsound(str);
		}
		reqsound.playsound();}
		break;
	case 0:{
		static soundContainer reqsound;
		if (reqsound.sound==-2) {
			static string str=vs_config->getVariable("cockpitaudio","docking_denied","request_denied");
			reqsound.loadsound(str);
		}
		reqsound.playsound();}
		break;
	}
}

static bool NoDockWithClear() {
	static bool nodockwithclear = XMLSupport::parse_bool (vs_config->getVariable ("physics","dock_with_clear_planets","true"));
	return nodockwithclear;
}
static bool SuperDock(Unit * parent, Unit* target) {  
  if (UnitUtil::isCloseEnoughToDock(parent, target)) {
    if (UnitUtil::isDockableUnit(target)) {
      for (unsigned int i=0;i<target->GetImageInformation().dockingports.size();++i) {
        if (target->GetImageInformation().dockingports[i].used==false) {
          return parent->ForceDock(target,i)!=0;
        }
      }
    }
  }
  return false;
}
static bool TryDock(Unit * parent, Unit * targ, unsigned char playa, int severity) {
  static float min_docking_relationship = XMLSupport::parse_float(vs_config->getVariable("AI","min_docking_relationship","-.002"));
  static bool can_dock_to_enemy_base = XMLSupport::parse_bool(vs_config->getVariable("AI","can_dock_to_enemy_base","true"));
  static bool nojumpinSPEC=XMLSupport::parse_bool(vs_config->getVariable("physics","noSPECJUMP","true"));
  bool SPEC_interference=targ&&parent&&nojumpinSPEC&&(targ->graphicOptions.InWarp||parent->graphicOptions.InWarp);
  unsigned char gender=0;
  vector <Animation *>* anim = NULL;

  if (SPEC_interference){
    //FIXME js_NUDGE -- need some indicator of non-interaction because one or both objects are in SPEC.
	return false;
  }

  anim=targ->pilot->getCommFaces(gender);
  
  bool isDone=false;  
  if (targ->getRelation(parent)>=min_docking_relationship||(can_dock_to_enemy_base&&UnitUtil::getFlightgroupName(targ)=="Base")) {
    bool hasDock = severity==0?parent->Dock(targ):SuperDock(parent,targ);

    CommunicationMessage c(targ,parent,anim,gender);    
    if (hasDock) {
      isDone=true;
      c.SetCurrentState (c.fsm->GetDockNode(),anim,gender);
      abletodock(3);
      //vectorOfKeyboardInput[playa].req=true;
      if (parent->getAIState()) parent->getAIState()->Communicate (c);
      parent->UpgradeInterface(targ);
    }else {
      if (UnDockNow(parent,targ)) {
        isDone=true;
        c.SetCurrentState (c.fsm->GetUnDockNode(),anim,gender);
        if (parent->getAIState()) parent->getAIState()->Communicate (c);
        abletodock(5);
      }
    }
  }else {
    if (parent->GetComputerData().target==targ) {
      CommunicationMessage c(targ,parent,anim,gender);  
      c.SetCurrentState (c.fsm->GetNoNode(),anim,gender);
      if (parent->getAIState()) parent->getAIState()->Communicate(c);
    }
  }
  return isDone;
}
static bool ExecuteRequestClearenceKey(Unit * parent, Unit * endt) {
  bool tmp=endt->RequestClearance(parent);
  if (endt->getRelation(parent)>=0) {
	if (endt->graphicOptions.InWarp){
		endt->graphicOptions.WarpRamping=1;
	}
	endt->graphicOptions.InWarp=0;
	static float clearencetime=(XMLSupport::parse_float (vs_config->getVariable ("general","dockingtime","20")));
	endt->EnqueueAIFirst (new ExecuteFor (new Orders::MatchVelocity (Vector(0,0,0),
		Vector(0,0,0),
		true,
		false,
		true),clearencetime));
  }
  return tmp;
}
static void DoDockingOps (Unit * parent, Unit * targ,unsigned char playa, unsigned char gender) {
  static int maxseverity=XMLSupport::parse_bool(vs_config->getVariable("AI","dock_to_area","false"))?2:1;
  Unit * endt=targ;
  bool nodockwithclear=NoDockWithClear();
  bool wasdock=vectorOfKeyboardInput[playa].doc;
  if (vectorOfKeyboardInput[playa].doc) {
    bool isDone=false;
    if (targ) {
      for (int severity=0;severity<maxseverity;++severity) { 
        targ->RequestClearance(parent);
        if ((isDone=TryDock(parent,targ,playa,severity))!=false){
          parent->EndRequestClearance(targ);
          break;
        }else {
            //if (targ!=parent->Target())
          parent->EndRequestClearance(targ);
        }
      }
    }
    if (!isDone) {
      for (int severity=0;severity<maxseverity&&!isDone;++severity) {
      for (un_iter u=_Universe->activeStarSystem()->getUnitList().createIterator();
           (targ=*u)!=NULL&&!isDone;
           ++u) {
	      //Let's make sure potentials are actually in range, and have
	      //docking ports before we try to dock with them.
	      if ((targ != parent) && 
			  (UnitUtil::isDockableUnit(targ)) &&
			  (UnitUtil::isCloseEnoughToDock(parent, targ))) {
				  
				  targ->RequestClearance(parent);
				  
				  if (TryDock(parent,targ,playa,severity)) {
					  parent->Target(targ);
					  isDone=true;
					  parent->EndRequestClearance(targ);
					  break;
				  }else {
					  //if (targ!=parent->Target())
					  parent->EndRequestClearance(targ);
				  }
		  }
	  }
	  }
	}
    if (!isDone) {
      if (endt) {
        ExecuteRequestClearenceKey(parent,endt);
      }
      abletodock(0);
    }
    vectorOfKeyboardInput[playa].doc=false;
    
  }
  if (vectorOfKeyboardInput[playa].req&&endt!=NULL) {
    bool request=ExecuteRequestClearenceKey(parent,endt);
    if (!request) {
      mission->msgcenter->add("game","all","[Computer] Cannot dock with insubstantidisabal object, target another object and retry.");
      abletodock(0);
      return;
    } else if (!wasdock) {
      abletodock(1);
    }
    vectorOfKeyboardInput[playa].req=false;
  }
  
  if (vectorOfKeyboardInput[playa].und&&endt!=NULL) {
    CommunicationMessage c(endt,parent,NULL,0);
    if (UnDockNow(parent,endt)) {
      c.SetCurrentState (c.fsm->GetUnDockNode(),NULL,0);
      abletodock(5);
    }else {
      c.SetCurrentState (c.fsm->GetFailDockNode(),NULL,0);
      abletodock(4);
    }
    parent->getAIState()->Communicate (c);
    vectorOfKeyboardInput[playa].und=0;
  }
}
using std::list;
unsigned int FireKeyboard::DoSpeechAndAni(Unit * un, Unit* parent, class CommunicationMessage&c) {
  this->AdjustRelationTo(un,c.getCurrentState()->messagedelta);
  unsigned int retval=DoSpeech (un,parent,*c.getCurrentState());
  if (parent==_Universe->AccessCockpit()->GetParent()) {
    _Universe->AccessCockpit()->SetCommAnimation (c.ani,un);
  }
  this->refresh_target=true;
  return retval;
}
static void MyFunction(){ 
      // quit it--he's dead all ready
      static string comm_static = vs_config->getVariable("graphics","comm_static","static.ani");
      // dead dead dead dead
      static Animation Statuc (comm_static.c_str());
      //yep really dead
      _Universe->AccessCockpit()->SetCommAnimation (&Statuc,NULL);
}
void FireKeyboard::ProcessCommMessage (class CommunicationMessage&c){

  Unit * un = c.sender.GetUnit();
  unsigned int whichsound=0;
  bool foundValidMessage=false;
  if (_Universe->AccessCockpit()->CheckCommAnimation(un)) {
    return;//wait till later
  }
  bool reallydospeech=false;
  if (un&&un->GetHull()>0) {
    reallydospeech=true;
    for (list<CommunicationMessage>::iterator i=resp.begin();i!=resp.end();i++) {
      if ((*i).sender.GetUnit()==un) {
		if ((i = resp.erase (i))==resp.end())
			break;
      }
    }
    resp.push_back(c);
    if (!foundValidMessage)
      whichsound=DoSpeechAndAni(un,parent,c);
  }else if (0) {

    //none of this happens
    whichsound=DoSpeech (NULL,NULL,*c.getCurrentState());
    // this is when a unit is already dead
    if (parent==_Universe->AccessCockpit()->GetParent()) {
      MyFunction();
      //mmhmm! Gcc-4.1 hack -- otherwise linker failure
    }

  }
  int sound=c.getCurrentState()->GetSound(c.sex,whichsound);
  //AUDAdjustSound(sound,parent->Position(),parent->GetVelocity());
  if (reallydospeech&&!AUDIsPlaying (sound)) {
    AUDPlay(sound,QVector(0,0,0),Vector(0,0,0),1);
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
extern std::set <Unit *> arrested_list_do_not_dereference;

void Arrested (Unit * parent) {
	std::string fac = UniverseUtil::GetGalaxyFaction ( UniverseUtil::getSystemFile());
	int own=FactionUtil::GetFactionIndex(fac);
	static string po = vs_config->getVariable("galaxy","police_faction","homeland-security");
	int police= FactionUtil::GetFactionIndex(po);
	int police2= FactionUtil::GetFactionIndex(po+"_"+fac);
	float ownrel=UnitUtil::getRelationFromFaction(parent,own);
	bool attack = ownrel<0;
	if (!attack) {
		Unit * contra =FactionUtil::GetContraband(own);
		if (contra) {
			for (unsigned int i=0;(!attack)&&i<parent->numCargo();++i) {
				Cargo * ci =&parent->GetCargo(i);
					
				for (unsigned int j=0;j<contra->numCargo();++j) {
					Cargo * cj =&contra->GetCargo(j);
					if (ci->content==cj->content) {
						attack=true;
						break;
					}
				}
			}
		}
	}
	if (!attack) {
		Unit * un;
		for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
			 (un=*i)!=NULL;
			 ++i) {
			if (un->faction==own||un->faction==police||un->faction==police2) {
				if (un->Target()==parent||un->getRelation(parent)<0) {
					int parentCp = _Universe->whichPlayerStarship(parent);
					if (parentCp!=-1) {
						UniverseUtil::adjustRelationModifier(parentCp, fac, -ownrel-.1);
					}
					attack=true;
					break;
				}
			}
		}
	}
	if (attack) {
	static std::string prison_system = vs_config->getVariable ("galaxy","PrisonSystem","Sol/Nu_Pheonix");
	std::string psys = prison_system+"_"+fac;
	if (UnitUtil::getUnitSystemFile(parent)!=psys) {
		UnitUtil::JumpTo(parent,psys);
		UniverseUtil::IOmessage(0,"game","all",parent->name+", you are under arrest!  You will be taken to the prison system and will be tried for your crimes.");
	}else {
		Unit * un;
		Unit * owner=NULL;
		Unit * base=NULL;
		for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();(un = *i)!=NULL;++i) {
			if (owner==NULL &&un->getFlightgroup() && un->faction==own)
				if (UnitUtil::isSignificant(un)&&(!un->isJumppoint())) {
					owner = un;
				}
			if (UnitUtil::isSignificant(un)&&(!un->isJumppoint())) {
				base=un;
			}
		}
		
		
		if (owner ==NULL)
			owner = base;
		if (owner) {
			Order * tmp =parent->aistate;
			parent->aistate=NULL;
			parent->PrimeOrders(new Orders::DockingOps (owner,tmp,true,true));
			arrested_list_do_not_dereference.insert (parent);
			for (int i = parent->numCargo()-1;i>=0;--i) {
				parent->RemoveCargo (i,parent->GetCargo((unsigned int)i).quantity,true);
			}
			UniverseUtil::IOmessage(0,"game","all",parent->name+", your cargo has been confiscated and scanned. Here your ship will be kept until you complete your reintegration into society through our reprogramming pod(tm) system.");
			int whichCp = _Universe->whichPlayerStarship(parent);
			UniverseUtil::adjustRelationModifierInt(whichCp,own,-UnitUtil::getRelationToFaction(parent,own));
		}
	}
	}
}
static void PowerDownShield(Shield *shield, float howmuch){
  switch (shield->number) {
  case 2:
    if (shield->shield2fb.front/howmuch>shield->shield2fb.frontmax)
      shield->shield2fb.front=shield->shield2fb.frontmax*howmuch;
    if (shield->shield2fb.back/howmuch>shield->shield2fb.backmax)
      shield->shield2fb.back=shield->shield2fb.backmax*howmuch;
    break;
  case 4:
    if (shield->shield4fbrl.front/howmuch>shield->shield4fbrl.frontmax)
      shield->shield4fbrl.front=shield->shield4fbrl.frontmax*howmuch;
    if (shield->shield4fbrl.right/howmuch>shield->shield4fbrl.rightmax)
      shield->shield4fbrl.right=shield->shield4fbrl.rightmax*howmuch;
    if (shield->shield4fbrl.left/howmuch>shield->shield4fbrl.leftmax)
      shield->shield4fbrl.left=shield->shield4fbrl.leftmax*howmuch;
    if (shield->shield4fbrl.back/howmuch>shield->shield4fbrl.backmax)
      shield->shield4fbrl.back=shield->shield4fbrl.backmax*howmuch;
    break;
  case 8:
    if (shield->shield8.frontlefttop/howmuch>shield->shield8.frontlefttopmax)
      shield->shield8.frontlefttop=shield->shield8.frontlefttopmax*howmuch;
    if (shield->shield8.frontleftbottom/howmuch>shield->shield8.frontleftbottommax)
      shield->shield8.frontleftbottom=shield->shield8.frontleftbottommax*howmuch;
    if (shield->shield8.frontrighttop/howmuch>shield->shield8.frontrighttopmax)
      shield->shield8.frontrighttop=shield->shield8.frontrighttopmax*howmuch;
    if (shield->shield8.frontrightbottom/howmuch>shield->shield8.frontrightbottommax)
      shield->shield8.frontrightbottom=shield->shield8.frontrightbottommax*howmuch;

    if (shield->shield8.backlefttop/howmuch>shield->shield8.backlefttopmax)
      shield->shield8.backlefttop=shield->shield8.backlefttopmax*howmuch;
    if (shield->shield8.backleftbottom/howmuch>shield->shield8.backleftbottommax)
      shield->shield8.backleftbottom=shield->shield8.backleftbottommax*howmuch;
    if (shield->shield8.backrighttop/howmuch>shield->shield8.backrighttopmax)
      shield->shield8.backrighttop=shield->shield8.backrighttopmax*howmuch;
    if (shield->shield8.backrightbottom/howmuch>shield->shield8.backrightbottommax)
      shield->shield8.backrightbottom=shield->shield8.backrightbottommax*howmuch;

    break;
  default:
    break;
  }

}
extern bool CheckAccessory(Unit*);
static void ForceChangeTarget(Unit*  parent) {
  Unit * curtarg = parent->Target();
  ChooseTargets(parent,TargUn,false);
  static bool force_change_only_unit=XMLSupport::parse_bool(vs_config->getVariable("graphics","target_null_if_no_unit","false"));
  if (parent->Target()==curtarg){
      if (force_change_only_unit) {
        parent->Target(NULL);
      }else {     
        ChooseTargets(parent,TargNear,false);
        if (parent->Target()==curtarg)
          ChooseTargets(parent,TargAll,false);
      }
  }
}

int SelectDockPort (Unit * utdw, Unit * parent);

bool isMissile(const weapon_info *);

void FireKeyboard::SetParent (Unit * parent1) {
  this->Order::SetParent(parent1);
  static bool allow_special_with_weapons=XMLSupport::parse_bool(vs_config->getVariable("physics","special_and_normal_gun_combo","true"));
  if (!allow_special_with_weapons) {
    parent->ToggleWeapon(false,true/*reverse*/);
    parent->ToggleWeapon(false,false/*reverse*/);
  }
}
void FireKeyboard::Execute () {
	
  while (vectorOfKeyboardInput.size()<=whichplayer||vectorOfKeyboardInput.size()<=whichjoystick) {
    vectorOfKeyboardInput.push_back(FIREKEYBOARDTYPE());
  }
  ProcessCommunicationMessages(SIMULATION_ATOM,true);
  Unit * targ = parent->Target();
  DoDockingOps(parent,targ,whichplayer,parent->pilot->getGender());
  if (SERVER || Network==NULL) {
    if (targ) {
      double mm=0.0;
      ShouldFire (targ);
      if (targ->GetHull()<0) {
        parent->Target(NULL);
        ForceChangeTarget(parent);
        refresh_target=true;
      }else if (false==parent->InRange(targ,mm,true,true,true)&&!parent->TargetLocked()) {
        ChooseTargets(parent,TargUn,false);//only go for other active units in cone
        if (parent->Target()==NULL) {
          parent->Target(targ);
        }
      }
    } else {
      ForceChangeTarget(parent);
      refresh_target=true;
    }
  }
  if (f().shieldpowerstate!=1) {
    Shield  * shield = &parent->shield;
    PowerDownShield(shield,f().shieldpowerstate);
    //f().shieldpowerstate=1;
  }
  if (f().firekey==PRESS||f().jfirekey==PRESS||j().firekey==DOWN||j().jfirekey==DOWN){
    if (!_Universe->AccessCockpit()->CanDrawNavSystem()) {
      static bool allow_special_with_weapons=XMLSupport::parse_bool(vs_config->getVariable("physics","special_and_normal_gun_combo","true"));
      if (!allow_special_with_weapons) {
        bool badness=false;
        bool special=false;
        bool normal=false;
        int nm=parent->GetNumMounts();
        int i;
        for (i=0;i<nm;++i) {
          if (parent->mounts[i].status==Mount::ACTIVE) {
            special=special||(parent->mounts[i].type->size&weapon_info::SPECIAL)!=0;
            normal=normal||(parent->mounts[i].type->size&(weapon_info::LIGHT|weapon_info::MEDIUM|weapon_info::HEAVY|weapon_info::CAPSHIPLIGHT|weapon_info::CAPSHIPHEAVY))!=0;
          }
        }
        for (i=0;i<nm;++i) {
          if (special&&normal) {
            //parent->ToggleWepaon(false,true);
            if (parent->mounts[i].status==Mount::ACTIVE) {
              if ((parent->mounts[i].type->size&weapon_info::SPECIAL)!=0) {
                parent->mounts[i].status=Mount::INACTIVE;
              }
            }
          }
        }
      }
      parent->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS,false);
    }
  }
  if (f().missilekey==PRESS||j().jmissilekey==PRESS) {
    if (!_Universe->AccessCockpit()->CanDrawNavSystem())
      parent->Fire(ROLES::FIRE_MISSILES|ROLES::EVERYTHING_ELSE,false);
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

    f().cloakkey=DOWN;
    parent->Cloak(cloaktoggle);
    cloaktoggle=!cloaktoggle;
  }
  if (f().lockkey==PRESS) {
    f().lockkey=DOWN;
    parent->LockTarget(!parent->TargetLocked());
  }
  if (f().ECMkey==PRESS) {
	  f().ECMkey=DOWN;
    parent->GetImageInformation().ecm=-parent->GetImageInformation().ecm;
    
  }
#ifdef CAR_SIM
  int origecm=UnitUtil::getECM(parent);
  if (origecm>=CAR::ON_NO_BLINKEN)
    origecm=CAR::FORWARD_BLINKEN;
  if (origecm<0)
    origecm=0;
  if (f().blinkleftkey==PRESS) {
    f().blinkleftkey=DOWN;
    if ((origecm&CAR::LEFT_BLINKEN)) {
      UnitUtil::setECM (parent,(origecm& (~CAR::LEFT_BLINKEN)));
    }else {
      UnitUtil::setECM (parent,origecm|CAR::LEFT_BLINKEN);
    }
  }
  if (f().blinkrightkey==PRESS) {
    f().blinkrightkey=DOWN;
    if ((origecm&CAR::RIGHT_BLINKEN)) {
      UnitUtil::setECM (parent,(origecm& (~CAR::RIGHT_BLINKEN)));
    }else {
      UnitUtil::setECM (parent,origecm|CAR::RIGHT_BLINKEN);
    }
  }
  if (f().sirenkey==PRESS) {
    f().sirenkey=DOWN;
    if ((origecm&CAR::SIREN_BLINKEN)) {
      UnitUtil::setECM (parent,(origecm& (~CAR::SIREN_BLINKEN)));
    }else {
      UnitUtil::setECM (parent,origecm|CAR::SIREN_BLINKEN);
    }
  }
  if (f().headlightkey==PRESS) {
    f().headlightkey=DOWN;
    if ((origecm&CAR::FORWARD_BLINKEN)) {
      UnitUtil::setECM (parent,(origecm& (~CAR::FORWARD_BLINKEN)));
    }else {
      UnitUtil::setECM (parent,origecm|CAR::FORWARD_BLINKEN);
    }
  }
#endif
  if (f().targetkey==PRESS||j().jtargetkey==PRESS) {
    f().targetkey=DOWN;
    j().jtargetkey=DOWN;
    ChooseTargets(parent,TargAll,false);
    refresh_target=true;
  }
  if (f().rtargetkey==PRESS) {
    f().rtargetkey=DOWN;
    ChooseTargets(parent,TargAll,true);
    refresh_target=true;
  }
  if (f().missiontargetkey==PRESS) {
    f().missiontargetkey=DOWN;
    ChooseTargets(parent,TargMission,false);
    refresh_target=true;
  }
  if (f().rmissiontargetkey==PRESS) {
    f().rmissiontargetkey=DOWN;
    ChooseTargets(parent,TargMission,true);
    refresh_target=true;
  }
  if (f().targetskey==PRESS) {
    f().targetskey=DOWN;
    ChooseTargets(parent,TargSig,false);
    refresh_target=true;
    parent->LockTarget(true);
  }
  if (f().targetukey==PRESS) {
    f().targetukey=DOWN;
    static bool smart_targetting=XMLSupport::parse_bool(vs_config->getVariable("graphics","smart_targetting_key","true"));
    Unit * tmp=parent->Target();
    bool sysobj=false;
    if (tmp)
      if (tmp->owner==getTopLevelOwner())
        sysobj=true;
    ChooseTargets(parent,TargUn,false);
    if ((Network==NULL||parent->Target()==NULL) && tmp==parent->Target()&&sysobj&&smart_targetting) {
      ChooseTargets(parent,TargSig,false);
      if (tmp==parent->Target()) {
        ChooseTargets(parent,TargAll,false);
      }
    }
    refresh_target=true;
  }
  if(f().picktargetkey==PRESS){
    f().picktargetkey=DOWN;
    ChooseTargets(parent,TargFront,false);
    refresh_target=true;
  }
  if (f().neartargetkey==PRESS) {
    ChooseTargets(parent,TargNear,false);
    f().neartargetkey=DOWN;
    refresh_target=true;
  }
  if (f().missiletargetkey==PRESS) {
    ChooseTargets(parent,TargMissile,false);
    f().missiletargetkey=DOWN;
    refresh_target=true;
  }

  if (f().incomingmissiletargetkey==PRESS) {
    ChooseTargets(parent,TargIncomingMissile,false);
    f().incomingmissiletargetkey=DOWN;
    refresh_target=true;
  }


  if (f().rmissiletargetkey==PRESS) {
    ChooseTargets(parent,TargMissile,true);
    f().rmissiletargetkey=DOWN;
    refresh_target=true;
  }

  if (f().rincomingmissiletargetkey==PRESS) {
    ChooseTargets(parent,TargIncomingMissile,true);
    f().rincomingmissiletargetkey=DOWN;
    refresh_target=true;
  }


  if (f().threattargetkey==PRESS) {
    ChooseTargets(parent,TargThreat,false);
    f().threattargetkey=DOWN;
    refresh_target=true;
  }
  if (f().subtargetkey==PRESS) {
    ChooseSubTargets(parent);
    f().subtargetkey=DOWN;
    refresh_target=true;
  }
  if(f().rpicktargetkey==PRESS){
    f().rpicktargetkey=DOWN;
    ChooseTargets(parent,TargFront,true);
    refresh_target=true;
  }
  if (f().rneartargetkey==PRESS) {
    ChooseTargets(parent,TargNear,true);
    f().rneartargetkey=DOWN;
    refresh_target=true;
  }
  if (f().rthreattargetkey==PRESS) {
    ChooseTargets(parent,TargThreat,true);
    f().rthreattargetkey=DOWN;
    refresh_target=true;
  }
  if (f().rtargetskey==PRESS) {
    f().rtargetskey=DOWN;
    ChooseTargets(parent,TargSig,true);
    refresh_target=true;
    parent->LockTarget(true);
  }
  if (f().rtargetukey==PRESS) {
    f().rtargetukey=DOWN;
    static bool smart_targetting=XMLSupport::parse_bool(vs_config->getVariable("graphics","smart_targetting_key","true"));
    Unit * tmp = parent->Target();
    bool sysobj=false;
    if (tmp)
      if (tmp->owner==getTopLevelOwner())
        sysobj=true;
    ChooseTargets(parent,TargUn,true);
    if (tmp==parent->Target()&&sysobj&&smart_targetting) {
      ChooseTargets(parent,TargFront,true);
      if (tmp==parent->Target()) {
        ChooseTargets(parent,TargAll,true);
      }
    }
    refresh_target=true;
  }

  if (f().turretaikey == PRESS) {
    parent->SetTurretAI();
    parent->TargetTurret(parent->Target());
    f().turretaikey=DOWN;
  }
  static bool noturretai=XMLSupport::parse_bool(vs_config->getVariable("AI","no_turret_ai","false"));
  static int taicounter=0;
  if(f().turretoffkey==PRESS||(noturretai&&taicounter++%128==0)) {
      parent->DisableTurretAI();
      f().turretoffkey = DOWN;
  }
  if(f().turretfaw==PRESS) {
      parent->TurretFAW();
      f().turretfaw = DOWN;
  }
  
  if (f().turretaikey==RELEASE) {
    f().turretaikey = UP;
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
  //Added for nearest unit targeting -ch
  if (f().nearesthostilekey==PRESS) {
	  getNearestTargetUnit (parent, 0);
	  f().nearesthostilekey=DOWN;
	  refresh_target=true;
  }

  if (f().nearestdangeroushostilekey==PRESS) {
	  getNearestTargetUnit (parent, 1);
	  f().nearestdangeroushostilekey=DOWN;
	  refresh_target=true;
  }

  if (f().nearestfriendlykey==PRESS) {
	  getNearestTargetUnit (parent, 2);
	  f().nearestfriendlykey=DOWN;
	  refresh_target=true;
  }

  if (f().nearestbasekey==PRESS) {
	  getNearestTargetUnit (parent, 3);
	  f().nearestbasekey=DOWN;
	  refresh_target=true;
  }
  if (f().nearestplanetkey==PRESS) {
	  getNearestTargetUnit (parent, 4);
	  f().nearestplanetkey=DOWN;
	  refresh_target=true;
  }
  if (f().nearestjumpkey==PRESS) {
	  getNearestTargetUnit (parent, 5);
	  f().nearestjumpkey=DOWN;
	  refresh_target=true;
  }

  if (f().weapk==PRESS||f().rweapk==PRESS) {
    bool forward;
    if (f().weapk==PRESS) {
      f().weapk=DOWN;
      forward=true;
    }
    if (f().rweapk==PRESS) {
      f().rweapk=DOWN;
      forward=false;
    }
    parent->UnFire();
    parent->ToggleWeapon (false, forward);
		static soundContainer weapsound;
		if (weapsound.sound<0) {
			static string str=vs_config->getVariable("cockpitaudio","weapon_switch","vdu_d");
			weapsound.loadsound(str);
		}
		weapsound.playsound();
  }
  if (f().toggleanimation == PRESS) {
	  f().toggleanimation=DOWN;
	  parent->graphicOptions.Animating = parent->graphicOptions.Animating?0:1;
  }
  
  if (f().toggleglow == PRESS) {
	  f().toggleglow=DOWN;
	 static bool isvis=true;
	  isvis=!isvis;
	  parent->SetGlowVisible (isvis);
  }

  if (f().togglewarpdrive == PRESS) {
	  f().togglewarpdrive=DOWN;
	  parent->graphicOptions.InWarp=1-parent->graphicOptions.InWarp;
	  parent->graphicOptions.WarpRamping=1;
  }
  if (f().toggleautotracking == PRESS) {
	  f().toggleautotracking=DOWN;
          if (autotrackingtoggle==1) {
            autotrackingtoggle = parent->GetComputerData().radar.trackingcone;
            parent->GetComputerData().radar.trackingcone=1;
          }else {
            parent->GetComputerData().radar.trackingcone=autotrackingtoggle;
            autotrackingtoggle=1;
          }
  }

  if (f().misk==PRESS||f().rmisk==PRESS) {
    bool forward;
    if (f().misk==PRESS) {
      f().misk=DOWN;
      forward=true;
    }
    if (f().rmisk==PRESS) {
      f().rmisk=DOWN;
      forward=false;
    }
    parent->ToggleWeapon(true, forward);
    static soundContainer missound;
    if (missound.sound<0) {
      static string str=vs_config->getVariable("cockpitaudio","missile_switch","vdu_d");
      missound.loadsound(str);
    }
    missound.playsound();
  }
  unsigned int i;
  for (i=0;i<NUMSAVEDTARGETS;i++) {  
    if (f().saveTargetKeys[i]==PRESS) {
      f().saveTargetKeys[i]=RELEASE;
      savedTargets[i]=parent->Target();
    }
    if (f().restoreTargetKeys[i]==PRESS&&parent->GetComputerData().radar.canlock) {
      f().restoreTargetKeys[i]=RELEASE;
      Unit * un;
      for (un_iter u=_Universe->activeStarSystem()->getUnitList().createIterator();
           (un=*u)!=NULL;
           ++u) {
        if (un==savedTargets[i]) {
          parent->Target(un);
          break;
        }
      }
    }
  }
  for (i=0;i<NUMCOMMKEYS;i++) {
    if (f().commKeys[i]==PRESS) {
      /*
      static string comm_switchstr=vs_config->getVariable("cockpitaudio","interface_target","vdu_e");
      static soundContainer comm_switch;

      if (comm_switch.sound<0)
        comm_switch.loadsound(comm_switchstr);
      comm_switch.playsound();
      */
      f().commKeys[i]=RELEASE;
      Unit * targ=parent->Target();
      if (targ) {
	CommunicationMessage * mymsg = GetTargetMessageQueue(targ,resp);       
        FSM *fsm =FactionUtil::GetConversation (parent->faction,targ->faction);
	if (mymsg==NULL||mymsg->curstate>=fsm->nodes.size()) {
	  CommunicationMessage c(parent,targ,i,NULL,parent->pilot->getGender());
	  unsigned int whichspeech=DoSpeech (targ,targ,*c.getCurrentState());
          int sound = c.getCurrentState()->GetSound(c.sex,whichspeech);
          //AUDAdjustSound(sound,parent->Position(),parent->GetVelocity());
	  if (!AUDIsPlaying (sound)) {
	    AUDPlay(sound,QVector(0,0,0),Vector(0,0,0),1);
	  }
	  Order * o = targ->getAIState();
	  if (o)
		  o->Communicate (c);
	}else {
          FSM * tmp = mymsg->fsm;
          mymsg->fsm = fsm;
	  FSM::Node * n = mymsg->getCurrentState();
	  if (i<n->edges.size()) {
	    CommunicationMessage c(parent,targ,*mymsg,i,NULL,parent->pilot->getGender());
	    unsigned int whichmessage=DoSpeech (targ,targ,*c.getCurrentState());
            int sound=c.getCurrentState()->GetSound(c.sex,whichmessage);
            //AUDAdjustSound(sound,parent->Position(),parent->GetVelocity());
	    if (!AUDIsPlaying (sound)) {
	      AUDPlay(sound,QVector(0,0,0),Vector(0,0,0),1);
	    }
            Order * oo = targ->getAIState();
            if (oo)
              oo->Communicate (c);
	  }
          mymsg->fsm=tmp;
	}
      }
    }
  }
  if (refresh_target) {
    Unit * targ;
    if ((targ =parent->Target())) {
      if (parent->isSubUnit())
        parent->TargetTurret(targ);
      CommunicationMessage *mymsg = GetTargetMessageQueue(targ,resp);
      FSM *fsm =FactionUtil::GetConversation (parent->faction,targ->faction);
      if (mymsg==NULL) {
	_Universe->AccessCockpit()->communication_choices=fsm->GetEdgesString(fsm->getDefaultState(parent->getRelation(targ)));
      }else {
       _Universe->AccessCockpit()->communication_choices=fsm->GetEdgesString(mymsg->curstate);
      }
    } else {
      _Universe->AccessCockpit()->communication_choices="\nNo Communication\nLink\nEstablished";
    }
  }
  if (f().enslave==PRESS||f().freeslave==PRESS) {
    Enslave(parent,f().enslave==PRESS);
    f().enslave=RELEASE;
    f().freeslave=RELEASE;
  }
  if (f().ejectcargo==PRESS||f().ejectnonmissioncargo==PRESS) {
    bool missiontoo=(f().ejectcargo==PRESS);
    f().ejectnonmissioncargo=RELEASE;
    f().ejectcargo=RELEASE;
    int offset = _Universe->AccessCockpit()->getScrollOffset (VDU::MANIFEST);
    if (offset<3) {
      offset=0;
    }else {
      offset-=3;
    }
    for(;offset<parent->numCargo();++offset) {
      Cargo * tmp=&parent->GetCargo(offset);
      if (tmp->GetCategory().find("upgrades")==string::npos&&(missiontoo||tmp->mission==false)) {
        parent->EjectCargo(offset);
        break;
      }
    }
    if (missiontoo) {
      f().ejectnonmissioncargo=DOWN;
    }else {
      f().ejectcargo=DOWN;
    }
  }
  // i think this ejects the pilot? yep it does
  if (f().eject==PRESS) {
    f().eject=DOWN;
    Cockpit * cp;
    if ((parent->name != "eject") && (parent->name != "Pilot") && (cp=_Universe->isPlayerStarship (parent))) {
      cp->Eject();
    }
  }
 
  // eject pilot and warp pilot to the docking screen instantly.

  if (f().ejectdock==PRESS) {
    f().ejectdock=DOWN;
	Unit * utdw = parent;
    Cockpit * cp; // check if docking ports exist, no docking ports = no need to ejectdock so don't do anything
    if ( (SelectDockPort (utdw, parent) > -1 ) && (cp=_Universe->isPlayerStarship (parent))) {
      cp->EjectDock(); // use specialized ejectdock in the future
//         DockedScript(parent,utdw);      
//         parent->Dock(utdw);

	}
  }
  static bool actually_arrest=XMLSupport::parse_bool(vs_config->getVariable("AI","arrest_energy_zero","false"));
  
  if (actually_arrest&&parent->EnergyRechargeData()==0) {
	  Arrested(parent);
  }
}
