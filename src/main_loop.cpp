#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "gfx.h"
#include "lin_time.h"
#include "cmd.h"
#include "in.h"
#include "cmd.h"
#include "gfx_sprite.h"
#include "physics.h"
#include "gfx_hud.h"
#include "gfxlib.h"
#include "gfx_location_select.h"
#include <string>
#include "cmd_input_dfa.h"
#include "UnitCollection.h"
#include "star_system.h"
#include "planet.h"
#include "gfx_sphere.h"

#include "gfx_mesh.h"
using namespace std;

#define KEYDOWN(name,key) (name[key] & 0x80)

extern Texture *logo;

BOOL capture;
BOOL quit = FALSE;

/*11-7-98
 *Cool shit happened when a position rotation matrix from a unit was used for the drawing of the background... not very useful though
 */
class Orbit;

class Line:public AI
{
	float count;
public:
	Line(Unit *parent1):AI(parent1){count = 0;};
		
	Line():AI()
	{
		count = 0;
	}
	AI *Execute();
};

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
	  parent->Pitch(PI/180);
		count ++;
		if(30 == count)
		{
			Unit *parent = this->parent;
			//delete this;
			//return new Line(parent);
			return this;
		}
		else
			return this;
	}
};

AI *Line::Execute()
{
	//parent->Position(); // query the position
	parent->ZSlide(0.100F);
	count ++;
	/*
	if(parent->Position().i > 0.75 ||
		parent->Position().i < 0.25 ||
		parent->Position().j > 0.75 ||
		parent->Position().j < 0.25)
		count = 10;
	*/
	if(10 == count)
	{
		Unit *parent = this->parent;
		delete this;
		return new Orbit(parent);
	}
	else
		return this;
}

const float timek = .01;
bool _Slew = true;
static void Slew (KBSTATE newState){
	
	if (newState==PRESS) {
		_Slew = !_Slew;
	}
	else if (newState==RELEASE)
	  ;
}
static void PitchDown(KBSTATE newState) {
	static Vector Q;
	static Vector R;
	if(newState==PRESS) {
		Q = _GFX->AccessCamera()->Q;
		R = _GFX->AccessCamera()->R;
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-Q, R,timek);
		//a =1;
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(Q, R,timek);
		//a=0;
	}
}

static void PitchUp(KBSTATE newState) {
	
	static Vector Q;
	static Vector R;

	if(newState==PRESS) {
		Q = _GFX->AccessCamera()->Q;
		R = _GFX->AccessCamera()->R;
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(Q, R,timek);
		
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-Q, R,timek);
		
	}
}

static void YawLeft(KBSTATE newState) {
	
	static Vector P;
	static Vector R;

	if(newState==PRESS) {
		P = _GFX->AccessCamera()->P;
		R = _GFX->AccessCamera()->R;
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-P, R,timek);
		
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(P, R,timek);
		
	}
}

static void YawRight(KBSTATE newState) {
	
	static Vector P;
	static Vector R;
	if(newState==PRESS) {
		P = _GFX->AccessCamera()->P;
		R = _GFX->AccessCamera()->R;
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(P, R,timek);
	
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-P, R,timek);
	       
	}
}

static void RollLeft(KBSTATE newState) {
	static Vector P;
	static Vector Q;
	
	if(newState==PRESS) {
		P=_GFX->AccessCamera()->P;
		Q=_GFX->AccessCamera()->Q;
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-P, Q,timek);
		//a=1;
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(P, Q,timek);
		//a=0;
		//Stop();
	}
}

static void RollRight(KBSTATE newState) {
	
	static Vector P;
	static Vector Q;
	
	if(newState==PRESS) {
		P=_GFX->AccessCamera()->P;
		Q=_GFX->AccessCamera()->Q;
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(P, Q,timek);
		
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyBalancedLocalTorque(-P, Q,timek);
		//Stop();
	}
}


static void SlideForward(KBSTATE newState) {
	
	static Vector R;
	if(newState==PRESS) {
		R = _GFX->AccessCamera()->R;
		_GFX->AccessCamera()->myPhysics.ApplyForce (R,timek);
	
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyForce (-R,timek);
		//Stop();
	}
}

static void SlideBackward(KBSTATE newState) {
	
	static Vector R;
	if(newState==PRESS) {
		R = _GFX->AccessCamera()->R;
		_GFX->AccessCamera()->myPhysics.ApplyForce (-R,timek);
		
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyForce (R,timek);
		//Stop();
	}
}

static void SlideUp(KBSTATE newState) {
	
	static Vector Q;
	if(newState==PRESS){
		Q = _GFX->AccessCamera()->Q;
		_GFX->AccessCamera()->myPhysics.ApplyForce(Q,timek);
		//a=1;
		//Stop();
	}
	else if(_Slew&&newState==RELEASE){
		_GFX->AccessCamera()->myPhysics.ApplyForce(-Q,timek);
		//a=0;
		//Stop();
	}
}

static void SlideDown(KBSTATE newState) {
	
	static Vector Q;
	if(newState==PRESS) {
		Q = _GFX->AccessCamera()->Q;
		_GFX->AccessCamera()->myPhysics.ApplyForce(-Q,timek);
	
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyForce(Q, timek);
		//Stop();
	}
}

static void SlideLeft(KBSTATE newState) {
	
	static Vector P;
	if(newState==PRESS) {
		P = _GFX->AccessCamera()->P;
		_GFX->AccessCamera()->myPhysics.ApplyForce(-P,timek);
	
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyForce(P,timek);
		//Stop();
	}
}

static void SlideRight(KBSTATE newState) {
       
	static Vector P;
	if(newState==PRESS) {
		P = _GFX->AccessCamera()->P;
		_GFX->AccessCamera()->myPhysics.ApplyForce(P,timek);
		//		a=1;
	}
	else if(_Slew&&newState==RELEASE) {
		_GFX->AccessCamera()->myPhysics.ApplyForce(-P,timek);
		//a=0;
	}
}
static void Stop (KBSTATE newState) {

	if (newState==PRESS) {
		_GFX->AccessCamera()->myPhysics.SetAngularVelocity (Vector (0,0,0));
		_GFX->AccessCamera()->myPhysics.SetVelocity (Vector (0,0,0));
		//_GFX->AccessCamera()->myPhysics.ResistiveTorqueThrust (-timek,_GFX->AccessCamera()->P);
		//_GFX->AccessCamera()->myPhysics.ResistiveThrust (-timek);

	}
	else if (newState==RELEASE) 
			;
}
static void Quit(KBSTATE newState) {
	if(newState==PRESS||newState==DOWN) {
		exit(0);
	}
}

Unit *carrier=NULL;
Unit *fighter = NULL;
Unit *fighter2=NULL;
const int numf = 5;
Unit *fighters[numf];
//LocationSelect *locSel=NULL;
Background * bg = NULL;
SphereMesh *bg2=NULL;
TextPlane *textplane = NULL;

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

static void FighterPitchDown(KBSTATE newState) {
	static Vector Q = fighter->Q();
	static Vector R = fighter->R();
	if(newState==PRESS) {
	  fighter->Pitch(PI/8);
	  //fighter->ApplyBalancedLocalTorque(-Q, R);
	}
	else if(_Slew&&newState==RELEASE) {
		//a=0;
	}
}

static void FighterPitchUp(KBSTATE newState) {
	
	static Vector Q = fighter->P();
	static Vector R = fighter->R();

	if(newState==PRESS) {
	  fighter->ApplyBalancedLocalTorque(Q, R);
	}
	else if(_Slew&&newState==RELEASE) {
	}
}

static void FighterYawLeft(KBSTATE newState) {
	
	static Vector P = fighter->P();
	static Vector R = fighter->R();

	if(newState==PRESS) {
	  fighter->ApplyBalancedLocalTorque(-P, R);
	}
	else if(_Slew&&newState==RELEASE) {
	}
}

static void FighterYawRight(KBSTATE newState) {
	
	static Vector P = fighter->P();
	static Vector R = fighter->R();
	if(newState==PRESS) {
	  fighter->ApplyBalancedLocalTorque(P, R);
	  fighter->ApplyForce(P*10);
	}
	else if(_Slew&&newState==RELEASE) {
	}
}

void InitializeInput() {
	BindKey(GLUT_KEY_F1, Slew);
	BindKey(GLUT_KEY_F12,Stop);
	BindKey(GLUT_KEY_UP, PitchDown);
	BindKey(GLUT_KEY_DOWN, PitchUp);
	BindKey(GLUT_KEY_LEFT, YawLeft);
	BindKey(GLUT_KEY_RIGHT, YawRight);
	BindKey('/', RollLeft);
	BindKey('*', RollRight);
	BindKey(GLUT_KEY_PAGE_DOWN, SlideDown);
	BindKey(GLUT_KEY_PAGE_UP, SlideUp);
	BindKey('-', SlideBackward);
	BindKey('+', SlideForward);
	BindKey(',', SlideLeft);
	BindKey('.',SlideRight);
	BindKey('q', Quit);

	/*	BindKey('a', FighterYawLeft);
	BindKey('d', FighterYawRight);
	BindKey('w', FighterPitchDown);
	BindKey('s', FighterPitchUp);*/
}
Animation * s;

void createObjects() {

  //SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);

  carrier = new Unit("ucarrier.dat");
  //star_system = new StarSystem(new Planet("test_system.dat"));  
  //Unit *fighter = new Unit("uospreys.dat");
  fighter = new Unit("uosprey.dat");
  //Unit *fighter2 = new Unit("uosprey.dat");
  fighter2 = new Unit("uosprey.dat");
  s = new Animation ("explosion_orange.ani",true,.1,false);
  s->SetPosition (0,0,5);
  bg = new Background("cube");
  //bg2 = new SphereMesh (20.0,8,8,"sun.bmp",true,true);
  //HUDElement *t = new HUDElement("ucarrier.dat");
  /*************
		locSel = new LocationSelect(Vector (0,-1,5),
		Vector(1,0,0), 
		Vector (0,-.4,-1));
//GOOD!!
  *************/
  //locSel = new LocationSelect (Vector (0,-1,5),
  //			       Vector (1,0,0),
  //			       Vector (0,-.35,-1));

  //fighter->SetPosition(Vector(5.0, 5.0, 5.0));
  fighter->SetPosition(Vector(0.0, 10.0, 0.0));
  fighter->SetAI(new Orbit);
  //fighter->Roll(PI/4);
  //fighter->Accelerate(Vector(25,0,0));
  carrier->SetPosition(Vector(0.0, 5.0, 10.0));
  carrier->Pitch(-PI/2);
  //carrier->Accelerate(Vector(0, 25, 0));
  ////  fighter->Scale(Vector(0.1,0.1,0.1));
  ////  fighter2->Scale(Vector(0.1,0.1,0.1));
  ////  carrier->Scale(Vector(0.2,0.2,0.2));
  //t->SetPosition(Vector(0.5, 0.5, 15.0));
  //t->Pitch(PI/2);
  
  GFXMaterial mat;
  GFXGetMaterial(0, mat);
  mat.ar = 1.0;
  mat.ag = 1.0;
  mat.ab = 1.0;
  mat.er = 1.0;
  mat.eg = 1.0;
  mat.eb = 1.0;
  GFXSetMaterial(0, mat);
  GFXSelectMaterial(0);
  
  //  s = new Sprite("carrier.spr");
  
  textplane = new TextPlane("9x12.fon");
  textplane->SetText(string("This is a test of the emergency broadcast system"));
  //textplane->SetPosition(Vector(0.250F, 0.250F, 1.00F));
  textplane->SetPosition(Vector(0.0F, 0.0F, 2.000F));
  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);


  fighter2->SetPosition(0.0, 1.0, 50.0);
  fighter2->Pitch(PI/2);
  
  //  
  //  
  for(int a = 0; a < numf; a++) {
    //fighters[a] = new Unit("uosprey.dat");
    //fighters[a] = new Unit("midway.xunit", true);
    switch(a%3) {
    case 0:
      //fighters[a] = new Unit("broadsword.xunit", true);
      fighters[a] = new Unit("midway.xunit", true);
      break;
    case 1:
      fighters[a] = new Unit("Cloakedfighter.xunit", true);
      break;
    case 2:
      fighters[a] = new Unit("Heavycorvette.xunit", true);
      break;/*
    case 3:
      fighters[a] = new Unit("Heavyinterceptor.xunit", true);
      break;
    case 4:
      fighters[a] = new Unit("Lightcorvette.xunit", true);
      break;
    case 5:
      fighters[a] = new Unit("Lightinterceptor.xunit", true);
      break;
    case 6:
      fighters[a] = new Unit("Homeworld-HeavyCorvette.xml", true);
      break;*/
    }
    //fighters[a] = new Unit("phantom.xunit", true);
    //fighters[a]->SetPosition((a%8)/8.0 - 2.0, (a/8)/8.0 - 2.0,5.0);


     fighters[a]->SetPosition((a%10)*35 - 40.0F, (a/10)*35 - 40.0F,7.0F);
  



    //fighters[a]->Pitch(PI/2);
    //fighters[a]->Roll(PI/2);
    //fighters[a]->Scale(Vector(0.5,0.5,0.5));
    _GFX->activeStarSystem()->AddUnit(fighters[a]);
  }


  //_GFX->activeStarSystem()->AddUnit(fighter);
  //_GFX->activeStarSystem()->AddUnit(carrier);
  shipList = _GFX->activeStarSystem()->getClickList();
    //BindKey (1,startselect);
    //BindKey (0,clickhandler);
  /*
    midway = new Unit("b_midway.xml", true);
  midway = new Unit("square.xml", true);
  midway->SetPosition(1,1, 10);
  _GFX->activeStarSystem()->AddUnit(midway);
  midway = new Unit("mid.xml", true);
  midway->SetPosition(5,5, 15);
  _GFX->activeStarSystem()->AddUnit(midway);
  */
  //midway = new Unit("Homeworld-HeavyCorvette.xml", true);
  //midway->SetPosition(8,-5, 10);
  //_GFX->activeStarSystem()->AddUnit(midway);
  //exit(0);
}

void destroyObjects() {  
  for(int a = 0; a < numf; a++)
  	delete fighters[a];
  delete textplane;
  //delete locSel;
  //delete t;
  //delete s;
  delete carrier;
  delete fighter2;
  delete fighter;
  delete bg;
  //  delete bg2;  if you delete a sphere wiht paletted texture and its refcount you'll get a malloc problem
  delete s;
}

void main_loop() {
  static int state = 0;

  _GFX->StartDraw();
  
  GFXDisable(DEPTHWRITE);
  GFXDisable(DEPTHTEST);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  //bg2->Draw();
  //bg->Draw();
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);
  //GFXDisable(TEXTURE1);
  GFXMaterial mat;
  GFXGetMaterial(0, mat);
  mat.ar = 1.0;
  mat.ag = 1.0;
  mat.ab = 1.0;
  mat.er = 1.0;
  mat.eg = 1.0;
  mat.eb = 1.0;
  GFXSetMaterial(0, mat);
  GFXSelectMaterial(0);


  _GFX->activeStarSystem()->Draw();
  _GFX->activeStarSystem()->Update();
  ProcessKB();
  
  GFXDisable(TEXTURE1);
  //textplane->Draw();
  _GFX->EndDraw();
}

