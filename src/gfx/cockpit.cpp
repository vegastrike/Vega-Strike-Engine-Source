#include "vs_globals.h"
#include "vegastrike.h"
#include "sprite.h"
#include "cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit.h"

void Cockpit::Init (const char * file) {
  Delete();
  LoadXML(file);
  if (Crosshairs) {
    float x,y;
    Crosshairs->GetPosition (x,y);
    Crosshairs->SetPosition (x,y+viewport_offset);  
  }
  
}

void Cockpit::SetParent (Unit * unit) {
  parent.SetUnit (unit);
}
void Cockpit::Delete () {
  for (int i=0;i<4;i++) {
    if (Pit[i]) {
      delete Pit[i];
      Pit[i] = NULL;
    }
  }
  if (Crosshairs) {
    delete Crosshairs;
    Crosshairs = NULL;
  }
}
Cockpit::Cockpit (const char * file, Unit * parent): parent (parent),Crosshairs(NULL),cockpit_offset(0), viewport_offset(0), view(CP_FRONT), zoomfactor (1.2) {
  Pit[0]=Pit[1]=Pit[2]=Pit[3]=NULL;
  Init (file);
}
void Cockpit::Draw() {
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);
  GFXBlendMode (ONE,ONE);
  if (Crosshairs&&view==CP_FRONT)//only draw crosshairs for front view
    Crosshairs->Draw();
  GFXBlendMode (ONE,ZERO);
  GFXAlphaTest (GREATER,.1);
  RestoreViewPort();
  if (view<CP_CHASE)
    if (Pit[view]) 
      Pit[view]->Draw();
  GFXHudMode (false);
  GFXAlphaTest (ALWAYS,0);
}
Cockpit::~Cockpit () {
  Delete();
}

void Cockpit::SetView (const enum VIEWSTYLE tmp) {
  view = tmp;
}

void Cockpit::RestoreViewPort() {
  GFXViewPort (0, 0, g_game.x_resolution,g_game.y_resolution);
}
void Cockpit::SetupViewPort () {
    GFXViewPort (0,(int)((view<CP_CHASE?viewport_offset:0)*g_game.y_resolution), g_game.x_resolution,g_game.y_resolution);
  _Universe->activeStarSystem()->AccessCamera()->setCockpitOffset (view<CP_CHASE?cockpit_offset:0);
  Unit * un;
  if ((un = parent.GetUnit())) {
    if (view!=CP_PAN) {
      un->UpdateHudMatrix();
      if (view==CP_CHASE) {
	_Universe->AccessCamera()->SetPosition(_Universe->AccessCamera()->GetPosition()-_Universe->AccessCamera()->GetR()*un->rSize()*zoomfactor);
      } else  {
	Vector p,q,r;
	_Universe->AccessCamera()->GetOrientation(p,q,r);
	if (view==CP_LEFT) {
	  Vector tmp = r;
	  r = -p;
	  p = tmp;
	  _Universe->AccessCamera()->SetOrientation(p,q,r);
	} else if (view==CP_RIGHT) {
	  Vector tmp = r;
	  r = p;
	  p = -tmp;
	  _Universe->AccessCamera()->SetOrientation(p,q,r);
	} else if (view==CP_BACK) {
	  r = -r;
	  p = -p;
	  _Universe->AccessCamera()->SetOrientation(p,q,r);
	}
      }
    }else {
      _Universe->AccessCamera()->SetPosition (un->Position()-_Universe->AccessCamera()->GetR()*un->rSize()*zoomfactor);
    }
    _Universe->activeStarSystem()->SetViewport();
    un->SetVisible(view>=CP_CHASE);
    
  }
 
  //  parent->UpdateHudMatrix();
}
