
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "gauge.h"
#include "cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_factory.h"
#include "cmd/iterator.h"
#include "cmd/collection.h"
#include "cmd/unit_util.h"
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
#include "main_loop.h"
#include <assert.h>	// needed for assert() calls
#include "savegame.h"
#include "animation.h"
#include "mesh.h"
#include "universe_util.h"
#include "in_mouse.h"
#include "gui/glut_support.h"
#include "networking/netclient.h"
#include "audiolib.h"
#include "save_util.h"
#include "cmd/base.h"
extern float rand01();
#define SWITCH_CONST .9

static soundContainer disableautosound;
static soundContainer enableautosound;

void soundContainer::loadsound (string soundfile,bool looping) {
	if (this->sound==-2&&soundfile.size()) {
		string sound=GameCockpit::getsoundfile(soundfile);
		if (sound.size()) {
			this->sound=AUDCreateSoundWAV(sound,looping);
		} else {
			this->sound=-1;
		}
	}
}
void soundContainer::playsound () {
	if (sound>=0) {
		AUDAdjustSound(sound,QVector(0,0,0),Vector(0,0,0));
		AUDPlay (sound,QVector(0,0,0),Vector(0,0,0),1);
	}
}
soundContainer::~soundContainer () {
	if (sound>=0) {
#if 0
		AUDStopPlaying(sound);
		AUDDeleteSound(sound,false);
#endif
		sound=-2;
	}
}

void GameCockpit::ReceivedTargetInfo()
{
  for (int j=0;j<vdu.size();j++) {
	vdu[j]->ReceivedTargetData();
  }
}

void DrawRadarCircles (float x, float y, float wid, float hei, const GFXColor &col) {
	GFXColorf(col);
	GFXEnable(SMOOTH);
	GFXCircle (x,y,wid/2,hei/2);
	GFXCircle (x,y,wid/2.4,hei/2.4);
	GFXCircle (x,y,wid/6,hei/6);
	const float sqrt2=sqrt( (double)2)/2;
	GFXBegin(GFXLINE);
		GFXVertex3f(x+(wid/6)*sqrt2,y+(hei/6)*sqrt2,0);
		GFXVertex3f(x+(wid/2.4)*sqrt2,y+(hei/2.4)*sqrt2,0);
		GFXVertex3f(x-(wid/6)*sqrt2,y+(hei/6)*sqrt2,0);
		GFXVertex3f(x-(wid/2.4)*sqrt2,y+(hei/2.4)*sqrt2,0);
		GFXVertex3f(x-(wid/6)*sqrt2,y-(hei/6)*sqrt2,0);
		GFXVertex3f(x-(wid/2.4)*sqrt2,y-(hei/2.4)*sqrt2,0);
		GFXVertex3f(x+(wid/6)*sqrt2,y-(hei/6)*sqrt2,0);
		GFXVertex3f(x+(wid/2.4)*sqrt2,y-(hei/2.4)*sqrt2,0);
	GFXEnd();
	GFXDisable(SMOOTH);
}
 void GameCockpit::LocalToRadar (const Vector & pos, float &s, float &t) {
  s = (pos.k>0?pos.k:0)+1;
  t = 2*sqrtf(pos.i*pos.i + pos.j*pos.j + s*s);
  s = -pos.i/t;
  t = pos.j/t;
}

void GameCockpit::SetSoundFile (string sound) {
	soundfile=AUDCreateSoundWAV (sound, false);
}

void GameCockpit::LocalToEliteRadar (const Vector & pos, float &s, float &t,float &h){
  s=-pos.i/1000.0;
  t=pos.k/1000.0;
  h=pos.j/1000.0;
}


GFXColor GameCockpit::unitToColor (Unit *un,Unit *target) {
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
	static bool reltocolor=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","DrawTheirRelationColor","true"));
	if (reltocolor) {
	  return relationToColor(target->getRelation(un));
	}else {
	  return relationToColor(un->getRelation(target));
	}
}

GFXColor GameCockpit::relationToColor (float relation) {
 if (relation>0) {
    return GFXColor (relation*friendly.r+(1-relation)*neutral.r,relation*friendly.g+(1-relation)*neutral.g,relation*friendly.b+(1-relation)*neutral.b,relation*friendly.a+(1-relation)*neutral.a);
  } 
 else if(relation==0){
   return GFXColor(neutral.r,neutral.g,neutral.b,neutral.a);
}else { 
    return GFXColor (-relation*enemy.r+(1+relation)*neutral.r,-relation*enemy.g+(1+relation)*neutral.g,-relation*enemy.b+(1+relation)*neutral.b,-relation*enemy.a+(1+relation)*neutral.a);
  }
}
void GameCockpit::DrawNavigationSymbol (const Vector &Loc, const Vector & P, const Vector & Q, float size) {
  GFXColor4f (1,1,1,1);
  static bool draw_nav_symbol=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawNavSymbol","false"));
  if (draw_nav_symbol) {
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
}

float GameCockpit::computeLockingSymbol(Unit * par) {
  return par->computeLockingPercent();
}
inline void DrawOneTargetBox (const QVector & Loc, float rSize, const Vector &CamP, const Vector & CamQ, const Vector & CamR, float lock_percent, bool ComputerLockon, bool Diamond=false) {

  static float rat = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","min_target_box_size",".01"));
  float len = (Loc).Magnitude();
  float curratio = rSize/len;
  if (curratio<rat) 
    rSize = len*rat;
  if (Diamond) {
    float ModrSize=rSize/1.41;
    GFXBegin (GFXLINESTRIP); 
    GFXVertexf (Loc+(.75*CamP+CamQ).Cast()*ModrSize);
    GFXVertexf (Loc+(CamP+.75*CamQ).Cast()*ModrSize);
    GFXVertexf (Loc+(CamP-.75*CamQ).Cast()*ModrSize);
    GFXVertexf (Loc+(.75*CamP-CamQ).Cast()*ModrSize);
    GFXVertexf (Loc+(-.75*CamP-CamQ).Cast()*ModrSize);
    GFXVertexf (Loc+(-CamP-.75*CamQ).Cast()*ModrSize);
    GFXVertexf (Loc+(.75*CamQ-CamP).Cast()*ModrSize);
    GFXVertexf (Loc+(CamQ-.75*CamP).Cast()*ModrSize);
    GFXVertexf (Loc+(.75*CamP+CamQ).Cast()*ModrSize);
    GFXEnd();    
  }else if (ComputerLockon) {
    GFXBegin (GFXLINESTRIP); 
    GFXVertexf (Loc+(CamP+CamQ).Cast()*rSize);
    GFXVertexf (Loc+(CamP-CamQ).Cast()*rSize);
    GFXVertexf (Loc+(-CamP-CamQ).Cast()*rSize);
    GFXVertexf (Loc+(CamQ-CamP).Cast()*rSize);
    GFXVertexf (Loc+(CamP+CamQ).Cast()*rSize);
    GFXEnd();
  }else {
    GFXBegin(GFXLINE);
    GFXVertexf (Loc+(CamP+CamQ).Cast()*rSize);
    GFXVertexf (Loc+(CamP+.66*CamQ).Cast()*rSize);

    GFXVertexf (Loc+(CamP-CamQ).Cast()*rSize);
    GFXVertexf (Loc+(CamP-.66*CamQ).Cast()*rSize);

    GFXVertexf (Loc+(-CamP-CamQ).Cast()*rSize);
    GFXVertexf (Loc+(-CamP-.66*CamQ).Cast()*rSize);


    GFXVertexf (Loc+(CamQ-CamP).Cast()*rSize);
    GFXVertexf (Loc+(CamQ-.66*CamP).Cast()*rSize);


    GFXVertexf (Loc+(CamP+CamQ).Cast()*rSize);
    GFXVertexf (Loc+(CamP+.66*CamQ).Cast()*rSize);


    GFXVertexf (Loc+(CamP+CamQ).Cast()*rSize);
    GFXVertexf (Loc+(.66*CamP+CamQ).Cast()*rSize);


    GFXVertexf (Loc+(CamP-CamQ).Cast()*rSize);
    GFXVertexf (Loc+(.66*CamP-CamQ).Cast()*rSize);

    GFXVertexf (Loc+(-CamP-CamQ).Cast()*rSize);
    GFXVertexf (Loc+(-.66*CamP-CamQ).Cast()*rSize);

    GFXVertexf (Loc+(CamQ-CamP).Cast()*rSize);
    GFXVertexf (Loc+(.66*CamQ-CamP).Cast()*rSize);

    GFXVertexf (Loc+(CamP+CamQ).Cast()*rSize);
    GFXVertexf (Loc+(.66*CamP+CamQ).Cast()*rSize);

    GFXEnd();
  }
  if (lock_percent<.99) {
    if (lock_percent<0) {
      lock_percent=0;
    }
    float max=2.05;
    //    VSFileSystem::Fprintf (stderr,"lock percent %f\n",lock_percent);
    float coord = 1.05+(max-1.05)*lock_percent;//rSize/(1-lock_percent);//this is a number between 1 and 100
   
    double rtot = 1./sqrtf(2);
    float theta = 4*M_PI*lock_percent;
    Vector LockBox (-cos(theta)*rtot,-rtot,sin(theta)*rtot);
    //    glLineWidth (4);
    Vector TLockBox (rtot*LockBox.i+rtot*LockBox.j,rtot*LockBox.j-rtot*LockBox.i,LockBox.k);
    Vector SLockBox (TLockBox.j,TLockBox.i,TLockBox.k);
    QVector Origin = (CamP+CamQ).Cast()*(rSize*coord);
    TLockBox = (TLockBox.i*CamP+TLockBox.j*CamQ+TLockBox.k*CamR);
    SLockBox = (SLockBox.i*CamP+SLockBox.j*CamQ+SLockBox.k*CamR);
    double r1Size = rSize*.58;
    GFXBegin (GFXLINESTRIP);
    max*=rSize*.75;
    if (lock_percent==0) {
      GFXVertexf (Loc+CamQ.Cast()*max*1.5);
      GFXVertexf (Loc+CamQ.Cast()*max);
    }

    GFXVertexf (Loc+Origin+(TLockBox.Cast()*r1Size));
    GFXVertexf (Loc+Origin);
    GFXVertexf (Loc+Origin+(SLockBox.Cast()*r1Size));
    if (lock_percent==0) {
      GFXVertexf (Loc+CamP.Cast()*max);
      GFXVertexf (Loc+CamP.Cast()*max*1.5);
      GFXEnd();
      GFXBegin(GFXLINESTRIP);
      GFXVertexf (Loc-CamP.Cast()*max);
    }else {
      GFXEnd();
      GFXBegin(GFXLINESTRIP);
    }
    GFXVertexf (Loc-Origin-(SLockBox.Cast()*r1Size));
    GFXVertexf (Loc-Origin);
    GFXVertexf (Loc-Origin-(TLockBox.Cast()*r1Size));

    Origin=(CamP-CamQ).Cast()*(rSize*coord);
    if (lock_percent==0) {
      GFXVertexf (Loc-CamQ.Cast()*max);
      GFXVertexf (Loc-CamQ.Cast()*max*1.5);
      GFXVertexf (Loc-CamQ.Cast()*max);
    }else {
      GFXEnd();
      GFXBegin(GFXLINESTRIP);
    }

    GFXVertexf (Loc+Origin+(TLockBox.Cast()*r1Size));
    GFXVertexf (Loc+Origin);
    GFXVertexf (Loc+Origin-(SLockBox.Cast()*r1Size));
    if (lock_percent==0) {
      GFXVertexf (Loc+CamP.Cast()*max);
      GFXEnd();
      GFXBegin(GFXLINESTRIP);
      GFXVertexf (Loc-CamP.Cast()*max*1.5);
      GFXVertexf (Loc-CamP.Cast()*max);
    }else {
      GFXEnd();
      GFXBegin(GFXLINESTRIP);
    }

    GFXVertexf (Loc-Origin+(SLockBox.Cast()*r1Size));
    GFXVertexf (Loc-Origin);
    GFXVertexf (Loc-Origin-(TLockBox.Cast()*r1Size));

    if (lock_percent==0) {
      GFXVertexf (Loc+CamQ.Cast()*max);
    }
    GFXEnd();
    glLineWidth (1);
  }

}

static GFXColor DockBoxColor (const string& name) {
  GFXColor dockbox;
  vs_config->getColor(name,&dockbox.r);    
  return dockbox;
}
inline void DrawDockingBoxes(Unit * un,Unit *target, const Vector & CamP, const Vector & CamQ, const Vector & CamR) {
  if (target->IsCleared (un)) {
    static GFXColor dockboxstop = DockBoxColor("docking_box_halt");
    static GFXColor dockboxgo = DockBoxColor("docking_box_proceed");
    if (dockboxstop.r==1&&dockboxstop.g==1&&dockboxstop.b==1) {
      dockboxstop.r=1;
      dockboxstop.g=0;
      dockboxstop.b=0;
    }
    if (dockboxgo.r==1&&dockboxgo.g==1&&dockboxgo.b==1) {
      dockboxgo.r=0;
      dockboxgo.g=1;
      dockboxgo.b=.5;
    }

    const vector <DockingPorts> d = target->DockingPortLocations();
    for (unsigned int i=0;i<d.size();i++) {
      float rad = d[i].radius/sqrt(2.0);
      GFXDisable (DEPTHTEST);
      GFXDisable (DEPTHWRITE);
      GFXColorf (dockboxstop);
      DrawOneTargetBox (Transform (target->GetTransformation(),d[i].pos.Cast())-_Universe->AccessCamera()->GetPosition(),rad ,CamP, CamQ, CamR,1,true,true);
      GFXEnable (DEPTHTEST);
      GFXEnable (DEPTHWRITE);
      GFXColorf (dockboxgo);
      DrawOneTargetBox (Transform (target->GetTransformation(),d[i].pos.Cast())-_Universe->AccessCamera()->GetPosition(),rad ,CamP, CamQ, CamR,1,true,true);

    }
    GFXDisable (DEPTHTEST);
    GFXDisable (DEPTHWRITE);
    GFXColor4f(1,1,1,1);

  }
}

void GameCockpit::DrawTargetBoxes(){
  
  Unit * un = parent.GetUnit();
  if (!un)
    return;
  if (un->GetNebula()!=NULL)
    return;

  StarSystem *ssystem=_Universe->activeStarSystem();
  UnitCollection *unitlist=&ssystem->getUnitList();
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
        QVector Loc(target->Position());

	GFXColor drawcolor=unitToColor(un,target);
	GFXColorf(drawcolor);

	if(target->isUnit()==UNITPTR){

	  if (un->Target()==target) {
	    DrawDockingBoxes(un,target,CamP,CamQ, CamR);
	    DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),true);
	  }else {
	    DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),false);
	  }
	}
    }
    target=(++uiter);
  }

  GFXEnable (TEXTURE0);

}


void GameCockpit::DrawTargetBox () {
  float speed,range;
  static GFXColor black_and_white=DockBoxColor ("black_and_white"); 
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
  QVector Loc(target->Position()-_Universe->AccessCamera()->GetPosition());
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);
  DrawNavigationSymbol (un->GetComputerData().NavPoint,CamP,CamQ, CamR.Cast().Dot((un->GetComputerData().NavPoint).Cast()-_Universe->AccessCamera()->GetPosition()));
  GFXColorf (un->GetComputerData().radar.color?unitToColor(un,target):black_and_white);

  if(draw_line_to_target){
    QVector my_loc(_Universe->AccessCamera()->GetPosition());
    GFXBegin(GFXLINESTRIP);
    GFXVertexf(my_loc);
    GFXVertexf(Loc);
    
    Unit *targets_target=target->Target();
    if(draw_line_to_targets_target && targets_target!=NULL){
      QVector ttLoc(targets_target->Position());
      GFXVertexf(ttLoc);
    }
    GFXEnd();
  }
  DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),un->TargetLocked());
  DrawDockingBoxes(un,target,CamP,CamQ,CamR);
  if (always_itts || un->GetComputerData().itts) {
	float mrange;
    un->getAverageGunSpeed (speed,range,mrange);
    float err = (.01*(1-un->CloakVisible()));
   QVector iLoc = target->PositionITTS (un->Position(),un->cumulative_velocity,speed,steady_itts)-_Universe->AccessCamera()->GetPosition()+10*err*QVector (-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX,-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX,-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX);
    
    GFXBegin (GFXLINESTRIP);
    if(draw_line_to_itts){
      GFXVertexf(Loc);
      GFXVertexf(iLoc);
    }
    GFXVertexf (iLoc+(CamP.Cast())*.25*un->rSize());
    GFXVertexf (iLoc+(-CamQ.Cast())*.25*un->rSize());
    GFXVertexf (iLoc+(-CamP.Cast())*.25*un->rSize());
    GFXVertexf (iLoc+(CamQ.Cast())*.25*un->rSize());
    GFXVertexf (iLoc+(CamP.Cast())*.25*un->rSize());
    GFXEnd();
  }
  GFXEnable (TEXTURE0);
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);

}

void GameCockpit::DrawTurretTargetBoxes () {

  static GFXColor black_and_white=DockBoxColor ("black_and_white");
  Unit * parun = parent.GetUnit();
  if (!parun)
    return;
  UnitCollection::UnitIterator iter = parun->getSubUnits();
  Unit * un;
  while (NULL!=(un=iter.current())) {
	if (!un)
      return;
	if (un->GetNebula()!=NULL)
	  return;
    Unit *target = un->Target();
    if (!target){
      iter.advance();
      continue;
    }
	Vector CamP,CamQ,CamR;
    _Universe->AccessCamera()->GetPQR(CamP,CamQ,CamR);
    //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
    QVector Loc(target->Position()-_Universe->AccessCamera()->GetPosition());
    GFXDisable (TEXTURE0);
    GFXDisable (TEXTURE1);
    GFXDisable (DEPTHTEST);
    GFXDisable (DEPTHWRITE);
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXDisable (LIGHTING);
    DrawNavigationSymbol (un->GetComputerData().NavPoint,CamP,CamQ, CamR.Cast().Dot((un->GetComputerData().NavPoint).Cast()-_Universe->AccessCamera()->GetPosition()));
    GFXColorf (un->GetComputerData().radar.color?unitToColor(un,target):black_and_white);

    //DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),un->TargetLocked());

	// ** jay
	float rSize = target->rSize();
	GFXBegin(GFXLINE);
	GFXVertexf (Loc+(CamP).Cast()*rSize*1.3);
    GFXVertexf (Loc+(CamP).Cast()*rSize*.8);

	GFXVertexf (Loc+(-CamP).Cast()*rSize*1.3);
    GFXVertexf (Loc+(-CamP).Cast()*rSize*.8);

	GFXVertexf (Loc+(CamQ).Cast()*rSize*1.3);
    GFXVertexf (Loc+(CamQ).Cast()*rSize*.8);

	GFXVertexf (Loc+(-CamQ).Cast()*rSize*1.3);
    GFXVertexf (Loc+(-CamQ).Cast()*rSize*.8);
	GFXEnd();


    GFXEnable (TEXTURE0);
    GFXEnable (DEPTHTEST);
    GFXEnable (DEPTHWRITE);

    iter.advance();
  }

}


void GameCockpit::drawUnToTarget ( Unit * un, Unit* target,float xcent,float ycent, float xsize, float ysize, bool reardar){
  static GFXColor black_and_white=DockBoxColor ("black_and_white"); 
      Vector localcoord (un->LocalCoordinates(target));
	  if (reardar)
		  localcoord.k=-localcoord.k;
	  float s,t;
      this->LocalToRadar (localcoord,s,t);
      GFXColor localcol (un->GetComputerData().radar.color?this->unitToColor (un,target):black_and_white);
      
      GFXColorf (localcol);
      
      float rerror = ((un->GetNebula()!=NULL)?.03:0)+(target->GetNebula()!=NULL?.06:0);
      Vector v(xcent+xsize*(s-.5*rerror+(rerror*rand())/RAND_MAX),ycent+ysize*(t+-.5*rerror+(rerror*rand())/RAND_MAX),0);
      GFXVertexf(v);
      if (target==un->Target()) {
	//	GFXEnd();
	//	GFXBegin(GFXLINE);
	GFXVertexf(v);
	GFXVertex3f((float)(v.i+(7.8)/g_game.x_resolution),v.j,v.k); //I need to tell it to use floats...
	GFXVertex3f((float)(v.i-(7.5)/g_game.x_resolution),v.j,v.k); //otherwise, it gives an error about
	GFXVertex3f(v.i,(float)(v.j-(7.5)/g_game.y_resolution),v.k); //not knowning whether to use floats
	GFXVertex3f(v.i,(float)(v.j+(7.8)/g_game.y_resolution),v.k); //or doubles.

	GFXVertex3f((float)(v.i+(3.9)/g_game.x_resolution),v.j,v.k); //I need to tell it to use floats...
	GFXVertex3f((float)(v.i-(3.75)/g_game.x_resolution),v.j,v.k); //otherwise, it gives an error about
	GFXVertex3f(v.i,(float)(v.j-(3.75)/g_game.y_resolution),v.k); //not knowning whether to use floats
	GFXVertex3f(v.i,(float)(v.j+(3.9)/g_game.y_resolution),v.k); //or doubles.

	//	GFXEnd();
	//	GFXBegin (GFXPOINT);
	
      }   
}

void GameCockpit::Eject() {
  ejecting=true;
}
void GameCockpit::DrawBlips (Unit * un) {


  Unit::Computer::RADARLIM * radarl = &un->GetComputerData().radar;

  UnitCollection * drawlist = &_Universe->activeStarSystem()->getUnitList();
  un_iter iter = drawlist->createIterator();

  Unit * target;
  Unit * makeBigger = un->Target();
  float s,t;
  
  float xsize[2],ysize[2],xcent[2],ycent[2];
  if (Radar[0]) {
	  Radar[0]->GetSize (xsize[0],ysize[0]);
	 xsize[0] = fabs (xsize[0]);
	 ysize[0] = fabs (ysize[0]);
	Radar[0]->GetPosition (xcent[0],ycent[0]);
  }
  if (Radar[1]) {
	Radar[1]->GetSize (xsize[1],ysize[1]);
	xsize[1] = fabs (xsize[1]);
	ysize[1] = fabs (ysize[1]);
	Radar[1]->GetPosition (xcent[1],ycent[1]);
  }
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  if (Radar[0]) 
  if ((!g_game.use_sprites)||Radar[0]->LoadSuccess()) {
    DrawRadarCircles (xcent[0],ycent[0],xsize[0],ysize[0],textcol);
  }
  if (Radar[1])
  if ((!g_game.use_sprites)||Radar[1]->LoadSuccess()) {
    DrawRadarCircles (xcent[1],ycent[1],xsize[1],ysize[1],textcol);
  }
  GFXPointSize (2);
  GFXBegin(GFXPOINT);
  while ((target = iter.current())!=NULL) {
    if (target!=un) {
      double dist;
      if (!un->InRange (target,dist,makeBigger==target,true,true)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter.advance();	
	continue;
      }
	  if (Radar[0])
		  drawUnToTarget (un,target,xcent[0],ycent[0],xsize[0],ysize[0],false);
	  if (Radar[1])
		  drawUnToTarget (un,target,xcent[1],ycent[1],xsize[1],ysize[1],true);
	  if (target->isPlanet()==PLANETPTR) {
		  Unit * sub=NULL;
		  for (un_iter i=target->getSubUnits();(sub=*i)!=NULL;++i) {
			  if (Radar[0])
			  drawUnToTarget(un,sub,xcent[0],ycent[0], xsize[0],ysize[0],false);
			  if (Radar[1])
			  drawUnToTarget(un,sub,xcent[1],ycent[1], xsize[1],ysize[1],true);
		  }
	  }
    }
    iter.advance();
  }
  GFXEnd();
  GFXPointSize (1);
  GFXColor4f (1,1,1,1);
  GFXEnable (TEXTURE0);
}

void GameCockpit::DrawEliteBlips (Unit * un) {
	if (!Radar[0])
		return;
  static GFXColor black_and_white=DockBoxColor ("black_and_white"); 
  Unit::Computer::RADARLIM * radarl = &un->GetComputerData().radar;
  UnitCollection * drawlist = &_Universe->activeStarSystem()->getUnitList();
  un_iter iter = drawlist->createIterator();
  Unit * target;
  Unit * makeBigger = un->Target();
  float s,t,es,et,eh;
  float xsize,ysize,xcent,ycent;
  Radar[0]->GetSize (xsize,ysize);
  xsize = fabs (xsize);
  ysize = fabs (ysize);
  Radar[0]->GetPosition (xcent,ycent);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  if (Radar[0]->LoadSuccess()) {
    DrawRadarCircles (xcent,ycent,xsize,ysize,textcol);
  }
  while ((target = iter.current())!=NULL) {
    if (target!=un) {
      double mm;

      if (!un->InRange (target,mm,(makeBigger==target),true,true)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter.advance();	
	continue;
      }
      Vector localcoord (un->LocalCoordinates(target));

	LocalToRadar (localcoord,s,t);
	LocalToEliteRadar(localcoord,es,et,eh);


      GFXColor localcol (radarl->color?unitToColor (un,target):black_and_white);
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
float GameCockpit::LookupTargetStat (int stat, Unit *target) {
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
  static bool lie=XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
  static float fpsval=0;
  const float fpsmax=1;
  static float numtimes=fpsmax;
  float armordat[8]; //short fix
  float retval;
  int armori;
  Unit * tmpunit;
  switch (stat) {
  case UnitImages::SHIELDF:
    return target->FShieldData();
  case UnitImages::SHIELDR:
    return target->RShieldData();
  case UnitImages::SHIELDL:
    return target->LShieldData();
  case UnitImages::SHIELDB:
    return target->BShieldData();
  case UnitImages::ARMORF:
  case UnitImages::ARMORR:
  case UnitImages::ARMORL:
  case UnitImages::ARMORB:
    target->ArmorData (armordat);
	for (armori=0;armori<8;++armori) {	   
		if (armordat[armori]>StartArmor[armori]) {
			StartArmor[armori]=armordat[armori];
		}
		armordat[armori]/=StartArmor[armori];		
	}
	switch (stat) {
	case UnitImages::ARMORR:
		return .25*(armordat[0]+armordat[1]+armordat[4]+armordat[5]);	   
	case UnitImages::ARMORL:
		return .25*(armordat[2]+armordat[3]+armordat[6]+armordat[7]);		
	case UnitImages::ARMORB:
		return .25*(armordat[1]+armordat[3]+armordat[5]+armordat[7]);
	case UnitImages::ARMORF:
	default:
		return .25*(armordat[0]+armordat[2]+armordat[4]+armordat[6]);		
	}
  case UnitImages::FUEL:
	if (target->FuelData()>maxfuel)
		maxfuel=target->FuelData();
    if (maxfuel>0) return target->FuelData()/maxfuel;return 0;
  case UnitImages::ENERGY:
    return target->EnergyData();
  case UnitImages::WARPENERGY:
    return target->WarpEnergyData();
  case UnitImages::HULL:
    if (maxhull<target->GetHull()) {
      maxhull = target->GetHull();
    }
    return target->GetHull()/maxhull;
  case UnitImages::EJECT:
    {
    int go=(((target->GetHull()/maxhull)<.25)&&(target->BShieldData()<.25||target->FShieldData()<.25))?1:0;
    static int overload=0;
    if (overload!=go) {
		if (go==0) {
			static soundContainer ejectstopsound;
			if (ejectstopsound.sound<0) {
				static string str=vs_config->getVariable("cockpitaudio","overload_stopped","overload_stopped");
				ejectstopsound.loadsound(str);
			}
			ejectstopsound.playsound();
		} else {
			static soundContainer ejectsound;
			if (ejectsound.sound<0) {
				static string str=vs_config->getVariable("cockpitaudio","overload","overload");
				ejectsound.loadsound(str);
			}
			ejectsound.playsound();
		}
		overload=go;
	}
    return go;
	}
  case UnitImages::LOCK:
    if  ((tmpunit = target->GetComputerData().threat.GetUnit())) {
      return (tmpunit->cosAngleTo (target,*&armordat[0],FLT_MAX,FLT_MAX)>.95);
    }
    return 0;
  case UnitImages::MISSILELOCK:
	  if (target->graphicOptions.missilelock)
		  return 1;
	  return 0;
  case UnitImages::COLLISION:{
	  static double collidepanic = XMLSupport::parse_float (vs_config->getVariable("physics","collision_inertial_time","1.25"));
	  return (getNewTime()-TimeOfLastCollision)<collidepanic;
	  break;
  }
  case UnitImages::ECM:
	  return target->GetImageInformation().ecm>0?1:0;
  case UnitImages::WARPFIELDSTRENGTH:
          return target->graphicOptions.WarpFieldStrength;
  case UnitImages::JUMP:
	  return jumpok?1:0;
  case UnitImages::KPS:
	if (lie) 
		return (target->GetVelocity().Magnitude())/game_speed;
	else
		return target->GetVelocity().Magnitude()*3.6;
  case UnitImages::SETKPS:
	if (lie) 
	    return target->GetComputerData().set_speed/game_speed;
	else
		return target->GetComputerData().set_speed*3.6;
  case UnitImages::AUTOPILOT:
    {
    static int wasautopilot=0;
	int abletoautopilot=0;
	static bool auto_valid = XMLSupport::parse_bool (vs_config->getVariable ("physics","insystem_jump_or_timeless_auto-pilot","false"));	
    if (target) {
		if (!auto_valid) {
			abletoautopilot=(target->graphicOptions.InWarp);
		}else {
			abletoautopilot=(target->AutoPilotTo(target,false)?1:0);
		}
    }
	if (abletoautopilot!=wasautopilot) {
		if (abletoautopilot==0) {
			static soundContainer autostopsound;
			if (autostopsound.sound<0) {
				static string str=vs_config->getVariable("cockpitaudio","autopilot_available","autopilot_available");
				autostopsound.loadsound(str);
			}
			autostopsound.playsound();
		} else {
			static soundContainer autosound;
			if (autosound.sound<0) {
				static string str=vs_config->getVariable("cockpitaudio","autopilot_unavailable","autopilot_unavailable");
				autosound.loadsound(str);
			}
			autosound.playsound();
		}
		wasautopilot=abletoautopilot;
	}
    return abletoautopilot;
    }
  case UnitImages::COCKPIT_FPS:
    if (fpsval>=0&&fpsval<.5*FLT_MAX)
      numtimes-=.1+fpsval;
    if (numtimes<=0) {
      numtimes = fpsmax;
      fpsval = GetElapsedTime();
    }
	if( fpsval)
   	 return 1./fpsval;
  }
  return 1;
}
void GameCockpit::DrawGauges(Unit * un) {

  int i;
  for (i=0;i<UnitImages::KPS;i++) {
    if (gauges[i]) {
      gauges[i]->Draw(LookupTargetStat (i,un));
/*      if (rand01()>un->GetImageInformation().cockpit_damage[0]) {
        static Animation gauge_ani("static.ani",true,.1,BILINEAR);
        gauge_ani.DrawAsVSSprite(Radar);
      }*/
      float damage = un->GetImageInformation().cockpit_damage[(1+MAXVDUS+i)%(MAXVDUS+1+UnitImages::NUMGAUGES)];
      if (gauge_time[i]>=0) {
        if (damage>.0001&&(cockpit_time>(gauge_time[i]+(1-damage)))) {
	  if (rand01()>SWITCH_CONST) {
            gauge_time[i]=-cockpit_time;
          }
        } else {
          static Animation vdu_ani("static.ani",true,.1,BILINEAR);
          vdu_ani.DrawAsVSSprite(gauges[i]);	
        }
      } else {
        if (cockpit_time>(((1-(-gauge_time[i]))+damage))) {
	      if (rand01()>SWITCH_CONST) {
            gauge_time[i]=cockpit_time;
          }
        }
      }
    }
  }
  if (!text)
	  return;
  text->SetSize (2,-2);
  GFXColorf (textcol);
  for (i=UnitImages::KPS;i<UnitImages::NUMGAUGES;i++) {
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
void GameCockpit::Init (const char * file) {
  Cockpit::Init( file);
  if (Panel.size()>0) {
    float x,y;
    Panel.front()->GetPosition (x,y);
    Panel.front()->SetPosition (x,y+viewport_offset);  
  }

  
}
void GameCockpit::Delete () {
  int i;
  if (text) {
    delete text;
    text = NULL;
  }
  if (mesh) {
    delete mesh;
    mesh = NULL;
  }
  if (soundfile>=0) {
	  AUDStopPlaying(soundfile);
	  AUDDeleteSound(soundfile,false);
	  soundfile=-1;
  }
  viewport_offset=cockpit_offset=0;
  for (i=0;i<4;i++) {
    if (Pit[i]) {
      delete Pit[i];
      Pit[i] = NULL;
    }
  }
  for (i=0;i<UnitImages::NUMGAUGES;i++) {
    if (gauges[i]) {
      delete gauges[i];
      gauges[i]=NULL;
    }
  }
  if (Radar[0]) {
    delete Radar[0];
    Radar[0] = NULL;
  }
  if (Radar[1]) {
    delete Radar[1];
    Radar[1] = NULL;
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
void GameCockpit::InitStatic () {  
  int i;
  for (i=0;i<UnitImages::NUMGAUGES;i++) {
    gauge_time[i]=0;
  }
  for (i=0;i<MAXVDUS;i++) {
    vdu_time[i]=0;
  }

  radar_time=0;
  cockpit_time=0;
}

/***** WARNING CHANGED ORDER *****/
GameCockpit::GameCockpit (const char * file, Unit * parent,const std::string &pilot_name): Cockpit( file, parent, pilot_name),textcol (1,1,1,1),text(NULL)
{
  static int headlag = XMLSupport::parse_int (vs_config->getVariable("graphics","head_lag","10"));
  int i;
  for (i=0;i<headlag;i++) {
    headtrans.push_back (Matrix());
    Identity(headtrans.back());
  }
  for (i=0;i<UnitImages::NUMGAUGES;i++) {
    gauges[i]=NULL;
  }
  mesh=NULL;
  Radar[0]=Radar[1]=Pit[0]=Pit[1]=Pit[2]=Pit[3]=NULL;

  draw_all_boxes=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAllTargetBoxes","false"));
  draw_line_to_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTarget","false"));
  draw_line_to_targets_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTargetsTarget","false"));
  draw_line_to_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToITTS","false"));
  always_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAlwaysITTS","false"));
  steady_itts=XMLSupport::parse_bool(vs_config->getVariable ("physics","steady_itts","false"));
  radar_type=vs_config->getVariable("graphics","hud","radarType","WC");

  // Compute the screen limits. Used to display the arrow pointing to the selected target.
  projection_limit_y = XMLSupport::parse_float(vs_config->getVariable("graphics","fov","78"));
  // The angle betwwen the center of the screen and the border is half the fov.
  projection_limit_y = tan(projection_limit_y * M_PI / (180*2));
  projection_limit_x = projection_limit_y * g_game.aspect;
  // Precompute this division... performance.
  inv_screen_aspect_ratio = 1 / g_game.aspect;

  friendly=GFXColor(-1,-1,-1,-1);
  enemy=GFXColor(-1,-1,-1,-1);
  neutral=GFXColor(-1,-1,-1,-1);
  targeted=GFXColor(-1,-1,-1,-1);
  targetting=GFXColor(-1,-1,-1,-1);
  planet=GFXColor(-1,-1,-1,-1);
  soundfile=-1;
  if (friendly.r==-1) {
    vs_config->getColor ("enemy",&enemy.r);
    vs_config->getColor ("friend",&friendly.r);
    vs_config->getColor ("neutral",&neutral.r);
    vs_config->getColor("target",&targeted.r);
    vs_config->getColor("targetting_ship",&targetting.r);
    vs_config->getColor("planet",&planet.r);
  }
  InitStatic();

}
void GameCockpit::SelectProperCamera () {
    SelectCamera(view);
}
extern vector <int> respawnunit;
extern vector <int> switchunit;
extern vector <int> turretcontrol;
extern vector <int> suicide;

void DoCockpitKeys()
{
	CockpitKeys::Pan(0,PRESS);
	CockpitKeys::Inside(0,PRESS);
}
void GameCockpit::NavScreen (int, KBSTATE k) // scheherazade
{
   if (k==PRESS)
     {
      //UniverseUtil::IOmessage(0,"game","all","hit key");
 
    if((_Universe->AccessCockpit())->CanDrawNavSystem())
      {
       (_Universe->AccessCockpit())->SetDrawNavSystem(0);
       //UniverseUtil::IOmessage(0,"game","all","DRAWNAV - OFF");
	   RestoreMouse();
      }
  else
    {
     (_Universe->AccessCockpit())->SetDrawNavSystem(1);
     //UniverseUtil::IOmessage(0,"game","all","DRAWNAV - ON");
	 
	 winsys_set_mouse_func(BaseInterface::ClickWin);
	 winsys_set_motion_func(BaseInterface::ActiveMouseOverWin);
	 winsys_set_passive_motion_func(BaseInterface::PassiveMouseOverWin);


//	 _Universe->AccessCockpit()->ThisNav.currentsystem=_Universe->AccessCockpit()->ThisNav.selectedsystem=UniverseUtil::getSystemFile();
    }
  }
}
bool GameCockpit::SetDrawNavSystem(bool what) {
  ThisNav.SetDraw(what);
  return what;
}
bool GameCockpit::CanDrawNavSystem() {
  return ThisNav.CheckDraw();
}
bool GameCockpit::DrawNavSystem() {

  
  bool ret = ThisNav.CheckDraw();
  if (ret) {
    float c_o = cockpit_offset;
    cam[currentcamera].setCockpitOffset(0);
    cam[currentcamera].UpdateGFX (GFXFALSE,GFXFALSE,GFXTRUE);
    ThisNav.Draw();
    cockpit_offset= c_o;
    cam[currentcamera].setCockpitOffset(c_o);
    cam[currentcamera].UpdateGFX (GFXFALSE,GFXFALSE,GFXTRUE);
    
  }


  return ret;
}
void RespawnNow (Cockpit * cp) {
  while (respawnunit.size()<=_Universe->numPlayers())
    respawnunit.push_back(0);
  for (unsigned int i=0;i<_Universe->numPlayers();i++) {
    if (_Universe->AccessCockpit(i)==cp) {
      respawnunit[i]=2;
    }
  }
}
void GameCockpit::SwitchControl (int,KBSTATE k) {
  if (k==PRESS) {
    while (switchunit.size()<=_Universe->CurrentCockpit())
      switchunit.push_back(0);
    switchunit[_Universe->CurrentCockpit()]=1;
  }

}
void SuicideKey (int, KBSTATE k) {
  if (k==PRESS) {
    while (suicide.size()<=_Universe->CurrentCockpit())
      suicide.push_back(0);
    suicide[_Universe->CurrentCockpit()]=1;
  }
  
}

class UnivMap {
  VSSprite * ul;
  VSSprite * ur;
  VSSprite * ll;
  VSSprite * lr;
public:
  UnivMap (VSSprite * ull, VSSprite *url, VSSprite * lll, VSSprite * lrl) {
    ul=ull;
    ur=url;
    ll=lll;
    lr = lrl;
  }
  void Draw() {
    if (ul||ur||ll||lr) {
      GFXBlendMode(SRCALPHA,INVSRCALPHA);
      GFXEnable(TEXTURE0);
      GFXDisable(TEXTURE1);
      GFXColor4f(1,1,1,1);
    }
    if (ul) 
      ul->Draw();
    if (ur)
      ur->Draw();
    if (ll)
      ll->Draw();
    if (lr)
      lr->Draw();
  }
  bool isNull() {
    return ul==NULL;
  }
};
std::vector <UnivMap> univmap;
void MapKey (int, KBSTATE k) {
  if (k==PRESS) {
    static VSSprite ul("upper-left-map.spr");
    static VSSprite ur("upper-right-map.spr");
    static VSSprite ll("lower-left-map.spr");
    static VSSprite lr("lower-right-map.spr");
    while (univmap.size()<=_Universe->CurrentCockpit())
      univmap.push_back(UnivMap(NULL,NULL,NULL,NULL));
    if (univmap[_Universe->CurrentCockpit()].isNull()) {
      univmap[_Universe->CurrentCockpit()]=UnivMap (&ul,&ur,&ll,&lr);
    }else {
      univmap[_Universe->CurrentCockpit()]=UnivMap(NULL,NULL,NULL,NULL);
    } 
  } 
}


void GameCockpit::TurretControl (int,KBSTATE k) {
  if (k==PRESS) {
    while (turretcontrol.size()<=_Universe->CurrentCockpit())
      turretcontrol.push_back(0);
    turretcontrol[_Universe->CurrentCockpit()]=1;
  }

}
void GameCockpit::Respawn (int,KBSTATE k) {
  if (k==PRESS) {
    while (respawnunit.size()<=_Universe->CurrentCockpit())
      respawnunit.push_back(0);
    respawnunit[_Universe->CurrentCockpit()]=1;
  }

}

static void FaceTarget (Unit * un, const QVector &ourpos, Unit * target) {
  if (target) {
    QVector RealPosition = target->LocalPosition();
    if (target->isSubUnit())
      RealPosition=target->Position();
      Vector methem(RealPosition.Cast()-ourpos.Cast());
      methem.Normalize();
      Vector p,q,r;
      un->GetOrientation(p,q,r);
      p=methem.Cross(r);
      if (p.MagnitudeSquared()){
	float theta =p.Magnitude();
	p*= (asin (theta)/theta);
	un->Rotate(p);
	un->GetOrientation (p,q,r);
	if (r.Dot(methem)<0) {
	  un->Rotate (p*(PI/theta));
	}

      }
  }
  
}

// SAME AS IN COCKPIT BUT ADDS SETVIEW and ACCESSCAMERA -> ~ DUPLICATE CODE
int GameCockpit::Autopilot (Unit * target) {
  static bool autopan = XMLSupport::parse_bool (vs_config->getVariable ("graphics","pan_on_auto","true"));
  int retauto = 0;
  if (target) {
    if (enableautosound.sound<0) {
      static string str=vs_config->getVariable("cockpitaudio","autopilot_enabled","autopilot");
      enableautosound.loadsound(str);
    }
    enableautosound.playsound();
    Unit * un=NULL;
    if ((un=GetParent())) {
      if((retauto = un->AutoPilotTo(un,false))) {//can he even start to autopilot
		if (autopan){
		  SetView (CP_PAN);
		}
		un->AutoPilotTo(target,false);
		static bool face_target_on_auto = XMLSupport::parse_bool (vs_config->getVariable ( "physics","face_on_auto", "false"));
		if (face_target_on_auto) {
			//	  FaceTarget(un,un->LocalPosition(),un->Target());
		}	  
		static double averagetime = GetElapsedTime()/getTimeCompression();
		static double numave = 1.0;
		averagetime+=GetElapsedTime()/getTimeCompression();
		static float autospeed = XMLSupport::parse_float (vs_config->getVariable ("physics","autospeed",".020"));//10 seconds for auto to kick in;
		numave++;
		if (autopan) {
		  AccessCamera(CP_PAN)->myPhysics.SetAngularVelocity(Vector(0,0,0));
		  AccessCamera(CP_PAN)->myPhysics.ApplyBalancedLocalTorque(_Universe->AccessCamera()->P,
								   _Universe->AccessCamera()->R,
								   averagetime*autospeed/(numave));
		  static float initialzoom=XMLSupport::parse_float(vs_config->getVariable("graphics","inital_zoom_factor","2.25"));
		  zoomfactor=initialzoom;
		}
		static float autotime = XMLSupport::parse_float (vs_config->getVariable ("physics","autotime","10"));//10 seconds for auto to kick in;

		autopilot_time=autotime;
		autopilot_target.SetUnit (target);
	  }
	}
  }
  return retauto;
}
extern void reset_time_compression(int i, KBSTATE a);
void GameCockpit::Shake (float amt) {
  static float shak= XMLSupport::parse_float(vs_config->getVariable("graphics","cockpit_shake","3"));
  static float shak_max= XMLSupport::parse_float(vs_config->getVariable("graphics","cockpit_shake_max","20"));
  shakin+=shak;
  if (shakin>shak_max) {
    shakin=shak_max;
  }
}

static void DrawCrosshairs (float x, float y, float wid, float hei, const GFXColor &col) {
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);
	GFXEnable(SMOOTH);
	GFXCircle(x,y,wid/4,hei/4);
	GFXCircle(x,y,wid/7,hei/7);
	GFXDisable(SMOOTH);
	GFXBegin(GFXLINE);
		GFXVertex3f(x-(wid/2),y,0);
		GFXVertex3f(x-(wid/6),y,0);
		GFXVertex3f(x+(wid/2),y,0);
		GFXVertex3f(x+(wid/6),y,0);
		GFXVertex3f(x,y-(hei/2),0);
		GFXVertex3f(x,y-(hei/6),0);
		GFXVertex3f(x,y+(hei/2),0);
		GFXVertex3f(x,y+(hei/6),0);
		GFXVertex3f(x-.001,y+.001,0);
		GFXVertex3f(x+.001,y-.001,0);
		GFXVertex3f(x+.001,y+.001,0);
		GFXVertex3f(x-.001,y-.001,0);
	GFXEnd();
	GFXEnable(TEXTURE0);
}
extern bool QuitAllow;
void GameCockpit::Draw() { 
  cockpit_time+=GetElapsedTime();
  if (cockpit_time>=100000)
    InitStatic();
  _Universe->AccessCamera()->UpdateGFX (GFXFALSE,GFXFALSE,GFXTRUE);
  GFXDisable (TEXTURE1);
  GFXLoadIdentity(MODEL);
  GFXDisable(LIGHTING);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXColor4f(1,1,1,1);
  static bool draw_any_boxes = XMLSupport::parse_bool (vs_config->getVariable("graphics","hud","DrawTargettingBoxes","true"));
  if (draw_any_boxes) {
  DrawTargetBox();
  DrawTurretTargetBoxes();
  if(draw_all_boxes){
    DrawTargetBoxes();
  }
  }
  if (view<CP_CHASE) {
    if (mesh) {
      Unit * par=GetParent();
      if (par) {
	//	Matrix mat;
      
	GFXLoadIdentity(MODEL);

	GFXDisable (DEPTHTEST);
	GFXDisable(DEPTHWRITE);
	GFXEnable (TEXTURE0);
	GFXEnable (LIGHTING);
	Vector P,Q,R;
	AccessCamera(CP_FRONT)->GetPQR (P,Q,R);

	headtrans.push_back (Matrix());
	VectorAndPositionToMatrix(headtrans.back(),P,Q,R,QVector(0,0,0));
	static float theta=0;
	static float shake_speed = XMLSupport::parse_float(vs_config->getVariable ("graphics","shake_speed","50"));
	theta+=shake_speed*GetElapsedTime();
	static float shake_reduction = XMLSupport::parse_float(vs_config->getVariable ("graphics","shake_reduction","8"));

	headtrans.front().p.i=shakin*cos(theta);//AccessCamera()->GetPosition().i+shakin*cos(theta);
	headtrans.front().p.j=shakin*cos(1.2*theta);//AccessCamera()->GetPosition().j+shakin*cos(theta);
	headtrans.front().p.k=0;//AccessCamera()->GetPosition().k;
	if (shakin>0) {
	  shakin-=GetElapsedTime()*shake_reduction;
	  if (shakin<=0) {
	    shakin=0;
	  }
	}

	mesh->DrawNow(1,true,headtrans.front());
	headtrans.pop_front();
	GFXDisable (LIGHTING);
	GFXDisable( TEXTURE0);
      }
    }
  }
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);
  GFXBlendMode (ONE,ONE);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);

  Unit * un;
  if (view==CP_FRONT) {
    if (Panel.size()>0) {
      static bool drawCrosshairs=parse_bool(vs_config->getVariable("graphics","draw_rendered_crosshairs","true"));
      if (drawCrosshairs) {
        float x,y,wid,hei;
        Panel.front()->GetSize(wid,hei);
        Panel.front()->GetPosition(x,y);
        DrawCrosshairs(x,y,wid,hei,textcol);
      } else {
        GFXBlendMode(SRCALPHA,INVSRCALPHA);
        GFXEnable (TEXTURE0);
        Panel.front()->Draw();//draw crosshairs
      }
    }
  }
  static bool mouseCursor = XMLSupport::parse_bool (vs_config->getVariable ("joystick","mouse_cursor","false"));
  if (mouseCursor) {  
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXColor4f (1,1,1,1);
    GFXEnable(TEXTURE0);
    //    GFXDisable (DEPTHTEST);
    //    GFXDisable(TEXTURE1);
    static int revspr = XMLSupport::parse_bool (vs_config->getVariable ("joystick","reverse_mouse_spr","true"))?1:-1;
    static string blah = vs_config->getVariable("joystick","mouse_crosshair","crosshairs.spr");
    static VSSprite MouseVSSprite (blah.c_str(),BILINEAR,GFXTRUE);
    MouseVSSprite.SetPosition (-1+float(mousex)/(.5*g_game.x_resolution),-revspr+float(revspr*mousey)/(.5*g_game.y_resolution));
    
    MouseVSSprite.Draw();
    //    DrawGlutMouse(mousex,mousey,&MouseVSSprite);
    //    DrawGlutMouse(mousex,mousey,&MouseVSSprite);
  }
  RestoreViewPort();
  GFXBlendMode (ONE,ZERO);
  static float AlphaTestingCutoff =XMLSupport::parse_float(vs_config->getVariable("graphics","AlphaTestCutoff",".8"));
  GFXAlphaTest (GREATER,AlphaTestingCutoff);
  GFXColor4f(1,1,1,1);
  if (view<CP_CHASE) {
    if (Pit[view]) 
      Pit[view]->Draw();
  }
  GFXAlphaTest (ALWAYS,0);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXColor4f(1,1,1,1);
	bool die=true;
  if ((un = parent.GetUnit())) {
    static bool drawF5VDU (XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_chase_cam","false")));
    static bool drawF6VDU (XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_panning_cam","false")));
    static bool drawF7VDU (XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_target_cam","false")));
    if (view==CP_FRONT||(view==CP_CHASE&&drawF5VDU)||(view==CP_PAN&&drawF6VDU)||(view==CP_TARGET&&drawF7VDU)) {//only draw crosshairs for front view
      if (Radar) {
	//Radar->Draw();
	if(radar_type=="Elite"){
	  DrawEliteBlips(un);
	}
	else{
	  DrawBlips(un);
	}
/*	if (rand01()>un->GetImageInformation().cockpit_damage[0]) {
		static Animation radar_ani("round_static.ani",true,.1,BILINEAR);
		radar_ani.DrawAsVSSprite(Radar);	
	}*/
	float damage =(un->GetImageInformation().cockpit_damage[0]);
	if (damage<.985) {
      if (radar_time>=0) {
        if (damage>.001&&(cockpit_time>radar_time+(1-damage))) {
	  if (rand01()>SWITCH_CONST) {
            radar_time=-cockpit_time;
          }
        } else {
          static Animation radar_ani("static_round.ani",true,.1,BILINEAR);
          radar_ani.DrawAsVSSprite(Radar[0]);	
          radar_ani.DrawAsVSSprite(Radar[1]);	
        }
      } else {
        if (cockpit_time>((1-(-radar_time))+damage)) {
	  if (rand01()>SWITCH_CONST) {
            radar_time=cockpit_time;
          }
        }
      }
	}
      }

      DrawGauges(un);
      
      GFXColor4f(1,1,1,1);
      for (unsigned int j=1;j<Panel.size();j++) {
	Panel[j]->Draw();
      }
      GFXColor4f(1,1,1,1);
      for (unsigned int vd=0;vd<vdu.size();vd++) {
	if (vdu[vd]) {
	  vdu[vd]->Draw(un,textcol);
	  GFXColor4f (1,1,1,1);
	  float damage = un->GetImageInformation().cockpit_damage[(1+vd)%(MAXVDUS+1)];
          if (vdu[vd]->staticable()) {
            if (damage<.985) {
              if (vdu_time[vd]>=0) {
                if (damage>.001&&(cockpit_time>(vdu_time[vd]+(1-damage)))) {
                  if (rand01()>SWITCH_CONST) {
                    vdu_time[vd]=-cockpit_time;
                  }
                } else {
                  static Animation vdu_ani("static.ani",true,.1,BILINEAR);
                  GFXEnable(TEXTURE0);
                  vdu_ani.DrawAsVSSprite(vdu[vd]);	
                  
                }
              } else {
                if (cockpit_time>((1-(-vdu_time[vd]))+(damage))) {
                  if (rand01()>SWITCH_CONST) {
                    vdu_time[vd]=cockpit_time;
                  }
                }
              }
	    }
	  }
        }              //process VDU, damage VDU, targetting VDU
      }      
    }
    GFXColor4f (1,1,1,1);
    if (un->GetHull()>=0)
      die = false;
    if (un->Threat()!=NULL) {
      if (0&&getTimeCompression()>1) {
	reset_time_compression(0,PRESS);
      }
      un->Threaten (NULL,0);
    }
    if (_Universe->CurrentCockpit()<univmap.size()) {
      univmap[_Universe->CurrentCockpit()].Draw();
    }
    // Draw the arrow to the target.
    DrawArrowToTarget(parent.GetUnit(), parent.GetUnit()->Target());
  }

  if (die) {
	if (un) {
		if (un->GetHull()>=0) {
			die=false;
		}
	}
	if (die) {
	static float dietime = 0;
	if (text) {
		GFXColor4f (1,1,1,1);
		text->SetSize(1,-1);
		float x; float y;
		if (dietime==0) {
		  if (respawnunit.size()>_Universe->CurrentCockpit()) 
		    if (respawnunit[_Universe->CurrentCockpit()]==1) {
		      respawnunit[_Universe->CurrentCockpit()]=0;
		    }
			text->GetCharSize (x,y);
			text->SetCharSize (x*4,y*4);
			text->SetPos (0-(x*2*14),0-(y*2));
			char playr[3];
			playr[0]='p';
			playr[1]='0'+_Universe->CurrentCockpit();
			playr[2]='\0';
			mission->msgcenter->add("game",playr,"#ff5555You Have Died!");
			mission->msgcenter->add("game",playr,"Press #8080FF;#000000 (semicolon) to respawn");
			mission->msgcenter->add("game",playr,"Or Press #8080FFEsc#000000 to quit");
		}
		GFXColorf (textcol);
		text->Draw ("#ff5555You Have Died!\n#000000Press #8080FF;#000000 (semicolon) to respawn\nOr Press #8080FFEsc and 'q'#000000 to quit");
		GFXColor4f (1,1,1,1);
	}
	dietime +=GetElapsedTime();
	SetView (CP_PAN);
	zoomfactor=dietime*10;
	}
  }
  if (QuitAllow) {
	  static VSSprite QuitSprite("quit.spr",BILINEAR,GFXTRUE);
	  GFXEnable(TEXTURE0);
	  QuitSprite.Draw();	  
  }
  GFXAlphaTest (ALWAYS,0);  
  GFXHudMode (false);
  DrawNavSystem();

  GFXEnable (DEPTHWRITE);
  GFXEnable (DEPTHTEST);
}
int GameCockpit::getScrollOffset (unsigned int whichtype) {
  for (unsigned int i=0;i<vdu.size();i++) {
    if (vdu[i]->getMode()&whichtype) {
      return vdu[i]->scrolloffset;
    }
  }
  return 0;
}

string GameCockpit::getsoundending(int which) {
	static bool gotten=false;
	static string strs [9];
	if (gotten==false) {
		char tmpstr[2]={'\0'};
		for (int i=0;i<9;i++) {
			tmpstr[0]=i+'1';
			string vsconfigvar=string("sounds_extension_")+tmpstr;
			strs[i]=vs_config->getVariable("cockpitaudio",vsconfigvar,"\n");
			if (strs[i]=="\n") {
				strs[i]="";
				break;
			}
		}
		gotten=true;
	}
	return strs[which];
}

#include <algorithm>
string GameCockpit::getsoundfile(string sound) {
	bool ok = false;
	int i;
	string lastsound="";
	string anothertmpstr = "";
	for (i=0;i<9&&!ok;i++) {
		anothertmpstr=getsoundending(i);
		bool foundyet=false;
		while (1) {
			std::string::iterator found=std::find(anothertmpstr.begin(),anothertmpstr.end(),'*');
			if (found!=anothertmpstr.end()) {
				anothertmpstr.erase(found);
				anothertmpstr.insert((found-anothertmpstr.begin()),sound);
				foundyet=true;
			} else {
				if (!foundyet) {
					anothertmpstr=sound+anothertmpstr;
				}
				break;
			}
		}
		if( VSFileSystem::LookForFile( anothertmpstr, SoundFile) < Ok)
			ok = true;
	}
	if( ok) 
	{
		//return lastsound;
		return anothertmpstr;
	} else {
		return "";
	}
}

void GameCockpit::UpdAutoPilot()
{
  static bool autopan = XMLSupport::parse_bool (vs_config->getVariable ("graphics","pan_on_auto","true"));
  if (autopilot_time!=0) {
    autopilot_time-=SIMULATION_ATOM;
    if (autopilot_time<= 0) {
      if (disableautosound.sound<0) {
	static string str=vs_config->getVariable("cockpitaudio","autopilot_disabled","autopilot_disabled");
	disableautosound.loadsound(str);
      }
      disableautosound.playsound();
      if (autopan) {
	AccessCamera(CP_PAN)->myPhysics.SetAngularVelocity(Vector(0,0,0));
	SetView(CP_FRONT);
      }
      autopilot_time=0;
      Unit * par = GetParent();
      if (par) {
	Unit * autoun = autopilot_target.GetUnit();
	autopilot_target.SetUnit(NULL);
	if (autoun&&autopan) {
	  par->AutoPilotTo(autoun,false);
	}
      }
    }
  }
}

void SwitchUnits2( Unit *nw)
{
  if (nw) {
    nw->PrimeOrders();
    nw->EnqueueAI (new FireKeyboard (_Universe->CurrentCockpit(),_Universe->CurrentCockpit()));
    nw->EnqueueAI (new FlyByJoystick (_Universe->CurrentCockpit()));

	nw->SetTurretAI();
	nw->DisableTurretAI();	
	
    static bool LoadNewCockpit = XMLSupport::parse_bool (vs_config->getVariable("graphics","UnitSwitchCockpitChange","false"));
    static bool DisCockpit = XMLSupport::parse_bool (vs_config->getVariable("graphics","SwitchCockpitToDefaultOnUnitSwitch","false"));
	
    if (nw->getCockpit().length()>0 || DisCockpit) {
      _Universe->AccessCockpit()->Init (nw->getCockpit().c_str(), LoadNewCockpit==false);
    }
	
	/* Here is the old code:
	   
    if (nw->getCockpit().length()>0&&LoadNewCockpit) {
      _Universe->AccessCockpit()->Init (nw->getCockpit().c_str());
    }else {
      if (DisCockpit) {
	_Universe->AccessCockpit()->Init ("disabled-cockpit.cpt");
      }
    }
	
	*/
  }
}

GameCockpit::~GameCockpit () {
  Delete();
  delete savegame;
}
int GameCockpit::getVDUMode(int vdunum) {
  if (vdunum<(int)vdu.size()) {
    if (vdu[vdunum]) {
      return vdu[vdunum]->getMode();
    }
  }return 0;
}
void GameCockpit::VDUSwitch (int vdunum) {
  if (soundfile>=0) {
    //AUDPlay (soundfile, AccessCamera()->GetPosition(), Vector (0,0,0), .5);
	AUDPlay(soundfile,QVector(0,0,0),Vector(0,0,0),1);
  }
  if (vdunum<(int)vdu.size()) {
    if (vdu[vdunum]) {
      vdu[vdunum]->SwitchMode( this->parent.GetUnit());
    }
  }
}
void GameCockpit::ScrollVDU (int vdunum, int howmuch) {
  if (soundfile>=0) {
    //AUDPlay (soundfile, AccessCamera()->GetPosition(), Vector (0,0,0),.5);
	AUDPlay (soundfile,QVector(0,0,0),Vector(0,0,0),1);
  }
  if (vdunum<(int)vdu.size()) {
    if (vdu[vdunum]) {
      vdu[vdunum]->Scroll(howmuch);
    }
  }
}
void GameCockpit::ScrollAllVDU (int howmuch) {
  for (unsigned int i=0;i<vdu.size();i++) {
    ScrollVDU (i,howmuch);
  }
}


void GameCockpit::SetCommAnimation (Animation * ani) {
  for (unsigned int i=0;i<vdu.size();i++) {
    if (vdu[i]->SetCommAnimation (ani)) {
      break;
    }
  }
}
void GameCockpit::RestoreViewPort() {
  _Universe->AccessCamera()->RestoreViewPort(0,0);
}

static void ShoveCamBehindUnit (int cam, Unit * un, float zoomfactor) {
  QVector unpos = un->GetPlanetOrbit()?un->LocalPosition():un->Position();
  _Universe->AccessCamera(cam)->SetPosition(unpos-_Universe->AccessCamera()->GetR().Cast()*(un->rSize()+g_game.znear*2)*zoomfactor,un->GetWarpVelocity(),un->GetAngularVelocity());
}
static void ShoveCamBelowUnit (int cam, Unit * un, float zoomfactor) {
  QVector unpos = un->GetPlanetOrbit()?un->LocalPosition():un->Position();
  Vector p,q,r;
  _Universe->AccessCamera(cam)->GetOrientation(p,q,r);
  static float ammttoshovecam = XMLSupport::parse_float(vs_config->getVariable("graphics","shove_camera_down",".3"));
  _Universe->AccessCamera(cam)->SetPosition(unpos-(r-ammttoshovecam*q).Cast()*(un->rSize()+g_game.znear*2)*zoomfactor,un->GetWarpVelocity(),un->GetAngularVelocity());
}
void GameCockpit::SetupViewPort (bool clip) {
  _Universe->AccessCamera()->RestoreViewPort (0,(view==CP_FRONT?viewport_offset:0));
   GFXViewPort (0,(int)((view==CP_FRONT?viewport_offset:0)*g_game.y_resolution), g_game.x_resolution,g_game.y_resolution);
  _Universe->AccessCamera()->setCockpitOffset (view<CP_CHASE?cockpit_offset:0);
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
#ifdef IWANTTOPVIEW
    _Universe->AccessCamera(CP_FRONT)->GetOrientation(p,q,r);
    tmp=r;
    r = -q;
    q = tmp;
    _Universe->AccessCamera(CP_CHASE)->SetOrientation(p,q,r);
#endif
    tgt = un->Target();
    if (tgt) {
      
      Vector p,q,r,tmp;
      un->GetOrientation (p,q,r);
      r = (tgt->Position()-un->Position()).Cast();
      r.Normalize();
      CrossProduct (r,q,tmp);
      CrossProduct (tmp,r,q);
      _Universe->AccessCamera(CP_VIEWTARGET)->SetOrientation(tmp,q,r);
      _Universe->AccessCamera(CP_TARGET)->SetOrientation(tmp,q,r);
      //      _Universe->AccessCamera(CP_PANTARGET)->SetOrientation(tmp,q,r);
      ShoveCamBelowUnit (CP_TARGET,un,zoomfactor);
      ShoveCamBehindUnit (CP_PANTARGET,tgt,zoomfactor);
    }else {
      un->UpdateHudMatrix (CP_VIEWTARGET);
      un->UpdateHudMatrix (CP_TARGET);
      un->UpdateHudMatrix (CP_PANTARGET);
    }
    ShoveCamBelowUnit (CP_CHASE,un,zoomfactor);
    //    ShoveCamBehindUnit (CP_PANTARGET,un,zoomfactor);




    ShoveCamBehindUnit (CP_PAN,un,zoomfactor);
    un->SetVisible(view>=CP_CHASE);

  }
  _Universe->AccessCamera()->UpdateGFX(clip?GFXTRUE:GFXFALSE);
    
  //  parent->UpdateHudMatrix();
}
void GameCockpit::SelectCamera(int cam){
    if(cam<NUM_CAM&&cam>=0)
      currentcamera = cam;
}
Camera* GameCockpit::AccessCamera(int num){
  if(num<NUM_CAM&&num>=0)
    return &cam[num];
  else
    return NULL;
}

/**
 * Draw the arrow pointing to the target.
 */
// THETA : angle between the arrow head and the two branches (divided by 2) (20 degrees here).
#define TARGET_ARROW_COS_THETA    0.93969262078590838405410927732473
#define TARGET_ARROW_SIN_THETA    0.34202014332566873304409961468226
#define TARGET_ARROW_SIZE         0.05
void GameCockpit::DrawArrowToTarget(Unit *un, Unit *target) {
  float s, t, s_normalized, t_normalized, inv_len;
  Vector p1, p2, p_n;

  if ( ! target )
    return;

  Vector localcoord(un->LocalCoordinates(target));

  // Project target position on k.
  inv_len = 1 / fabs(localcoord.k);
  s = -localcoord.i * inv_len;
  t = localcoord.j * inv_len;

  if ( localcoord.k > 0 ) {       // The unit is in front of us.
    // Check if the unit is in the screen.
    if ( (fabs(s) < projection_limit_x) && (fabs(t) < projection_limit_y) )
      return;     // The unit is in the screen, do not display the arrow.
  }

  inv_len = 1 / sqrt(s*s + t*t);
  s_normalized = s * inv_len;                 // Save the normalized projected coordinates.
  t_normalized = t * inv_len;

  // Apply screen aspect ratio correction.
  s *= inv_screen_aspect_ratio;

  if ( fabs(t) > fabs(s) ) {    // Normalize t.
    if ( t > 0 ) {
      s /= t;
      t = 1;
    }
    else if ( t < 0 ) {
      s /= -t;
      t = -1;
    }                           // case t == 0, do nothing everything is ok.
  }
  else {                        // Normalize s.
    if ( s > 0 ) {
      t /= s;
      s = 1;
    }
    else if ( s < 0 ) {
      t /= -s;
      s = -1;
    }                          // case s == 0, do nothing everything is ok.
  }

  // Compute points p1 and p2 composing the arrow. Hard code a 2D rotation.
  // p1 = p - TARGET_ARROW_SIZE * p.normalize().rot(THETA), p being the arrow head position (s,t).
  // p2 = p - TARGET_ARROW_SIZE * p.normalize().rot(-THETA)
  p_n.i = -TARGET_ARROW_SIZE * s_normalized;  // Vector p will be used to compute the two branches of the arrow.
  p_n.j = -TARGET_ARROW_SIZE * t_normalized;
  p1.i = p_n.i*TARGET_ARROW_COS_THETA - p_n.j*TARGET_ARROW_SIN_THETA;     // p1 = p.rot(THETA)
  p1.j = p_n.j*TARGET_ARROW_COS_THETA + p_n.i*TARGET_ARROW_SIN_THETA;
  p2.i = p_n.i*TARGET_ARROW_COS_THETA - p_n.j*(-TARGET_ARROW_SIN_THETA);  // p2 = p.rot(-THETA)
  p2.j = p_n.j*TARGET_ARROW_COS_THETA + p_n.i*(-TARGET_ARROW_SIN_THETA);
  p1.i += s;
  p1.j *= g_game.aspect;
  p1.j += t;
  p2.i += s;
  p2.j *= g_game.aspect;
  p2.j += t;
  p2.k = p1.k = 0;

  static GFXColor black_and_white = DockBoxColor("black_and_white");
  GFXColorf(un->GetComputerData().radar.color ? unitToColor(un, target) : black_and_white);

  glBegin(GL_LINE_LOOP);
  GFXVertex3f(s, t, 0);
  GFXVertexf(p1);
  GFXVertexf(p2);
  GFXEnd();
  GFXColor4f (1,1,1,1);
}
