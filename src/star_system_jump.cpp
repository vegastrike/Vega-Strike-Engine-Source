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
struct unorigdest {
  UnitContainer un;
  UnitContainer jumppoint;
  StarSystem * orig;
  StarSystem * dest;
  float delay;
  int animation;
  bool justloaded;
  unorigdest (Unit * un,Unit * jumppoint, StarSystem * orig, StarSystem * dest, float delay,  int ani, bool justloaded):un(un),jumppoint(jumppoint),orig(orig),dest(dest), delay(delay), animation(ani),justloaded(justloaded){}
};
void CacheJumpStar (bool destroy) {
  static Animation * cachedani=new Animation (vs_config->getVariable ("graphics","jumpgate","explosion_orange.ani").c_str(),true,.1,MIPMAP,false);
  if (destroy)
    delete cachedani;
}
static std::vector <unorigdest *> pendingjump;
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

void DealPossibleJumpDamage (Unit *un) {
  float speed = un->GetVelocity().Magnitude();
  float damage = un->GetJumpStatus().damage+(rand()%100<1)?(rand()%20):0;
  float dam =speed*(damage/10);
  if (dam>100) dam=100;
  if (dam>1) {
    un->ApplyDamage ((un->Position()+un->GetVelocity().Cast()).Cast(),
		     un->GetVelocity(), 
		     dam,
		     un,
		     GFXColor (((float)(rand()%100))/100,
			       ((float)(rand()%100))/100,
			       ((float)(rand()%100))/100),NULL);
    un->SetCurPosition (un->LocalPosition()+(((float)rand())/RAND_MAX)*dam*un->GetVelocity().Cast());
  }
}

static void VolitalizeJumpAnimation (const int ani) {
  if (ani !=-1) {
    static float VolAnimationPer = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpanimationshrink",".95"));
    VolatileJumpAnimations.push_back (ResizeAni(JumpAnimations[ani].a,VolAnimationPer));

    JumpAnimations[ani].a=NULL;
    AnimationNulls.push_back (ani);
  }
}
inline bool CompareDest (Unit * un, StarSystem * origin) {
  for (unsigned int i=0;i<un->GetDestinations().size();i++) {
    if ((origin==star_system_table.Get (string(un->GetDestinations()[i])))||(origin==star_system_table.Get (string(un->GetDestinations()[i])+string (".system")))) 
      return true;
  }
  return false;
}
inline std::vector <Unit *> ComparePrimaries (Unit * primary, StarSystem *origin) {
  std::vector <Unit *> myvec;
  if (CompareDest (primary, origin))
    myvec.push_back (primary);
  /*
  if (primary->isUnit()==PLANETPTR) {
    Iterator *iter = ((Planet *)primary)->createIterator();
    Unit * unit;
    while((unit = iter->current())!=NULL) {
      if (unit->isUnit()==PLANETPTR)
	if (CompareDest ((Planet*)unit,origin)) {
	  myvec.push_back (unit);
	}
      iter->advance();
    }
    delete iter;
  }
  */
  return myvec;
}

void GameUnit::TransferUnitToSystem (unsigned int kk, StarSystem * &savedStarSystem, bool dosightandsound) {
  if (pendingjump[kk]->orig==activeStarSystem||activeStarSystem==NULL) {
    if (TransferUnitToSystem (pendingjump[kk]->dest)) {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit removed from star system\n");
#endif

      ///eradicating from system, leaving no trace
      TransferUnitToSystem(pendingjump[kk]->dest);


      UnitCollection::UnitIterator iter = pendingjump[kk]->orig->getUnitList().createIterator();
      Unit * unit;
      while((unit = iter.current())!=NULL) {
	if (unit->Threat()==this) {
	  unit->Threaten (NULL,0);
	}
	if (unit->VelocityReference()==this) {
	  unit->VelocityReference(NULL);
	}
	if (unit->Target()==this) {
	  unit->Target (pendingjump[kk]->jumppoint.GetUnit());
	  unit->ActivateJumpDrive (0);
	}else {
	  Flightgroup * ff = unit->getFlightgroup();
	  if (ff) {
		  if (this==ff->leader.GetUnit()&&(ff->directive=="f"||ff->directive=="F")) {
			unit->Target (pendingjump[kk]->jumppoint.GetUnit());
			unit->getFlightgroup()->directive="F";
			unit->ActivateJumpDrive (0);
		  }
	  }
	}
	iter.advance();
      }
      Cockpit * an_active_cockpit = _Universe->isPlayerStarship(this);
      if (an_active_cockpit!=NULL) {
	an_active_cockpit->activeStarSystem=pendingjump[kk]->dest;
      }
      if (this==_Universe->AccessCockpit()->GetParent()) {
	fprintf (stderr,"Unit is the active player character...changing scene graph\n");
	savedStarSystem->SwapOut();
	savedStarSystem = pendingjump[kk]->dest;
	pendingjump[kk]->dest->SwapIn();
      }
      
      _Universe->setActiveStarSystem(pendingjump[kk]->dest);
      vector <Unit *> possibilities;
      iter = pendingjump[kk]->dest->getUnitList().createIterator();
      Unit * primary;
      while ((primary = iter.current())!=NULL) {
	vector <Unit *> tmp;
	tmp = ComparePrimaries (primary,pendingjump[kk]->orig);
	if (!tmp.empty()) {
	  possibilities.insert (possibilities.end(),tmp.begin(), tmp.end());
	}
	iter.advance();
      }
      if (!possibilities.empty()) {
	static int jumpdest=235034;
	this->SetCurPosition(possibilities[jumpdest%possibilities.size()]->Position());
	jumpdest+=23231;
      }
      DealPossibleJumpDamage (this);
      static int jumparrive=AUDCreateSound(vs_config->getVariable ("unitaudio","jumparrive", "sfx43.wav"),false);
      if (dosightandsound)
	AUDPlay (jumparrive,this->LocalPosition(),this->GetVelocity(),1);
    } else {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit FAILED remove from star system\n");
#endif
    }
    if (docked&DOCKING_UNITS) {
      for (unsigned int i=0;i<image->dockedunits.size();i++) {
	Unit * unut;
	if (NULL!=(unut=image->dockedunits[i]->uc.GetUnit())) {
	  unut->TransferUnitToSystem (kk,savedStarSystem,dosightandsound);
	}
      }
    }
    if (docked&(DOCKED|DOCKED_INSIDE)) {
      Unit * un = image->DockedTo.GetUnit();
      if (!un) {
	docked &= (~(DOCKED|DOCKED_INSIDE));
      }else {
	Unit * targ=NULL;
	for (un_iter i=pendingjump[kk]->dest->getUnitList().createIterator();
	     (targ = (*i));
	     ++i) {
	  if (targ==un) {
	    break;
	  }
	}
	if (targ!=un)
	  UnDock (un);
      }
    }
  }else {
    fprintf (stderr,"Already jumped\n");
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


void StarSystem::ProcessPendingJumps() {
  for (unsigned int kk=0;kk<pendingjump.size();kk++) {
    if (pendingjump[kk]->delay>=0) {
      pendingjump[kk]->delay-=GetElapsedTime();
      continue;
    } else {
#ifdef JUMP_DEBUG
  fprintf (stderr,"Volitalizing pending jump animation.\n");
#endif

      VolitalizeJumpAnimation (pendingjump[kk]->animation);
    }
    

    Unit * un=pendingjump[kk]->un.GetUnit();
    
    if (un==NULL||!_Universe->StillExists (pendingjump[kk]->dest)||!_Universe->StillExists(pendingjump[kk]->orig)) {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Adez Mon! Unit destroyed during jump!\n");
#endif
      delete pendingjump[kk];
      pendingjump.erase (pendingjump.begin()+kk);
      kk--;
      continue;
    }
    StarSystem * savedStarSystem = _Universe->activeStarSystem();
    bool dosightandsound = ((pendingjump[kk]->dest==savedStarSystem)||un==_Universe->AccessCockpit()->GetParent());
    _Universe->setActiveStarSystem (pendingjump[kk]->orig);
    un->TransferUnitToSystem (kk, savedStarSystem,dosightandsound);
    static float JumpStarSize = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpgatesize","1.75"));
    if (dosightandsound) {
      Vector p,q,r;
      un->GetOrientation (p,q,r);
      unsigned int myani = AddJumpAnimation (un->LocalPosition(),un->rSize()*JumpStarSize,true);
      VolatileJumpAnimations[myani].a->SetOrientation (p,q,r);
    }
    delete pendingjump[kk];
    pendingjump.erase (pendingjump.begin()+kk);
    kk--;
    _Universe->setActiveStarSystem(savedStarSystem);
  }

}
void TentativeJumpTo (StarSystem * ss, Unit * un, Unit * jumppoint, const std::string &system) {
  for (unsigned int i=0;i<pendingjump.size();i++) {
    if (pendingjump[i]->un.GetUnit()==un) {
      return;
    }
  }
  ss->JumpTo (un,jumppoint,system);
}
bool StarSystem::JumpTo (Unit * un, Unit * jumppoint, const std::string &system) {
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
