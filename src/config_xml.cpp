/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"
#include <assert.h>


#include "config_xml.h"
#include "easydom.h"
#include "cmd/ai/flykeyboard.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/music.h"
#include "gfx/loc_select.h"
#include "audiolib.h"
#include "in_joystick.h"
#include "main_loop.h" // for CockpitKeys
#include "gfx/cockpit.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

/* *********************************************************** */

GameVegaConfig::GameVegaConfig(const char *configfile): VegaConfig( configfile)
{
	/*
  configNodeFactory *domf = new configNodeFactory();

  configNode *top=(configNode *)domf->LoadXML(configfile);
  if(top==NULL){
    cout << "Panic exit - no configuration" << endl;
    exit(0);
  }
  variables=NULL;
  colors=NULL;
	*/
  //top->walk(0);
  
  initCommandMap();
  initKeyMap();

  // set hatswitches to off
  for(int h=0;h<MAX_HATSWITCHES;h++){
    hatswitch_margin[h]=2.0;
    for(int v=0;v<MAX_VALUES;v++){
      hatswitch[h][v]=2.0;
    }
  }

  for(int i=0;i<=MAX_AXES;i++){
    axis_axis[i]=-1;
    axis_joy[i]=-1;
  }
}

/*
for i in `cat cmap` ; do echo "  command_map[\""$i"\"]=FlyByKeyboard::"$i ";" ; done
 */

/* *********************************************************** */

#if 1
const float volinc = 1;
const float dopinc = .1;
void incmusicvol (int i, KBSTATE a);
void decmusicvol (int i, KBSTATE a);

void incvol (int i, KBSTATE a) {
#ifdef HAVE_AL
	if (a==DOWN) {
		AUDChangeVolume (AUDGetVolume()+volinc);
	}
#endif
}

void decvol (int i, KBSTATE a) {
#ifdef HAVE_AL
	if (a==DOWN) {
		AUDChangeVolume (AUDGetVolume()-volinc);
	}
#endif
}
void mute (int i, KBSTATE a) { 
#ifdef HAVE_AL
//	if (a==PRESS)
//		AUDChangeVolume (0);broken
#endif
}

void incdop (int i, KBSTATE a) {
#ifdef HAVE_AL
	if (a==DOWN) {
		AUDChangeDoppler (AUDGetDoppler()+dopinc);
	}

#endif
}
void decdop (int i, KBSTATE a) {
#ifdef HAVE_AL
	if (a==DOWN) {
		AUDChangeDoppler (AUDGetDoppler()-dopinc);
	}
#endif
}
#endif
/* *********************************************************** */

void GameVegaConfig::initKeyMap(){
  // mapping from special key string to glut key
  key_map["space"]=' ';
  key_map["return"]=13;
  key_map["enter"]=WSK_KP_ENTER;
  key_map["function-1"]=WSK_F1;
  key_map["function-2"]=WSK_F2;
  key_map["function-3"]=WSK_F3;
  key_map["function-4"]=WSK_F4;
  key_map["function-5"]=WSK_F5;
  key_map["function-6"]=WSK_F6;
  key_map["function-7"]=WSK_F7;
  key_map["function-8"]=WSK_F8;
  key_map["function-9"]=WSK_F9;
  key_map["function-10"]=WSK_F10;
  key_map["function-11"]=WSK_F11;
  key_map["function-12"]=WSK_F12;
  key_map["keypad-insert"]=WSK_KP0;
  key_map["keypad-0"]=WSK_KP0;
  key_map["keypad-1"]=WSK_KP1;
  key_map["keypad-2"]=WSK_KP2;
  key_map["keypad-3"]=WSK_KP3;
  key_map["keypad-4"]=WSK_KP4;
  key_map["keypad-5"]=WSK_KP5;
  key_map["keypad-6"]=WSK_KP6;
  key_map["keypad-7"]=WSK_KP7;
  key_map["keypad-8"]=WSK_KP8;
  key_map["keypad-9"]=WSK_KP9;
  key_map["less-than"]='<';
  key_map["greater-than"]='>';
  key_map["keypad-numlock"]=WSK_NUMLOCK;
  key_map["keypad-period"]=WSK_KP_PERIOD;
  key_map["keypad-delete"]=WSK_KP_PERIOD;
  key_map["keypad-divide"]=WSK_KP_DIVIDE;
  key_map["keypad-multiply"]=WSK_KP_MULTIPLY;
  key_map["keypad-minus"]=WSK_KP_MINUS;
  key_map["keypad-plus"]=WSK_KP_PLUS;
  key_map["keypad-enter"]=WSK_KP_ENTER;
  key_map["keypad-equals"]=WSK_KP_EQUALS;
  key_map["scrollock"]=WSK_SCROLLOCK;
  key_map["right-ctrl"]=WSK_RCTRL;
  key_map["left-ctrl"]=WSK_LCTRL;
  key_map["right-alt"]=WSK_RALT;
  key_map["left-alt"]=WSK_LALT;
  key_map["right-meta"]=WSK_RMETA;
  key_map["left-meta"]=WSK_LMETA;

  key_map["cursor-left"]=WSK_LEFT;
  key_map["cursor-up"]=WSK_UP;
  key_map["cursor-right"]=WSK_RIGHT;
  key_map["cursor-down"]=WSK_DOWN;

  key_map["cursor-pageup"]=WSK_PAGEUP;
  key_map["cursor-pagedown"]=WSK_PAGEDOWN;
  key_map["cursor-home"]=WSK_HOME;
  key_map["cursor-end"]=WSK_END;
  key_map["cursor-insert"]=WSK_INSERT;
  key_map["backspace"]=
#ifdef __APPLE__
    127;
#else
    8;
#endif
  key_map["capslock"]=WSK_CAPSLOCK;
  key_map["cursor-delete"]=
#ifdef __APPLE__
    8;
#else
    127;
#endif
  key_map["tab"]='\t';
  key_map["esc"]=27;
  key_map["break"]=WSK_BREAK;
  key_map["pause"]=WSK_BREAK;

}

#if 0
  sed 's/\(.*void \)\(.*\)(.*/ command_map[\"Cockpit::\2\"]=CockpitKeys::\2;/'
#endif

/* *********************************************************** */
extern void  inc_time_compression (int i, KBSTATE a);
extern void  JoyStickToggleKey (int i, KBSTATE a);
extern void  SuicideKey (int i, KBSTATE a);
extern void pause_key (int i, KBSTATE a);
extern void dec_time_compression(int i, KBSTATE a);
extern void reset_time_compression(int i, KBSTATE a);
extern void MapKey(int i, KBSTATE a);
extern void VolUp(int i, KBSTATE a);
extern void VolDown(int i, KBSTATE a);

  using namespace CockpitKeys;

void GameVegaConfig::initCommandMap(){
  //  I don't knwo why this gives linker errors!
  command_map["NoPositionalKey"]=mute;
  command_map["DopplerInc"]=incdop;
  command_map["Cockpit::NavScreen"]=GameCockpit::NavScreen;
  command_map["DopplerDec"]=decdop;
  command_map["VolumeInc"]=VolUp;
  command_map["VolumeDec"]=VolDown;
  command_map["MusicVolumeInc"]=incmusicvol;
  command_map["MusicVolumeDec"]=decmusicvol;
  command_map["SetShieldsOneThird"]=FireKeyboard::SetShieldsOneThird;
  command_map["SetShieldsTwoThird"]=FireKeyboard::SetShieldsTwoThird;
  command_map["SwitchControl"]=GameCockpit::SwitchControl;
  command_map["Respawn"]=GameCockpit::Respawn;
  command_map["TurretControl"]=GameCockpit::TurretControl;

  command_map["TimeInc"]=inc_time_compression;
  command_map["TimeDec"]=dec_time_compression;
  command_map["TimeReset"]=reset_time_compression;
  // mapping from command string to keyboard handler
  // Networking bindings


  command_map["SwitchWebcam"]=FlyByKeyboard::SwitchWebcam ;
  command_map["SwitchSecured"]=FlyByKeyboard::SwitchSecured ;
  command_map["ChangeCommStatus"]=FlyByKeyboard::ChangeCommStatus ;
  command_map["UpFreq"]=FlyByKeyboard::UpFreq ;
  command_map["DownFreq"]=FlyByKeyboard::DownFreq ;

  command_map["ThrustModeKey"]=FlyByKeyboard::KSwitchFlightMode ;
  command_map["ThrustRight"]=FlyByKeyboard::KThrustRight ;
  command_map["ThrustLeft"]=FlyByKeyboard::KThrustLeft ;
  command_map["ThrustBack"]=FlyByKeyboard::KThrustBack ;
  command_map["ThrustFront"]=FlyByKeyboard::KThrustFront ;
  command_map["ThrustDown"]=FlyByKeyboard::KThrustDown ;
  command_map["ThrustUp"]=FlyByKeyboard::KThrustUp ;

  command_map["JoyStickToggleKey"]=JoyStickToggleKey;
  command_map["SheltonKey"]=FlyByKeyboard::SheltonKey ;
  command_map["MatchSpeedKey"]=FlyByKeyboard::MatchSpeedKey ;
  command_map["PauseKey"]=pause_key;
  command_map["JumpKey"]=FlyByKeyboard::JumpKey;
  command_map["AutoKey"]=FlyByKeyboard::AutoKey;
  command_map["SwitchCombatMode"]=FlyByKeyboard::SwitchCombatModeKey;
  command_map["StartKey"]=FlyByKeyboard::StartKey ;
  command_map["StopKey"]=FlyByKeyboard::StopKey ;
  command_map["UpKey"]=FlyByKeyboard::UpKey ;
  command_map["DownKey"]=FlyByKeyboard::DownKey ;
  command_map["LeftKey"]=FlyByKeyboard::LeftKey ;
  command_map["RightKey"]=FlyByKeyboard::RightKey ;
  command_map["ABKey"]=FlyByKeyboard::ABKey ;
  command_map["AccelKey"]=FlyByKeyboard::AccelKey ;
  command_map["DecelKey"]=FlyByKeyboard::DecelKey ;
  command_map["RollLeftKey"]=FlyByKeyboard::RollLeftKey ;
  command_map["RollRightKey"]=FlyByKeyboard::RollRightKey ;
  command_map["SetVelocityRefKey"]=FlyByKeyboard::SetVelocityRefKey ;
  command_map["SetVelocityNullKey"]=FlyByKeyboard::SetNullVelocityRefKey ;
  command_map["ToggleGlow"]=FireKeyboard::ToggleGlow;
  command_map["ToggleWarpDrive"]=FireKeyboard::ToggleWarpDrive;
  command_map["ToggleAnimation"]=FireKeyboard::ToggleAnimation;  
  command_map["CommAttackTarget"]=FireKeyboard::AttackTarget;
  
  command_map["CommHelpMeOutCrit"]=FireKeyboard::HelpMeOutCrit;
  command_map["CommHelpMeOutFaction"]=FireKeyboard::HelpMeOutFaction;
  command_map["JoinFlightgroup"]=FireKeyboard::JoinFg;
  command_map["CommAttackTarget"]=FireKeyboard::AttackTarget;
  command_map["CommHelpMeOut"]=FireKeyboard::HelpMeOut;
  command_map["CommFormUp"]=FireKeyboard::FormUp;
#ifdef CAR_SIM
  command_map["BlinkLeftKey"]=FireKeyboard::BlinkLeftKey;
  command_map["BlinkRightKey"]=FireKeyboard::BlinkRightKey;
  command_map["HeadlightKey"]=FireKeyboard::HeadlightKey;
  command_map["SirenKey"]=FireKeyboard::SirenKey;
#endif
  command_map["CommBreakForm"]=FireKeyboard::BreakFormation;
  command_map["Comm1Key"]=FireKeyboard::PressComm1Key ;
  command_map["Comm2Key"]=FireKeyboard::PressComm2Key ;
  command_map["Comm3Key"]=FireKeyboard::PressComm3Key ;
  command_map["Comm4Key"]=FireKeyboard::PressComm4Key ;
  command_map["Comm5Key"]=FireKeyboard::PressComm5Key ;
  command_map["Comm6Key"]=FireKeyboard::PressComm6Key ;
  command_map["Comm7Key"]=FireKeyboard::PressComm7Key ;
  command_map["Comm8Key"]=FireKeyboard::PressComm8Key ;
  command_map["Comm9Key"]=FireKeyboard::PressComm9Key ;
  command_map["Comm10Key"]=FireKeyboard::PressComm10Key ;
  command_map["EjectCargoKey"]=FireKeyboard::EjectCargoKey ;  
  command_map["EjectKey"]=FireKeyboard::EjectKey ;
  command_map["SuicideKey"]=SuicideKey ;
  command_map["TurretAIKey"]=FireKeyboard::TurretAI ;
  command_map["DockKey"]=FireKeyboard::DockKey ;
  command_map["UnDockKey"]=FireKeyboard::UnDockKey ;
  command_map["RequestClearenceKey"]=FireKeyboard::RequestClearenceKey ;
  command_map["FireKey"]=FireKeyboard::FireKey ;
  command_map["MissileKey"]=FireKeyboard::MissileKey ;
  command_map["TargetKey"]=FireKeyboard::TargetKey ;
  command_map["LockTargetKey"]=FireKeyboard::LockKey ;
  command_map["ReverseTargetKey"]=FireKeyboard::ReverseTargetKey ;
  command_map["PickTargetKey"]=FireKeyboard::PickTargetKey ;
  command_map["SubUnitTargetKey"]=FireKeyboard::SubUnitTargetKey ;
  command_map["NearestTargetKey"]=FireKeyboard::NearestTargetKey ;
  command_map["ThreatTargetKey"]=FireKeyboard::ThreatTargetKey ;
  command_map["SigTargetKey"]=FireKeyboard::SigTargetKey ;
  command_map["UnitTargetKey"]=FireKeyboard::UnitTargetKey ;
  command_map["ReversePickTargetKey"]=FireKeyboard::ReversePickTargetKey ;
  command_map["ReverseNearestTargetKey"]=FireKeyboard::ReverseNearestTargetKey ;
  command_map["ReverseThreatTargetKey"]=FireKeyboard::ReverseThreatTargetKey ;
  command_map["ReverseSigTargetKey"]=FireKeyboard::ReverseSigTargetKey ;
  command_map["ReverseUnitTargetKey"]=FireKeyboard::ReverseUnitTargetKey ;
  command_map["TurretTargetKey"]=FireKeyboard::TargetTurretKey ;
  command_map["TurretPickTargetKey"]=FireKeyboard::PickTargetTurretKey ;
  command_map["TurretNearestTargetKey"]=FireKeyboard::NearestTargetTurretKey ;
  command_map["TurretThreatTargetKey"]=FireKeyboard::ThreatTargetTurretKey ;
  command_map["WeapSelKey"]=FireKeyboard::WeapSelKey ;
  command_map["MisSelKey"]=FireKeyboard::MisSelKey ;
  command_map["ReverseWeapSelKey"]=FireKeyboard::ReverseWeapSelKey ;
  command_map["ReverseMisSelKey"]=FireKeyboard::ReverseMisSelKey ;
  command_map["CloakKey"]=FireKeyboard::CloakKey;
  command_map["ECMKey"]=FireKeyboard::ECMKey;
 command_map["Cockpit::ScrollDown"]=CockpitKeys::ScrollDown;
 command_map["Cockpit::ScrollUp"]=CockpitKeys::ScrollUp;

 command_map["Cockpit::PitchDown"]=CockpitKeys::PitchDown;
 command_map["Cockpit::PitchUp"]=CockpitKeys::PitchUp;
 command_map["Cockpit::YawLeft"]=CockpitKeys::YawLeft;
 command_map["Cockpit::YawRight"]=CockpitKeys::YawRight;
 command_map["Cockpit::Inside"]=CockpitKeys::Inside;
 command_map["Cockpit::ViewTarget"]=CockpitKeys::ViewTarget;
 command_map["Cockpit::OutsideTarget"]=CockpitKeys::OutsideTarget;
 command_map["Cockpit::PanTarget"]=CockpitKeys::PanTarget;
 command_map["Cockpit::ZoomOut"]=CockpitKeys::ZoomOut ;
 command_map["Cockpit::ZoomIn"]=CockpitKeys::ZoomIn ;
 command_map["Cockpit::InsideLeft"]=CockpitKeys::InsideLeft;
 command_map["Cockpit::InsideRight"]=CockpitKeys::InsideRight;
 command_map["Cockpit::InsideBack"]=CockpitKeys::InsideBack;
 command_map["Cockpit::SwitchLVDU"]=CockpitKeys::SwitchLVDU;
 command_map["Cockpit::MapKey"]=MapKey;
 command_map["Cockpit::SwitchRVDU"]=CockpitKeys::SwitchRVDU;
 command_map["Cockpit::SwitchMVDU"]=CockpitKeys::SwitchMVDU;
 command_map["Cockpit::SwitchURVDU"]=CockpitKeys::SwitchURVDU;
 command_map["Cockpit::SwitchULVDU"]=CockpitKeys::SwitchULVDU;
 command_map["Cockpit::SwitchUMVDU"]=CockpitKeys::SwitchUMVDU;
 command_map["Cockpit::Behind"]=CockpitKeys::Behind;
 command_map["Cockpit::Pan"]=CockpitKeys::Pan;
 command_map["Cockpit::SkipMusicTrack"]=CockpitKeys::SkipMusicTrack;

 command_map["Cockpit::Quit"]=CockpitKeys::Quit;

 // command_map["LocationSelect::MoveMoveHandle"]=LocationSelect::MouseMoveHandle;
 //command_map["LocationSelect::incConstanst"]=LocationSelect::incConstant;
 //command_map["LocationSelect::decConstant"]=LocationSelect::decConstant;

 //command_map["CoordinateSelect::MouseeMoveHandle"]=CoordinateSelect::MouseMoveHandle;

}

/* *********************************************************** */

void GameVegaConfig::doBindings(configNode *node){
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);

    if((cnode)->Name()=="bind"){
      checkBind(cnode);
    }
    else if(((cnode)->Name()=="axis")){
      doAxis(cnode);
    }
    else{
      cout << "Unknown tag: " << (cnode)->Name() << endl;
    }
  }
}

/* *********************************************************** */

void GameVegaConfig::doAxis(configNode *node){

  string name=node->attr_value("name");
  string myjoystick=node->attr_value("joystick");
  string axis=node->attr_value("axis");
  string invertstr=node->attr_value("inverse");
  string mouse_str=node->attr_value("mouse");

  if(name.empty() || (mouse_str.empty()&&myjoystick.empty()) || axis.empty()){
    cout << "no correct axis desription given " << endl;
    return;
  }

  int joy_nr=atoi(myjoystick.c_str());
  if (!mouse_str.empty()) {
    joy_nr = MOUSE_JOYSTICK;
  }
  int axis_nr=atoi(axis.c_str());

  // no checks for correct number yet 

  bool inverse=false;
  if(!invertstr.empty()){
    inverse=XMLSupport::parse_bool(invertstr);
  }

  if(name=="x"){
    axis_joy[0]=joy_nr;
    joystick[joy_nr]->axis_axis[0]=axis_nr;
    joystick[joy_nr]->axis_inverse[0]=inverse;
  }
  else if(name=="y"){
    axis_joy[1]=joy_nr;
    joystick[joy_nr]->axis_axis[1]=axis_nr;
    joystick[joy_nr]->axis_inverse[1]=inverse;
  }
  else if(name=="z"){
    axis_joy[2]=joy_nr;
    joystick[joy_nr]->axis_axis[2]=axis_nr;
    joystick[joy_nr]->axis_inverse[2]=inverse;
  }
  else if(name=="throttle"){
    axis_joy[3]=joy_nr;
    joystick[joy_nr]->axis_axis[3]=axis_nr;
    joystick[joy_nr]->axis_inverse[3]=inverse;

  }
  else if(name=="hatswitch"){
    string nr_str=node->attr_value("nr");
    string margin_str=node->attr_value("margin");

    if(nr_str.empty() || margin_str.empty()){
      cout << "you have to assign a number and a margin to the hatswitch" << endl;
      return;
    }
    int nr=atoi(nr_str.c_str());

    float margin=atof(margin_str.c_str());
    hatswitch_margin[nr]=margin;

    hatswitch_axis[nr]=axis_nr;
    hatswitch_joystick[nr]=joy_nr;

    vector<easyDomNode *>::const_iterator siter;
  
    hs_value_index=0;
    for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
      configNode *cnode=(configNode *)(*siter);
      checkHatswitch(nr,cnode);
    }
  }
  else{
    cout << "unknown axis " << name << endl;
    return;
  }

}

/* *********************************************************** */

void GameVegaConfig::checkHatswitch(int nr,configNode *node){
  if(node->Name()!="hatswitch"){
    cout << "not a hatswitch node " << endl;
    return;
  }

  string strval=node->attr_value("value");

  float val=atof(strval.c_str());

  if(val>1.0 || val<-1.0){
    cout << "only hatswitch values from -1.0 to 1.0 allowed" << endl;
    return;
  }

  hatswitch[nr][hs_value_index]=val;

  cout << "setting hatswitch nr " << nr << " " << hs_value_index << " = " << val << endl;

  hs_value_index++;
}

/* *********************************************************** */

void GameVegaConfig::checkBind(configNode *node){
  if(node->Name()!="bind"){
    cout << "not a bind node " << endl;
    return;
  }
  std::string tmp=node->attr_value("modifier");
  int modifier=getModifier(tmp.c_str());
  
  string cmdstr=node->attr_value("command");
  string player_bound=node->attr_value("player");
  if (player_bound.empty())
    player_bound="0";
  KBHandler handler=command_map[cmdstr];
  
  if(handler==NULL){
    cout << "No such command: " << cmdstr << endl;
    return;
  }
  string player_str=node->attr_value("player");
  string joy_str=node->attr_value("joystick");
  string mouse_str=node->attr_value("mouse");
  string keystr=node->attr_value("key");
  string buttonstr=node->attr_value("button");
  string hat_str=node->attr_value("hatswitch");
  string dighswitch=node->attr_value("digital-hatswitch");
  string direction=node->attr_value("direction");
  if (!player_str.empty()) {
    if (!joy_str.empty()) {
      int jn = atoi(joy_str.c_str());
      if (jn<MAX_JOYSTICKS) {
	joystick[jn]->player=atoi(player_str.c_str());
      }
    }else if (!mouse_str.empty()) {
      joystick[MOUSE_JOYSTICK]->player=atoi(player_str.c_str());
    }
  }

  if(!keystr.empty()){
    // normal keyboard key
      // now map the command to a callback function and bind it
    if(keystr.length()==1){
      BindKey(keystr[0],modifier,XMLSupport::parse_int(player_bound), handler);
    }
    else{
      int glut_key=key_map[keystr];
      if(glut_key==0){
	cout << "No such special key: " << keystr << endl;
	return;
      }
      BindKey(glut_key,modifier,XMLSupport::parse_int(player_bound),handler);
    }

    //    cout << "bound key " << keystr << " to " << cmdstr << endl;

  }
  else if(!buttonstr.empty()){
    // maps a joystick button or analogue hatswitch button
      int button_nr=atoi(buttonstr.c_str());

      if(joy_str.empty()&&mouse_str.empty()){
	// it has to be the analogue hatswitch
	if(hat_str.empty()){
	  cout << "you got to give a analogue hatswitch number" << endl ;
	  return;
	}

	int hatswitch_nr=atoi(hat_str.c_str());

	BindHatswitchKey(hatswitch_nr,button_nr,handler);
	
	//	cout << "Bound hatswitch nr " << hatswitch_nr << " button: " << button_nr << " to " << cmdstr << endl;
      }
      else{
	// joystick button
	int joystick_nr;
	if (mouse_str.empty())
	  joystick_nr=atoi(joy_str.c_str());
	else
	  joystick_nr=(MOUSE_JOYSTICK);
	if(joystick[joystick_nr]->isAvailable()){
	  // now map the command to a callback function and bind it

	  // yet to check for correct buttons/joy-nr


	  BindJoyKey(joystick_nr,button_nr,handler);

	//cout << "Bound joy= " << joystick_nr << " button= " << button_nr << "to " << cmdstr << endl;
	}
	else{
          static bool first=true;
          if (first) {
            cout << "\nrefusing to bind command to joystick (joy-nr too high)" << endl;
            first=false;
          }
	}
      }
    }
    else if(!(dighswitch.empty() || direction.empty() || (mouse_str.empty()&&joy_str.empty()))){
      // digital hatswitch or ...

      if(dighswitch.empty() || direction.empty() || (mouse_str.empty()&&joy_str.empty())){
	cout << "you have to specify joystick,digital-hatswitch,direction" << endl;
	return;
      }

      int hsw_nr=atoi(dighswitch.c_str());

      int joy_nr;
      if (mouse_str.empty()) {
	joy_nr=atoi(joy_str.c_str());
      } else {
	joy_nr=MOUSE_JOYSTICK;
      }
      if(!(joystick[joy_nr]->isAvailable() && hsw_nr<joystick[joy_nr]->nr_of_hats)){
	cout << "refusing to bind digital hatswitch: no such hatswitch" << endl;
	return;
      }
      int dir_index;

      if(direction=="center"){
	dir_index=VS_HAT_CENTERED;
      }
      else if(direction=="up"){
	dir_index=VS_HAT_UP;
      }
      else if(direction=="right"){
	dir_index=VS_HAT_RIGHT;
      }      else if(direction=="left"){
	dir_index=VS_HAT_LEFT;
      }      else if(direction=="down"){
	dir_index=VS_HAT_DOWN;
      }      else if(direction=="rightup"){
	dir_index=VS_HAT_RIGHTUP;
      }      else if(direction=="rightdown"){
	dir_index=VS_HAT_RIGHTDOWN;
      }      else if(direction=="leftup"){
	dir_index=VS_HAT_LEFTUP;
      }      else if(direction=="leftdown"){
	dir_index=VS_HAT_LEFTDOWN;
      }
      else{
	cout << "no valid direction string" << endl;
	return;
      }

      BindDigitalHatswitchKey(joy_nr,hsw_nr,dir_index,handler);

      cout << "Bound joy " << joy_nr << " hatswitch " << hsw_nr << " dir_index " << dir_index << " to command " << cmdstr << endl;

    }
#if 1
  else{
    cout << "no correct key or joystick binding" << endl;
    return;
  }
#endif
}

/* *********************************************************** */

void GameVegaConfig::bindKeys(){
  doBindings(bindings);
}

/* *********************************************************** */
