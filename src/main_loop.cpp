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
		parent->ZSlide(0.100F);
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
}
Unit *carrier=NULL;
Unit *fighter = NULL;
Unit *fighter2=NULL;

LocationSelect *locSel=NULL;

Background * bg = NULL;
TextPlane *textplane = NULL;
void createObjects() {

  //SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
  carrier = new Unit("ucarrier.dat");
  //Unit *fighter = new Unit("uosprey.dat");
  fighter = new Unit("uosprey.dat");
  //Unit *fighter2 = new Unit("uosprey.dat");
  fighter2 = new Unit("uosprey.dat");
  bg = new Background("cube");
  //HUDElement *t = new HUDElement("ucarrier.dat");
  locSel = new LocationSelect(Vector (0,0,5),
			      Vector(.707,.707,0), 
			      Vector (.707,-.707,0));
  //fighter->SetPosition(Vector(5.0, 5.0, 5.0));
  fighter->SetPosition(Vector(0.0, 0.0, 15.0));
  fighter->SetAI(new Orbit);
  fighter->Roll(PI/4);
  carrier->SetPosition(Vector(0.0, 5.0, 10.0));
  carrier->Pitch(PI/2);
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
  
  //Sprite *s = new Sprite("carrier.spr");
  //s->SetPosition(0,0);
  
  textplane = new TextPlane("9x12.fon");
  textplane->SetText(string("This is a test of the emergency broadcast system"));
  textplane->SetPosition(Vector(0.250F, 0.250F, 1.010F));
  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);

  
  fighter2->SetPosition(0.0, 1.0, 50.0);
  fighter2->Pitch(PI/2);
  
  //  const int numf = 256;
  //  Unit *fighters[numf];
  //for(int a = 0; a < numf; a++) {
    //fighters[a] = new Unit("uosprey.dat");
    //fighters[a]->SetPosition((a%8)/8.0 - 2.0, (a/8)/8.0 - 2.0,5.0);
    //fighters[a]->SetPosition((a%16)*5 - 40.0F, (a/16)*5 - 40.0F,25.0F);
    //fighters[a]->Pitch(PI/2);
  //}
}

void destroyObjects() {  
  //	for(a = 0; a < numf; a++)
  //	delete fighters[a];
  delete textplane;
  delete locSel;
  //delete t;
  //delete s;
  delete carrier;
  delete fighter2;
  delete fighter;
  delete bg;
}

void main_loop() {
  static int state = 0;
  _GFX->StartDraw();
  
  //GFXDisable(TEXTURE0);
  //GFXDisable(TEXTURE1);
  //GFXBegin(QUADS);
  //GFXColor4f(0.0,0.0,1.0,1.0);
  //GFXVertex3f(10.0,10.0,1.0);
  //GFXVertex3f(10.0,-10.0,1.0);
  //GFXVertex3f(-10.0,-10.0,1.0);
  //GFXVertex3f(-10.0,10.0,1.0);
  //GFXEnd();
  
  GFXDisable(DEPTHWRITE);
  GFXDisable(DEPTHTEST);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  bg->Draw();
  
  GFXEnable(DEPTHWRITE);
  GFXEnable(DEPTHTEST);
  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);
  
  carrier->TDraw();
  fighter->TDraw();
  fighter2->Draw();
  
  //for(a = 0; a < numf; a++) {
  //fighters[a]->TDraw();
  //fighters[a]->Yaw(rand()%2==0?-1.0:1.0*PI/180);
  //fighters[a]->Yaw(PI/180);
  //}
  
  //fighter2->DrawStreak(Vector(0.0, ((float)state/10.0), 0.0));
  //fighter2->DrawStreak(Vector(0.0, ((float)state/100.0), 0.0));
  //fighter2->SetPosition(fighter2->Position() + Vector(0.0, 0.1, 0.0));
  //s->Draw();
  GFXDisable(TEXTURE1);
  //textplane->TDraw();
  textplane->Yaw(PI/180);
  
  //t->TDraw();
  //s->Yaw(PI/180);
  locSel->Draw();
  _GFX->EndDraw();
  carrier->Rotate(Vector(0.010F,0.010F,0.000F));
  
  if(state<50)
    textplane->Scale(Vector(((float)state)/50.000F,0.100F,1.000F));
  else if(state<100)
    textplane->Scale(Vector(1.000F,((float)state-50)/50.000F,1.000F));
  else
    textplane->Scale(Vector(1.000F,1.000F,1.000F));
  state++;
  if(state > 100)
    state = 100;
  ProcessKB();
  
  //fighter->Roll(PI/180);
}

