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
  if (Pit) {
    delete Pit;
    Pit = NULL;
  }
  if (Crosshairs) {
    delete Crosshairs;
    Crosshairs = NULL;
  }
}
Cockpit::Cockpit (const char * file, Unit * parent): parent (parent), Pit(NULL),Crosshairs(NULL),cockpit_offset(0), viewport_offset(0), view(CP_FRONT), zoomfactor (1.2) {
  Init (file);
}
void Cockpit::Draw() {
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);
  GFXBlendMode (ONE,ONE);
  if (Crosshairs)
    Crosshairs->Draw();
  GFXBlendMode (ONE,ZERO);
  GFXAlphaTest (GREATER,.1);
  RestoreViewPort();
  if (Pit) 
    Pit->Draw();
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
    GFXViewPort (0,(int)(viewport_offset*g_game.y_resolution), g_game.x_resolution,g_game.y_resolution);
  _Universe->activeStarSystem()->AccessCamera()->setCockpitOffset (cockpit_offset);
  Unit * un;
  if ((un = parent.GetUnit())) {
    if (view!=CP_PAN) {
      un->UpdateHudMatrix();
      if (view==CP_CHASE) {
	_Universe->AccessCamera()->SetPosition(_Universe->AccessCamera()->GetPosition()-_Universe->AccessCamera()->GetR()*un->rSize()*zoomfactor);
      } else  {
	if (view==CP_LEFT) {
	
	} else if (view==CP_RIGHT) {
	
	} else if (view==CP_BACK) {

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
