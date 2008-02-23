#include <stdlib.h>
#ifndef _WIN32
#include <fenv.h>
#endif
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "lin_time.h"
#include "cmd/unit.h"
#include "cmd/unit_factory.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "in.h"
#include "gfx/mesh.h"
#include "gfx/sprite.h"
#include "physics.h"
//#include "cmd_hud.h"
#include "gfxlib.h"
#include "cmd/bolt.h"
#include "gfx/loc_select.h"
#include <string>
#include "cmd/ai/input_dfa.h"
#include "cmd/collection.h"
#include "star_system.h"
#include "cmd/planet.h"
#include "gfx/sphere.h"
#include "gfx/coord_select.h"
#include "cmd/building.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/navigation.h"
#include "cmd/beam.h"
#include  "gfx/halo.h"
#include "gfx/matrix.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/script.h"
#include "gfx/cockpit.h"
#include "gfx/aux_texture.h"
#include "gfx/background.h"
#include "cmd/music.h"
#include "main_loop.h"
#include "cmd/music.h"
#include "audiolib.h"
#include "cmd/nebula.h"
#include "vsfilesystem.h"
#include "cmd/script/mission.h"
#include "xml_support.h"
#include "config_xml.h"
#include "cmd/ai/missionscript.h"
#include "cmd/enhancement.h"
#include "cmd/cont_terrain.h"
#include "cmd/script/flightgroup.h"
#include "force_feedback.h"
#include "universe_util.h"
#include "networking/netclient.h"
#include "save_util.h"
#include "in_kb_data.h"
#include "vs_random.h"
using namespace std;
extern std::string global_username;
#define KEYDOWN(name,key) (name[key] & 0x80)

static Texture *tmpcockpittexture;
Unit **fighters;
Unit *carrier=NULL;
Unit *fighter=NULL;
Unit *fighter2=NULL;
Unit *midway = NULL;
//Cockpit *cockpit;
void SuicideKey(const KBData&,KBSTATE newState);
//  void Respawn(const KBData&,KBSTATE newState);
GFXBOOL capture;
GFXBOOL quit = GFXFALSE;
bool _Slew = true;
bool QuitAllow=false;
extern bool cleanexit;
//  extern int allexcept;
#ifndef _WIN32
int allexcept=FE_DIVBYZERO;//|FE_INVALID;//|FE_OVERFLOW|FE_UNDERFLOW;
#else
int allexcept=0;
#endif
int shiftup(int);
string getUnitNameAndFgNoBase(Unit * target);
const Unit * loadUnitByCache(std::string name,int faction);
ContinuousTerrain * myterrain;
int numf = 0;
CoordinateSelect *locSel=NULL;
//Background * bg = NULL;
SphereMesh *bg2=NULL;
ClickList *shipList =NULL;
//const float timek = .001;

void VolUp(const KBData&,KBSTATE newState) {
   if(newState==PRESS){
     float gain=AUDGetListenerGain();
     if (gain<1) {
       gain+=.0625;
       if (gain>1) gain=1;
       AUDListenerGain (gain);
       Music::ChangeVolume(0);
     }
   }
 }

/*11-7-98
 *Cool shit happened when a position rotation matrix from a unit was used for the drawing of the background... not very useful though
 */

/*
class Orbit:public AI{
	float count;
public:
	Orbit(Unit *parent1):AI(parent1){count = 0;};
	Orbit():AI()
	{
		count = 0;
	};
	AI *Execute()
	{
		//parent->Position(); // query the position
	  //parent->ZSlide(0.100F);
	  //parent->Pitch(PI/180);
		count ++;
		if(30 == count)
		{
		  //			Unit *parent = this->parent;
			//delete this;
			//return new Line(parent);
			return this;
		}
		else
			return this;
	}
};
*/

void VolDown(const KBData&,KBSTATE newState) {
   if(newState==PRESS){
     float gain=AUDGetListenerGain();
     if (gain>0) {       
       gain-=.03125;
       if (gain<0) gain=0;
       AUDListenerGain (gain);
       Music::ChangeVolume(0);
     }
   }
 }

static void SwitchVDUTo(VDU::VDU_MODE v) {
    int i;
    static int whichvdu=1; 
    for (int j=0;j<3;++j) {
      if (v!=_Universe->AccessCockpit()->getVDUMode(whichvdu)||(v!=VDU::VIEW&&v!=VDU::WEAPON)){
        whichvdu+=1;
        whichvdu%=2;
      }      
      int curmode = _Universe->AccessCockpit()->getVDUMode(whichvdu);
      
      if (v==_Universe->AccessCockpit()->getVDUMode(whichvdu)) {
        if (v==VDU::VIEW)
          _Universe->AccessCockpit()->VDUSwitch(whichvdu);//get different view mode
        return;
      }
      for (i=0;i<32;++i) {
        _Universe->AccessCockpit()->VDUSwitch(whichvdu);
        if (v==_Universe->AccessCockpit()->getVDUMode(whichvdu)) {
          return;
        }
      }
      for (i=0;i<32;++i) {
        _Universe->AccessCockpit()->VDUSwitch(whichvdu);
        if (curmode==_Universe->AccessCockpit()->getVDUMode(whichvdu))       
          break;
      }
    }
  }

void ExamineWhenTargetKey() {
  static bool reallySwitch=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","switchToTargetModeOnKey","true"));
  if (reallySwitch) {
    int view=0;
    int examine=0;    
    for (;view<2;++view) {
      if (_Universe->AccessCockpit()->getVDUMode(view)==VDU::VIEW)
        break;
    }
    for (;examine<2;++examine) {
      if (_Universe->AccessCockpit()->getVDUMode(examine)==VDU::TARGET)
        break;
    }
    if ((examine==2) && (view==2)){
      SwitchVDUTo(VDU::TARGET);
    }
   
  }
}

namespace CockpitKeys {
  unsigned int textmessager=0;
  static bool waszero=false;

  void TextMessageCallback(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    GameCockpit *gcp=static_cast<GameCockpit*>(_Universe->AccessCockpit(textmessager));
    gcp->editingTextMessage=true;
    if ((release&&(waszero||ch==WSK_KP_ENTER||ch==WSK_ESCAPE))||(release==false&&(ch==']'||ch=='['))){
      waszero=false;
      gcp->editingTextMessage=false;
      RestoreKB();

    }
    if (release||(ch==']'||ch=='['))return;
    
    unsigned int code=((WSK_MOD_LSHIFT==(mod&WSK_MOD_LSHIFT))||(WSK_MOD_RSHIFT==(mod&WSK_MOD_RSHIFT)))?shiftup(ch):ch;
    if (textmessager<_Universe->numPlayers()) {
      if (ch==WSK_BACKSPACE||ch==WSK_DELETE) {
        gcp->textMessage= gcp->textMessage.substr(0,gcp->textMessage.length()-1);
      } else if (ch==WSK_RETURN||ch==WSK_KP_ENTER) {
        if (gcp->textMessage.length()!=0) {
          std::string name=gcp->savegame->GetCallsign();
          if (Network!=NULL) {
            
            Unit*par=gcp->GetParent();
            if (0&&par) {
              name=getUnitNameAndFgNoBase(par);
            }
            Network[textmessager].textMessage(gcp->textMessage);
          } else if (gcp->textMessage[0]=='/') {
			  string cmd;
			  string args;
			  std::string::size_type space = gcp->textMessage.find(' ');
			  if (space) {
				  cmd = gcp->textMessage.substr(1, space-1);
				  args = gcp->textMessage.substr(space+1);
			  } else {
				  cmd = gcp->textMessage.substr(1);
			  }
			  // Send custom message to itself.
			  UniverseUtil::receivedCustom(textmessager, true, cmd, args, string());
          }
          /*
            static std::string anglestring=" >";
            //unit util checks for null, it's ok
            UniverseUtil::IOmessage(0,"game","all",name+"> "+gcp->textMessage);
          *///server seems to send it again
          waszero=false;
        }else waszero=true;
        gcp->textMessage="";
      }else if (code!=0&&code<=127) {
        char newstr[2]={(char)code,0};
        gcp->textMessage+=newstr;      
        
      }
    }else {
      RestoreKB();
      gcp->editingTextMessage=false;
    }
  }

  void TextMessageKey(const KBData&,KBSTATE newState) {
    if(newState==PRESS){
      winsys_set_keyboard_func(TextMessageCallback);
      textmessager=_Universe->CurrentCockpit();
    }
  }
      void QuitNow () {
    {
	  static bool writeSaveOnQuit = XMLSupport::parse_bool(vs_config->getVariable("general","write_savegame_on_exit","true"));
      cleanexit=true;
	  if(Network==NULL && writeSaveOnQuit)
		_Universe->WriteSaveGame(true);//gotta do important stuff first
      for (unsigned int i=0;i<active_missions.size();i++) {
	if (active_missions[i]) {
	  active_missions[i]->DirectorEnd();
	}
      }
	  if( forcefeedback)
		delete forcefeedback;
      VSExit(0);
    }
    
  }
 
   void SkipMusicTrack(const KBData&,KBSTATE newState) {
   if(newState==PRESS){
     printf("skipping\n");
    muzak->Skip();
   }
 }

 void PitchDown(const KBData&,KBSTATE newState) {
      static float panspeed=XMLSupport::parse_float(vs_config->getVariable("graphics","camera_pan_speed","0.0001"));
	static Vector Q;
	static Vector R;
	for (int i=0;i<NUM_CAM;i++) {
	if(newState==PRESS) {
	  if (QuitAllow) {
	    QuitNow();
	  }
		Q = _Universe->AccessCockpit()->AccessCamera(i)->Q;
		R = _Universe->AccessCockpit()->AccessCamera(i)->R;
		_Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(-Q, R, panspeed);
		//a =1;
	}
	if (_Slew&&newState==RELEASE) {
	  _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0,0,0));
	}//a=0;
	}	
}

 void PitchUp(const KBData&,KBSTATE newState) {
      static float panspeed=XMLSupport::parse_float(vs_config->getVariable("graphics","camera_pan_speed","0.0001"));
	static Vector Q;
	static Vector R;
	for (int i=0;i<NUM_CAM;i++) {
	if(newState==PRESS) {

		Q = _Universe->AccessCockpit()->AccessCamera(i)->Q;
		R = _Universe->AccessCockpit()->AccessCamera(i)->R;
		_Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(Q, R, panspeed);
		
	}
	if (_Slew&&newState==RELEASE) {
	  _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0,0,0));
	}
	}}

  void YawLeft(const KBData&,KBSTATE newState) {
      static float panspeed=XMLSupport::parse_float(vs_config->getVariable("graphics","camera_pan_speed","0.0001"));
	static Vector P;
	static Vector R;
	for (int i=0;i<NUM_CAM;i++) {
	if(newState==PRESS) {

		P = _Universe->AccessCockpit()->AccessCamera(i)->P;
		R = _Universe->AccessCockpit()->AccessCamera(i)->R;
		_Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(-P, R, panspeed);
		
	}
	if (_Slew&&newState==RELEASE) {
	  _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0,0,0));
	}
	}
}

  void YawRight(const KBData&,KBSTATE newState) {
	for (int i=0;i<NUM_CAM;i++) {
      static float panspeed=XMLSupport::parse_float(vs_config->getVariable("graphics","camera_pan_speed","0.0001"));
	   static Vector P;
	   static Vector R;
	   if(newState==PRESS) {
		   P = _Universe->AccessCockpit()->AccessCamera(i)->P;
		   R = _Universe->AccessCockpit()->AccessCamera(i)->R;
		   _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(P, R, panspeed);
	   
	   }
	   if (_Slew&&newState==RELEASE) {
	   _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0,0,0));
	   }
	}
}

  void Quit(const KBData&,KBSTATE newState) {
	if(newState==PRESS) {
	  if (QuitAllow) {
            /*
	    UniverseUtil::IOmessage(0,"game","all","#ffff00Quit Mode cancelled, Camera Keys restored to former function.");
	    UniverseUtil::IOmessage(0,"game","all","#00ff00Press Esc and then q to Quit at a later point.");
            */
	  }else {
            /*
	    UniverseUtil::IOmessage(0,"game","all","#ff0000You have pressed the quit key.");
	    UniverseUtil::IOmessage(0,"game","all","#00ffffIf you hit q, the game will quit.");
	    UniverseUtil::IOmessage(0,"game","all","#00ff55Pressing ESC again will cancel quit confirm mode.");
	    */
	  }
	  QuitAllow = !QuitAllow;
	}

  }
bool cockpitfront=true;
  void Inside(const KBData&,KBSTATE newState) {
    {
      static bool back= XMLSupport::parse_bool (vs_config->getVariable ("graphics","background","true"));
      if (_Universe->activeStarSystem()&&_Universe->activeStarSystem()->getBackground())
        _Universe->activeStarSystem()->getBackground()->EnableBG(back);
    }
  static int tmp=(XMLSupport::parse_bool (vs_config->getVariable ("graphics","cockpit","true"))?1:0);
  static bool switch_to_disabled=XMLSupport::parse_bool(vs_config->getVariable("graphics","disabled_cockpit_allowed","true"));
  if(newState==PRESS&&(_Universe->AccessCockpit()->GetView()==CP_FRONT)&&switch_to_disabled&&switch_to_disabled) {
      YawLeft (KBData(),RELEASE);
      YawRight (KBData(),RELEASE);
      PitchUp(KBData(),RELEASE);
      PitchDown (KBData(),RELEASE);
	  string cockpit="disabled-cockpit.cpt";
	  if (_Universe->AccessCockpit()->GetParent())
		  cockpit=_Universe->AccessCockpit()->GetParent()->getCockpit();

      Unit *u;
      if (  (_Universe->AccessCockpit()->GetParent()!=NULL)
          &&(_Universe->AccessCockpit()->GetParent()->name=="return_to_cockpit")
          &&(_Universe->AccessCockpit()->GetParent()->owner!=NULL)
          &&(u=findUnitInStarsystem(_Universe->AccessCockpit()->GetParent()->owner))  )
		  cockpit=u->getCockpit();


	  _Universe->AccessCockpit()->Init (cockpit.c_str(), ((tmp)&&_Universe->AccessCockpit()->GetParent())==false);	    
    tmp=(tmp+1)%2;
  }
  if(newState==PRESS||newState==DOWN) {
    cockpitfront=true;
    _Universe->AccessCockpit()->SetView (CP_FRONT);
  }
}
  void ZoomOut (const KBData&,KBSTATE newState) {
  if(newState==PRESS||newState==DOWN) 
  _Universe->AccessCockpit()->zoomfactor+=GetElapsedTime()/getTimeCompression();  
}

  static float scrolltime=0;
  void ScrollUp (const KBData&,KBSTATE newState) {
   scrolltime+=GetElapsedTime();
   if(newState==PRESS||(newState==DOWN&&scrolltime>=.5)){
     scrolltime=0;
     /*
     if (_Universe->AccessCockpit()->GetParent()) {
       _Universe->AccessCockpit()->GetParent()->DamageRandSys( _Universe->AccessCockpit()->GetParent()->GetHull()*(rand()/(float)RAND_MAX),Vector(rand()*2.0/(float)RAND_MAX-1.0,rand()*2.0/(float)RAND_MAX-1.0,rand()*2.0/(float)RAND_MAX-1.0),rand()/(float)RAND_MAX,rand()*3.14*2/(float)RAND_MAX);
     }
     */
     //feenableexcept(allexcept);
     printf ("Enabling exceptions %d\n",allexcept);
     _Universe->AccessCockpit()->ScrollAllVDU (-1);

   }    
  }
  void ScrollDown (const KBData&,KBSTATE newState) {
   scrolltime+=GetElapsedTime();
   if(newState==PRESS||(newState==DOWN&&scrolltime>=.5)){
     scrolltime=0;
     /*
     Unit * thus=_Universe->AccessCockpit()->GetParent();
     if (thus) {
       const char * DamagedCategory="upgrades/Damaged/";
       int which = rand()%thus->numCargo();
       static std::string Restricted_items=vs_config->getVariable("physics","indestructable_cargo_items","");
       if (thus->GetCargo(which).category.find("upgrades/")==0&& thus->GetCargo(which).category.find(DamagedCategory)!=0 &&thus->GetCargo(which).content.find("mult_")!=0&&Restricted_items.find(thus->GetCargo(which).content)==string::npos) {//why not downgrade _add GetCargo(which).content.find("add_")!=0&&
         int lenupgrades = strlen("upgrades/");
         thus->GetCargo(which).category = string(DamagedCategory)+thus->GetCargo(which).category.substr(lenupgrades);
         static bool NotActuallyDowngrade=XMLSupport::parse_bool(vs_config->getVariable("physics","separate_system_flakiness_component","false"));
         if (!NotActuallyDowngrade) {
           const Unit * downgrade=loadUnitByCache(thus->GetCargo(which).content,FactionUtil::GetFactionIndex("upgrades"));
           if (downgrade) {
             if (0==downgrade->GetNumMounts()&&downgrade->SubUnits.empty()) {
               double percentage=0;
               thus->Downgrade(downgrade,0,0,percentage,NULL);
             }
           }
         }
       }
     }
     */
     //feenableexcept(0);
     printf("Disabling exceptions\n");
     _Universe->AccessCockpit()->ScrollAllVDU (1);


   }    

  }
  void ZoomIn (const KBData&,KBSTATE newState) {
  if(newState==PRESS||newState==DOWN) 
  _Universe->AccessCockpit()->zoomfactor-=GetElapsedTime()/getTimeCompression();  
}

  void InsideLeft(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
      YawLeft (std::string(),RELEASE);
      YawRight (std::string(),RELEASE);
      PitchUp(std::string(),RELEASE);
      PitchDown (std::string(),RELEASE);

	  cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_LEFT);
	}
}
  void InsideRight(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_RIGHT);
	}
}
  void PanTarget(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_PANTARGET);
	}
  }
  void ViewTarget(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
      YawLeft (std::string(),RELEASE);
      YawRight (std::string(),RELEASE);
      PitchUp(std::string(),RELEASE);
      PitchDown (std::string(),RELEASE);

	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_VIEWTARGET);
	}
  }
  void OutsideTarget(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
      YawLeft (std::string(),RELEASE);
      YawRight (std::string(),RELEASE);
      PitchUp(std::string(),RELEASE);
      PitchDown (std::string(),RELEASE);

	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_TARGET);
	}
  }


  void InsideBack(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
      YawLeft (std::string(),RELEASE);
      YawRight (std::string(),RELEASE);
      PitchUp(std::string(),RELEASE);
      PitchDown (std::string(),RELEASE);

	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_BACK);
	}
}
  void CommModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS) {
      static soundContainer sc;
      if (sc.sound<0) {

        static string str=vs_config->getVariable("cockpitaudio","comm","vdu_c");
        sc.loadsound(str);
      }
      sc.playsound();
      SwitchVDUTo(VDU::COMM);
    }
  }
  void ScanningModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS){
      static soundContainer sc;
      if (sc.sound<0) {
      
        static string str=vs_config->getVariable("cockpitaudio","scanning","vdu_c");
        sc.loadsound(str);
      }
      sc.playsound();      
      SwitchVDUTo(VDU::SCANNING);
    }
  }
  void ObjectiveModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS) {
      static soundContainer sc;
      if (sc.sound<0) {
      
        static string str=vs_config->getVariable("cockpitaudio","objective","vdu_c");
        sc.loadsound(str);
      }
      sc.playsound();      

      SwitchVDUTo(VDU::OBJECTIVES);
    }
  }
  void TargetModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS) {
      static soundContainer sc;
      if (sc.sound<0) {
      
        static string str=vs_config->getVariable("cockpitaudio","examine","vdu_b");
        sc.loadsound(str);
      }
      sc.playsound();      

      SwitchVDUTo(VDU::TARGET);
    }
  }
  void ViewModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS) {
      static soundContainer sc;
      if (sc.sound<0) {
      
        static string str=vs_config->getVariable("cockpitaudio","view","vdu_b");
        sc.loadsound(str);
      }
      sc.playsound();      
      SwitchVDUTo(VDU::VIEW);
    }
  }
  void DamageModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS) {
      static soundContainer sc;
      if (sc.sound<0) {
      
        static string str=vs_config->getVariable("cockpitaudio","repair","vdu_a");
        sc.loadsound(str);
      }
      sc.playsound();      

      SwitchVDUTo(VDU::DAMAGE);
    }
  }
  void ManifestModeVDU(const KBData&,KBSTATE newState){
    if (newState==PRESS) {
      static soundContainer sc;
      if (sc.sound<0) {
      
        static string str=vs_config->getVariable("cockpitaudio","manifest","vdu_a");
        sc.loadsound(str);
      }
      sc.playsound();      

      SwitchVDUTo(VDU::MANIFEST);
    }
  }
  void GunModeVDU(const KBData&s,KBSTATE newState){
    if (newState==PRESS) SwitchVDUTo(VDU::WEAPON);
    FireKeyboard::WeapSelKey(s,newState);
  }
  void ReverseGunModeVDU(const KBData&s,KBSTATE newState){
    if (newState==PRESS) SwitchVDUTo(VDU::WEAPON);
    FireKeyboard::ReverseWeapSelKey(s,newState);
  }
  void MissileModeVDU(const KBData&s,KBSTATE newState){
    if (newState==PRESS) SwitchVDUTo(VDU::WEAPON);
    FireKeyboard::MisSelKey(s,newState);
  }
  void ReverseMissileModeVDU(const KBData&s,KBSTATE newState){
    if (newState==PRESS) SwitchVDUTo(VDU::WEAPON);
    FireKeyboard::ReverseMisSelKey(s,newState);
  }
  void SwitchLVDU(const KBData&,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (0);
	}
  }
  void SwitchRVDU(const KBData&,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (1);
	}
  }
  void SwitchMVDU(const KBData&,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (2);
	}
  }
  void SwitchULVDU(const KBData&,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (3);
	}
  }
  void SwitchURVDU(const KBData&,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (4);
	}
  }
  void SwitchUMVDU(const KBData&,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (5);
	}
  }

  void Behind(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
      YawLeft (std::string(),RELEASE);
      YawRight (std::string(),RELEASE);
      PitchUp(std::string(),RELEASE);
      PitchDown (std::string(),RELEASE);

	  cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_CHASE);
	}
}
  void Pan(const KBData&,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
      YawLeft (std::string(),RELEASE);
      YawRight (std::string(),RELEASE);
      PitchUp(std::string(),RELEASE);
      PitchDown (std::string(),RELEASE);

	  cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_PAN);
	}
}

}

using namespace CockpitKeys;

/*
int oldx =0;
int  oldy=0;
void startselect (KBSTATE k, int x,int y, int delx, int dely, int mod) {
  if (k==PRESS) {
    oldx = x;
    oldy = y;
  }
}

void clickhandler (KBSTATE k, int x, int y, int delx, int dely, int mod) {

  if (k==DOWN) {
    UnitCollection *c = shipList->requestIterator (oldx,oldy,x,y);
    if (c->createIterator()->current()!=NULL)
      VSFileSystem::Fprintf (stderr,"Select Box Hit single target");
    if (c->createIterator()->advance()!=NULL)
      VSFileSystem::Fprintf (stderr,"Select Box Hit Multiple Targets");
  }else {
    oldx = x;
    oldy = y;
  }
  if (k==PRESS) {
    VSFileSystem::Fprintf (stderr,"click?");
    UnitCollection * c = shipList->requestIterator (x,y);
    if (c->createIterator()->current()!=NULL)
      VSFileSystem::Fprintf (stderr,"Hit single target");
    if (c->createIterator()->advance()!=NULL)
      VSFileSystem::Fprintf (stderr,"Hit Multiple Targets");
    VSFileSystem::Fprintf (stderr,"\n");
    
  }
}
*/

void InitializeInput() {

	BindKey(27,0,0, Quit,KBData()); // always have quit on esc
}

void IncrementStartupVariable () {
	if (mission->getVariable ("savegame","").length()==0)
		return;
	int len=getSaveDataLength (0,"436457r1K3574r7uP71m35");
	float var=FLT_MAX;
	if (len<=0) {
		pushSaveData(0,"436457r1K3574r7uP71m35",1);
		var=1;
	} else {
		var=getSaveData(0,"436457r1K3574r7uP71m35",0);
		putSaveData(0,"436457r1K3574r7uP71m35",0,var+1);
	}
	if (var<=XMLSupport::parse_int(vs_config->getVariable("general","times_to_show_help_screen","3"))) {
          GameCockpit::NavScreen(KBData(),PRESS);//HELP FIXME
		
	}
}

void createObjects(std::vector <std::string> &fighter0name, std::vector <StarSystem *> &ssys, std::vector <QVector>& savedloc, vector<vector<std::string> > &savefiles) {
  vector <std::string> fighter0mods;
  vector <int> fighter0indices;
  //  GFXFogMode (FOG_OFF);


  static Vector TerrainScale (XMLSupport::parse_float (vs_config->getVariable ("terrain","xscale","1")),XMLSupport::parse_float (vs_config->getVariable ("terrain","yscale","1")),XMLSupport::parse_float (vs_config->getVariable ("terrain","zscale","1")));
  static float TerrainMass = XMLSupport::parse_float (vs_config->getVariable ("terrain","mass","100"));
  static float TerrainRadius = XMLSupport::parse_float (vs_config->getVariable ("terrain", "radius", "10000"));

  myterrain=NULL;
  std::string stdstr= mission->getVariable("terrain","");
  if (stdstr.length()>0) {
    Terrain * terr = new Terrain (stdstr.c_str(), TerrainScale, TerrainMass, TerrainRadius);
    Matrix tmp;
    ScaleMatrix (tmp,TerrainScale);
    //    tmp.r[0]=TerrainScale.i;tmp[5]=TerrainScale.j;tmp[10]=TerrainScale.k;
    QVector pos;
    mission->GetOrigin (pos,stdstr);
    tmp.p = -pos;
    terr->SetTransformation (tmp);

  }
  stdstr= mission->getVariable("continuousterrain","");
  if (stdstr.length()>0) {
    myterrain=new ContinuousTerrain (stdstr.c_str(),TerrainScale,TerrainMass);
    Matrix tmp;
    Identity (tmp);
    QVector pos;
    mission->GetOrigin (pos,stdstr);
    tmp.p=-pos;
    myterrain->SetTransformation (tmp);
  }
  //  qt = new QuadTree("terrain.xml");
  /****** 
  locSel = new LocationSelect(Vector (0,-2,2),
			      Vector(1,0,-1), 
			      Vector (-1,0,-1));
//GOOD!!
  ****/
  BindKey (1,CoordinateSelect::MouseMoveHandle);

  //int numf=mission->number_of_flightgroups;
  int numf=mission->number_of_ships;

  //  cout << "numships: " << numf << endl;

  fighters = new Unit * [numf];
  int * tmptarget = new int [numf];

  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);

  stdext::hash_map<string,int> targetmap;


  char fightername [1024]="hornet.xunit";
  int a=0;

  vector<Flightgroup *>::const_iterator siter;
  vector<Flightgroup *> fg=mission->flightgroups;
  int squadnum=0;
  for(siter= fg.begin() ; siter!=fg.end() ; siter++)
  {
    Flightgroup *fg=*siter;
    string fg_name=fg->name;
    string fullname=fg->type;// + ".xunit";
    //    int fg_terrain = fg->terrain_nr;
    //    bool isvehicle = (fg->unittype==Flightgroup::VEHICLE);
    strcpy(fightername,fullname.c_str());
    string ainame=fg->ainame;
    float fg_radius=0.0;
	Cockpit * cp = NULL;

    for(int s=0;s < fg->nr_ships;s++)
	{
      if (a >= mission->number_of_ships) {
        a-=22;
        printf("Error: in createObjects: more ships in flightgroups than in total for mission!\n"
            "Variables a=%d, fg-number-of-ships=%d, total nr=%d, fact=%s, fgname=%s\n",
            a, fg->nr_ships, mission->number_of_ships, fg->faction.c_str(), fg->name.c_str());
        break;
      }
      numf++;
      QVector pox (1000+150*a,100*a,100);
      
      pox.i=fg->pos.i+s*fg_radius*3;
      pox.j=fg->pos.j+s*fg_radius*3;
      pox.k=fg->pos.k+s*fg_radius*3;
      //	  cout << "loop pos " << fg_name << " " << pox.i << pox.j << pox.k << " a=" << a << endl;
      
      if (pox.i==pox.j&&pox.j==pox.k&&pox.k==0)
	  {
		  pox.i=rand()*10000./RAND_MAX-5000;
		  pox.j=rand()*10000./RAND_MAX-5000;
		  pox.k=rand()*10000./RAND_MAX-5000;
      }
      
      tmptarget[a]=FactionUtil::GetFactionIndex(fg->faction); // that should not be in xml?
      int fg_terrain=-1;
      //	  cout << "before unit" << endl;
      if (fg_terrain==-1||(fg_terrain==-2&&myterrain==NULL))
	  {
		  string modifications ("");
		  if (s==0&&squadnum<(int)fighter0name.size())
		  {
			  cp = _Universe->AccessCockpit(squadnum);
			  cp->activeStarSystem=ssys[squadnum];
			  fighter0indices.push_back(a);
			  
			  if (fighter0name[squadnum].length()==0)
				fighter0name[squadnum]=string(fightername);
			  else
				strcpy(fightername,fighter0name[squadnum].c_str());

			  if (mission->getVariable ("savegame","").length()>0)
			  {
				if (savedloc[squadnum].i!=FLT_MAX)
				{
					pox = UniverseUtil::SafeEntrancePoint(savedloc[squadnum]);
				}

                                vector<std::string> *dat=&cp->savegame->getMissionStringData("jump_from");
                                
                                if (dat->size()) {
                                  std::string srcsys=(*dat)[0];
                                  Unit * grav;
                                  for (un_iter ui=cp->activeStarSystem->gravitationalUnits().createIterator();
                                       (grav=*ui)!=NULL;
                                       ++ui) {
                                    size_t siz=grav->GetDestinations().size();
                                    for (unsigned int i=0;i<siz;++i) {
                                      if (srcsys==grav->GetDestinations()[i]) {
                                        QVector newpos=grav->LocalPosition()+QVector(vsrandom.uniformExc(-grav->rSize()/4,grav->rSize()/4),
                                                                                     vsrandom.uniformExc(-grav->rSize()/4,grav->rSize()/4),
                                                                                     vsrandom.uniformExc(-grav->rSize()/4,grav->rSize()/4));

                                        if (grav->isUnit()!=PLANETPTR) {
                                          newpos = UniverseUtil::SafeEntrancePoint(newpos);
                                        }
                                        cp->savegame->SetPlayerLocation(newpos);
                                        pox=newpos;
                                      }
                                    }
                                  }
                                  
                                  dat->clear();
                                }

				fighter0mods.push_back(modifications =vs_config->getVariable (string("player")+((squadnum>0)?tostring(squadnum+1):string("")),"callsign","pilot"));
                                if (squadnum==0&&global_username.length()) {
                                  fighter0mods.back()=global_username;
                                  modifications=global_username;
                                }
				fprintf( stderr, "FOUND MODIFICATION = %s FOR PLAYER #%d\n", modifications.c_str(), squadnum);
			  }
			  else
			  {
				  fighter0mods.push_back("");
			  }
		  }
        
		  Cockpit * backupcp = _Universe->AccessCockpit();

		  if (squadnum<(int)fighter0name.size())
		  {
			_Universe->pushActiveStarSystem (_Universe->AccessCockpit(squadnum)->activeStarSystem);
            _Universe->SetActiveCockpit(_Universe->AccessCockpit(squadnum));
		  }
		  // In networking mode we name the ship save with .xml as they are xml files
		  if( Network!=NULL && squadnum<(int)fighter0name.size()) {
//				if (backupcp==NULL) {
					//modifications = modifications+".xml";
					if (Network[squadnum].getCallsign() != modifications) {
						cout<<"Not CREATING A NETWORK PLAYER "<<fightername<<" FOR "<<modifications<<endl;
						break;
					}
					cout<<"CREATING A NETWORK PLAYER : "<<fightername<<endl;
					fighters[a] = UnitFactory::createUnit(fightername, false,tmptarget[a],"",fg,s, &savefiles[squadnum][1]);
					// Set the faction we have in the save file instead of the mission file (that is to be ignored in networking mode)
					fighters[a]->faction = FactionUtil::GetFactionIndex( cp->savegame->GetPlayerFaction());
					fighters[a]->SetNetworkMode();
					fighters[a]->SetSerial(Network[squadnum].serial);
					Network[squadnum].setUnit( fighters[a]);
					cout<<"Creating fighter["<<squadnum<<"] from "<<modifications<<" on Network["<<squadnum<<"] named "<<Network[squadnum].getCallsign()<<endl;
//				}
			}
		  else if (Network!=NULL)
			{
				cout<<"Not CREATING A LOCAL SHIP : "<<fightername<<endl;
				break;
			}
		  else
			{
				cout<<"CREATING A LOCAL SHIP : "<<fightername<<endl;
  				fighters[a] = UnitFactory::createUnit(fightername, false,tmptarget[a],modifications,fg,s);
			}
		  _Universe->activeStarSystem()->AddUnit(fighters[a]);
		  if (s==0&&squadnum<(int)fighter0name.size())
		  {
			  _Universe->AccessCockpit(squadnum)->Init (fighters[a]->getCockpit().c_str());
			  _Universe->AccessCockpit(squadnum)->SetParent(fighters[a],fighter0name[squadnum].c_str(),fighter0mods[squadnum].c_str(),pox);
		  }
        
		  if (squadnum<(int)fighter0name.size())
		  {
			  _Universe->popActiveStarSystem ();
              _Universe->SetActiveCockpit(backupcp);
		  }
	  }
	  else
	  {
		bool isvehicle=false;
		if (fg_terrain==-2)
		{
			fighters[a]= UnitFactory::createBuilding (myterrain,isvehicle,fightername,false,tmptarget[a],string(""),fg);
		}
		else
		{
			if (fg_terrain>=(int)_Universe->activeStarSystem()->numTerrain())
			{
	    
				ContinuousTerrain * t;
				assert (fg_terrain-_Universe->activeStarSystem()->numTerrain()<_Universe->activeStarSystem()->numContTerrain());
				t =_Universe->activeStarSystem()->getContTerrain(fg_terrain-_Universe->activeStarSystem()->numTerrain());
				fighters[a]= UnitFactory::createBuilding (t,isvehicle,fightername,false,tmptarget[a],string(""),fg);
			}
			else 
			{
				Terrain *t=_Universe->activeStarSystem()->getTerrain(fg_terrain);
				fighters[a]= UnitFactory::createBuilding (t,isvehicle,fightername,false,tmptarget[a],string(""),fg);
			}
		}
		_Universe->activeStarSystem()->AddUnit(fighters[a]);
	  }
	  printf ("pox %lf %lf %lf\n",pox.i,pox.j,pox.k);
	  fighters[a]->SetPosAndCumPos (pox);
      fg_radius=fighters[a]->rSize();
      //    fighters[a]->SetAI(new Order());
      // cout << "before ai" << endl;
      if (benchmark>0.0  || (s!=0||squadnum>=(int)fighter0name.size()))
	  {
		  fighters[a]->LoadAIScript(ainame);
		  fighters[a]->SetTurretAI ();
      }
      a++;
    
   } // for nr_ships
   squadnum++;
  } // end of for flightgroups
  


  delete [] tmptarget;
  //muzak = new Music (fighters[0]);
  //muzak->Skip();
  for (int m_i=0; m_i<muzak_count; m_i++)
    muzak[m_i].SetParent(fighters[0]);
  FactionUtil::LoadFactionPlaylists();
  AUDListenerSize (fighters[0]->rSize()*4);
  for (unsigned int cnum=0;cnum<fighter0indices.size();cnum++) {
    if(benchmark==-1){
      fighters[fighter0indices[cnum]]->EnqueueAI(new FlyByJoystick (cnum));
      fighters[fighter0indices[cnum]]->EnqueueAI(new FireKeyboard (cnum,cnum));
    }
    fighters[fighter0indices[cnum]]->SetTurretAI ();
    fighters[fighter0indices[cnum]]->DisableTurretAI ();	
  }

  shipList = _Universe->activeStarSystem()->getClickList();
  locSel = new CoordinateSelect (QVector (0,0,5));
  UpdateTime();
//  _Universe->activeStarSystem()->AddUnit(UnitFactory::createNebula ("mynebula.xml","nebula",false,0,NULL,0));

  mission->DirectorInitgame();
  IncrementStartupVariable();
}

void AddUnitToSystem (const SavedUnits *su) {
  Unit * un=NULL;
  switch (su->type) {
  case ENHANCEMENTPTR:
    un = UnitFactory::createEnhancement (su->filename.get().c_str(),FactionUtil::GetFactionIndex (su->faction),string(""));
    un->SetPosition(QVector(0,0,0));
    break;
  case UNITPTR:
  default:
    un = UnitFactory::createUnit (su->filename.get().c_str(),false,FactionUtil::GetFactionIndex (su->faction));
    un->EnqueueAI (new Orders::AggressiveAI ("default.agg.xml"));
    un->SetTurretAI ();
    if (_Universe->AccessCockpit()->GetParent()) {
      un->SetPosition (_Universe->AccessCockpit()->GetParent()->Position()+QVector(rand()*10000./RAND_MAX-5000,rand()*10000./RAND_MAX-5000,rand()*10000./RAND_MAX-5000));
    }
    break;
  }
  _Universe->activeStarSystem()->AddUnit(un);

}

void destroyObjects() {  
  if (myterrain)
    delete myterrain;
  Terrain::DeleteAll();
  delete tmpcockpittexture;
  //  delete cockpit;
  delete [] fighters;
  delete locSel;
}

int getmicrosleep () {
  static int microsleep = XMLSupport::parse_int (vs_config->getVariable ("audio","threadtime","1"));
  return microsleep;
}

void restore_main_loop() {
  RestoreKB();
  //  winsys_show_cursor(false);
  RestoreMouse();
  GFXLoop (main_loop);

}

void main_loop() {
  // Evaluate number of loops per second each XX loops
  if( loop_count==500)
  {
	last_check = cur_check;
	cur_check = getNewTime();
	//cout<<"Checkpoint at "<<cur_check<<" - last check at "<<last_check<<endl;
	if( last_check!=1)
	{
		// Time to update test
		avg_loop = ( (nb_checks-1)*avg_loop + (loop_count/(cur_check-last_check)) )/(nb_checks);
		//cout<<"Nb checks : "<<nb_checks<<" -- loop_count : "<<loop_count<<endl;
		//cout<<"Time elasped : "<<(cur_check - last_check)<<" -- Loop average : "<<avg_loop<<" -- Ratio : "<<(loop_count/(cur_check-last_check))<<endl;
		nb_checks=nb_checks+1;
	}
 	loop_count=-1;
 }
  loop_count++;
  //  SuicideKey (std::string(),PRESS);

  //  Cockpit::Respawn (std::string(),PRESS);
  /*  static int i=0;
  setTimeCompression(i);
  i++;
  if (i>1000)
  i=0;*/

  // Execute DJ script
  Music::MuzakCycle();

  _Universe->StartDraw();
  if(myterrain){ 
    myterrain->AdjustTerrain(_Universe->activeStarSystem());
  }

  if( Network!=NULL)
  {
	  for( int jj=0; jj<_Universe->numPlayers(); jj++)
	  {
			Network[jj].checkMsg( NULL);
			//Network[jj].sendMsg();
	  }
  }
}
