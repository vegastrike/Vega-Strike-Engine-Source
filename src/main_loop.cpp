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
#include "gfx_coordinate_select.h"
#include "gfx_mesh.h"
#include "cmd_navigation_orders.h"
#include "cmd_beam.h"
#include  "gfx_halo.h"
#include "gfx_transform_matrix.h"
#include "in_ai.h"
using namespace std;

#define KEYDOWN(name,key) (name[key] & 0x80)

extern Texture *logo;

GFXBOOL capture;
GFXBOOL quit = GFXFALSE;

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
	  //parent->Pitch(PI/180);
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
  //parent->ZSlide(0.100F);
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
static void Slew (int,KBSTATE newState){
	
	if (newState==PRESS) {
		_Slew = !_Slew;
	}
	else if (newState==RELEASE)
	  ;
}
static void PitchDown(int,KBSTATE newState) {
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

static void PitchUp(int,KBSTATE newState) {
	
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

static void YawLeft(int,KBSTATE newState) {
	
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

static void YawRight(int,KBSTATE newState) {
	
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

static void RollLeft(int,KBSTATE newState) {
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

static void RollRight(int,KBSTATE newState) {
	
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


static void SlideForward(int,KBSTATE newState) {
	
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

static void SlideBackward(int,KBSTATE newState) {
	
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

static void SlideUp(int,KBSTATE newState) {
	
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

static void SlideDown(int,KBSTATE newState) {
	
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

static void SlideLeft(int,KBSTATE newState) {
	
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

static void SlideRight(int,KBSTATE newState) {
       
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
static void reCenter (int, KBSTATE newState) {
  if (newState==PRESS) {
    _GFX->AccessCamera()->SetPosition(Vector (0,0,0));
  }

}
static void Stop (int,KBSTATE newState) {

	if (newState==PRESS) {
		_GFX->AccessCamera()->myPhysics.SetAngularVelocity (Vector (0,0,0));
		_GFX->AccessCamera()->myPhysics.SetVelocity (Vector (0,0,0));
		//_GFX->AccessCamera()->myPhysics.ResistiveTorqueThrust (-timek,_GFX->AccessCamera()->P);
		//_GFX->AccessCamera()->myPhysics.ResistiveThrust (-timek);

	}
	else if (newState==RELEASE) 
			;
}
static void Quit(int,KBSTATE newState) {
	if(newState==PRESS||newState==DOWN) {
		exit(0);
	}
}

Unit *carrier=NULL;
Unit *fighter = NULL;
Unit *fighter2=NULL;
const int numf = 1;
Unit *fighters[numf];
CoordinateSelect *locSel=NULL;
Background * bg = NULL;
SphereMesh *bg2=NULL;
TextPlane *textplane = NULL;
Beam * DABEAM;
ClickList *shipList =NULL;
Unit *midway = NULL;
static void Fire (int, KBSTATE newState) {
  if (newState==DOWN) {
    fighters[0]->Fire();
  }
}

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
/*
static void FighterPitchDown(int,KBSTATE newState) {
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

static void FighterPitchUp(int,KBSTATE newState) {
	
	static Vector Q = fighter->P();
	static Vector R = fighter->R();

	if(newState==PRESS) {
	  fighter->ApplyBalancedLocalTorque(Q, R);
	}
	else if(_Slew&&newState==RELEASE) {
	}
}

static void FighterYawLeft(int,KBSTATE newState) {
	
	static Vector P = fighter->P();
	static Vector R = fighter->R();

	if(newState==PRESS) {
	  fighter->ApplyBalancedLocalTorque(-P, R);
	}
	else if(_Slew&&newState==RELEASE) {
	}
}

static void FighterYawRight(int,KBSTATE newState) {
	
	static Vector P = fighter->P();
	static Vector R = fighter->R();
	if(newState==PRESS) {
	  fighter->ApplyBalancedLocalTorque(P, R);
	  fighter->ApplyForce(P*10);
	}
	else if(_Slew&&newState==RELEASE) {
	}
}
*/

void InitializeInput() {
	BindKey(GLUT_KEY_F1, Slew);
	BindKey(GLUT_KEY_F12,Stop);
	BindKey('w', PitchDown);
	BindKey('z', PitchUp);
	BindKey('a', YawLeft);
	BindKey('s', YawRight);
	BindKey('c', RollLeft);
	BindKey('v', RollRight);
	BindKey(GLUT_KEY_PAGE_DOWN, SlideDown);
	BindKey(GLUT_KEY_PAGE_UP, SlideUp);
	BindKey('1', SlideBackward);
	BindKey('2', SlideForward);
	//	BindKey(',', SlideLeft);
	//	BindKey('.',SlideRight);
	BindKey('~', Quit);
	BindKey('q', Quit);
	BindKey ('c',reCenter);
	BindKey (' ',Fire);
	/*	BindKey('a', FighterYawLeft);
	BindKey('d', FighterYawRight);
	BindKey('w', FighterPitchDown);
	BindKey('s', FighterPitchUp);*/
}
void createObjects() {
  LoadWeapons("weapon_list.xml");
  DABEAM= new Beam (identity_transformation, weapon_info(weapon_info::BEAM), NULL);
  //SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);

  //0.0.4  carrier = new Unit("ucarrier.dat");
  //star_system = new StarSystem(new Planet("test_system.dat"));  
  //Unit *fighter = new Unit("uospreys.dat");
  //0.0.4fighter = new Unit("uosprey.dat");
  //Unit *fighter2 = new Unit("uosprey.dat");
  //0.0.4fighter2 = new Unit("uosprey.dat");
  bg = new Background("cube");
  //bg2 = new SphereMesh (20.0,8,8,"sun.bmp",true,true);
  //HUDElement *t = new HUDElement("ucarrier.dat");
  /******
  locSel = new LocationSelect(Vector (0,-2,2),
			      Vector(1,0,-1), 
			      Vector (-1,0,-1));
//GOOD!!
  ****/
  BindKey (1,CoordinateSelect::MouseMoveHandle);

  locSel = new CoordinateSelect (Vector (0,0,5));

  //  GFXSelectMaterial(0);
  
  //  s = new Sprite("carrier.spr");
  
  textplane = new TextPlane("9x12.fon");
  textplane->SetText(string("This is a test of the emergency broadcast system"));
  //textplane->SetPosition(Vector(0.250F, 0.250F, 1.00F));
  textplane->SetPosition(Vector(0.0F, 0.0F, 2.000F));
  GFXEnable(TEXTURE0);
  GFXEnable(TEXTURE1);
    

  //  
  //  
  for(int a = 0; a < numf; a++) {
    //fighters[a] = new Unit("uosprey.dat");
    //fighters[a] = new Unit("Homeworld-HeavyCorvette.xml", true);
    switch((a)%6) {
    case 1:
      //fighters[a] = new Unit("broadsword.xunit", true);
      fighters[a] = new Unit("midway.xunit", true);
      break;
    case 0:
      fighters[a] = new Unit("hornet.xunit", true);
      break;
    case 2:
      fighters[a] = new Unit("Heavycorvette.xunit", true);
      break;
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
      break;
      /*
    case 7:
      fighters[a] = new Unit("uosprey.dat");
      break;
      */
    }
    //fighters[a] = new Unit("phantom.xunit", true);
    //fighters[a]->SetPosition((a%8)/8.0 - 2.0, (a/8)/8.0 - 2.0,5.0);

    //Vector position((a%20)/0.25 - 4.0F, (a/20)/0.25 - 4.0F,2.0F);
    Vector position = Vector(3,0,-2);
    fighters[a]->SetPosition(position);
    position.k += 6;
    Vector v(0,1,0);
    v.Normalize();

    v = Vector(1,1,-1);
    v.Normalize();
    fighters[a]->SetAI(new Order());
    ////fighters[a]->EnqueueAI(new Orders::ChangeHeading(v, 0.04));
    fighters[a]->EnqueueAI(new Orders::MoveTo(Vector (-20,30,10), true, .05));
    //fighters[a]->EnqueueAI(new FlyByKeyboard ());
    //fighters[a]->EnqueueAI(new Orders::ChangeHeading(Vector (0,0,-1).Normalize(), 0.04));
    //fighters[a]->SetPosition(0, 0, -2.0F);
  
    //fighters[a]->Pitch(PI/2);
    //fighters[a]->Roll(PI/2);
    //fighters[a]->Scale(Vector(0.5,0.5,0.5));
    //    fighters[a]->SetAI(new Orders::MoveTo(Vector(5,10,-4), true,1.0));
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
  delete DABEAM;
  for(int a = 0; a < numf; a++)
  	delete fighters[a];
  delete textplane;
  delete locSel;
  //delete t;
  //delete s;
  //delete carrier;
  //delete fighter2;
  //delete fighter;
  delete bg;
  //  delete bg2;  if you delete a sphere wiht paletted texture and its refcount you'll get a malloc problem
}

void main_loop() {
  static int state = 0;
  static bool midcachunk=false;
  _GFX->StartDraw();
  
  //bg2->Draw();
  GFXBlendMode (ONE,ZERO);
  bg->Draw();
  //GFXDisable(TEXTURE1);
  _GFX->activeStarSystem()->Draw();
  _GFX->activeStarSystem()->Update();
  
    GFXDisable(TEXTURE1);
  //  GFXBlendMode(ONE,ONE);
    locSel->Draw();
  static float time=0;
  //  DABEAM->Draw(identity_transformation,identity_matrix);
  DABEAM->Draw (identity_transformation,identity_matrix);
  Halo::ProcessDrawQueue();
  Beam::ProcessDrawQueue();

  //textplane->Draw();
  _GFX->EndDraw();
      
  ProcessInput();
}

