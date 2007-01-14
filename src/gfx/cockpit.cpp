
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
#include "cmd/unit_find.h" // for radar iteration.
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
#include "in_kb_data.h"
#include "main_loop.h"
#include <set>
#include <string>

static float mymin(float a, float b)
{
	return (a<b) ? a : b;
}

static float mymax(float a, float b)
{
	return (a>b) ? a : b;
}

using namespace std;
extern float rand01();
using VSFileSystem::SoundFile;
#define SWITCH_CONST .9
#define VERYNEAR_CONST 0.004f /* The smaller VERYNEAR_CONST is, the worse Z-Buffer precision will be. So keep this above 0.004) */
#define COCKPITZ_HEADROOM 1.01f /*so that znear/zfar are not too close to max/min values, and account for off-center cockpits */
static GFXColor RetrColor (const string& name, GFXColor def=GFXColor(1,1,1,1)) {
  vs_config->getColor(name,&def.r);    
  return def;
}
extern Unit*getTopLevelOwner();
static soundContainer disableautosound;
static soundContainer enableautosound;

#define sqr(x) (x*x)
std::string GameCockpit::GetNavSelectedSystem() {
  return AccessNavSystem()->getSelectedSystem();
}
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


GFXColor GameCockpit::unitToColor (Unit *un,Unit *target, char ifflevel) {
  static GFXColor basecol=RetrColor("base",GFXColor(-1,-1,-1,-1));
  static GFXColor jumpcol=RetrColor("jump",GFXColor(0,1,1,.8));
  static GFXColor navcol=RetrColor("nav",GFXColor(1,1,1,1));
  static GFXColor suncol=RetrColor("star",GFXColor(1,1,1,1));
  static GFXColor missilecol=RetrColor("missile",GFXColor(.25,0,.5,1));
  static GFXColor cargocol=RetrColor("cargo",GFXColor(.6,.2,0,1));
  int cargofac=FactionUtil::GetUpgradeFaction();
  static GFXColor black_and_white=RetrColor ("black_and_white",GFXColor(.5,.5,.5));  
  if (ifflevel==0)
    return black_and_white;
  if (target->GetDestinations().size()>0&&ifflevel>1) {
      return jumpcol;
  }
  if (ifflevel>1) {
    if(target->isUnit()==PLANETPTR){
      Planet * plan = static_cast<Planet*>(target);
      if (plan->hasLights()) {
        return suncol;
      }
      if (plan->isAtmospheric()) {
        return navcol;
      }
      return planet;
    }else if (target->isUnit()==MISSILEPTR) {
      return missilecol;      
    }else if (target->faction==cargofac){
      return cargocol;
    }else if(target==un->Target()&&draw_all_boxes){//if we only draw target box we cannot tell what faction enemy is!
      // my target
      return targeted;
    }
  }
  if(target->Target()==un){
    // the other ships is targetting me
    return targetting;
  }
  if (basecol.r>0&&basecol.g>0&&basecol.b>0&&UnitUtil::getFlightgroupName(target)=="Base"&&ifflevel>0) {
    return basecol;
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
  static bool absolute_relation_color=XMLSupport::parse_bool(vs_config->getVariable("graphics","absolute_relation_color","false"));
  if (absolute_relation_color) {
    if (relation>0) {
      return friendly;
    }
    if (relation<0) {
      return enemy;
    }
    return neutral;
  }
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

  if (1) {
    static float crossthick = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","NavCrossLineThickness","1")); //1.05;
    GFXLineWidth(crossthick);
    size = .125*GFXGetZPerspective (size);
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXEnable (SMOOTH);
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
    GFXDisable(SMOOTH);
    GFXLineWidth(1);
  }
}

float GameCockpit::computeLockingSymbol(Unit * par) {
  return par->computeLockingPercent();
}
inline void DrawOneTargetBox (const QVector & Loc, float rSize, const Vector &CamP, const Vector & CamQ, const Vector & CamR, float lock_percent, bool ComputerLockon, bool Diamond=false) {
  static float boxthick = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","BoxLineThickness","1"));
  static float rat = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","min_target_box_size",".01"));
  float len = (Loc).Magnitude();
  float curratio = rSize/len;
  if (curratio<rat) 
    rSize = len*rat;
  GFXLineWidth(boxthick);
  GFXEnable(SMOOTH);
  GFXBlendMode(SRCALPHA,INVSRCALPHA);
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
//  eallySwitch=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","switchToTargetModeOnKey","true"));
    static float diamondthick = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","DiamondLineThickness","1"));; // 1.05;
    glLineWidth(diamondthick);
    static bool center=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","LockCenterCrosshair","false"));
    static float absmin = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","min_lock_box_size",".001"));
    static float endreticle = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","MinMissileBracketSize","1.05"));; // 1.05;
    static float startreticle = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","MaxMissileBracketSize","2.05"));; // 2.05;
    static float diamondsize = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","DiamondSize","2.05"));; // 1.05;
    static float bracketsize = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","RotatingBracketSize","0.58"));; // 1.05;
    static float thetaspeed = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","DiamondRotationSpeed","1"));; // 1.05;
    static float lockline = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","LockConfirmLineLength","1.5"));; // 1.05;

    if (center) {
      static float bracketwidth = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","RotatingBracketWidth","0.1"));; // 1.05;
      static bool bracketinnerouter = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","RotatingBracketInner","true"));; // 1.05;
      float innerdis=endreticle+(startreticle-endreticle)*lock_percent;
      float outerdis=innerdis+bracketsize;
      float bracketdis=(bracketinnerouter?innerdis:outerdis);
      float r=rSize<absmin?absmin:rSize;
      GFXBegin(GFXLINE);
      
      GFXVertexf(Loc+CamP*(innerdis*r));
      GFXVertexf(Loc+CamP*(outerdis*r));

      GFXVertexf(Loc+CamP*(bracketdis*r)+CamQ*(bracketwidth*r));
      GFXVertexf(Loc+CamP*(bracketdis*r)-CamQ*(bracketwidth*r));

      GFXVertexf(Loc-CamP*(innerdis*r));
      GFXVertexf(Loc-CamP*(outerdis*r));

      GFXVertexf(Loc-CamP*(bracketdis*r)+CamQ*(bracketwidth*r));
      GFXVertexf(Loc-CamP*(bracketdis*r)-CamQ*(bracketwidth*r));

      GFXVertexf(Loc+CamQ*(innerdis*r));
      GFXVertexf(Loc+CamQ*(outerdis*r));

      GFXVertexf(Loc+CamQ*(bracketdis*r)+CamP*(bracketwidth*r));
      GFXVertexf(Loc+CamQ*(bracketdis*r)-CamP*(bracketwidth*r));

      GFXVertexf(Loc-CamQ*(innerdis*r));
      GFXVertexf(Loc-CamQ*(outerdis*r));

      GFXVertexf(Loc-CamQ*(bracketdis*r)+CamP*(bracketwidth*r));
      GFXVertexf(Loc-CamQ*(bracketdis*r)-CamP*(bracketwidth*r));

      
      GFXEnd();
    }else {
      float max=diamondsize; 
      //    VSFileSystem::Fprintf (stderr,"lock percent %f\n",lock_percent);
      float coord = endreticle+(startreticle-endreticle)*lock_percent;//rSize/(1-lock_percent);//this is a number between 1 and 100
      double rtot = 1./sqrtf(2);

	  // this causes the rotation!
      float theta = 4*M_PI*lock_percent*thetaspeed;
      Vector LockBox (-cos(theta)*rtot,-rtot,sin(theta)*rtot);
	  //Vector LockBox (0*rtot,-rtot,1*rtot);

      static float diamondthick = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","DiamondLineThickness","1"));; // 1.05;
      GFXLineWidth(diamondthick);
      Vector TLockBox (rtot*LockBox.i+rtot*LockBox.j,rtot*LockBox.j-rtot*LockBox.i,LockBox.k);
      Vector SLockBox (TLockBox.j,TLockBox.i,TLockBox.k);
      QVector Origin = (CamP+CamQ).Cast()*(rSize*coord);
      TLockBox = (TLockBox.i*CamP+TLockBox.j*CamQ+TLockBox.k*CamR);
      SLockBox = (SLockBox.i*CamP+SLockBox.j*CamQ+SLockBox.k*CamR);
      double r1Size = rSize*bracketsize;
	  if (r1Size < absmin)
		r1Size = absmin;
      GFXBegin (GFXLINESTRIP);
      max*=rSize*.75*endreticle;
	  if (lock_percent==0) {
		GFXVertexf (Loc+CamQ.Cast()*max*lockline);
		GFXVertexf (Loc+CamQ.Cast()*max);
	  }

      GFXVertexf (Loc+Origin+(TLockBox.Cast()*r1Size));
      GFXVertexf (Loc+Origin);
      GFXVertexf (Loc+Origin+(SLockBox.Cast()*r1Size));
      if (lock_percent==0) {
        GFXVertexf (Loc+CamP.Cast()*max);
        GFXVertexf (Loc+CamP.Cast()*max*lockline);

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
        GFXVertexf (Loc-CamQ.Cast()*max*lockline);

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
        GFXVertexf (Loc-CamP.Cast()*max*lockline);

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
    }
  }
  GFXLineWidth(1);
  GFXDisable(SMOOTH);
}

static GFXColor DockBoxColor (const string& name) {
  GFXColor dockbox(1,1,1,1);
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

	GFXColor drawcolor=unitToColor(un,target,un->GetComputerData().radar.iff);
	GFXColorf(drawcolor);

	if(target->isUnit()==UNITPTR){

	  if (un->Target()==target) {
            static bool draw_dock_box =XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_docking_boxes","true"));
            if (draw_dock_box) 
              DrawDockingBoxes(un,target,CamP,CamQ, CamR);
	    DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),true);
	  }else {
	    DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),false);
	  }
	}
    }
    target=(++uiter);
  }

}


void GameCockpit::DrawTargetBox () {
  float speed,range;
  static GFXColor black_and_white=DockBoxColor ("black_and_white"); 
  int neutral = FactionUtil::GetNeutralFaction();
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
  static bool draw_nav_symbol=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawNavSymbol","false"));
  if (draw_nav_symbol) {
    static GFXColor suncol=RetrColor("nav",GFXColor(1,1,1,1));
    DrawNavigationSymbol (un->GetComputerData().NavPoint,CamP,CamQ, CamR.Cast().Dot((un->GetComputerData().NavPoint).Cast()-_Universe->AccessCamera()->GetPosition()));
  }
  GFXColorf (unitToColor(un,target,un->GetComputerData().radar.iff));

  if(draw_line_to_target){
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXEnable (SMOOTH);
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
    GFXDisable (SMOOTH);
  }
  float distance = UnitUtil::getDistance(un,target);
  static bool draw_target_nav_symbol =XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_target_nav_symbol","true"));
  static bool draw_jump_nav_symbol =XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_jump_target_nav_symbol","true"));
  bool nav_symbol=false;
  if (draw_target_nav_symbol&&((target->faction==neutral&&target->isUnit()==UNITPTR)||target->isUnit()==ASTEROIDPTR||(target->isPlanet()&&((Planet*)target)->isAtmospheric()&&(draw_jump_nav_symbol||target->GetDestinations().empty()))||distance>un->GetComputerData().radar.maxrange)) {
    static float nav_symbol_size = XMLSupport::parse_float(vs_config->getVariable("graphics","nav_symbol_size",".25"));
    GFXColor4f(1,1,1,1);
    DrawNavigationSymbol (Loc,CamP,CamQ, Loc.Magnitude()*nav_symbol_size);  
    nav_symbol=true;
  }else {
    static bool lock_nav_symbol =XMLSupport::parse_bool(vs_config->getVariable("graphics","lock_significant_target_box","true"));
    DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),un->TargetLocked()&&(lock_nav_symbol||!UnitUtil::isSignificant(target)));
  }
  static bool draw_dock_box =XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_docking_boxes","true"));
  if (draw_dock_box) {
    DrawDockingBoxes(un,target,CamP,CamQ,CamR);
  }
  if ((always_itts || un->GetComputerData().itts)&&!nav_symbol) {
    float mrange;
    un->getAverageGunSpeed (speed,range,mrange);
    float err = (.01*(1-un->CloakVisible()));
    QVector iLoc = target->PositionITTS (un->Position(),un->cumulative_velocity,speed,steady_itts)-_Universe->AccessCamera()->GetPosition()+10*err*QVector (-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX,-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX,-.5*.25*un->rSize()+rand()*.25*un->rSize()/RAND_MAX);
    
    GFXEnable (SMOOTH);
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
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
    GFXDisable (SMOOTH);
  }

}

void GameCockpit::DrawCommunicatingBoxes () {


  Vector CamP,CamQ,CamR;
  _Universe->AccessCamera()->GetPQR(CamP,CamQ,CamR);
  //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
  for (unsigned int i=0;i<vdu.size();++i) {
    Unit*target= vdu[i]->GetCommunicating();   
    if (target) {
      static GFXColor black_and_white=DockBoxColor ("communicating"); 
      QVector Loc(target->Position()-_Universe->AccessCamera()->GetPosition());
      GFXDisable (TEXTURE0);
      GFXDisable (TEXTURE1);
      GFXDisable (DEPTHTEST);
      GFXDisable (DEPTHWRITE);
      GFXBlendMode (SRCALPHA,INVSRCALPHA);
      GFXDisable (LIGHTING);
      GFXColorf (black_and_white);

      DrawOneTargetBox (Loc, target->rSize()*1.05, CamP, CamQ, CamR,1,0);

     
    }
  }

}



void GameCockpit::DrawTurretTargetBoxes () {

  static GFXColor black_and_white=DockBoxColor ("black_and_white");
  Unit * parun = parent.GetUnit();
  if (!parun)
    return;
  UnitCollection::UnitIterator iter = parun->getSubUnits();
  Unit * un;

  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXDisable (LIGHTING);

  //This avoids rendering the same target box more than once
  std::set<void*> drawn_targets;

  while (NULL!=(un=iter.current())) {
	if (!un)
      return;
	if (un->GetNebula()!=NULL)
	  return;
    Unit *target = un->Target();
    void *vtgt=(void*)target;
    if (!target||(drawn_targets.count(vtgt)>0)){
      iter.advance();
      continue;
    }
    drawn_targets.insert(vtgt);

	Vector CamP,CamQ,CamR;
    _Universe->AccessCamera()->GetPQR(CamP,CamQ,CamR);
    //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
    QVector Loc(target->Position()-_Universe->AccessCamera()->GetPosition());
    static bool draw_nav_symbol=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawNavSymbol","false"));
    if (draw_nav_symbol){
      GFXColor4f(1,1,1,1);
      DrawNavigationSymbol (un->GetComputerData().NavPoint,CamP,CamQ, CamR.Cast().Dot((un->GetComputerData().NavPoint).Cast()-_Universe->AccessCamera()->GetPosition()));
    }
    GFXColorf (unitToColor(un,target,un->GetComputerData().radar.iff));

    //DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),un->TargetLocked());

	// ** jay
	float rSize = target->rSize();

    float drift=rand()/(float)RAND_MAX;
    GFXEnable(SMOOTH);
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
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
    GFXDisable(SMOOTH);

    iter.advance();
  }

}


void GameCockpit::DrawTacticalTargetBox () {
  static bool drawtactarg=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","DrawTacticalTarget","false"));
  if (!drawtactarg)
    return;
  static GFXColor black_and_white=DockBoxColor ("black_and_white");
  Unit * parun = parent.GetUnit();
  if (!parun)
    return;
  if (parun->getFlightgroup()==NULL)
    return;

  Unit *target = parun->getFlightgroup()->target.GetUnit();
    if (target){
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

    static float thethick=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","TacTargetThickness","1.0"));
    static float fudge=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","TacTargetLength","0.1"));
    static float foci=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","TacTargetFoci","0.5"));
    glLineWidth ((int)thethick); // temp
    GFXColorf (unitToColor(parun,target,parun->GetComputerData().radar.iff));

    //DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,computeLockingSymbol(un),un->TargetLocked());

	// ** jay
	float rSize = target->rSize();

	GFXBegin(GFXLINE);
	GFXVertexf (Loc+((-CamP).Cast()+(-CamQ).Cast())*rSize*(foci+fudge));
    GFXVertexf (Loc+((-CamP).Cast()+(-CamQ).Cast())*rSize*(foci-fudge));

	GFXVertexf (Loc+((-CamP).Cast()+(CamQ).Cast())*rSize*(foci+fudge));
    GFXVertexf (Loc+((-CamP).Cast()+(CamQ).Cast())*rSize*(foci-fudge));

	GFXVertexf (Loc+((CamP).Cast()+(-CamQ).Cast())*rSize*(foci+fudge));
    GFXVertexf (Loc+((CamP).Cast()+(-CamQ).Cast())*rSize*(foci-fudge));

	GFXVertexf (Loc+((CamP).Cast()+(CamQ).Cast())*rSize*(foci+fudge));
    GFXVertexf (Loc+((CamP).Cast()+(CamQ).Cast())*rSize*(foci-fudge));
	GFXEnd();

    glLineWidth ((int)1); // temp
  }
}

void GameCockpit::drawUnToTarget ( Unit * un, Unit* target,float xcent,float ycent, float xsize, float ysize, bool reardar){
  static GFXColor black_and_white=DockBoxColor ("black_and_white"); 
  static GFXColor communicating=DockBoxColor ("communicating"); 
  static float fademax=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","BlipRangeMaxFade","1.0"));
  static float fademin=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","BlipRangeMinFade","1.0"));
  static float fademidpoint=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","BlipRangeFadeMidpoint","0.2"));
      Vector localcoord (un->LocalCoordinates(target));
	  if (reardar)
		  localcoord.k=-localcoord.k;
	  float s,t;
      this->LocalToRadar (localcoord,s,t);
      GFXColor localcol (this->unitToColor (un,target,un->GetComputerData().radar.iff));
	  float fade = float(localcoord.Magnitude()/un->GetComputerData().radar.maxrange);
	  fade = mymin(1.f,mymax(0.f,(fade<fademidpoint)?(0.5f*fade/fademidpoint):(0.5f+0.5f*(fade-fademidpoint)/(1.0f-fademidpoint))));
	  if (target==un->Target())
		  fade = 0;
	  localcol.a *= fade * fademax + (1.0f-fade) * fademin;
      if (1) {
        unsigned int s=vdu.size();
        for (unsigned int i=0;i<s;++i) {
          if (vdu[i]->GetCommunicating()==target)
            localcol=communicating;
        }
      }
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
  going_to_dock_screen=false;
}
void GameCockpit::EjectDock() {
  ejecting=true;
  going_to_dock_screen=true;
}

static void DoAutoLanding(Cockpit * thus, Unit * un, Unit * target) {
  if (!thus || !un || !target)
    return;
  if (UnitUtil::isDockableUnit(target)==false)
    return;
  static std::set<std::string> autoLandingExcludeList;
  static std::set<std::string> autoLandingExcludeWarningList;
  static bool autoLandingExcludeList_initialised=false;
  if (!autoLandingExcludeList_initialised) {
	autoLandingExcludeList_initialised = true;
	std::string excludes;
	
	excludes = vs_config->getVariable("physics","AutoLandingExcludeList","");
	if (!excludes.empty()) {
		std::string::size_type pos=0, epos=0;
		while (epos != std::string::npos) {
			autoLandingExcludeList.insert(excludes.substr(pos,epos=excludes.find(' ',pos)));
			pos = epos+1;
		}
	}

	excludes = vs_config->getVariable("physics","AutoLandingExcludeWarningList","");
	if (!excludes.empty()) {
		std::string::size_type pos=0, epos=0;
		while (epos != std::string::npos) {
			autoLandingExcludeWarningList.insert(excludes.substr(pos,epos=excludes.find(' ',pos)));
			pos = epos+1;
		}
	}
  }
  if (autoLandingExcludeList.count(target->name)>0) 
    return;
  static float lessthan=XMLSupport::parse_float(vs_config->getVariable("physics","AutoLandingDockDistance","50"));
  static float warnless=XMLSupport::parse_float(vs_config->getVariable("physics","AutoLandingWarningDistance","350"));
  static float AutoLandingMoveDistance=XMLSupport::parse_float(vs_config->getVariable("physics","AutoLandingMoveDistance","50"));
  static float moveout=XMLSupport::parse_float(vs_config->getVariable("physics","AutoLandingDisplaceDistance","50"));
  static float autorad=XMLSupport::parse_float(vs_config->getVariable("physics","unit_default_autodock_radius","0"));
  static bool adjust_unit_radius=XMLSupport::parse_float(vs_config->getVariable("physics","use_unit_autodock_radius","false"));
  float rsize=target->isPlanet()?target->rSize():(autorad+(adjust_unit_radius?target->rSize():0));
  QVector diffvec=un->Position()-target->Position();
  float dist =diffvec.Magnitude()-un->rSize()-rsize;
  diffvec.Normalize();

  static bool haswarned=false;
  static void * lastwarned=NULL;
  static float docktime=-FLT_MAX;

  if (dist<lessthan&&haswarned&&lastwarned==target) {
      //CrashForceDock(target,un,true);      
      un->SetPosAndCumPos(target->Position()+diffvec.Scale(un->rSize()+rsize+AutoLandingMoveDistance));
      FireKeyboard::DockKey(KBData(),PRESS);
      haswarned=false;
      lastwarned=target;
      docktime=getNewTime();
  } else if (haswarned==false&&lastwarned==target) {
    if (getNewTime()-docktime>SIMULATION_ATOM*2) {
      haswarned=false;
      un->SetPosAndCumPos(UniverseUtil::SafeEntrancePoint(target->Position()+diffvec*(rsize+moveout+un->rSize()),un->rSize()*1.1));
      lastwarned=NULL;
    }
  } else if (dist<warnless){
    if (lastwarned!=target||!haswarned) {
      if (autoLandingExcludeWarningList.count(target->name)==0) {
        static string str=vs_config->getVariable("cockpitaudio","automatic_landing_zone","als");
        static string str1=vs_config->getVariable("cockpitaudio","automatic_landing_zone1","als");
        static string str2=vs_config->getVariable("cockpitaudio","automatic_landing_zone2","als");
        static string autolandinga=vs_config->getVariable("graphics","automatic_landing_zone_warning","comm_docking.ani");
        static string autolandinga1=vs_config->getVariable("graphics","automatic_landing_zone_warning1","comm_docking.ani");
        static string autolandinga2=vs_config->getVariable("graphics","automatic_landing_zone_warning2","comm_docking.ani");
        static string message=vs_config->getVariable("graphics","automatic_landing_zone_warning_text","Now Entering an \"Automatic Landing Zone\".");
        UniverseUtil::IOmessage(0,"game","all",message);
        static Animation * ani0=new Animation(autolandinga.c_str());
        static Animation * ani1=new Animation(autolandinga1.c_str());
        static Animation * ani2=new Animation(autolandinga2.c_str());
        static soundContainer warnsound;
        static soundContainer warnsound1;
        static soundContainer warnsound2;
        int num=rand()<RAND_MAX/2?0:(rand()<RAND_MAX/2?1:2);
        if (warnsound.sound<0) {
          warnsound.loadsound(str);
          warnsound1.loadsound(str1);
          warnsound2.loadsound(str2);
        }
        switch (num){        
        case 0:warnsound.playsound();    thus->SetCommAnimation(ani0,target);  break;
        case 1: warnsound1.playsound();  thus->SetCommAnimation(ani1,target);break;
        default: warnsound2.playsound(); thus->SetCommAnimation(ani2,target);break;
        }
      }
      haswarned=true; 
      lastwarned=target;
    }
  } else if (lastwarned==target) {
    haswarned=false;
    lastwarned=NULL;
  }
}

class DrawUnitBlip {
	Unit *un;
	Unit * makeBigger;
	GameCockpit *parent;
	float *xsize;
	float *ysize;
	float *xcent;
	float *ycent;
	bool *reardar;
	int numradar;
public:
	DrawUnitBlip() {}
	void init (Unit *un, GameCockpit *parent,
			   int numradar, float *xsize, float *ysize, float *xcent, float *ycent, bool *reardar) {
		this->un=un;
		this->parent=parent;
		this->makeBigger=un->Target();
		this->numradar=numradar;
		this->xsize=xsize;
		this->ysize=ysize;
		this->xcent=xcent;
		this->ycent=ycent;
		this->reardar=reardar;
	}
	bool acquire(Unit *target, float distance) {
		if (target!=un) {
			double dist;
			int rad;
			static bool draw_significant_blips = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_significant_blips","true"));
			static bool untarget_out_cone=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","untarget_beyond_cone","false"));
			static bool autolanding_enable=XMLSupport::parse_bool(vs_config->getVariable("physics","AutoLandingEnable","false"));

			if (autolanding_enable)
				DoAutoLanding(parent,un,target);

			if (!un->InRange (target,dist,makeBigger==target&&untarget_out_cone,true,true)) {
				if (makeBigger==target) {
					un->Target(NULL);
				}
				return true;
			}
			if (makeBigger!=target&&draw_significant_blips==false&&getTopLevelOwner()==target->owner&&distance>un->GetComputerData().radar.maxrange){
				return true;
			}
			static float minblipsize=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","min_radarblip_size","0"));
			if (target->radial_size > minblipsize) {
				for (rad=0;rad<numradar;++rad) {
					parent->drawUnToTarget (un,target,xcent[rad],ycent[rad],xsize[rad],ysize[rad],reardar[rad]);
				}
			}
			if (target->isPlanet()==PLANETPTR && target->radial_size > 0) {
				Unit * sub=NULL;
				for (un_iter i=target->getSubUnits();(sub=*i)!=NULL;++i) {
					if (target->radial_size > minblipsize) {
						for (rad=0;rad<numradar;++rad) {
							parent->drawUnToTarget(un,sub,xcent[rad],ycent[rad], xsize[rad],ysize[rad],reardar[rad]);
						}
					}
				}
			}
		}
                return true;
	}
};

void GameCockpit::DrawBlips (Unit * un) {

  Unit * target;
  float s,t;
  
  float xsize[2],ysize[2],xcent[2],ycent[2];
  bool reardar[2];
  int numradar=0;
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  for (int i=0;i<2;++i) {
	  if (Radar[numradar]) {
		  Radar[numradar]->GetSize (xsize[numradar],ysize[numradar]);
		  xsize[numradar] = fabs (xsize[numradar]);
		  ysize[numradar] = fabs (ysize[numradar]);
		  Radar[numradar]->GetPosition (xcent[numradar],ycent[numradar]);
		  if ((!g_game.use_sprites)||Radar[numradar]->LoadSuccess()) {
			  DrawRadarCircles (xcent[numradar],ycent[numradar],xsize[numradar],ysize[numradar],textcol);
		  }
		  reardar[numradar]=i?true:false;
		  numradar++;
	  }
  }
  GFXPointSize (2);
  GFXBegin(GFXPOINT);
  static float unitRad = XMLSupport::parse_float(vs_config->getVariable("graphics","hud","radar_search_extra_radius","1000"));

  UnitWithinRangeLocator<DrawUnitBlip> unitLocator(un->GetComputerData().radar.maxrange, unitRad);
  unitLocator.action.init(un, this, numradar, xsize, ysize, xcent, ycent, reardar);
  if (!is_null(un->location[Unit::UNIT_ONLY])) {
    findObjects(_Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY], un->location[Unit::UNIT_ONLY], &unitLocator);
  }
  static bool allGravUnits=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_gravitational_objects","true"));
  if (allGravUnits){
    Unit *u;
    for (un_iter i=_Universe->activeStarSystem()->gravitationalUnits().createIterator();(u=*i)!=NULL;++i){
      unitLocator.action.acquire(u,UnitUtil::getDistance(un,u));      
    }
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
  GFXEnable  (SMOOTH);
  if (Radar[0]->LoadSuccess()) {
    DrawRadarCircles (xcent,ycent,xsize,ysize,textcol);
  }
  static bool draw_significant_blips = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_significant_blips","true"));
  while ((target = iter.current())!=NULL) {
    if (target!=un) {
      static bool autolanding_enable=XMLSupport::parse_bool(vs_config->getVariable("physics","AutoLandingEnable","false"));
      if (autolanding_enable)
        DoAutoLanding(this,un,target);

      double mm;
      static bool untarget_out_cone=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","untarget_beyond_cone","false"));
      if (!un->InRange (target,mm,makeBigger==target&&untarget_out_cone,true,true)) {
	if (makeBigger==target) {
	  un->Target(NULL);
	}
	iter.advance();	
	continue;
      }
      if (makeBigger!=target&&draw_significant_blips==false&&getTopLevelOwner()==target->owner){
	iter.advance();	
	continue;      
      }

      Vector localcoord (un->LocalCoordinates(target));

	LocalToRadar (localcoord,s,t);
	LocalToEliteRadar(localcoord,es,et,eh);


      GFXColor localcol (unitToColor (un,target,radarl->iff));
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
  GFXDisable (SMOOTH);
  GFXPointSize (1);
  GFXColor4f (1,1,1,1);
  GFXEnable (TEXTURE0);
}
float GameCockpit::LookupTargetStat (int stat, Unit *target) {
  static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
  static bool display_in_meters = XMLSupport::parse_bool (vs_config->getVariable("physics","display_in_meters","true"));
  static bool lie=XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
  static float fpsval=0;
  const float fpsmax=1;
  static float numtimes=fpsmax;
  float armordat[8]; //short fix
  float retval;
  int armori;
  Unit * tmpunit;
  if (shield8) {
    switch (stat) {
    case UnitImages::SHIELDF:
    case UnitImages::SHIELDR:
    case UnitImages::SHIELDL:
    case UnitImages::SHIELDB:
    case UnitImages::SHIELD4:
    case UnitImages::SHIELD5:
    case UnitImages::SHIELD6:
    case UnitImages::SHIELD7:
      if (target->shield.shield.max[stat-UnitImages::SHIELDF]) {
        return target->shield.shield.cur[stat-UnitImages::SHIELDF]/target->shield.shield.max[stat-UnitImages::SHIELDF];
      }else return 0;
    default:break;
    }
  }
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
  case UnitImages::ARMOR4:
  case UnitImages::ARMOR5:
  case UnitImages::ARMOR6:
  case UnitImages::ARMOR7:
    target->ArmorData (armordat);
    if (armor8) {
      return armordat[stat-UnitImages::ARMORF]/StartArmor[stat-UnitImages::ARMORF];
    }else {
	for (armori=0;armori<8;++armori) {	   
		if (armordat[armori]>StartArmor[armori]) {
			StartArmor[armori]=armordat[armori];
		}
		armordat[armori]/=StartArmor[armori];		
	}
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
  case UnitImages::WARPENERGY:{
    static bool warpifnojump=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","display_warp_energy_if_no_jump_drive","true"));
    return (warpifnojump||target->GetJumpStatus().drive!=-2)?target->WarpEnergyData():0;
  }
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
    {
        static float locklight_time = XMLSupport::parse_float( vs_config->getVariable("graphics","locklight_time","1") );
        bool res=false;
        if  ((tmpunit = target->GetComputerData().threat.GetUnit())) {
            res = tmpunit->cosAngleTo (target,*&armordat[0],FLT_MAX,FLT_MAX)>.95;
            if (res) last_locktime = UniverseUtil::GetGameTime();
        }
        return (res||((UniverseUtil::GetGameTime()-last_locktime) < locklight_time))?1.0f:0.0f;
    }
  case UnitImages::MISSILELOCK:
    {
        static float locklight_time = XMLSupport::parse_float( vs_config->getVariable("graphics","locklight_time","1") );
        bool res = target->graphicOptions.missilelock;
        if (res) last_mlocktime = UniverseUtil::GetGameTime();
        return (res||((UniverseUtil::GetGameTime()-last_mlocktime) < locklight_time))?1.0f:0.0f;
    }
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
  case UnitImages::SETKPS:
  case UnitImages::MAXKPS:
  case UnitImages::MAXCOMBATKPS:
  case UnitImages::MAXCOMBATABKPS:
    {
        float value;
        switch (stat) {
            case UnitImages::KPS:            value=target->GetVelocity().Magnitude(); break;
            case UnitImages::SETKPS:         value=target->GetComputerData().set_speed; break;
            case UnitImages::MAXKPS:         value=target->GetComputerData().max_speed(); break;
            case UnitImages::MAXCOMBATKPS:   value=target->GetComputerData().max_combat_speed; break;
            case UnitImages::MAXCOMBATABKPS: value=target->GetComputerData().max_combat_ab_speed; break;
            default:                         value=0;
        }
	    if (lie) 
	        return value/game_speed; else
		    return display_in_meters?value:value*3.6; //JMS 6/28/05 - converted back to raw meters/second
    }

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
                  static float no_auto_light_below = XMLSupport::parse_float(vs_config->getVariable("physics","no_auto_light_below","2000"));
                  Unit * targtarg=target->Target();;
                  if (targtarg)
                    if ((target->Position()-targtarg->Position()).Magnitude()-targtarg->rSize()-target->rSize()<no_auto_light_below) {
                      abletoautopilot=false;
                    }
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
          static string gauge_static = vs_config->getVariable("graphics","gauge_static","static.ani");          
          static Animation vdu_ani(gauge_static.c_str(),true,.1,BILINEAR);
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
  smooth_fov=g_game.fov;
  editingTextMessage=false;
  armor8=false;
  shield8=false;
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
  for (i=0;i<(int)mesh.size();++i) {
    if (mesh[i]) {
      delete mesh[i];
    }
    mesh[i] = NULL;
  }
  mesh.clear();
  if (soundfile>=0) {
	  AUDStopPlaying(soundfile);
	  AUDDeleteSound(soundfile,false);
	  soundfile=-1;
  }
  viewport_offset=cockpit_offset=0;
  for (i=0;i<4;i++) {
    /*
    if (Pit[i]) {
      delete Pit[i];
      Pit[i] = NULL;
    }
    */
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
GameCockpit::GameCockpit (const char * file, Unit * parent,const std::string &pilot_name): Cockpit( file, parent, pilot_name),shake_time(0),shake_type(0),textcol (1,1,1,1),text(NULL)
{
  autoMessageTime=0;
  shield8=false;
  editingTextMessage=false;
  static int headlag = XMLSupport::parse_int (vs_config->getVariable("graphics","head_lag","10"));
  int i;
  for (i=0;i<headlag;i++) {
    headtrans.push_back (Matrix());
    Identity(headtrans.back());
  }
  for (i=0;i<UnitImages::NUMGAUGES;i++) {
    gauges[i]=NULL;
  }
  Radar[0]=Radar[1]=Pit[0]=Pit[1]=Pit[2]=Pit[3]=NULL;

  static bool st_draw_all_boxes=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAllTargetBoxes","false"));
  static bool st_draw_line_to_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTarget","false"));
  static bool st_draw_line_to_targets_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTargetsTarget","false"));
  static bool st_draw_line_to_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToITTS","false"));
  static bool st_always_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAlwaysITTS","false"));
  static bool st_steady_itts=XMLSupport::parse_bool(vs_config->getVariable ("physics","steady_itts","false"));
  static std::string st_radar_type=vs_config->getVariable("graphics","hud","radarType","WC");

  draw_all_boxes=st_draw_all_boxes;
  draw_line_to_target=st_draw_line_to_target;
  draw_line_to_targets_target=st_draw_line_to_targets_target;
  draw_line_to_itts=st_draw_line_to_itts;
  always_itts=st_always_itts;
  steady_itts=st_steady_itts;
  radar_type=st_radar_type;
  last_locktime = last_mlocktime = -FLT_MAX;

  // Compute the screen limits. Used to display the arrow pointing to the selected target.
  static float st_projection_limit_y = XMLSupport::parse_float(vs_config->getVariable("graphics","fov","78"));
  projection_limit_y = st_projection_limit_y;
  // The angle betwwen the center of the screen and the border is half the fov.
  projection_limit_y = tan(projection_limit_y * M_PI / (180*2));
  projection_limit_x = projection_limit_y * g_game.aspect;
  // Precompute this division... performance.
  inv_screen_aspect_ratio = 1 / g_game.aspect;

  oaccel=Vector(0,0,0);

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

void DoCockpitKeys()
{
	CockpitKeys::Pan(KBData(),PRESS);
	CockpitKeys::Inside(KBData(),PRESS);
}
void GameCockpit::NavScreen (const KBData&,KBSTATE k) // scheherazade
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
	float o_fov = cam[currentcamera].GetFov();
	static float standard_fov=XMLSupport::parse_float(vs_config->getVariable("graphics","base_fov","90"));
	cam[currentcamera].SetFov(standard_fov);
    cam[currentcamera].setCockpitOffset(0);
    cam[currentcamera].UpdateGFX (GFXFALSE,GFXFALSE,GFXTRUE);
    ThisNav.Draw();
    cockpit_offset= c_o;
	cam[currentcamera].SetFov(o_fov);
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
void GameCockpit::SwitchControl (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    while (switchunit.size()<=_Universe->CurrentCockpit())
      switchunit.push_back(0);
    switchunit[_Universe->CurrentCockpit()]=1;
  }

}
void GameCockpit::ForceSwitchControl (const KBData&,KBSTATE k) {

    while (switchunit.size()<=_Universe->CurrentCockpit())
      switchunit.push_back(0);
    switchunit[_Universe->CurrentCockpit()]=1;

}
void SuicideKey (const KBData&,KBSTATE k) {
  static int orig=0;
  if (k==PRESS) {
      int newtime=time(NULL);
      if (newtime-orig>8||orig==0) {
          orig=newtime;
          Unit * un=NULL;
          if ((un = _Universe->AccessCockpit()->GetParent())) {
              float armor[8]; //short fix
              un->ArmorData(armor);
              un->DealDamageToHull(Vector(0,0,.1),un->GetHull()*256.+2+2*(armor[1]+armor[2]+armor[3]+armor[4]+armor[5]+armor[6]+armor[7]+armor[0]));
          }
      }
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
void MapKey (const KBData&,KBSTATE k) {
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


void GameCockpit::TurretControl (const KBData&,KBSTATE k) {
  if (k==PRESS) {
    while (turretcontrol.size()<=_Universe->CurrentCockpit())
      turretcontrol.push_back(0);
    turretcontrol[_Universe->CurrentCockpit()]=1;
  }

}
void GameCockpit::Respawn (const KBData&,KBSTATE k) {
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
      autoMessage=std::string();
      autoMessageTime=UniverseUtil::GetGameTime();
      QVector posA=un->LocalPosition();
      bool retautoA=false;
      if((retauto = retautoA=un->AutoPilotToErrorMessage(un,false,autoMessage))) {//can he even start to autopilot
        retauto=un->AutoPilotToErrorMessage(target,false,autoMessage);
        QVector posB=un->LocalPosition();
        bool movedatall=(posA-posB).Magnitude()>un->rSize();
        if (autoMessage.length()==0&&!movedatall) {
          autoMessage=XMLSupport::escaped_string(vs_config->getVariable("graphics","hud","AlreadyNearMessage","#ff0000Already Near#000000"));    
          retauto=false;
        }else if ((retauto||retautoA)&&movedatall) {
		if (autopan){
		  SetView (CP_FIXEDPOS);
                  Vector P(1,0,0),Q(0,1,0),R(0,0,1);
                  Vector uP,uQ,uR;
                  un->GetOrientation(uP,uQ,uR);
                  static float auto_side_bias=XMLSupport::parse_float(vs_config->getVariable("graphics","autopilot_side_bias","1.1"));
                  static float auto_front_bias=XMLSupport::parse_float(vs_config->getVariable("graphics","autopilot_front_bias","1.65"));
                  P+=uP*auto_side_bias+uR*auto_front_bias;
                  P.Normalize();
                  R= P.Cross(Q);
                  AccessCamera(CP_FIXED)->SetPosition(un->LocalPosition()+2*un->rSize()*P,Vector(0,0,0),Vector(0,0,0),Vector(0,0,0));
                  AccessCamera(CP_FIXED)->SetOrientation(R,Q,-P);
                  AccessCamera(CP_FIXEDPOS)->SetPosition(un->LocalPosition()+2*un->rSize()*P,Vector(0,0,0),Vector(0,0,0),Vector(0,0,0));
                  AccessCamera(CP_FIXEDPOS)->SetOrientation(R,Q,-P);
		}
		static bool face_target_on_auto = XMLSupport::parse_bool (vs_config->getVariable ( "physics","face_on_auto", "false"));
		if (face_target_on_auto) {
			//	  FaceTarget(un,un->LocalPosition(),un->Target());
		}	  

		static double numave = 1.0;
		
		static float autotime = XMLSupport::parse_float (vs_config->getVariable ("physics","autotime","10"));//10 seconds for auto to kick in;

		autopilot_time=autotime;
		autopilot_target.SetUnit (target);
        }
      }
    }
  }
  return retauto;
}
extern void reset_time_compression(const KBData&, KBSTATE a);
void GameCockpit::Shake (float amt,int dtype) {
  static float shak= XMLSupport::parse_float(vs_config->getVariable("graphics","cockpit_shake","3"));
  static float shak_max= XMLSupport::parse_float(vs_config->getVariable("graphics","cockpit_shake_max","20"));
  shakin+=shak;
  if (shakin>shak_max) {
    shakin=shak_max;
  }
  this->shake_time=getNewTime();
  this->shake_type=dtype;
}
static void DrawDamageFlash(int dtype) {
  const int numtypes=3;
  static string shieldflash=vs_config->getVariable("graphics","shield_flash_animation","");
  static string armorflash=vs_config->getVariable("graphics","armor_flash_animation","armorflash.ani");
  static string hullflash=vs_config->getVariable("graphics","hull_flash_animation","hullflash.ani");
  string flashes[numtypes];flashes[0]=shieldflash;flashes[1]=armorflash;flashes[2]=hullflash;
  float fallbackcolor[numtypes][4]={{0,1,.5,.2},{1,0,.2,.25},{1,0,0,.5}};
  
  static bool init=false;
  static Animation* aflashes[numtypes];
  static bool doflash[numtypes];
  if (!init) {
    init=true;
    for (int i=0;i<numtypes;++i) {
      doflash[i]=(flashes[i].length()>0);
      if (doflash[i]) {
        aflashes[i]=new Animation(flashes[i].c_str(),true,.1,BILINEAR,false,false);
      }else{
        aflashes[i]=NULL;
      }
    }
    vs_config->getColor("shield_flash",fallbackcolor[0]);
    vs_config->getColor("armor_flash",fallbackcolor[1]);
    vs_config->getColor("hull_flash",fallbackcolor[2]);
  }
  if (dtype<numtypes) {
    int i=dtype;

    if (aflashes[i]) {
      GFXPushBlendMode();
      static bool damage_flash_alpha=XMLSupport::parse_bool(vs_config->getVariable("graphics","damage_flash_alpha","true"));
      if (damage_flash_alpha)
        GFXBlendMode(SRCALPHA,INVSRCALPHA);
      else
        GFXBlendMode(ONE,ZERO);
      if (aflashes[i]->LoadSuccess()) {
        aflashes[i]->MakeActive();        
        GFXColor4f(1,1,1,1);
        GFXBegin (GFXQUAD);       
        float width=1,height=1;
        GFXTexCoord2f (0.00F,1.00F);
        GFXVertex3f (-width,-height,0.00F);  //lower left
        GFXTexCoord2f (1.00F,1.00F);
        GFXVertex3f (width,-height,0.00F);  //upper left
        GFXTexCoord2f (1.00F,0.00F);
        GFXVertex3f (width,height,0.00F);  //upper right
        GFXTexCoord2f (0.00F,0.00F);
        GFXVertex3f (-width,height,0.00F);  //lower right
        GFXEnd ();
        
      }else{ 
        GFXColor4f(fallbackcolor[i][0],
                   fallbackcolor[i][1],
                   fallbackcolor[i][2],
                   fallbackcolor[i][3]);
        GFXDisable(TEXTURE0);
        GFXBegin(GFXQUAD);
        GFXVertex3f(-1.0f,-1.0f,0.0f);
        GFXVertex3f(-1.0f,1.0f,0.0f);
        GFXVertex3f(1.0f,1.0f,0.0f);
        GFXVertex3f(1.0f,-1.0f,0.0f);
        GFXEnd();
        GFXEnable(TEXTURE0);
      }
      GFXPopBlendMode();        
    }

  }
  GFXColor4f(1,1,1,1);


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
extern bool screenshotkey;
QVector SystemLocation(std::string system);
double howFarToJump();
void GameCockpit::Draw() {
  static GFXColor destination_system_color=DockBoxColor ("destination_system_color");             
  Vector destination_system_location(0,0,0);
  cockpit_time+=GetElapsedTime();
  if (cockpit_time>=100000)
    InitStatic();
  _Universe->AccessCamera()->UpdateGFX (GFXFALSE,GFXFALSE,GFXTRUE); //Preliminary frustum
  GFXDisable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXLoadIdentity(MODEL);
  GFXDisable(LIGHTING);
  GFXDisable(DEPTHTEST);
  GFXDisable(DEPTHWRITE);
  GFXColor4f(1,1,1,1);
  static bool draw_any_boxes = XMLSupport::parse_bool (vs_config->getVariable("graphics","hud","DrawTargettingBoxes","true"));
  static bool draw_boxes_inside_only=XMLSupport::parse_bool (vs_config->getVariable("graphics","hud","DrawTargettingBoxesInside","true"));
  if (draw_any_boxes&&screenshotkey==false&&(draw_boxes_inside_only==false||view<CP_CHASE)) {
      DrawTargetBox();
      DrawTurretTargetBoxes();
      DrawTacticalTargetBox();
      DrawCommunicatingBoxes();
      if(draw_all_boxes){
        DrawTargetBoxes();
      }
      {
        std::string destination_system=AccessNavSystem()->getSelectedSystem();
        std::string current_system=_Universe->activeStarSystem()->getFileName();
        if (destination_system!=current_system) {
          QVector cur=SystemLocation(current_system);
          QVector dest=SystemLocation(destination_system);
          QVector delta=dest-cur;
          if (delta.i!=0||dest.j!=0||dest.k!=0) {
            delta.Normalize();         
            Unit * par=GetParent();
            delta=delta*howFarToJump()*1.01-(par?(par->Position()):QVector(0,0,0));
            destination_system_location=delta.Cast();
	    Vector P,Q,R;
            static float nav_symbol_size = XMLSupport::parse_float(vs_config->getVariable("graphics","nav_symbol_size",".25"));
	    AccessCamera()->GetPQR (P,Q,R);

            
            GFXColor4f(destination_system_color.r,
                       destination_system_color.g,
                       destination_system_color.b,
                       destination_system_color.a);
            
			static GFXColor suncol=RetrColor("remote_star",GFXColor(0,1,1,.8));
			GFXColorf(suncol);
            DrawNavigationSymbol(delta.Cast(),P,Q,delta.Magnitude()*nav_symbol_size);


            GFXColor4f(1,1,1,1);
          }
          
        }
      }
      GFXEnable (TEXTURE0);
      GFXEnable (DEPTHTEST);
      GFXEnable (DEPTHWRITE);      
  }
  if (view<CP_CHASE) {
    if (mesh.size()){
      Unit * par=GetParent();
      if (par) {
	    //cockpit is unaffected by FOV WARP-Link
        float oldfov=AccessCamera()->GetFov();
        AccessCamera()->SetFov(g_game.fov);

	    GFXLoadIdentity(MODEL);

        size_t i,j;
        float cockpitradial=1; //LET IT NOT BE ZERO!
		for (i=0; i<mesh.size();++i) {
			float meshmaxdepth = mesh[i]->corner_min().Max(mesh[i]->corner_max()).Magnitude();
            if (meshmaxdepth>cockpitradial) 
                cockpitradial=meshmaxdepth;
		}
        cockpitradial *= COCKPITZ_HEADROOM;

	    GFXEnable (DEPTHTEST);
	    GFXEnable(DEPTHWRITE);
	    GFXEnable (TEXTURE0);
	    GFXEnable (LIGHTING);
	    Vector P,Q,R;
	    AccessCamera(CP_FRONT)->GetPQR (P,Q,R);

        headtrans.clear();

	    headtrans.push_back (Matrix());
	    VectorAndPositionToMatrix(headtrans.back(),P,Q,R,QVector(0,0,0));
	    static float theta=0,wtheta=0;
	    static float shake_speed = XMLSupport::parse_float(vs_config->getVariable ("graphics","shake_speed","50"));
	    static float shake_reduction = XMLSupport::parse_float(vs_config->getVariable ("graphics","shake_reduction","8"));
        static float shake_limit = XMLSupport::parse_float(vs_config->getVariable ("graphics","shake_limit","25"));
        static float shake_mag   = XMLSupport::parse_float(vs_config->getVariable ("graphics","shake_magnitude","0.3"));
        static float drift_limit = XMLSupport::parse_float(vs_config->getVariable ("graphics","cockpit_drift_limit","1.00"));
        static float drift_amount= XMLSupport::parse_float(vs_config->getVariable ("graphics","cockpit_drift_amount","0.15"));
        static float drift_ref_accel = XMLSupport::parse_float(vs_config->getVariable ("graphics","cockpit_drift_ref_accel","100"));

        static float warp_shake_mag = XMLSupport::parse_float(vs_config->getVariable ("graphics","warp_shake_magnitude","0.125"));
        static float warp_shake_speed = XMLSupport::parse_float(vs_config->getVariable ("graphics","warp_shake_speed","70"));
        static float warp_shake_ref = XMLSupport::parse_float(vs_config->getVariable ("graphics","warp_shake_ref","2000"));

        if (warp_shake_ref<=0) warp_shake_ref=1;
	    theta+=shake_speed*GetElapsedTime()*sqrt(fabs(shakin))/10; //For small shakes, slower shakes
        wtheta+=warp_shake_speed*GetElapsedTime(); //SPEC-related shaking

        float self_kps = ((GetParent()!=NULL)?LookupTargetStat(UnitImages::KPS,GetParent()):0);
        float self_setkps = max(1.0f,((GetParent()!=NULL)?LookupTargetStat(UnitImages::SETKPS,GetParent()):0));
        float warp_strength = max(0.0f,min(max(0.0f,min(1.0f,self_kps/self_setkps)),((GetParent()!=NULL)?LookupTargetStat(UnitImages::WARPFIELDSTRENGTH,GetParent()):0.0f) / warp_shake_ref));

        if (shakin>shake_limit) shakin=shake_limit;
	    headtrans.front().p.i=shake_mag*shakin*cos(theta)*cockpitradial/100;//AccessCamera()->GetPosition().i+shakin*cos(theta);
	    headtrans.front().p.j=shake_mag*shakin*cos(1.3731*theta)*cockpitradial/100;//AccessCamera()->GetPosition().j+shakin*cos(theta);
	    headtrans.front().p.k=0;//AccessCamera()->GetPosition().k;
	    headtrans.front().p.i+=warp_shake_mag*cos(wtheta)*sqr(warp_strength)*cockpitradial/100;//AccessCamera()->GetPosition().i+shakin*cos(theta);
	    headtrans.front().p.j+=warp_shake_mag*cos(1.165864*wtheta)*sqr(warp_strength)*cockpitradial/100;//AccessCamera()->GetPosition().j+shakin*cos(theta);
	    if (shakin>0) {
	      shakin-=GetElapsedTime()*shake_reduction*(shakin/5); //Fast convergence to 5% shaking, slow stabilization
	      if (shakin<=0) {
	        shakin=0;
	      }
	    }

        //Now, compute head drift
        Vector caccel = AccessCamera(CP_FRONT)->GetAcceleration();
        float mag=caccel.Magnitude();
        float ref=drift_ref_accel*drift_ref_accel;
        if ((mag>0)&&(ref>0)) caccel *= - (drift_amount*min(drift_limit,(float)(mag*mag/ref)))/mag; else caccel=Vector(0,0,0);
        float driftphase = pow(0.25,GetElapsedTime());
        oaccel = (1-driftphase)*caccel+driftphase*oaccel;
        headtrans.front().p += -cockpitradial*oaccel;
        float driftmag = cockpitradial*oaccel.Magnitude();

        //if (COCKPITZ_PARTITIONS>1) GFXClear(GFXFALSE,GFXFALSE,GFXTRUE);//only clear stencil buffer
        static int COCKPITZ_PARTITIONS = XMLSupport::parse_int(vs_config->getVariable ("graphics","cockpit_z_partitions","1")); //Should not be needed if VERYNEAR_CONST is propperly set, but would be useful with stenciled inverse order rendering.
        float zrange=cockpitradial*(1-VERYNEAR_CONST)+driftmag;
        float zfloor=cockpitradial*VERYNEAR_CONST;
        for (j=COCKPITZ_PARTITIONS-1; j<COCKPITZ_PARTITIONS; j--) {
            AccessCamera()->UpdateGFX(GFXTRUE,GFXTRUE,GFXTRUE,GFXTRUE,zfloor+zrange*j/COCKPITZ_PARTITIONS,zfloor+zrange*(j+1)/COCKPITZ_PARTITIONS); //cockpit-specific frustrum (with clipping, with frustrum update)
            GFXClear(GFXFALSE,GFXTRUE,GFXFALSE);//only clear Z
            /*if (COCKPITZ_PARTITIONS>1) {
                //Setup stencil
                GFXStencilOp(KEEP,KEEP,REPLACE);
                GFXStencilFunc(LEQUAL,COCKPITZ_PARTITIONS-j,~0);
                GFXStencilMask(~0);
                GFXEnable(STENCIL);
            };*/
            _Universe->activateLightMap();
            for (i=0;i<mesh.size();++i) {
                //mesh[i]->DrawNow(1,true,headtrans.front());
                mesh[i]->Draw(FLT_MAX,headtrans.front());
            }

            //Whether cockpits shouldn't cull faces - not sure why, probably because 
            //modellers always set normals the wrong way for cockpits.
            static bool nocockpitcull = XMLSupport::parse_bool(vs_config->getVariable ("graphics","cockpit_no_face_cull","true")); 

            Mesh::ProcessZFarMeshes(true);
            if (nocockpitcull) GFXDisable(CULLFACE);
            Mesh::ProcessUndrawnMeshes(false,true);
        };
	    headtrans.pop_front();
        //if (COCKPITZ_PARTITIONS>1) GFXDisable(STENCIL);
	    GFXDisable(LIGHTING);
	    GFXDisable(TEXTURE0);
        GFXDisable(TEXTURE1);
        AccessCamera()->SetFov(oldfov);
      }
    }

    _Universe->AccessCamera()->UpdateGFX(GFXFALSE,GFXFALSE,GFXTRUE,GFXFALSE,0,1000000); //Restore normal frustrum
  }
  GFXHudMode (true);
  static float damage_flash_length=XMLSupport::parse_float(vs_config->getVariable("graphics","damage_flash_length",".1"));
  static bool damage_flash_first=XMLSupport::parse_bool(vs_config->getVariable("graphics","flash_behind_hud","true"));
  if (view<CP_CHASE&&damage_flash_first&&getNewTime()-shake_time<damage_flash_length) {
    DrawDamageFlash(shake_type);
  }
  GFXColor4f (1,1,1,1);
  GFXBlendMode (ONE,ONE);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);

  Unit * un;
  float crosscenx=0,crossceny=0;
  static bool crosshairs_on_chasecam=parse_bool(vs_config->getVariable("graphics","hud","crosshairs_on_chasecam","false"));
  static bool crosshairs_on_padlock =parse_bool(vs_config->getVariable("graphics","hud","crosshairs_on_padlock","false"));

  if (  (view==CP_FRONT)
	  ||(view==CP_CHASE&&crosshairs_on_chasecam)
	  ||(view==CP_VIEWTARGET&&crosshairs_on_padlock)  ) {
    if (Panel.size()>0&&Panel.front()&&screenshotkey==false) {
      static bool drawCrosshairs=parse_bool(vs_config->getVariable("graphics","hud","draw_rendered_crosshairs",vs_config->getVariable("graphics","draw_rendered_crosshairs","true")));
      Panel.front()->GetPosition(crosscenx,crossceny);
      if (drawCrosshairs) {
        float x,y,wid,hei;
        Panel.front()->GetSize(wid,hei);
        x=crosscenx;y=crossceny;
        DrawCrosshairs(x,y,wid,hei,textcol);
      } else {
        GFXBlendMode(SRCALPHA,INVSRCALPHA);
        GFXEnable (TEXTURE0);
        Panel.front()->Draw();//draw crosshairs
      }
    }
  }
  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  GFXEnable (TEXTURE0);

  RestoreViewPort();

  static bool blend_panels = XMLSupport::parse_bool(vs_config->getVariable("graphics","blend_panels","false"));
  static bool blend_cockpit= XMLSupport::parse_bool(vs_config->getVariable("graphics","blend_cockpit","false"));
  static bool drawChaseVDU  = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_chase_cam","false"));
  static bool drawPanVDU    = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_panning_cam","false"));
  static bool drawTgtVDU    = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_target_cam","false"));
  static bool drawPadVDU    = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_vdus_from_padlock_cam","false"));

  static bool drawChasecp   = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_cockpit_from_chase_cam","false"));
  static bool drawPancp     = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_cockpit_from_panning_cam","false"));
  static bool drawTgtcp     = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_cockpit_from_target_cam","false"));
  static bool drawPadcp     = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_cockpit_from_padlock_cam","false"));

  static float AlphaTestingCutoff = XMLSupport::parse_float(vs_config->getVariable("graphics","AlphaTestCutoff",".8"));

  if (blend_cockpit) {
    GFXAlphaTest (ALWAYS,0);
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
  } else {
    GFXBlendMode (ONE,ZERO);
    GFXAlphaTest (GREATER,AlphaTestingCutoff);
  }
  GFXColor4f(1,1,1,1);
  if (view<CP_CHASE) {
    if (Pit[view]) {
      Pit[view]->Draw();
    }
  } else if ((view==CP_CHASE&&drawChasecp)||(view==CP_PAN&&drawPancp)||(view==CP_TARGET&&drawTgtcp)||(view==CP_VIEWTARGET&&drawPadcp)) {
    if (Pit[0])
      Pit[0]->Draw();

  }
  if (blend_panels) {
    GFXAlphaTest (ALWAYS,0);
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
  } else {
    GFXBlendMode (ONE,ZERO);
    GFXAlphaTest (GREATER,AlphaTestingCutoff);
  }
  GFXColor4f(1,1,1,1);
  if (view==CP_FRONT||(view==CP_CHASE&&drawChaseVDU)||(view==CP_PAN&&drawPanVDU)||(view==CP_TARGET&&drawTgtVDU)||(view==CP_VIEWTARGET&&drawPadVDU))
      for (unsigned int j=1;j<Panel.size();j++)
          if (Panel[j])
              Panel[j]->Draw();

  GFXAlphaTest (ALWAYS,0);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXColor4f(1,1,1,1);
  bool die=true;
  if ((un = parent.GetUnit())) {
    if (view==CP_FRONT||(view==CP_CHASE&&drawChaseVDU)||(view==CP_PAN&&drawPanVDU)||(view==CP_TARGET&&drawTgtVDU)||(view==CP_VIEWTARGET&&drawPadVDU)) {//only draw crosshairs for front view
      DrawGauges(un);

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

      
      GFXColor4f(1,1,1,1);
      for (unsigned int vd=0;vd<vdu.size();vd++) {
	if (vdu[vd]) {
	  vdu[vd]->Draw(this,un,textcol);
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
                  static string vdustatic=vs_config->getVariable("graphics","vdu_static","static.ani");
                  static Animation vdu_ani(vdustatic.c_str(),true,.1,BILINEAR);
                  static soundContainer ejectstopsound;
                  if (ejectstopsound.sound<0) {
                    static string str=vs_config->getVariable("cockpitaudio","vdu_static","vdu_static");
                    ejectstopsound.loadsound(str);
                  }
                  if (!AUDIsPlaying(ejectstopsound.sound)) {
                    ejectstopsound.playsound();
                  }

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
	  //////////////////// DISPLAY CURRENT POSITION ////////////////////
	  static bool debug_position = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","debug_position","false"));
	  if (debug_position) {
		  TextPlane tp;
		  char str[400]; // don't make the sprintf format too big... :-P
		  Unit *you=parent.GetUnit();
		  if (you) {
			  sprintf(str,"Your Position: (%lf,%lf,%lf); Velocity: (%f,%f,%f); Frame: %lf\n",you->curr_physical_state.position.i,you->curr_physical_state.position.j,you->curr_physical_state.position.k,you->Velocity.i,you->Velocity.j,you->Velocity.k, getNewTime());
			  Unit *yourtarg=you->computer.target.GetUnit();
			  if (yourtarg) {
				  sprintf(str+strlen(str),"Target Position: (%lf,%lf,%lf); Velocity: (%f,%f,%f); Now: %lf\n",yourtarg->curr_physical_state.position.i,yourtarg->curr_physical_state.position.j,yourtarg->curr_physical_state.position.k,yourtarg->Velocity.i,yourtarg->Velocity.j,yourtarg->Velocity.k, queryTime());
			  }
		  }
		  tp.SetPos(-0.8,-0.8);
		  tp.SetText(str);
		  tp.Draw();
	  }
	  //////////////////////////////////////////////////////////////////////////
    }
    GFXColor4f (1,1,1,1);
    if (un->GetHull()>=0)
      die = false;
    if (un->Threat()!=NULL) {
      if (0&&getTimeCompression()>1) {
	reset_time_compression(std::string(),PRESS);
      }
      un->Threaten (NULL,0);
    }
    if (_Universe->CurrentCockpit()<univmap.size()) {
      univmap[_Universe->CurrentCockpit()].Draw();
    }
    // Draw the arrow to the target.
    static bool drawarrow = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","draw_arrow_to_target","true"));
    
    {
      Unit * parent=NULL;
      if (drawarrow&&(parent=this->parent.GetUnit())) {
        DrawArrowToTarget(parent, parent->Target());
        if (destination_system_location.i||destination_system_location.j||destination_system_location.k) {
          GFXColorf(destination_system_color);
          DrawArrowToTarget(parent,parent->ToLocalCoordinates(destination_system_location));
        }
      }
    }
  }
  GFXColor4f(1,1,1,1);
  if (QuitAllow||getTimeCompression()<.5) {
    if (QuitAllow){ 
      if (!die){
        static VSSprite QuitSprite("quit.spr",BILINEAR,GFXTRUE);
        
        GFXEnable(TEXTURE0);
        QuitSprite.Draw();	  
      }
    }else {
      static VSSprite PauseSprite("pause.spr",BILINEAR,GFXTRUE);          
      GFXEnable(TEXTURE0);
      PauseSprite.Draw();	        
    }
  }
  static float dietime = 0;
  if (die) {
    if (un) {
      if (un->GetHull()>=0) {
        die=false;
      }
    }
    if (die) {
      
      if (text) {
        GFXColor4f (1,1,1,1);
        text->SetSize(1,-1);
        float x; float y;
        if (dietime==0) {
          editingTextMessage=false;
          RestoreKB();
          if (respawnunit.size()>_Universe->CurrentCockpit()) 
            if (respawnunit[_Universe->CurrentCockpit()]==1&&_Universe->numPlayers()==1) {
              respawnunit[_Universe->CurrentCockpit()]=0;
            }
          text->GetCharSize (x,y);
          text->SetCharSize (x*4,y*4);
          text->SetPos (0-(x*2*14),0-(y*2));
          char playr[3];
          playr[0]='p';
          playr[1]='0'+_Universe->CurrentCockpit();
          playr[2]='\0';
          

        }
        GFXColorf (textcol);
        static bool show_died_text=XMLSupport::parse_bool(vs_config->getVariable("graphics","show_respawn_text","false"));
        if (show_died_text) {
          text->Draw ("#ff5555You Have Died!\n#000000Press #8080FF;#000000 (semicolon) to respawn\nOr Press #8080FFEsc and 'q'#000000 to quit");
        }
        GFXColor4f (1,1,1,1);

        
        static float min_die_time= XMLSupport::parse_float(vs_config->getVariable("graphics","death_scene_time","4"));
        if (dietime>min_die_time) {
          static VSSprite DieSprite("died.spr",BILINEAR,GFXTRUE);
          GFXBlendMode(SRCALPHA,INVSRCALPHA);
          GFXEnable(TEXTURE0);
          DieSprite.Draw();                         
        }
	dietime +=GetElapsedTime();
	SetView (CP_PAN);
        CockpitKeys::YawLeft (std::string(),RELEASE);
        CockpitKeys::YawRight (std::string(),RELEASE);
        CockpitKeys::PitchUp(std::string(),RELEASE);
        CockpitKeys::PitchDown (std::string(),RELEASE);
	zoomfactor=dietime*10;
      }
      QuitAllow=true;
    }else {
    }
  }else{
    if (dietime!=0) {
      QuitAllow=false;
      dietime=0;
    }    
  }
  //if(CommandInterpretor.console){
  //  GFXColorf(textcol);
  //  CommandInterpretor.renderconsole();
  //}
  GFXAlphaTest (ALWAYS,0);  
  static bool mouseCursor = XMLSupport::parse_bool (vs_config->getVariable ("joystick","mouse_cursor","false"));
  if (mouseCursor&&screenshotkey==false) {  
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
    GFXColor4f (1,1,1,1);
    GFXEnable(TEXTURE0);
    //    GFXDisable (DEPTHTEST);
    //    GFXDisable(TEXTURE1);
    static int revspr = XMLSupport::parse_bool (vs_config->getVariable ("joystick","reverse_mouse_spr","true"))?1:-1;
    static string blah = vs_config->getVariable("joystick","mouse_crosshair","crosshairs.spr");
    static int num=printf ("CROSS %f\n",crossceny);
    static VSSprite MouseVSSprite (blah.c_str(),BILINEAR,GFXTRUE);
    MouseVSSprite.SetPosition ((-1+float(mousex)/(.5*g_game.x_resolution))*(1-fabs(crosscenx))+crosscenx,(-revspr+float(revspr*mousey)/(.5*g_game.y_resolution))*(1-fabs(crossceny))+crossceny);
    
    MouseVSSprite.Draw();
    //    DrawGlutMouse(mousex,mousey,&MouseVSSprite);
    //    DrawGlutMouse(mousex,mousey,&MouseVSSprite);
  }
  if (view<CP_CHASE&&damage_flash_first==false&&getNewTime()-shake_time<damage_flash_length) {
    DrawDamageFlash(shake_type);
  }

  GFXHudMode (false);

  {
    //again, NAV computer is unaffected by FOV WARP-Link
    float oldfov=AccessCamera()->GetFov();
    AccessCamera()->SetFov(g_game.fov);
    AccessCamera()->UpdateGFXAgain();
    DrawNavSystem();
    AccessCamera()->SetFov(oldfov);
    AccessCamera()->UpdateGFXAgain();
  }

  GFXEnable (DEPTHWRITE);
  GFXEnable (DEPTHTEST);
  GFXEnable (TEXTURE0);
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
void	SetStartupView(Cockpit*);
void GameCockpit::UpdAutoPilot()
{
  static bool autopan = XMLSupport::parse_bool (vs_config->getVariable ("graphics","pan_on_auto","true"));
  if (autopilot_time!=0) {
    autopilot_time-=SIMULATION_ATOM;
    {
      static float autospeed = XMLSupport::parse_float (vs_config->getVariable ("physics","autospeed",".020"));//10 seconds for auto to kick in;
      if (autopan) {
        Vector origR=Vector(0,0,1);
        Vector origP=Vector(1,0,0);
        
        static float rotspd=XMLSupport::parse_float(vs_config->getVariable("graphics","autopilot_rotation_speed",".15"));

        static float curtime=0;
        curtime+=SIMULATION_ATOM;
        float ang = curtime*rotspd;
        origR.Yaw(ang);
        origP.Yaw(ang);
        Vector origQ = Vector(0,1,0);
        origP.Normalize();
        origQ.Normalize();
        origR.Normalize();
        AccessCamera(CP_FIXED)->myPhysics.SetAngularVelocity(Vector(0,0,0));//hack
        //AccessCamera(CP_FIXED)->SetOrientation(origP,origQ,origR);
        static float initialzoom=XMLSupport::parse_float(vs_config->getVariable("graphics","inital_zoom_factor","2.25"));
        zoomfactor=initialzoom;
      }

    }
    if (autopilot_time<= 0) {
      AccessCamera(CP_FIXED)->myPhysics.SetAngularVelocity(Vector(0,0,0));
      if (disableautosound.sound<0) {
	static string str=vs_config->getVariable("cockpitaudio","autopilot_disabled","autopilot_disabled");
	disableautosound.loadsound(str);
      }
      disableautosound.playsound();
      if (autopan) {
	AccessCamera(CP_FIXED)->myPhysics.SetAngularVelocity(Vector(0,0,0));
	SetStartupView(this);
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
  int i;
  for (i=0;i<4;i++) {
    if (Pit[i]) {
      delete Pit[i];
      Pit[i] = NULL;
    }
  }

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
  if (ThisNav.CheckDraw()) {
	  ThisNav.scroll(howmuch);
  } else {
    for (unsigned int i=0;i<vdu.size();i++) {
      ScrollVDU (i,howmuch);
    }
  }
}

void GameCockpit::SetStaticAnimation () {
  static string comm_static = vs_config->getVariable("graphics","comm_static","static.ani");
  static Animation Statuc (comm_static.c_str());
  for (unsigned int i=0;i<vdu.size();i++) {
    if (vdu[i]->getMode()==VDU::COMM) {
      vdu[i]->SetCommAnimation (&Statuc,NULL,true);
    }
  }
}
void GameCockpit::SetCommAnimation (Animation * ani,Unit*un) {
  bool seti=false;
  for (unsigned int i=0;i<vdu.size();i++) {
    if (vdu[i]->SetCommAnimation (ani,un,false)) {
      seti=true;
      break;
    }
  }
  if (!seti) {
    for (unsigned int i=0;i<vdu.size();i++) {
      if (vdu[i]->SetCommAnimation (ani,un,true)) {
        break;
      }
    }    
  }
}
void GameCockpit::RestoreViewPort() {
  _Universe->AccessCamera()->RestoreViewPort(0,0);
}
static void FaceCamTarget(Cockpit * cp, int cam, Unit * un) {
    QVector diff=un->Position()-cp->AccessCamera()->GetPosition();
    diff.Normalize();
    if (diff.i!=0&&diff.k!=0) {
        Vector z=diff.Cross(QVector(0,1,0)).Cast();
        cp->AccessCamera(cam)->SetOrientation(z,Vector(0,1,0),diff.Cast());
    }
}
static void ShoveCamBehindUnit (int cam, Unit * un, float zoomfactor) {
  QVector unpos = un->GetPlanetOrbit()?un->LocalPosition():un->Position();
  _Universe->AccessCamera(cam)->SetPosition(unpos-_Universe->AccessCamera()->GetR().Cast()*(un->rSize()+g_game.znear*2)*zoomfactor,un->GetWarpVelocity(),un->GetAngularVelocity(),un->GetAcceleration());
}
static void ShoveCamBelowUnit (int cam, Unit * un, float zoomfactor) {
  QVector unpos = un->GetPlanetOrbit()?un->LocalPosition():un->Position();
  Vector p,q,r;
  _Universe->AccessCamera(cam)->GetOrientation(p,q,r);
  static float ammttoshovecam = XMLSupport::parse_float(vs_config->getVariable("graphics","shove_camera_down",".3"));
  _Universe->AccessCamera(cam)->SetPosition(unpos-(r-ammttoshovecam*q).Cast()*(un->rSize()+g_game.znear*2)*zoomfactor,un->GetWarpVelocity(),un->GetAngularVelocity(),un->GetAcceleration());
}
static Vector lerp(const Vector &a, const Vector &b, float t)
{
	t = min(1.0f,max(0.0f,t));
	return a*(1-t)+b*t;
}
static void translate_as(Vector &p, Vector &q, Vector &r, Vector p1, Vector q1, Vector r1, Vector p2, Vector q2, Vector r2)
{
	// Translate p,q,r to <p1,q1,r1> base
	p = Vector(p.Dot(p1),p.Dot(q1),p.Dot(r1));
	q = Vector(q.Dot(p1),q.Dot(q1),q.Dot(r1));
	r = Vector(r.Dot(p1),r.Dot(q1),r.Dot(r1));
	// Interpret now as if it were in <p2,q2,r2> base
	p = p2*p.i + q2*p.j + r2*p.k;
	q = p2*q.i + q2*q.j + r2*q.k;
	r = p2*r.i + q2*r.j + r2*r.k;
}

void GameCockpit::SetupViewPort (bool clip) {
  _Universe->AccessCamera()->RestoreViewPort (0,(view==CP_FRONT?viewport_offset:0));
   GFXViewPort (0,(int)((view==CP_FRONT?viewport_offset:0)*g_game.y_resolution), g_game.x_resolution,g_game.y_resolution);
  _Universe->AccessCamera()->setCockpitOffset (view<CP_CHASE?cockpit_offset:0);
  Unit * un, *tgt;
  if ((un = parent.GetUnit())) {
    //Previous frontal orientation - useful, sometimes...
    Vector prev_fp,prev_fq,prev_fr;
    _Universe->AccessCamera(CP_FRONT)->GetOrientation(prev_fp,prev_fq,prev_fr);

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

	  // Padlock block
	  if (view == CP_VIEWTARGET) {
		  static float PadlockViewLag       = XMLSupport::parse_float( vs_config->getVariable("graphics","hud","PadlockViewLag","1.5") );
		  static float PadlockViewLag_inv   = 1.f/PadlockViewLag;
		  static float PadlockViewLag_fix   = XMLSupport::parse_float( vs_config->getVariable("graphics","hud","PadlockViewLagFixZone","0.0872") ); // ~5 deg
		  static float PadlockViewLag_fixcos= (float)cos(PadlockViewLag_fix);

		  // pp,qq,rr <-- world-relative padlock target
		  // p_p,p_q,p_r <-- previous head orientation translated to new front orientation
		  Vector p_p,p_q,p_r,f_p,f_q,f_r,pp=tmp,qq=q,rr=r;
		  _Universe->AccessCamera(CP_VIEWTARGET)->GetOrientation(p_p,p_q,p_r);
		  _Universe->AccessCamera(CP_FRONT)->GetOrientation(f_p,f_q,f_r);
		  translate_as(p_p,p_q,p_r,prev_fp,prev_fq,prev_fr,f_p,f_q,f_r);

		  // Compute correction amount (vtphase), accounting for lag and fix-zone
		  un->UpdateHudMatrix (CP_VIEWTARGET);
		  bool fixzone = (rr.Dot(p_r)>=PadlockViewLag_fixcos)&&(qq.Dot(p_q)>=PadlockViewLag_fixcos);
		  float vtphase = 1.0f-(float)pow(0.1,GetElapsedTime()*PadlockViewLag_inv*(fixzone?0.1f:1.0f));

		  // Apply correction
		  _Universe->AccessCamera(CP_VIEWTARGET)->SetOrientation(
			  lerp(p_p,pp,vtphase).Normalize(),
			  lerp(p_q,qq,vtphase).Normalize(),
			  lerp(p_r,rr,vtphase).Normalize());
	  } else {
		  // Reset padlock matrix
		  un->UpdateHudMatrix (CP_VIEWTARGET);
	  }

      _Universe->AccessCamera(CP_TARGET)->SetOrientation(tmp,q,r);
      //      _Universe->AccessCamera(CP_PANTARGET)->SetOrientation(tmp,q,r);
      ShoveCamBelowUnit (CP_TARGET,un,zoomfactor);
      ShoveCamBehindUnit (CP_PANTARGET,tgt,zoomfactor);
      FaceCamTarget(this,CP_FIXEDPOSTARGET,tgt);

    }else {
      un->UpdateHudMatrix (CP_VIEWTARGET);
      un->UpdateHudMatrix (CP_TARGET);
      un->UpdateHudMatrix (CP_PANTARGET);
    }
    ShoveCamBelowUnit (CP_CHASE,un,zoomfactor);
    //    ShoveCamBehindUnit (CP_PANTARGET,un,zoomfactor);
    FaceCamTarget(this,CP_FIXEDPOS,un);


    ShoveCamBehindUnit (CP_PAN,un,zoomfactor);
    un->SetVisible(view>=CP_CHASE);

    // WARP-FOV link
    {
        static float stable_lowarpref   = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.loref","1") );
        static float stable_hiwarpref   = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.hiref","100000") );
        static float stable_refexp      = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.exp","0.5") );
        static bool  stable_asymptotic  = XMLSupport::parse_bool ( vs_config->getVariable("graphics","warp.fovlink.stable.asymptotic","1") );
        static float stable_offset_f    = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.offset.front","0") );
        static float stable_offset_b    = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.offset.back","0") );
        static float stable_offset_p    = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.offset.perpendicular","0") );
        static float stable_multiplier_f= XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.multiplier.front","0.85") );
        static float stable_multiplier_b= XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.multiplier.back","1.5") );
        static float stable_multiplier_p= XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.stable.multiplier.perpendicular","1.25") );

        static float shake_lowarpref    = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.loref","10000") );
        static float shake_hiwarpref    = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.hiref","200000") );
        static float shake_refexp       = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.exp","1.5") );
        static bool  shake_asymptotic   = XMLSupport::parse_bool ( vs_config->getVariable("graphics","warp.fovlink.shake.asymptotic","1") );
        static float shake_speed        = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.speed","10") );
        static float shake_offset_f     = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.offset.front","0") );
        static float shake_offset_b     = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.offset.back","0") );
        static float shake_offset_p     = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.offset.perpendicular","0") );
        static float shake_multiplier_f = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.multiplier.front","0") );
        static float shake_multiplier_b = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.multiplier.back","0") );
        static float shake_multiplier_p = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.shake.multiplier.perpendicular","0") );

        static float refkpsoverride     = XMLSupport::parse_float( vs_config->getVariable("graphics","warp.fovlink.referencekps","0") ); // 0 means automatic

        static float theta=0;
        theta+=shake_speed*GetElapsedTime();

        if (stable_lowarpref==stable_hiwarpref) stable_hiwarpref = stable_lowarpref+1;
        if (shake_lowarpref ==shake_hiwarpref ) shake_hiwarpref  = shake_lowarpref+1;

        float warpfieldstrength=LookupTargetStat(UnitImages::WARPFIELDSTRENGTH,un);
        float refkps=(refkpsoverride>0)?refkpsoverride:LookupTargetStat(UnitImages::MAXCOMBATABKPS,un); //This one is stable, as opposed to SETKPS - for full stability, use the override (user override of governor settings will create weird behaviour if done under SPEC)
        float kps=LookupTargetStat(UnitImages::KPS,un);
        float st_warpfieldstrength=pow((max(stable_lowarpref,min(stable_asymptotic?FLT_MAX:stable_hiwarpref,warpfieldstrength))-stable_lowarpref)/(stable_hiwarpref-stable_lowarpref),stable_refexp);
        float sh_warpfieldstrength=pow((max(shake_lowarpref,min(shake_asymptotic?FLT_MAX:shake_hiwarpref,warpfieldstrength))-shake_lowarpref)/(shake_hiwarpref-shake_lowarpref),shake_refexp);
        float costheta = cos(theta);
        if (stable_asymptotic) st_warpfieldstrength = atan(st_warpfieldstrength);
        if (shake_asymptotic)  sh_warpfieldstrength = atan(sh_warpfieldstrength);
        if (refkps<=1) refkps=1;
        if (kps>refkps) kps=refkps;
        float unv = un->GetVelocity().Magnitude();
        float camv = _Universe->AccessCamera()->GetR().Magnitude();
        if (unv<=1) unv=1;
        if (camv<=1) camv=1;
        float cosangle = (un->GetVelocity() * _Universe->AccessCamera()->GetR()) / (unv*camv) * (kps/refkps);
        float st_offs,sh_offs,st_mult,sh_mult;
        if (cosangle>0) {
             st_offs = stable_offset_f*cosangle + stable_offset_p*(1-cosangle);
             sh_offs = shake_offset_f *cosangle + shake_offset_p *(1-cosangle);
             st_mult = stable_multiplier_f*cosangle + stable_multiplier_p*(1-cosangle);
             sh_mult = shake_multiplier_f *cosangle + shake_multiplier_p *(1-cosangle);
        } else {
             st_offs = stable_offset_b*-cosangle+ stable_offset_p*(1+cosangle);
             sh_offs = shake_offset_b *-cosangle+ shake_offset_p *(1+cosangle);
             st_mult = stable_multiplier_b*-cosangle + stable_multiplier_p*(1+cosangle);
             sh_mult = shake_multiplier_b *-cosangle + shake_multiplier_p *(1+cosangle);
        }
        st_offs *= st_warpfieldstrength;
        sh_offs *= sh_warpfieldstrength*costheta;
        st_mult = (1-st_warpfieldstrength)+st_mult*st_warpfieldstrength;
        sh_mult *= sh_warpfieldstrength*costheta;
        static float fov_smoothing=XMLSupport::parse_float(vs_config->getVariable("graphics","warp.fovlink.smoothing",".4"));
		float fov_smoot = pow(double(fov_smoothing),GetElapsedTime());
        smooth_fov=min(170.0f,max(5.0f,(1-fov_smoot)*smooth_fov+fov_smoot*(g_game.fov*(st_mult+sh_mult)+st_offs+sh_offs)));
        _Universe->AccessCamera()->SetFov(smooth_fov);
      
    }
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
  if (un&&target) {
    GFXColorf(unitToColor(un, target,un->GetComputerData().radar.iff));
    DrawArrowToTarget(un,un->LocalCoordinates(target));
  }
}
void GameCockpit::DrawArrowToTarget(Unit *un, Vector localcoord) {
  float s, t, s_normalized, t_normalized, inv_len;
  Vector p1, p2, p_n;


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
  GFXEnable(SMOOTH);
  GFXDisable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXBlendMode(SRCALPHA,INVSRCALPHA);

  glBegin(GL_LINE_LOOP);
  GFXVertex3f(s, t, 0);
  GFXVertexf(p1);
  GFXVertexf(p2);
  GFXEnd();
  GFXColor4f (1,1,1,1);

  GFXDisable(SMOOTH);
}
bool GameCockpit::CheckCommAnimation(Unit*  un) {
  for (unsigned int i=0;i<vdu.size();++i) {
    if (vdu[i]->CheckCommAnimation(un))
      return true;
  }
  return false;
}
