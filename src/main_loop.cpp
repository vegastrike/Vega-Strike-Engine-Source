#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "lin_time.h"
#include "cmd/unit.h"
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
#include "vs_path.h"
#include "mission.h"
#include "xml_support.h"
#include "config_xml.h"

using namespace std;

static Music * muzak=NULL;

#define KEYDOWN(name,key) (name[key] & 0x80)



 GFXBOOL capture;
GFXBOOL quit = GFXFALSE;

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

const float timek = .005;
bool _Slew = true;

namespace CockpitKeys {

 void SkipMusicTrack(int,KBSTATE newState) {
   static bool flag=false;
   if(newState==PRESS){
     printf("skipping\n");
    muzak->Skip();
   }
 }

 void PitchDown(int,KBSTATE newState) {
	static Vector Q;
	static Vector R;
	if(newState==PRESS) {
		Q = _Universe->AccessCamera()->Q;
		R = _Universe->AccessCamera()->R;
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-Q, R,timek);
		//a =1;
	}
	else if(_Slew&&newState==RELEASE) {
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(Q, R,timek);
		//a=0;
	}
}

 void PitchUp(int,KBSTATE newState) {
	
	static Vector Q;
	static Vector R;

	if(newState==PRESS) {
		Q = _Universe->AccessCamera()->Q;
		R = _Universe->AccessCamera()->R;
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(Q, R,timek);
		
	}
	else if(_Slew&&newState==RELEASE) {
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-Q, R,timek);
		
	}
}

  void YawLeft(int,KBSTATE newState) {
	
	static Vector P;
	static Vector R;

	if(newState==PRESS) {
		P = _Universe->AccessCamera()->P;
		R = _Universe->AccessCamera()->R;
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-P, R,timek);
		
	}
	else if(_Slew&&newState==RELEASE) {
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(P, R,timek);
		
	}
}

  void YawRight(int,KBSTATE newState) {
	
	static Vector P;
	static Vector R;
	if(newState==PRESS) {
		P = _Universe->AccessCamera()->P;
		R = _Universe->AccessCamera()->R;
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(P, R,timek);
	
	}
	else if(_Slew&&newState==RELEASE) {
		_Universe->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-P, R,timek);
	       
	}
}
  void Quit(int,KBSTATE newState) {
	if(newState==PRESS||newState==DOWN) {
		exit(0);
	}
}
bool cockpitfront=true;
  void Inside(int,KBSTATE newState) {
  const int cockpiton=1;
  const int backgroundoff=2;
  const int max = 4;
  static int tmp=1;
  if(newState==PRESS&&cockpitfront) {
    if (tmp&cockpiton) {
      _Universe->AccessCockpit()->Init ("hornet-cockpit.cpt");	    
    }else {
      _Universe->AccessCockpit()->Init ("disabled-cockpit.cpt");
    }
    _Universe->activeStarSystem()->getBackground()->EnableBG(!(tmp&backgroundoff));
    tmp--;
    if (tmp<0) tmp=max-1;
  }
  if(newState==PRESS||newState==DOWN) {
    cockpitfront=true;
    _Universe->AccessCockpit()->SetView (CP_FRONT);
  }
}
  void ZoomOut (int, KBSTATE newState) {
  if(newState==PRESS||newState==DOWN) 
  _Universe->AccessCockpit()->zoomfactor+=GetElapsedTime();  
}

  void ZoomIn (int, KBSTATE newState) {
  if(newState==PRESS||newState==DOWN) 
  _Universe->AccessCockpit()->zoomfactor-=GetElapsedTime();  
}

  void InsideLeft(int,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	  cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_LEFT);
	}
}
  void InsideRight(int,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_RIGHT);
	}
}
  void InsideBack(int,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	    cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_BACK);
	}
}

  void SwitchLVDU(int,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (0);
	}
}
  void SwitchRVDU(int,KBSTATE newState) {

	if(newState==PRESS) {
	  _Universe->AccessCockpit()->VDUSwitch (1);
	}
}

  void Behind(int,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	  cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_CHASE);
	}
}
  void Pan(int,KBSTATE newState) {

	if(newState==PRESS||newState==DOWN) {
	  cockpitfront=false;
	  _Universe->AccessCockpit()->SetView (CP_PAN);
	}
}

}

using namespace CockpitKeys;

Unit *carrier=NULL;
Unit *fighter = NULL;
Unit *fighter2=NULL;
int numf = 3;
Unit **fighters;
CoordinateSelect *locSel=NULL;
//Background * bg = NULL;
SphereMesh *bg2=NULL;
Animation *  explosion= NULL;
ClickList *shipList =NULL;
Unit *midway = NULL;
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
      fprintf (stderr,"Select Box Hit single target");
    if (c->createIterator()->advance()!=NULL)
      fprintf (stderr,"Select Box Hit Multiple Targets");
  }else {
    oldx = x;
    oldy = y;
  }
  if (k==PRESS) {
    fprintf (stderr,"click?");
    UnitCollection * c = shipList->requestIterator (x,y);
    if (c->createIterator()->current()!=NULL)
      fprintf (stderr,"Hit single target");
    if (c->createIterator()->advance()!=NULL)
      fprintf (stderr,"Hit Multiple Targets");
    fprintf (stderr,"\n");
    
  }
}
*/

void InitializeInput() {

	BindKey(27, Quit); // always have quit on esc
}

//Cockpit *cockpit;
static Texture *tmpcockpittexture;

void createObjects() {
  explosion= new Animation ("explosion_orange.ani",false,.1,BILINEAR,false);
  LoadWeapons("weapon_list.xml");


  /****** 
  locSel = new LocationSelect(Vector (0,-2,2),
			      Vector(1,0,-1), 
			      Vector (-1,0,-1));
//GOOD!!
  ****/
  BindKey (1,CoordinateSelect::MouseMoveHandle);

  int numf=mission->number_of_flightgroups;

  fighters = new Unit * [numf];
  int * tmptarget = new int [numf];

  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);
  
  char fightername [1024]="hornet.xunit";
  int a=0;
    vector<Flightgroup *>::const_iterator siter;
    vector<Flightgroup *> fg=mission->flightgroups;

  for(siter= fg.begin() ; siter!=fg.end() ; siter++,a++){
    Vector pox (1000+150*a,100*a,100);

    Flightgroup *fg=*siter;

    string fullname=fg->type;// + ".xunit";

    strcpy(fightername,fullname.c_str());
	//	strcat(fightername,".xunit");

	pox.i=fg->pos[0];
	pox.j=fg->pos[1];
	pox.k=fg->pos[2];

	string ainame=fg->ainame;

	tmptarget[a]=1; // that should not be in xml?

      if (pox.i==pox.j&&pox.j==pox.k&&pox.k==0) {
	pox.i=rand()*10000./RAND_MAX-5000;
	pox.j=rand()*10000./RAND_MAX-5000;
	pox.k=rand()*10000./RAND_MAX-5000;

      }



    fighters[a] = new Unit(fightername, true, false,tmptarget[a]);
    fighters[a]->SetPosition (pox);
    
    //    fighters[a]->SetAI(new Order());
    if (a!=0) {
      string ai_agg=ainame+".agg.xml";
      string ai_int=ainame+".int.xml";

      char ai_agg_c[1024];
      char ai_int_c[1024];
      strcpy(ai_agg_c,ai_agg.c_str());
      strcpy(ai_int_c,ai_int.c_str());
      //      printf("1 - %s  2 - %s\n",ai_agg_c,ai_int_c);

      fighters[a]->EnqueueAI( new Orders::AggressiveAI (ai_agg_c, ai_int_c));
      for (int kk=0;kk<fighters[a]->getNumSubUnits();kk++) {
	fighters[a]->EnqueueAI (new Orders::FireAt(.2,15),kk);
	fighters[a]->EnqueueAI (new Orders::FaceTarget (false,3),kk);
      }
    }
    _Universe->activeStarSystem()->AddUnit(fighters[a]);
  } // end of for flightgroups

  //  for (a=0;a<numf;a++) {
  //      fighters[a]->Target (fighters[tmptarget[a]]);
  //  }//now it just sets their faction :-D
  delete [] tmptarget;

 
#ifdef IPILOTTURRET
  fighters[0]->EnqueueAI (new Orders::AggressiveAI ("default.agg.xml", "default.int.xml"));
  fighters[0]->getSubUnit (0)->EnqueueAI(new FlyByJoystick (0,"player1.kbconf"));
  fighters[0]->getSubUnit (0)->EnqueueAI(new FireKeyboard (0,""));
#else
  fighters[0]->EnqueueAI(new FlyByJoystick (0,"player1.kbconf"));
  fighters[0]->EnqueueAI(new FireKeyboard (0,""));

  for (int kk=0;kk<fighters[0]->getNumSubUnits();kk++) {
    fighters[0]->EnqueueAI (new Orders::FireAt(.2,15),kk);
    fighters[0]->EnqueueAI (new Orders::FaceTarget (false,3),kk);
  }
#endif
  vschdir ("hornet-cockpit.cpt");
  tmpcockpittexture = new Texture ("hornet-cockpit.bmp","hornet-cockpitalp.bmp",0,NEAREST);
  vscdup();

  muzak = new Music (fighters[0]);
  AUDListenerSize (fighters[0]->rSize()*4);
  _Universe->AccessCockpit()->Init ("hornet-cockpit.cpt");
#ifdef IPILOTTURRET
  _Universe->AccessCockpit()->SetParent(fighters[0]->getSubUnit(0));
#else
  _Universe->AccessCockpit()->SetParent(fighters[0]);
#endif
  shipList = _Universe->activeStarSystem()->getClickList();
  locSel = new CoordinateSelect (Vector (0,0,5));
  UpdateTime();

}

void destroyObjects() {  
  for(int a = 0; a < numf; a++)
  	delete fighters[a];
  delete tmpcockpittexture;
  delete muzak;
  //  delete cockpit;
  delete [] fighters;
  delete locSel;
  delete explosion;
  explosion=NULL;
  //delete t;
  //delete s;
  //delete carrier;
  //delete fighter2;
  //delete fighter;
}
extern void micro_sleep (unsigned int n);
void main_loop() {
  static int microsleep = XMLSupport::parse_int (vs_config->getVariable ("audio","threadtime","2000"));

  _Universe->StartDraw();
  muzak->Listen();
  _Universe->activeStarSystem()->Draw();
  
  //fighters[0]->UpdateHudMatrix();
  //_Universe->activeStarSystem()->SetViewport();


  _Universe->activeStarSystem()->Update();
  micro_sleep (microsleep);//so we don't starve the audio thread  
  GFXEndScene();
      
  ProcessInput();

}



