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
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include <assert.h>	// needed for assert() calls
#include "savegame.h"


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
	else if(target==un->Target()&&draw_all_boxes){//if we only draw target box we cannot tell what faction enemy is!
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
inline void DrawOneTargetBox (const Vector & Loc, const float rSize, const Vector &CamP, const Vector & CamQ, const Vector & CamR) {
  GFXBegin (GFXLINESTRIP); 
  GFXVertexf (Loc+(CamP+CamQ)*rSize);
  GFXVertexf (Loc+(CamP-CamQ)*rSize);
  GFXVertexf (Loc+(-CamP-CamQ)*rSize);
  GFXVertexf (Loc+(CamQ-CamP)*rSize);
  GFXVertexf (Loc+(CamP+CamQ)*rSize);
  GFXEnd();
}

static GFXColor DockBoxColor (const string& name) {
  GFXColor dockbox;
  vs_config->getColor(name,&dockbox.r);    
  return dockbox;
}
inline void DrawDockingBoxes(Unit * un,Unit *target, const Vector & CamP, const Vector & CamQ, const Vector & CamR) {
  if (target->IsCleared (un)) {
    static GFXColor dockbox = DockBoxColor("docking_box");
    GFXColorf (dockbox);
    const vector <DockingPorts> d = target->DockingPortLocations();
    for (unsigned int i=0;i<d.size();i++) {
      float rad = d[i].radius/sqrt(2.0);
      DrawOneTargetBox (Transform (target->GetTransformation(),d[i].pos),rad ,CamP, CamQ, CamR);
    }
  }
}

void Cockpit::DrawTargetBoxes(){
  
  Unit * un = parent.GetUnit();
  if (!un)
    return;
  if (un->GetNebula()!=NULL)
    return;

  StarSystem *ssystem=_Universe->activeStarSystem();
  UnitCollection *unitlist=ssystem->getUnitList();
  //UnitCollection::UnitIterator *uiter=unitlist->createIterator();
  un_iter uiter=unitlist->createIterator();
  
  Vector CamP,CamQ,CamR;
  _Universe->AccessCamera()->GetPQR(CamP,CamQ,CamR);
 
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);

  Unit *target=uiter.current();
  while(target!=NULL){
    if(target!=un){
        Vector Loc(target->Position());

	GFXColor drawcolor=unitToColor(un,target);
	GFXColorf(drawcolor);

	if(target->isUnit()==UNITPTR){
	  DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR);
	  if (un->Target()==target) {
	    DrawDockingBoxes(un,target,CamP,CamQ, CamR);
	  }
	}
    }
    target=(++uiter);
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
  if (un->GetNebula()!=NULL)
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
  DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR);
  DrawDockingBoxes(un,target,CamP,CamQ,CamR);
  if (always_itts || un->GetComputerData().itts) {
    un->getAverageGunSpeed (speed,range);
    float err = (.01*(1-un->CloakVisible()));
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
  un_iter iter = drawlist->createIterator();
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
  while ((target = iter.current())!=NULL) {
    if (target!=un) {
      Vector localcoord;
      if (!un->InRange (target,localcoord)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter.advance();	
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
      float rerror = ((un->GetNebula()!=NULL)?.03:0)+(target->GetNebula()!=NULL?.06:0);
      GFXVertex3f (xcent+xsize*(s-.5*rerror+(rerror*rand())/RAND_MAX),ycent+ysize*(t+-.5*rerror+(rerror*rand())/RAND_MAX),0);
      if (target==makeBigger) {
	GFXEnd();
	GFXPointSize (2);
	GFXBegin(GFXPOINT);
      }
      
    }
    iter.advance();
  }
  GFXEnd();
  GFXPointSize (1);
  GFXColor4f (1,1,1,1);
  GFXEnable (TEXTURE0);
}

void Cockpit::DrawEliteBlips (Unit * un) {
  Unit::Computer::RADARLIM * radarl = &un->GetComputerData().radar;
  UnitCollection * drawlist = _Universe->activeStarSystem()->getUnitList();
  un_iter iter = drawlist->createIterator();
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

  while ((target = iter.current())!=NULL) {
    if (target!=un) {
      Vector localcoord;
      if (!un->InRange (target,localcoord)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter.advance();	
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
      float rerror = ((un->GetNebula()!=NULL)?.03:0)+(target->GetNebula()!=NULL?.06:0);
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
    iter.advance();
  }
  GFXPointSize (1);
  GFXColor4f (1,1,1,1);
  GFXEnable (TEXTURE0);
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
      GFXColorf (textcol);
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

void Cockpit::SetParent (Unit * unit, const char * filename, const char * unitmodname, const Vector & pos) {
  parent.SetUnit (unit);
  unitlocation=pos;
  this->unitfilename=std::string(filename);
  this->unitmodname=std::string(unitmodname);
  if (unit) {
    this->unitfaction = unit->faction;
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
void Cockpit::SelectProperCamera () {
    _Universe->activeStarSystem()->SelectCamera(view);
}
static int respawnunit=0;
static int switchunit=0;
static int turretcontrol=0;
void Cockpit::SwitchControl (int,KBSTATE k) {
  if (k==PRESS) {
    switchunit=1;
  }

}
void Cockpit::TurretControl (int,KBSTATE k) {
  if (k==PRESS) {
    turretcontrol=1;
  }

}
void Cockpit::Respawn (int,KBSTATE k) {
  if (k==PRESS) {
    respawnunit=1;
  }

}

void SwitchUnits (Unit * ol, Unit * nw) {
  if (ol) {
    ol->PrimeOrders();
    ol->SetAI (new Orders::AggressiveAI ("default.agg.xml","default.int.xml"));
    ol->SetVisible (true);
  }
  if (nw) {
    nw->PrimeOrders();
    nw->EnqueueAI (new FireKeyboard (0,""));
    nw->EnqueueAI (new FlyByJoystick (0,"player1.kbconf"));
  }
}


extern void reset_time_compression(int i, KBSTATE a);
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

      if (Radar) {
	Radar->Draw();
	if(radar_type=="Elite"){
	  DrawEliteBlips(un);
	}
	else{
	  DrawBlips(un);
	}
      }
      for (unsigned int j=1;j<Panel.size();j++) {
	Panel[j]->Draw();
      }
      for (unsigned int vd=0;vd<vdu.size();vd++) {
	if (vdu[vd]) {
	  GFXColorf (textcol);
	  vdu[vd]->Draw(un);
	  GFXColor4f (1,1,1,1);
	  //process VDU, damage VDU, targetting VDU
	}
      }

    }
    if (un->GetHull()>0)
      die = false;
    if (un->Threat()!=NULL) {
      FlyByKeyboard::StopAutoKey (0,PRESS);
      reset_time_compression(0,PRESS);
      un->Threaten (NULL,0);
    }
  }
  if (die) {
	static float dietime = 0;
	if (text) {
		GFXColor4f (1,1,1,1);
		text->SetSize(1,-1);
		float x; float y;
		if (dietime==0) {
			respawnunit=0;
			text->GetCharSize (x,y);
			text->SetCharSize (x*4,y*4);
			text->SetPos (0-(x*2*14),0-(y*2));
			mission->msgcenter->add("game","all","You Have Died!");
		}
		GFXColorf (textcol);
		text->Draw ("You Have Died!");
		GFXColor4f (1,1,1,1);
	}
	dietime +=GetElapsedTime();
	SetView (CP_PAN);
	zoomfactor=dietime*10;
	if (respawnunit){
   	  zoomfactor=1;
	  respawnunit=0;
	  Unit * un = new Unit (unitfilename.c_str(),false,this->unitfaction,unitmodname);
	  un->SetCurPosition (unitlocation);
	  _Universe->activeStarSystem()->AddUnit (un);
	  this->SetParent(un,unitfilename.c_str(),unitmodname.c_str(),unitlocation);
	  //un->SetAI(new FireKeyboard ())
	  SwitchUnits (NULL,un);
	  credits = GetSavedCredits();
	}
  }
  if (turretcontrol) {
    turretcontrol=0;
    Unit * par = GetParent();
    if (par) {
      static int index=0;
      int i=0;bool tmp=false;bool tmpgot=false;
      if (parentturret.GetUnit()==NULL) {
	tmpgot=true;
	un_iter ui= par->getSubUnits();
	Unit * un;
	while ((un=ui.current())) {
	  if (i++==index) {
	    tmp=true;
	    index++;

	    SwitchUnits(par,un);
	    parentturret.SetUnit(par);
	    un_iter uj= un->getSubUnits();
	    Unit * tur;
	    while ((tur=uj.current())) {
	      SwitchUnits (NULL,tur);
	      this->SetParent(tur,this->unitfilename.c_str(),this->unitmodname.c_str(),unitlocation);
	      ++uj;
	    }
	    break;
	  }
	  ++ui;
	}
      }
      if (tmp==false) {
	if (tmpgot) index=0;
	Unit * un = parentturret.GetUnit();
	if (un) {
	  
	  SetParent (un,unitfilename.c_str(),this->unitmodname.c_str(),unitlocation);
	  SwitchUnits (NULL,un);
	  parentturret.SetUnit(NULL);
	  un->SetTurretAI();
	}
      }
    }
  }
  if (switchunit) {
    zoomfactor=1;
    static int index=0;
    switchunit=0;
    un_iter ui= _Universe->activeStarSystem()->getUnitList()->createIterator();
    Unit * un;
    bool found=false;
    int i=0;
    while ((un=ui.current())) {
      if (un->faction==this->unitfaction) {
	
	if ((i++)>=index&&(un!=GetParent())) {
	  found=true;
	  Unit * k=GetParent(); 
	  SwitchUnits (k,un);
	  this->SetParent(un,this->unitfilename.c_str(),this->unitmodname.c_str(),unitlocation);
	  //un->SetAI(new FireKeyboard ())
	  break;
	}
      }
      ++ui;
    }
    if (!found)
      index=0;
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
      _Universe->AccessCamera(CP_VIEWTARGET)->SetOrientation(tmp,q,r);
      _Universe->AccessCamera(CP_TARGET)->SetOrientation(tmp,q,r);
      _Universe->AccessCamera(CP_PANTARGET)->SetOrientation(tmp,q,r);
      ShoveCamBehindUnit (CP_TARGET,tgt,zoomfactor);
    }else {
      un->UpdateHudMatrix (CP_VIEWTARGET);
      un->UpdateHudMatrix (CP_TARGET);
      un->UpdateHudMatrix (CP_PANTARGET);
    }
    ShoveCamBehindUnit (CP_CHASE,un,zoomfactor);
    ShoveCamBehindUnit (CP_PANTARGET,un,zoomfactor);

    ShoveCamBehindUnit (CP_PAN,un,zoomfactor);

    un->SetVisible(view>=CP_CHASE);

  }
  _Universe->activeStarSystem()->AccessCamera()->UpdateGFX(clip?GFXTRUE:GFXFALSE);
    
  //  parent->UpdateHudMatrix();
}
