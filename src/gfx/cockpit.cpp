#include "vs_globals.h"
#include "vegastrike.h"
#include "sprite.h"
#include "cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit.h"
#include "cmd/iterator.h"
#include "cmd/collection.h"
void LocalToRadar (const Vector & pos, float &s, float &t) {
  s = (pos.k>0?pos.k:0)+1;
  t = 2*sqrtf(pos.i*pos.i + pos.j*pos.j + s*s);
  s = -pos.i/t;
  t = pos.j/t;
}

GFXColor relationToColor (float relation) {
  return 
    (relation>=0)?
    GFXColor (1-relation,1-relation,1,1)
    :
    GFXColor (1,relation+1,relation+1,1);
}

void Cockpit::DrawBlips (Unit * un) {
  UnitCollection * drawlist = _Universe->activeStarSystem()->getUnitList();
  Iterator * iter = drawlist->createIterator();
  Unit * target;
  Unit * makeBigger = un->Target();
  float s,t;
  float xsize,ysize,xcent,ycent;
  Radar->GetSize (xsize,ysize);
  Radar->GetPosition (xcent,ycent);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  GFXPointSize (2);
  GFXBegin(GFXPOINT);
  while ((target = iter->current())!=NULL) {
    if (target!=un) {
      LocalToRadar (un->ToLocalCoordinates(target->Position()-un->Position()),s,t);
      GFXColorf (relationToColor (_Universe->GetRelation(un->faction,target->faction)));

      
      if (target==makeBigger) {
	GFXEnd();
	GFXPointSize(4);
	GFXBegin (GFXPOINT);
      }
      GFXVertex3f (xcent+xsize*s,ycent+ysize*t,0);
      if (target==makeBigger) {
	GFXEnd();
	GFXPointSize (2);
	GFXBegin(GFXPOINT);
      }
      
    }
    iter->advance();
  }
  GFXEnd();
  GFXPointSize (1);
  GFXColor (1,1,1,1);
  GFXEnable (TEXTURE0);
  delete iter;
}


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
  if (Radar) {
    delete Radar;
    Radar = NULL;
  }
  if (VDU[0]) {
    delete VDU[0];
    VDU[0]=NULL;
  }
  if (VDU[1]) {
    delete VDU[1];
    VDU[1]=NULL;
  }
  if (Shield[0]) {
    delete Shield[0];
    Shield[0]=NULL;
  }
  if (Shield[1]) {
    delete Shield[1];
    Shield[1]=NULL;
  }
  if (Crosshairs) {
    delete Crosshairs;
    Crosshairs = NULL;
  }
}
Cockpit::Cockpit (const char * file, Unit * parent): parent (parent),Crosshairs(NULL),cockpit_offset(0), viewport_offset(0), view(CP_FRONT), zoomfactor (1.2) {
  Radar=Crosshairs=VDU[0]=VDU[1]=Shield[0]=Shield[1]=Pit[0]=Pit[1]=Pit[2]=Pit[3]=NULL;
  Init (file);
}
void Cockpit::Draw() {
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);
  GFXBlendMode (ONE,ONE);
  Unit * un;
  if (view==CP_FRONT &&Crosshairs)
    Crosshairs->Draw();
  RestoreViewPort();
  GFXBlendMode (ONE,ZERO);
  GFXAlphaTest (GREATER,.1);
  if (view<CP_CHASE) {
    if (Pit[view]) 
      Pit[view]->Draw();
  }
  GFXAlphaTest (ALWAYS,0);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  if ((un = parent.GetUnit())) {
    if (view==CP_FRONT) {//only draw crosshairs for front view
      if (VDU[0]) {
	VDU[0]->Draw();
	//process VDU 0:targetting VDU
      }
      if (VDU[1]) {
      VDU[1]->Draw();
      //process VDU, damage VDU, targetting VDU
      }
      if (Radar) {
	Radar->Draw();
	DrawBlips(un);
      }
      if (Shield[0]) {
      Shield[0]->Draw();
      //show shield status
      }
      if (Shield[1]) {
	Shield[1]->Draw();
	//show fuel status
      }
    }
  }
  GFXHudMode (false);

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
