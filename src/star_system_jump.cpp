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
static Hashtable<std::string, StarSystem ,char [127]> star_system_table;
inline bool CompareDest (Planet * un, StarSystem * origin) {
  for (unsigned int i=0;i<un->GetDestinations().size();i++) {
    if ((origin==star_system_table.Get (string(un->GetDestinations()[i])))||(origin==star_system_table.Get (string(un->GetDestinations()[i])+string (".system")))) 
      return true;
  }
  return false;
}
inline std::vector <Unit *> ComparePrimaries (Unit * primary, StarSystem *origin) {
  std::vector <Unit *> myvec;
  if (primary->isUnit()==PLANETPTR) {
    if (CompareDest ((Planet *) primary, origin))
      myvec.push_back (primary);
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
  return myvec;
}

extern Unit ** fighters;
struct unorigdest {
  UnitContainer un;
  UnitContainer jumppoint;
  StarSystem * orig;
  StarSystem * dest;
  float delay;
  unsigned int animation;
  unorigdest (Unit * un, Planet * jumppoint, StarSystem * orig, StarSystem * dest, float delay,  unsigned int ani):un(un),jumppoint(jumppoint),orig(orig),dest(dest), delay(delay), animation(ani){}
};
void CacheJumpStar (bool destroy) {
  static Animation * cachedani=new Animation (vs_config->getVariable ("graphics","jumpgate","explosion_orange.ani").c_str(),true,.1,MIPMAP,false);
  if (destroy)
    delete cachedani;
}
static std::vector <unorigdest *> pendingjump;
static std::vector <unsigned int> AnimationNulls;
static std::vector <Animation *>JumpAnimations;
static std::vector <Animation *>VolatileJumpAnimations;
static unsigned int AddJumpAnimation (const Vector & pos, const float size, bool mvolatile=false ) {
  std::vector <Animation *> *ja= mvolatile?&VolatileJumpAnimations:&JumpAnimations;

  Animation * ani=new Animation (vs_config->getVariable ("graphics","jumpgate","explosion_orange.ani").c_str(),true,.1,MIPMAP,false);
  unsigned int i;
  if (mvolatile||AnimationNulls.empty()){
    i = ja->size();
    ja->push_back(ani);
  }else {
    assert (JumpAnimations[AnimationNulls.back()]==NULL);
    JumpAnimations[AnimationNulls.back()]= ani;
    i = AnimationNulls.back();
    AnimationNulls.pop_back();
  }
  (*ja)[i]->SetDimensions(size,size);
  (*ja)[i]->SetPosition (pos);
  return i;
}
void DealPossibleJumpDamage (Unit *un) {
  float speed = un->GetVelocity().Magnitude();
  float damage = un->GetJumpStatus().damage;
}

static void VolitalizeJumpAnimation (const unsigned int ani) {
  VolatileJumpAnimations.push_back (JumpAnimations[ani]);
  JumpAnimations[ani]=NULL;
  AnimationNulls.push_back (ani);
} 
void StarSystem::AddStarsystemToUniverse(const string &mname) {
  star_system_table.Put (mname,this);
}
void StarSystem::DrawJumpStars() {
  for (unsigned int kk=0;kk<pendingjump.size();kk++) { 
    unsigned int k=pendingjump[kk]->animation;
    Unit * un = pendingjump[kk]->un.GetUnit();
    if (un) {
      Vector p,q,r;
      un->GetOrientation (p,q,r);
      JumpAnimations[k]->SetPosition (un->Position()+r*un->rSize()*(pendingjump[kk]->delay+.25));
      JumpAnimations[k]->SetOrientation (p,q,r);
      static float JumpStarSize = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpgatesize","1.75"));
      float dd = un->rSize()*JumpStarSize*(un->GetJumpStatus().delay-pendingjump[kk]->delay)/(float)un->GetJumpStatus().delay;
      JumpAnimations[k]->SetDimensions (dd,dd);
    }
  }
  unsigned int i;
  for (i=0;i<JumpAnimations.size();i++) {
    if (JumpAnimations[i])
      JumpAnimations[i]->Draw();
  }
  for (i=0;i<VolatileJumpAnimations.size();i++) {
    if (VolatileJumpAnimations[i]) {
      float hei, wid;
      VolatileJumpAnimations[i]->GetDimensions(hei,wid);
      VolatileJumpAnimations[i]->SetDimensions(.95*hei,.95*wid);
      if (VolatileJumpAnimations[i]->Done()) {
	delete VolatileJumpAnimations[i];
	VolatileJumpAnimations.erase (VolatileJumpAnimations.begin()+i);
	i--;
      } else {
	VolatileJumpAnimations[i]->Draw();
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
    StarSystem * savedStarSystem = _Universe->activeStarSystem();
    Unit * un=pendingjump[kk]->un.GetUnit();
    if (un==NULL) {
#ifdef JUMP_DEBUG
  fprintf (stderr,"Adez Mon! Unit destroyed during jump!\n");
#endif
      delete pendingjump[kk];
      pendingjump.erase (pendingjump.begin()+kk);
      kk--;
      continue;
    }
    _Universe->setActiveStarSystem (pendingjump[kk]->orig);
    if (pendingjump[kk]->orig->RemoveUnit (un)) {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit removed from star system\n");
#endif

      un->RemoveFromSystem();
      pendingjump[kk]->dest->AddUnit (un);
      un->Target(NULL);
      Iterator * iter = pendingjump[kk]->orig->drawList->createIterator();
      Unit * unit;
      while((unit = iter->current())!=NULL) {
	if (unit->Target()==un) {
	  unit->Target (pendingjump[kk]->jumppoint.GetUnit());
	  unit->ActivateJumpDrive (0);
	}
	iter->advance();
      }
      delete iter;
      if (un==fighters[0]) {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit is a player character...changing scene graph\n");
#endif
	savedStarSystem->SwapOut();
	savedStarSystem = pendingjump[kk]->dest;
	pendingjump[kk]->dest->SwapIn();
      }
      _Universe->setActiveStarSystem(pendingjump[kk]->dest);
      vector <Unit *> possibilities;
      for (int i=0;i<pendingjump[kk]->dest->numprimaries;i++) {
	vector <Unit *> tmp;
	tmp = ComparePrimaries (pendingjump[kk]->dest->primaries[i],pendingjump[kk]->orig);
	if (!tmp.empty()) {
	  possibilities.insert (possibilities.end(),tmp.begin(), tmp.end());
	}
      }
      if (!possibilities.empty()) {
	static int jumpdest=235034;
	un->SetCurPosition(possibilities[jumpdest%possibilities.size()]->Position());
	jumpdest+=23231;
      }
      DealPossibleJumpDamage (un);
    } else {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit FAILED remove from star system\n");
#endif
    }
    Vector p,q,r;
    un->GetOrientation (p,q,r);
      static float JumpStarSize = XMLSupport::parse_float (vs_config->getVariable ("graphics","jumpgatesize","1.75"));
    unsigned int myani = AddJumpAnimation (un->LocalPosition(),un->rSize()*JumpStarSize,true);
    VolatileJumpAnimations[myani]->SetOrientation (p,q,r);
    delete pendingjump[kk];
    pendingjump.erase (pendingjump.begin()+kk);
    kk--;
    _Universe->setActiveStarSystem(savedStarSystem);
  }

}


bool StarSystem::JumpTo (Unit * un, Planet * jumppoint, const std::string &system) {
#ifdef JUMP_DEBUG
  fprintf (stderr,"jumping to %s.  ",system.c_str());
#endif
  StarSystem *ss = star_system_table.Get(system);

  if (!ss) {
    ss = star_system_table.Get (system+".system");
#ifdef JUMP_DEBUG
    fprintf (stderr,"Failed to find system. looking with .system appended\n");
#endif
    if (!ss) {
#ifdef JUMP_DEBUG
      fprintf (stderr,"System not loaded, loading file\n");
#endif
      std::string ssys (system);
      FILE * fp = fopen (ssys.c_str(),"r");
      if (!fp) {
#ifdef JUMP_DEBUG
    fprintf (stderr,"Failed to find system. looking with .system appended\n");
#endif
	ssys+=".system";
	fp = fopen (ssys.c_str(),"r");
      }
      if (!fp) {
#ifdef JUMP_DEBUG
	fprintf (stderr,"Failed to find system. Abort jump!\n");
#endif
	return false;
      }
      fclose (fp);
      ss = new StarSystem (ssys.c_str(),Vector (0,0,0),un->name);
      _Universe->LoadStarSystem (ss);
      ss->SwapOut();
      _Universe->activeStarSystem()->SwapIn();
    }
  }
  if(ss) {
#ifdef JUMP_DEBUG
	fprintf (stderr,"Pushing back to pending queue!\n");
#endif
    Vector p,q,r;
    un->GetOrientation (p,q,r);
    pendingjump.push_back (new unorigdest (un,jumppoint, this,ss,un->GetJumpStatus().delay,    AddJumpAnimation (un->Position()+r*un->rSize()*(un->GetJumpStatus().delay+.25), 10*un->rSize())));
  } else {
#ifdef JUMP_DEBUG
	fprintf (stderr,"Failed to retrieve!\n");
#endif
    return false;
  }
  return true;
}
