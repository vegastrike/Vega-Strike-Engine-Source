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

VegaConfig::VegaConfig(char *configfile){

  configNodeFactory *domf = new configNodeFactory();

  configNode *top=(configNode *)domf->LoadXML(configfile);

  if(top==NULL){
    cout << "Panic exit - no configuration" << endl;
    exit(0);
  }
  //top->walk(0);
  
  initCommandMap();
  initKeyMap();

  variables=NULL;
  colors=NULL;

  // set hatswitches to off
  for(int h=0;h<MAX_HATSWITCHES;h++){
    hatswitch_margin[h]=2.0;
    for(int v=0;v<MAX_VALUES;v++){
      hatswitch[h][v]=2.0;
    }
  }

  for(int i=0;i<=3;i++){
    axis_axis[i]=-1;
    axis_joy[i]=-1;
  }

  checkConfig(top);
}

/*
for i in `cat cmap` ; do echo "  command_map[\""$i"\"]=FlyByKeyboard::"$i ";" ; done
 */

/* *********************************************************** */

#if 1
const float volinc = 1;
const float dopinc = .1;

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

void VegaConfig::initKeyMap(){
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
  key_map["backspace"]=8;
  key_map["capslock"]=WSK_CAPSLOCK;
  key_map["cursor-delete"]=127;
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
extern void  SuicideKey (int i, KBSTATE a);
extern void pause_key (int i, KBSTATE a);
extern void dec_time_compression(int i, KBSTATE a);
extern void reset_time_compression(int i, KBSTATE a);
extern void MapKey(int i, KBSTATE a);
extern void VolUp(int i, KBSTATE a);
extern void VolDown(int i, KBSTATE a);
  using namespace CockpitKeys;

void VegaConfig::initCommandMap(){
#if 1
  //  I don't knwo why this gives linker errors!
  command_map["NoPositionalKey"]=mute;
  command_map["DopplerInc"]=incdop;
  command_map["DopplerDec"]=decdop;
  command_map["VolumeInc"]=VolUp;
  command_map["VolumeDec"]=VolDown;
#endif
  command_map["SwitchControl"]=Cockpit::SwitchControl;
  command_map["Respawn"]=Cockpit::Respawn;
  command_map["TurretControl"]=Cockpit::TurretControl;

  command_map["TimeInc"]=inc_time_compression;
  command_map["TimeDec"]=dec_time_compression;
  command_map["TimeReset"]=reset_time_compression;
  // mapping from command string to keyboard handler
  command_map["SheltonKey"]=FlyByKeyboard::SheltonKey ;
  command_map["MatchSpeedKey"]=FlyByKeyboard::MatchSpeedKey ;
  command_map["PauseKey"]=pause_key;
  command_map["JumpKey"]=FlyByKeyboard::JumpKey;
  command_map["AutoKey"]=FlyByKeyboard::AutoKey;
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
  command_map["CommAttackTarget"]=FireKeyboard::AttackTarget;
  command_map["CommHelpMeOutCrit"]=FireKeyboard::HelpMeOutCrit;
  command_map["CommHelpMeOutFaction"]=FireKeyboard::HelpMeOutFaction;
  command_map["JoinFlightgroup"]=FireKeyboard::JoinFg;
  command_map["CommAttackTarget"]=FireKeyboard::AttackTarget;
  command_map["CommHelpMeOut"]=FireKeyboard::HelpMeOut;
  command_map["CommFormUp"]=FireKeyboard::FormUp;
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
  command_map["NearestTargetKey"]=FireKeyboard::NearestTargetKey ;
  command_map["ThreatTargetKey"]=FireKeyboard::ThreatTargetKey ;
  command_map["TurretTargetKey"]=FireKeyboard::TargetTurretKey ;
  command_map["TurretPickTargetKey"]=FireKeyboard::PickTargetTurretKey ;
  command_map["TurretNearestTargetKey"]=FireKeyboard::NearestTargetTurretKey ;
  command_map["TurretThreatTargetKey"]=FireKeyboard::ThreatTargetTurretKey ;
  command_map["WeapSelKey"]=FireKeyboard::WeapSelKey ;
  command_map["MisSelKey"]=FireKeyboard::MisSelKey ;
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

bool VegaConfig::checkConfig(configNode *node){
  if(node->Name()!="vegaconfig"){
    cout << "this is no Vegastrike config file" << endl;
    return false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);

    if(cnode->Name()=="variables"){
      doVariables(cnode);
    }
    else if(cnode->Name()=="colors"){
      doColors(cnode);
    }
    else if(cnode->Name()=="bindings"){
      bindings=cnode; // delay the bindings until keyboard/joystick is initialized
      //doBindings(cnode);
    }
    else{
      cout << "Unknown tag: " << cnode->Name() << endl;
    }
  }
  return true;
}

/* *********************************************************** */

void VegaConfig::doVariables(configNode *node){
  if(variables!=NULL){
    cout << "only one variable section allowed" << endl;
    return;
  }
  variables=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkSection(cnode,SECTION_VAR);
  }
}

/* *********************************************************** */

void VegaConfig::doSection(configNode *node, enum section_t section_type){
  string section=node->attr_value("name");
  if(section.empty()){
    cout << "no name given for section" << endl;
  }
  
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if(section_type==SECTION_COLOR){
      checkColor(cnode);
    }
    else if(section_type==SECTION_VAR){
      if(cnode->Name()=="var"){
	doVar(cnode);
      }
      else if(cnode->Name()=="section"){
	doSection(cnode,section_type);
      }
      else{
	cout << "neither a variable nor a section" << endl;
      }
    }
  }
}

/* *********************************************************** */

void VegaConfig::checkSection(configNode *node, enum section_t section_type){
    if(node->Name()!="section"){
      cout << "not a section" << endl;
      node->printNode(cout,0,1);

      return;
  }

    doSection(node,section_type);
}

/* *********************************************************** */

void VegaConfig::doVar(configNode *node){
  string name=node->attr_value("name");
  string value=node->attr_value("value");

  //  cout << "checking var " << name << " value " << value << endl;
  if(name.empty() || value.empty()){
    cout << "no name or value given for variable" << endl;
  }
}

/* *********************************************************** */

void VegaConfig::checkVar(configNode *node){
    if(node->Name()!="var"){
      cout << "not a variable" << endl;
    return;
  }

    doVar(node);
}

/* *********************************************************** */

bool VegaConfig::checkColor(configNode *node){
  if(node->Name()!="color"){
    cout << "no color definition" << endl;
    return false;
  }

  if(node->attr_value("name").empty()){
    cout << "no color name given" << endl;
    return false;
  }

  vColor *color;


  if(node->attr_value("ref").empty()){
    string r=node->attr_value("r");
    string g=node->attr_value("g");
    string b=node->attr_value("b");
    string a=node->attr_value("a");
    if(r.empty() || g.empty() || b.empty() || a.empty()){
      cout << "neither name nor r,g,b given for color " << node->Name() << endl;
      return false;
    }
    float rf=atof(r.c_str());
    float gf=atof(g.c_str());
    float bf=atof(b.c_str());
    float af=atof(a.c_str());

    color=new vColor;

    color->r=rf;
    color->g=gf;
    color->b=bf;
    color->a=af;
  }
  else{
    float refcol[4];

    string ref_section=node->attr_value("section");
    if(ref_section.empty()){
      cout << "you have to give a referenced section when referencing colors" << endl;
      ref_section="default";
    }

    //    cout << "refsec: " << ref_section << " ref " << node->attr_value("ref") << endl;
    getColor(ref_section,node->attr_value("ref"),refcol);

    color=new vColor;

    color->r=refcol[0];
    color->g=refcol[1];
    color->b=refcol[2];
    color->a=refcol[3];

  }

  color->name=node->attr_value("name");

  node->color=color;
  //  colors.push_back(color);

  return true;
}

/* *********************************************************** */

void VegaConfig::doColors(configNode *node){
  if(colors!=NULL){
    cout << "only one variable section allowed" << endl;
    return;
  }
  colors=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkSection(cnode,SECTION_COLOR);
  }

#if 0
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkColor(cnode);
  }
#endif
}

/* *********************************************************** */

void VegaConfig::doBindings(configNode *node){
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

void VegaConfig::doAxis(configNode *node){

  string name=node->attr_value("name");
  string joystick=node->attr_value("joystick");
  string axis=node->attr_value("axis");
  string invertstr=node->attr_value("inverse");

  if(name.empty() || joystick.empty() || axis.empty()){
    cout << "no correct axis desription given " << endl;
    return;
  }

  int joy_nr=atoi(joystick.c_str());
  int axis_nr=atoi(axis.c_str());

  // no checks for correct number yet 

  bool inverse=false;
  if(!invertstr.empty()){
    inverse=XMLSupport::parse_bool(invertstr);
  }

  if(name=="x"){
    axis_axis[0]=axis_nr;
    axis_joy[0]=joy_nr;
    axis_inverse[0]=inverse;
  }
  else if(name=="y"){
    axis_axis[1]=axis_nr;
    axis_joy[1]=joy_nr;
    axis_inverse[1]=inverse;
  }
  else if(name=="z"){
    axis_axis[2]=axis_nr;
    axis_joy[2]=joy_nr;
    axis_inverse[2]=inverse;
  }
  else if(name=="throttle"){
    axis_axis[3]=axis_nr;
    axis_joy[3]=joy_nr;
    axis_inverse[3]=inverse;
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

void VegaConfig::checkHatswitch(int nr,configNode *node){
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

void VegaConfig::checkBind(configNode *node){
  if(node->Name()!="bind"){
    cout << "not a bind node " << endl;
    return;
  }

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
      BindKey(keystr[0],XMLSupport::parse_int(player_bound), handler);
    }
    else{
      int glut_key=key_map[keystr];
      if(glut_key==0){
	cout << "No such special key: " << keystr << endl;
	return;
      }
      BindKey(glut_key,XMLSupport::parse_int(player_bound),handler);
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
	  cout << "refusing to bind command to joystick (joy-nr too high)" << endl;
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

string VegaConfig::getVariable(string section,string subsection,string name,string defaultvalue){
  configNode *secnode=findSection(section,variables);
  if(secnode!=NULL){
    configNode *subnode=findSection(subsection,secnode);
    if(subnode!=NULL){
      configNode *entrynode=findEntry(name,subnode);
      if(entrynode!=NULL){
	return entrynode->attr_value("value");
      }
    }
  }

  return defaultvalue;
}

/* *********************************************************** */

string VegaConfig::getVariable(string section,string name,string defaultval){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= variables->subnodes.begin() ; siter!=variables->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return getVariable(cnode,name,defaultval);
    }
  }

  cout << "WARNING: no section named " << section << endl;

  return defaultval;
}

/* *********************************************************** */

string VegaConfig::getVariable(configNode *section,string name,string defaultval){
    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= section->subnodes.begin() ; siter!=section->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if((cnode)->attr_value("name")==name){
      return (cnode)->attr_value("value");
    }
  }

  cout << "WARNING: no var named " << name << " in section " << section->attr_value("name") << " using default: " << defaultval << endl;

  return defaultval; 
}

/* *********************************************************** */

void VegaConfig::gethColor(string section, string name, float color[4],int hexcolor){
  color[3]=((float)(hexcolor & 0xff))/256.0;
  color[2]=((float)((hexcolor & 0xff00)>>8))/256.0;
  color[1]=((float)((hexcolor & 0xff0000)>>16))/256.0;
  color[0]=((float)((hexcolor & 0xff000000)>>24))/256.0;
  
  getColor(section,name,color,true);
}

/* *********************************************************** */

void VegaConfig::getColor(string section, string name, float color[4],bool have_color){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= colors->subnodes.begin() ; siter!=colors->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //          cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      getColor(cnode,name,color,have_color);
      return;
    }
  }

  cout << "WARNING: no section named " << section << endl;

  return;
  
}

/* *********************************************************** */

void VegaConfig::getColor(configNode *node,string name,float color[4],bool have_color){
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    //            cout << "scanning color " << (cnode)->attr_value("name") << endl;
    if((cnode)->attr_value("name")==name){
      color[0]=(cnode)->color->r;
      color[1]=(cnode)->color->g;
      color[2]=(cnode)->color->b;
      color[3]=(cnode)->color->a;
      return;
    }
  }

  if(have_color==false){
    color[0]=1.0;
    color[1]=1.0;
    color[2]=1.0;
    color[3]=1.0;

    cout << "WARNING: color " << name << " not defined, using default (white)" << endl;
  }
  else{
    cout << "WARNING: color " << name << " not defined, using default (hexcolor)" << endl;
  }

}

/* *********************************************************** */

void VegaConfig::bindKeys(){
  doBindings(bindings);
}

/* *********************************************************** */

configNode *VegaConfig::findEntry(string name,configNode *startnode){
  return findSection(name,startnode);
}

/* *********************************************************** */

configNode *VegaConfig::findSection(string section,configNode *startnode){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= startnode->subnodes.begin() ; siter!=startnode->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return cnode;
    }
  }

  cout << "WARNING: no section/variable/color named " << section << endl;

  return NULL;
 
  
}

/* *********************************************************** */

void VegaConfig::setVariable(configNode *entry,string value){
      entry->set_attribute("value",value);
}

/* *********************************************************** */

bool VegaConfig::setVariable(string section,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *varnode=findEntry(name,sectionnode);

    if(varnode!=NULL){
      // now set the thing
      setVariable(varnode,value);
      return true;
    }
  }
  return false;
}


bool VegaConfig::setVariable(string section,string subsection,string name,string value){

  configNode *sectionnode=findSection(section,variables);

  if(sectionnode!=NULL){

    configNode *subnode=findSection(name,sectionnode);



	if(subnode!=NULL) {

		configNode *varnode=findEntry(name,subnode);

		if(varnode!=NULL){

			// now set the thing

			setVariable(varnode,value);

			return true;

		}

	}

  }

  return false;

}

