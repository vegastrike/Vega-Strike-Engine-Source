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
#include "cmd/ai/flykeyboard.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/script.h"
#include "gfx/cockpit.h"

using namespace std;

#define KEYDOWN(name,key) (name[key] & 0x80)

extern Texture *logo;

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
static void PitchDown(int,KBSTATE newState) {
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

static void PitchUp(int,KBSTATE newState) {
	
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

static void YawLeft(int,KBSTATE newState) {
	
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

static void YawRight(int,KBSTATE newState) {
	
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
static void Quit(int,KBSTATE newState) {
	if(newState==PRESS||newState==DOWN) {
		exit(0);
	}
}

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
	BindKey('w', PitchDown);
	BindKey('z', PitchUp);
	BindKey('a', YawLeft);
	BindKey('s', YawRight);
	BindKey(27, Quit);
}

Cockpit *cockpit;
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
  FILE * fp = fopen ("testmission.txt", "r");
  if (fp) {
    fscanf (fp, "%d\n", &numf);
  }	
  fighters = new Unit * [numf];
  int * tmptarget = new int [numf];

  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);
    

  
  char fightername [1024]="hornet.xunit";
  int a;
  for(a = 0; a < numf; a++) {

    Vector pox (1000+150*a,100*a,100);
    if (fp) {      
      if (!feof(fp))
	fscanf (fp, "%s %f %f %f %d\n",fightername,&pox.i, &pox.j, &pox.k,&tmptarget[a]);

    }
    //    if (tmptarget[a]<0||tmptarget[a]>numf)
    //      tmptarget[a]=0;
    fighters[a] = new Unit(fightername, true, false,tmptarget[a]);
    fighters[a]->SetPosition (pox);
    
    //    fighters[a]->SetAI(new Order());
    if (a!=0) {
      fighters[a]->EnqueueAI( new Orders::AggressiveAI ("default.agg.xml", "default.int.xml"));

    }
    _Universe->activeStarSystem()->AddUnit(fighters[a]);
  }
  //  for (a=0;a<numf;a++) {
  //      fighters[a]->Target (fighters[tmptarget[a]]);
  //  }//now it just sets their faction :-D
  delete [] tmptarget;
  if (fp)
      fclose (fp);
 
  fighters[0]->EnqueueAI(new AIScript("aitest.xml"));
  fighters[0]->EnqueueAI(new FlyByKeyboard ());
  fighters[0]->EnqueueAI(new FireKeyboard ());
  cockpit = new Cockpit ("hornet-cockpit.cpt", fighters[0]);
  shipList = _Universe->activeStarSystem()->getClickList();
  locSel = new CoordinateSelect (Vector (0,0,5));
}

void destroyObjects() {  
  for(int a = 0; a < numf; a++)
  	delete fighters[a];
  delete cockpit;
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

void main_loop() {
  cockpit->SetupViewPort();

  fighters[0]->SetCameraToCockpit();
  _Universe->StartDraw();

  _Universe->activeStarSystem()->Draw();
  cockpit->Draw();
  _Universe->activeStarSystem()->Update();

  GFXEndScene();
      
  ProcessInput();
}



