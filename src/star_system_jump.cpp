#include "vegastrike.h"
#include "star_system.h"
#include "cmd/planet.h"
#include "lin_time.h"
#include "hashtable.h"
#include "gfx/animation.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "cmd/container.h"
#include "xml_support.h"
#include <assert.h>
#include "gfx/cockpit.h"
#include "audiolib.h"
#include "cmd/images.h"
#include "cmd/script/flightgroup.h"
extern Unit ** fighters;
extern Hashtable<std::string, StarSystem ,char [127]> star_system_table;
void CacheJumpStar (bool destroy) {
  static Animation * cachedani=new Animation (vs_config->getVariable ("graphics","jumpgate","explosion_orange.ani").c_str(),true,.1,MIPMAP,false);
  if (destroy)
    delete cachedani;
}
std::vector <unorigdest *> pendingjump;
static std::vector <unsigned int> AnimationNulls;
class ResizeAni {
public:
  Animation *a;
  float percent;
  ResizeAni (Animation *ani, float percent) {
    a = ani;
    this->percent=percent;
  }
};
static std::vector <ResizeAni>JumpAnimations;
static std::vector <ResizeAni>VolatileJumpAnimations;
Animation * GetVolatileAni (unsigned int which) {
  if (which < VolatileJumpAnimations.size()) {
    return VolatileJumpAnimations[which].a;
  }
  return NULL;
}
unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name , float percentgrow) {
  std::vector <ResizeAni> *ja= mvolatile?&VolatileJumpAnimations:&JumpAnimations;

  Animation * ani=new Animation (name.c_str(),true,.1,MIPMAP,false);
  unsigned int i;
  if (mvolatile||AnimationNulls.empty()){
    i = ja->size();
    ja->push_back(ResizeAni (ani,percentgrow));
  }else {
    assert (JumpAnimations[AnimationNulls.back()].a==NULL);
    JumpAnimations[AnimationNulls.back()]= ResizeAni(ani,percentgrow);
    i = AnimationNulls.back();
    AnimationNulls.pop_back();
  }
  (*ja)[i].a->SetDimensions(size,size);
  (*ja)[i].a->SetPosition (pos);
  return i;
}
static unsigned int AddJumpAnimation (const QVector & pos, const float size, bool mvolatile=false) {
  return AddAnimation (pos,size,mvolatile,vs_config->getVariable("graphics","jumpgate","warp.ani"),.95);
}

void GameStarSystem::VolitalizeJumpAnimation (const int ani) {
  if (ani !=-1) {
    static float VolAnimationPer = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpanimationshrink",".95"));
    VolatileJumpAnimations.push_back (ResizeAni(JumpAnimations[ani].a,VolAnimationPer));

    JumpAnimations[ani].a=NULL;
    AnimationNulls.push_back (ani);
  }
}

void GameStarSystem::DrawJumpStars() {
  for (unsigned int kk=0;kk<pendingjump.size();kk++) { 
    int k=pendingjump[kk]->animation;
    if (k!=-1) {
      Unit * un = pendingjump[kk]->un.GetUnit();
      if (un) {
	Vector p,q,r;
	un->GetOrientation (p,q,r);
	JumpAnimations[k].a->SetPosition (un->Position()+r.Cast()*un->rSize()*(pendingjump[kk]->delay+.25));
	JumpAnimations[k].a->SetOrientation (p,q,r);
	static float JumpStarSize = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpgatesize","1.75"));
	float dd = un->rSize()*JumpStarSize*(un->GetJumpStatus().delay-pendingjump[kk]->delay)/(float)un->GetJumpStatus().delay;
	JumpAnimations[k].a->SetDimensions (dd,dd);
      }
    }
  }
  unsigned int i;
  for (i=0;i<JumpAnimations.size();i++) {
    if (JumpAnimations[i].a)
      JumpAnimations[i].a->Draw();
  }
  for (i=0;i<VolatileJumpAnimations.size();i++) {
    if (VolatileJumpAnimations[i].a) {
      float hei, wid;
      VolatileJumpAnimations[i].a->GetDimensions(hei,wid);
      VolatileJumpAnimations[i].a->SetDimensions(VolatileJumpAnimations[i].percent*hei,VolatileJumpAnimations[i].percent*wid);
      if (VolatileJumpAnimations[i].a->Done()) {
	delete VolatileJumpAnimations[i].a;
	VolatileJumpAnimations.erase (VolatileJumpAnimations.begin()+i);
	i--;
      } else {
	VolatileJumpAnimations[i].a->Draw();
      }
    }
  }
}


void GameStarSystem::DoJumpingSightAndSound (Unit * un) {
      static float JumpStarSize = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpgatesize","1.75"));
      Vector p,q,r;
      un->GetOrientation (p,q,r);
      unsigned int myani = AddJumpAnimation (un->LocalPosition(),un->rSize()*JumpStarSize,true);
      VolatileJumpAnimations[myani].a->SetOrientation (p,q,r);
}
void TentativeJumpTo (StarSystem * ss, Unit * un, Unit * jumppoint, const std::string &system) {
  for (unsigned int i=0;i<pendingjump.size();i++) {
    if (pendingjump[i]->un.GetUnit()==un) {
      return;
    }
  }
  ss->JumpTo (un,jumppoint,system);
}
bool GameStarSystem::JumpTo (Unit * un, Unit * jumppoint, const std::string &system) {
  if ((un->DockedOrDocking()&(~Unit::DOCKING_UNITS))!=0) {
    return false;
  }
#ifdef JUMP_DEBUG
  fprintf (stderr,"jumping to %s.  ",system.c_str());
#endif
  StarSystem *ss = star_system_table.Get(system);
  std::string ssys (system+".system");
  if (!ss) {
    ss = star_system_table.Get (ssys);
  }
  bool justloaded=false;
  if (!ss) {
    justloaded=true;
    ss = _Universe->GenerateStarSystem (ssys.c_str(),filename.c_str(),Vector (0,0,0));
  }
  if(ss) {
#ifdef JUMP_DEBUG
	fprintf (stderr,"Pushing back to pending queue!\n");
#endif
    Vector p,q,r;
    un->GetOrientation (p,q,r);
    bool dosightandsound = ((this==_Universe->getActiveStarSystem (0))||un==_Universe->AccessCockpit()->GetParent());
    int ani =-1;
    if (dosightandsound) {
      ani = AddJumpAnimation (un->Position()+r.Cast()*un->rSize()*(un->GetJumpStatus().delay+.25), 10*un->rSize());
      static int jumpleave=AUDCreateSound(vs_config->getVariable ("unitaudio","jumpleave", "sfx43.wav"),false);
      AUDPlay (jumpleave,un->LocalPosition(),un->GetVelocity(),1);
    }
    pendingjump.push_back (new unorigdest (un,jumppoint, this,ss,un->GetJumpStatus().delay,ani,justloaded ));
#if 0
    UnitImages * im=  &un->GetImageInformation();
    for (unsigned int i=0;i<=im->dockedunits.size();i++) {
      Unit* unk =NULL;
      if (i<im->dockedunits.size()) {
	im->dockedunits[i]->uc.GetUnit();
      }else {
	unk = im->DockedTo.GetUnit();
      }
      if (unk!=NULL) {
	TentativeJumpTo (this,unk,jumppoint,system);
      }
    }
    
#endif
  } else {
#ifdef JUMP_DEBUG
	fprintf (stderr,"Failed to retrieve!\n");
#endif
    return false;
  }
  return true;
}
