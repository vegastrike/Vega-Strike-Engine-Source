#include "vs_globals.h"
#include "vs_path.h"
#include "vegastrike.h"
#include "gauge.h"
#include "cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit.h"
#include "cmd/iterator.h"
#include "cmd/collection.h"
#include "hud.h"
#include "vdu.h"
#include "lin_time.h"//for fps
#include "config_xml.h"
#include "lin_time.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"

#include <assert.h>	// needed for assert() calls



 void Cockpit::LocalToRadar (const Vector & pos, float &s, float &t) {
  s = (pos.k>0?pos.k:0)+1;
  t = 2*sqrtf(pos.i*pos.i + pos.j*pos.j + s*s);
  s = -pos.i/t;
  t = pos.j/t;
}

void Cockpit::LocalToEliteRadar (const Vector & pos, float &s, float &t,float &h){
  s=-pos.i/1000.0;
  t=pos.k/1000.0;
  h=pos.j/1000.0;
}


GFXColor Cockpit::unitToColor (Unit *un,Unit *target) {
 	if(target->isUnit()==PLANETPTR){
	  // this is a planet
	  return planet;
	}
	else if(target==un->Target()){
	  // my target
	  return targeted;
	}
	else if(target->Target()==un){
	  // the other ships is targetting me
	  return targetting;
	}

	// other spaceships
	return relationToColor(_Universe->GetRelation(un->faction,target->faction));
	
}

GFXColor Cockpit::relationToColor (float relation) {
 if (relation>0) {
    return GFXColor (relation*friendly.r+(1-relation)*neutral.r,relation*friendly.g+(1-relation)*neutral.g,relation*friendly.b+(1-relation)*neutral.b,relation*friendly.a+(1-relation)*neutral.a);
  } 
 else if(relation==0){
   return GFXColor(neutral.r,neutral.g,neutral.b,neutral.a);
}else { 
    return GFXColor (-relation*enemy.r+(1+relation)*neutral.r,-relation*enemy.g+(1+relation)*neutral.g,-relation*enemy.b+(1+relation)*neutral.b,-relation*enemy.a+(1+relation)*neutral.a);
  }
}
void Cockpit::DrawNavigationSymbol (const Vector &Loc, const Vector & P, const Vector & Q, float size) {
  GFXColor4f (1,1,1,1);
  size = .125*GFXGetZPerspective (size);
  GFXBegin (GFXLINE);
  GFXVertexf(Loc+P*size);
  GFXVertexf(Loc+.125*P*size);
  GFXVertexf(Loc-P*size);
  GFXVertexf(Loc-.125*P*size);
  GFXVertexf(Loc+Q*size);
  GFXVertexf(Loc+.125*Q*size);
  GFXVertexf(Loc-Q*size);
  GFXVertexf(Loc-.125*Q*size);
  GFXVertexf(Loc+.0625*Q*size);
  GFXVertexf(Loc+.0625*P*size);
  GFXVertexf(Loc-.0625*Q*size);
  GFXVertexf(Loc-.0625*P*size);
  GFXVertexf(Loc+.9*P*size+.125*Q*size);
  GFXVertexf(Loc+.9*P*size-.125*Q*size);
  GFXVertexf(Loc-.9*P*size+.125*Q*size);
  GFXVertexf(Loc-.9*P*size-.125*Q*size);
  GFXVertexf(Loc+.9*Q*size+.125*P*size);
  GFXVertexf(Loc+.9*Q*size-.125*P*size);
  GFXVertexf(Loc-.9*Q*size+.125*P*size);
  GFXVertexf(Loc-.9*Q*size-.125*P*size);

  GFXEnd();
}

void Cockpit::DrawTargetBoxes(){
  float speed,range;
  
  Unit * un = parent.GetUnit();
  if (!un)
    return;
  if (un->Getnebula()!=NULL)
    return;

  StarSystem *ssystem=_Universe->activeStarSystem();
  UnitCollection *unitlist=ssystem->getUnitList();
  //UnitCollection::UnitIterator *uiter=unitlist->createIterator();
  Iterator *uiter=unitlist->createIterator();
  
  Vector CamP,CamQ,CamR;
  _Universe->AccessCamera()->GetPQR(CamP,CamQ,CamR);
 
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);

  Unit *target=uiter->current();
  while(target!=NULL){
    if(target!=un){
        Vector Loc(target->Position());

	GFXColor drawcolor=unitToColor(un,target);
	GFXColorf(drawcolor);

	if(target->isUnit()==UNITPTR){
	  GFXBegin (GFXLINESTRIP); 
	  GFXVertexf (Loc+(CamP+CamQ)*target->rSize());
	  GFXVertexf (Loc+(CamP-CamQ)*target->rSize());
	  GFXVertexf (Loc+(-CamP-CamQ)*target->rSize());
	  GFXVertexf (Loc+(CamQ-CamP)*target->rSize());
	  GFXVertexf (Loc+(CamP+CamQ)*target->rSize());
	  GFXEnd();
	}
    }
    target=uiter->advance();
  }

  GFXEnable (TEXTURE0);
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);

}


void Cockpit::DrawTargetBox () {
  float speed,range;
  
  Unit * un = parent.GetUnit();
  if (!un)
    return;
  if (un->Getnebula()!=NULL)
    return;
  Unit *target = un->Target();
  if (!target)
    return;
  Vector CamP,CamQ,CamR;
  _Universe->AccessCamera()->GetPQR(CamP,CamQ,CamR);
  //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
  Vector Loc(target->Position());
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);
  DrawNavigationSymbol (un->GetComputerData().NavPoint,CamP,CamQ, CamR.Dot(un->GetComputerData().NavPoint-un->Position()));
  GFXColorf (un->GetComputerData().radar.color?unitToColor(un,target):GFXColor(1,1,1,1));

  if(draw_line_to_target){
    Vector my_loc(un->Position());
    GFXBegin(GFXLINESTRIP);
    GFXVertexf(my_loc);
    GFXVertexf(Loc);

    Unit *targets_target=target->Target();
    if(draw_line_to_targets_target && targets_target!=NULL){
      Vector ttLoc(targets_target->Position());
      GFXVertexf(ttLoc);
    }
    GFXEnd();
  }

  GFXBegin (GFXLINESTRIP); 
  GFXVertexf (Loc+(CamP+CamQ)*target->rSize());
  GFXVertexf (Loc+(CamP-CamQ)*target->rSize());
  GFXVertexf (Loc+(-CamP-CamQ)*target->rSize());
  GFXVertexf (Loc+(CamQ-CamP)*target->rSize());
  GFXVertexf (Loc+(CamP+CamQ)*target->rSize());
  GFXEnd();
  if (always_itts || un->GetComputerData().itts) {
    un->getAverageGunSpeed (speed,range);
    float err = un->GetComputerData().radar.error*(1+.01*(1-un->CloakVisible()));
   Vector iLoc = target->PositionITTS (un->Position(),speed)+10*err*Vector (-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX,-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX,-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX);
    
    GFXBegin (GFXLINESTRIP);
    if(draw_line_to_itts){
      GFXVertexf(Loc);
      GFXVertexf(iLoc);
    }
    GFXVertexf (iLoc+(CamP)*.25*un->rSize());
    GFXVertexf (iLoc+(-CamQ)*.25*un->rSize());
    GFXVertexf (iLoc+(-CamP)*.25*un->rSize());
    GFXVertexf (iLoc+(CamQ)*.25*un->rSize());
    GFXVertexf (iLoc+(CamP)*.25*un->rSize());
    GFXEnd();
  }
  GFXEnable (TEXTURE0);
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);

}


void Cockpit::DrawBlips (Unit * un) {
  Unit::Computer::RADARLIM * radarl = &un->GetComputerData().radar;
  UnitCollection * drawlist = _Universe->activeStarSystem()->getUnitList();
  Iterator * iter = drawlist->createIterator();
  Unit * target;
  Unit * makeBigger = un->Target();
  float s,t;
  float xsize,ysize,xcent,ycent;
  Radar->GetSize (xsize,ysize);
  xsize = fabs (xsize);
  ysize = fabs (ysize);
  Radar->GetPosition (xcent,ycent);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  GFXPointSize (2);
  GFXBegin(GFXPOINT);
  while ((target = iter->current())!=NULL) {
    if (target!=un) {
      Vector localcoord;
      if (!un->InRange (target,localcoord)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter->advance();	
	continue;
      }
      LocalToRadar (localcoord,s,t);
      GFXColor localcol (radarl->color?unitToColor (un,target):GFXColor(1,1,1,1));
      GFXColorf (localcol);
      if (target==makeBigger) {
	GFXEnd();
	GFXPointSize(4);
	GFXBegin (GFXPOINT);
      }
      float rerror = radarl->error+((un->Getnebula()!=NULL)?.03:0)+(target->Getnebula()!=NULL?.06:0);
      GFXVertex3f (xcent+xsize*(s-.5*rerror+(rerror*rand())/RAND_MAX),ycent+ysize*(t+-.5*rerror+(rerror*rand())/RAND_MAX),0);
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
  GFXColor4f (1,1,1,1);
  GFXEnable (TEXTURE0);
  delete iter;
}

void Cockpit::DrawEliteBlips (Unit * un) {
  Unit::Computer::RADARLIM * radarl = &un->GetComputerData().radar;
  UnitCollection * drawlist = _Universe->activeStarSystem()->getUnitList();
  Iterator * iter = drawlist->createIterator();
  Unit * target;
  Unit * makeBigger = un->Target();
  float s,t,es,et,eh;
  float xsize,ysize,xcent,ycent;
  Radar->GetSize (xsize,ysize);
  xsize = fabs (xsize);
  ysize = fabs (ysize);
  Radar->GetPosition (xcent,ycent);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);

  while ((target = iter->current())!=NULL) {
    if (target!=un) {
      Vector localcoord;
      if (!un->InRange (target,localcoord)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter->advance();	
	continue;
      }


	LocalToRadar (localcoord,s,t);
	LocalToEliteRadar(localcoord,es,et,eh);


      GFXColor localcol (radarl->color?unitToColor (un,target):GFXColor(1,1,1,1));
      GFXColorf (localcol);
      int headsize=4;
#if 1
      if (target==makeBigger) {
	headsize=6;
	//cout << "localcoord" << localcoord << " s=" << s << " t=" << t << endl;
      }
#endif
      float xerr,yerr,y2,x2;
      float rerror = radarl->error+((un->Getnebula()!=NULL)?.03:0)+(target->Getnebula()!=NULL?.06:0);
      xerr=xcent+xsize*(es-.5*rerror+(rerror*rand())/RAND_MAX);
      yerr=ycent+ysize*(et+-.5*rerror+(rerror*rand())/RAND_MAX);
      x2=xcent+xsize*((es+0)-.5*rerror+(rerror*rand())/RAND_MAX);
      y2=ycent+ysize*((et+t)-.5*rerror+(rerror*rand())/RAND_MAX);

      //      printf("xerr,yerr: %f %f xcent %f xsize %f\n",xerr,yerr,xcent,xsize);

      ///draw the foot
      GFXPointSize(2);
      GFXBegin(GFXPOINT);
      GFXVertex3f (xerr,yerr,0);
      GFXEnd();

      ///draw the leg
      GFXBegin(GFXLINESTRIP);
      GFXVertex3f(x2,yerr,0);
      GFXVertex3f(x2,y2,0);
      GFXEnd();

      ///draw the head
      GFXPointSize(headsize);
      GFXBegin(GFXPOINT);
      GFXVertex3f(xerr,y2,0);
      GFXEnd();

      GFXPointSize(1);
    }
    iter->advance();
  }
  GFXPointSize (1);
  GFXColor4f (1,1,1,1);
  GFXEnable (TEXTURE0);
  delete iter;
}
float Cockpit::LookupTargetStat (int stat, Unit *target) {
  static float fpsval=0;
  const float fpsmax=1;
  static float numtimes=fpsmax;
  unsigned short armordat[4];
  Unit * tmpunit;
  switch (stat) {
  case SHIELDF:
    return target->FShieldData();
  case SHIELDR:
    return target->RShieldData();
  case SHIELDL:
    return target->LShieldData();
  case SHIELDB:
    return target->BShieldData();
  case ARMORF:
  case ARMORR:
  case ARMORL:
  case ARMORB:
    target->ArmorData (armordat);
    return ((float)armordat[stat-ARMORF])/StartArmor[stat-ARMORF];
  case FUEL:
    return target->FuelData()/maxfuel;
  case ENERGY:
    return target->EnergyData();
  case HULL:
    return target->GetHull()/maxhull;
  case EJECT:
    return (((target->GetHull()/maxhull)<.25)&&(target->BShieldData()<.25||target->FShieldData()<.25))?1:0;
  case LOCK:
    if  ((tmpunit = target->GetComputerData().threat.GetUnit())) {
      return (tmpunit->cosAngleTo (target,*(float*)&armordat[0],FLT_MAX,FLT_MAX)>.95);
    }
    return 0;
  case KPS:
    return (target->GetVelocity().Magnitude())*10;
  case SETKPS:
    return target->GetComputerData().set_speed*10;
  case FPS:
    if (fpsval>=0&&fpsval<.5*FLT_MAX)
      numtimes-=.1+fpsval;
    if (numtimes<=0) {
      numtimes = fpsmax;
      fpsval = GetElapsedTime();
    }
    return 1./fpsval;
  }
  return 1;
}
void Cockpit::DrawGauges(Unit * un) {

  int i;
  for (i=0;i<KPS;i++) {
    if (gauges[i]) {
      gauges[i]->Draw(LookupTargetStat (i,un));
    }
  }
  if (!text)
	  return;
  text->SetSize (2,-2);
  GFXColorf (textcol);
  for (i=KPS;i<NUMGAUGES;i++) {
    if (gauges[i]) {
      float sx,sy,px,py;
      gauges[i]->GetSize (sx,sy);
      gauges[i]->GetPosition (px,py);
      text->SetCharSize (sx,sy);
      text->SetPos (px,py);
      int tmp = (int)LookupTargetStat (i,un);
      char ourchar[32];
      sprintf (ourchar,"%d", tmp);
      text->Draw (string (ourchar));
    }
  }
  GFXColor4f (1,1,1,1);
}
void Cockpit::Init (const char * file) {
  Delete();
  vschdir (file);
  LoadXML(file);
  vscdup();
  if (Panel.size()>0) {
    float x,y;
    Panel.front()->GetPosition (x,y);
    Panel.front()->SetPosition (x,y+viewport_offset);  
  }
}

void Cockpit::SetParent (Unit * unit) {
  parent.SetUnit (unit);
  if (unit) {
    unit->ArmorData (StartArmor);
    if (StartArmor[0]==0) StartArmor[0]=1;
    if (StartArmor[1]==0) StartArmor[1]=1;
    if (StartArmor[2]==0) StartArmor[2]=1;
    if (StartArmor[3]==0) StartArmor[3]=1;
    maxfuel = unit->FuelData();
    maxhull = unit->GetHull();
  }
}
void Cockpit::Delete () {
  int i;
  if (text) {
    delete text;
    text = NULL;
  }
  viewport_offset=cockpit_offset=0;
  for (i=0;i<4;i++) {
    if (Pit[i]) {
      delete Pit[i];
      Pit[i] = NULL;
    }
  }
  for (i=0;i<NUMGAUGES;i++) {
    if (gauges[i]) {
      delete gauges[i];
      gauges[i]=NULL;
    }
  }
  if (Radar) {
    delete Radar;
    Radar = NULL;
  }
  unsigned int j;
  for (j=0;j<vdu.size();j++) {
    if (vdu[j]) {
      delete vdu[j];
      vdu[j]=NULL;
    }
  }
  vdu.clear();
  for (j=0;j<Panel.size();j++) {
    assert (Panel[j]);
    delete Panel[j];
  }
  Panel.clear();
}
Cockpit::Cockpit (const char * file, Unit * parent): parent (parent),textcol (1,1,1,1),text(NULL),cockpit_offset(0), viewport_offset(0), view(CP_FRONT), zoomfactor (1.2) {
  Radar=Pit[0]=Pit[1]=Pit[2]=Pit[3]=NULL;
  for (int i=0;i<NUMGAUGES;i++) {
    gauges[i]=NULL;
  }

  draw_all_boxes=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAllTargetBoxes","false"));
  draw_line_to_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTarget","false"));
  draw_line_to_targets_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTargetsTarget","false"));
  draw_line_to_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToITTS","false"));
  always_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAlwaysITTS","false"));
  radar_type=vs_config->getVariable("graphics","hud","radarType","WC");

  friendly=GFXColor(-1,-1,-1,-1);
  enemy=GFXColor(-1,-1,-1,-1);
  neutral=GFXColor(-1,-1,-1,-1);
  targeted=GFXColor(-1,-1,-1,-1);
  targetting=GFXColor(-1,-1,-1,-1);
  planet=GFXColor(-1,-1,-1,-1);
  if (friendly.r==-1) {
    vs_config->getColor ("enemy",&enemy.r);
    vs_config->getColor ("friend",&friendly.r);
    vs_config->getColor ("neutral",&neutral.r);
    vs_config->getColor("target",&targeted.r);
    vs_config->getColor("targetting_ship",&targetting.r);
    vs_config->getColor("planet",&planet.r);
  }
 

  Init (file);
}
void Cockpit::Draw() { 
  GFXDisable (TEXTURE1);
  GFXLoadIdentity(MODEL);
  DrawTargetBox();
  if(draw_all_boxes){
    DrawTargetBoxes();
  }
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);
  GFXBlendMode (ONE,ONE);
  Unit * un;
  if (view==CP_FRONT) {
    if (Panel.size()>0) {
      Panel.front()->Draw();//draw crosshairs
    }
  }
  RestoreViewPort();
  GFXBlendMode (ONE,ZERO);
  GFXAlphaTest (GREATER,.1);
  if (view<CP_CHASE) {
    if (Pit[view]) 
      Pit[view]->Draw();
  }
  GFXAlphaTest (ALWAYS,0);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
	bool die=true;
  if ((un = parent.GetUnit())) {
    if (view==CP_FRONT) {//only draw crosshairs for front view
      DrawGauges(un);
      for (unsigned int vd=0;vd<vdu.size();vd++) {
	if (vdu[vd]) {
	  vdu[vd]->Draw(un);
	  //process VDU, damage VDU, targetting VDU
	}
      }

      if (Radar) {
	Radar->Draw();
	if(radar_type=="Elite"){
	  DrawEliteBlips(un);
	}
	else{
	  DrawBlips(un);
	}
      }
    }
    if (view==CP_FRONT) {
      for (unsigned int j=1;j<Panel.size();j++) {
	Panel[j]->Draw();
      }
    }
	if (un->GetHull()>0)
		die = false;
  }
  if (die) {
	static float dietime = 0;
	if (text) {
		GFXColor4f (1,1,1,1);
		text->SetSize(1,-1);
		float x; float y;
		if (dietime==0) {
			text->GetCharSize (x,y);
			text->SetCharSize (x*4,y*4);
			text->SetPos (0-(x*2*14),0-(y*2));
			mission->msgcenter->add("game","all","You Have Died!");
		}
		text->Draw ("You Have Died!");
	}
	dietime +=GetElapsedTime();
	SetView (CP_PAN);
	zoomfactor=dietime*10;
  }
  GFXHudMode (false);

}
Cockpit::~Cockpit () {
  Delete();
}

void Cockpit::SetView (const enum VIEWSTYLE tmp) {
  view = tmp;
}
void Cockpit::VDUSwitch (int vdunum) {
  if (vdunum<(int)vdu.size()) {
    if (vdu[vdunum]) {
      vdu[vdunum]->SwitchMode();
    }
  }
}
void Cockpit::ScrollVDU (int vdunum, int howmuch) {
  if (vdunum<(int)vdu.size()) {
    if (vdu[vdunum]) {
      vdu[vdunum]->Scroll(howmuch);
    }
  }
}
void Cockpit::ScrollAllVDU (int howmuch) {
  for (unsigned int i=0;i<vdu.size();i++) {
    ScrollVDU (i,howmuch);
  }
}
void Cockpit::RestoreViewPort() {
  GFXViewPort (0, 0, g_game.x_resolution,g_game.y_resolution);
}

static void ShoveCamBehindUnit (int cam, Unit * un, float zoomfactor) {
  Vector unpos = un->GetPlanetOrbit()?un->LocalPosition():un->Position();
  _Universe->AccessCamera(cam)->SetPosition(unpos-_Universe->AccessCamera()->GetR()*un->rSize()*zoomfactor);
}
void Cockpit::SetupViewPort (bool clip) {
    GFXViewPort (0,(int)((view==CP_FRONT?viewport_offset:0)*g_game.y_resolution), g_game.x_resolution,g_game.y_resolution);
  _Universe->activeStarSystem()->AccessCamera()->setCockpitOffset (view<CP_CHASE?cockpit_offset:0);
  Unit * un, *tgt;
  if ((un = parent.GetUnit())) {
    un->UpdateHudMatrix (CP_FRONT);
    un->UpdateHudMatrix (CP_LEFT);
    un->UpdateHudMatrix (CP_RIGHT);
    un->UpdateHudMatrix (CP_BACK);
    un->UpdateHudMatrix (CP_CHASE);
    
    Vector p,q,r;
    _Universe->AccessCamera(CP_FRONT)->GetOrientation(p,q,r);
    Vector tmp = r;
    r = -p;
    p = tmp;
    _Universe->AccessCamera(CP_LEFT)->SetOrientation(p,q,r);
    _Universe->AccessCamera(CP_FRONT)->GetOrientation(p,q,r);
    tmp = r;
    r = p;
    p = -tmp;
    _Universe->AccessCamera(CP_RIGHT)->SetOrientation(p,q,r);
    _Universe->AccessCamera(CP_FRONT)->GetOrientation(p,q,r);
    r = -r;
    p = -p;
    _Universe->AccessCamera(CP_BACK)->SetOrientation(p,q,r);

    tgt = un->Target();
    if (tgt) {
      
      Vector p,q,r,tmp;
      un->GetOrientation (p,q,r);
      r = tgt->Position()-un->Position();
      r.Normalize();
      CrossProduct (r,q,tmp);
      CrossProduct (tmp,r,q);
      _Universe->AccessCamera(CP_TARGET)->SetOrientation(tmp,q,r);
      _Universe->AccessCamera(CP_PANTARGET)->SetOrientation(tmp,q,r);
    }else {
      un->UpdateHudMatrix (CP_TARGET);
      un->UpdateHudMatrix (CP_PANTARGET);
    }
    ShoveCamBehindUnit (CP_CHASE,un,zoomfactor);
    ShoveCamBehindUnit (CP_PANTARGET,un,zoomfactor);
    ShoveCamBehindUnit (CP_PAN,un,zoomfactor);

    un->SetVisible(view>=CP_CHASE);
    _Universe->activeStarSystem()->SelectCamera(view);
  }
  _Universe->activeStarSystem()->AccessCamera()->UpdateGFX(clip?GFXTRUE:GFXFALSE);
    
  //  parent->UpdateHudMatrix();
}
