#include "vegastrike.h"
#include "star_system.h"
#include "cmd/planet.h"
#include "lin_time.h"
#include "hashtable.h"
#include "gfx/animation.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "cmd/container.h"
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


static std::vector <unorigdest *> pendingjump;
static std::vector <unsigned int> AnimationNulls;
static std::vector <Animation *>JumpAnimations;
static std::vector <Animation *>VolatileJumpAnimations;
static unsigned int AddJumpAnimation (const Vector & pos, const float size, bool mvolatile=false ) {
  std::vector <Animation *> *ja= mvolatile?&VolatileJumpAnimations:&JumpAnimations;
  Animation * ani=new Animation ("explosion_orange.ani",true,.1,MIPMAP,true);
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
      JumpAnimations[k]->SetPosition (un->Position()+r*un->rSize()*pendingjump[kk]->delay);
    }
  }
  unsigned int i;
  for (i=0;i<JumpAnimations.size();i++) {
    if (JumpAnimations[i])
      JumpAnimations[i]->Draw();
  }
  for (i=0;i<VolatileJumpAnimations.size();i++) {
    if (VolatileJumpAnimations[i]) {
      VolatileJumpAnimations[i]->Draw();
      if (VolatileJumpAnimations[i]->Done()) {
	delete VolatileJumpAnimations[i];
	VolatileJumpAnimations.erase (VolatileJumpAnimations.begin()+i);
	i--;
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
      VolitalizeJumpAnimation (pendingjump[kk]->animation);
    }
    StarSystem * savedStarSystem = _Universe->activeStarSystem();
    Unit * un=pendingjump[kk]->un.GetUnit();
    if (un==NULL) {
      delete pendingjump[kk];
      pendingjump.erase (pendingjump.begin()+kk);
      kk--;
      continue;
    }
    _Universe->setActiveStarSystem (pendingjump[kk]->orig);
    if (pendingjump[kk]->orig->RemoveUnit (un)) {
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
    }
    AddJumpAnimation (un->Position(),un->rSize()*10,true);
    delete pendingjump[kk];
    pendingjump.erase (pendingjump.begin()+kk);
    kk--;
    _Universe->setActiveStarSystem(savedStarSystem);
  }

}

bool StarSystem::JumpTo (Unit * un, Planet * jumppoint, const std::string &system) {

  StarSystem *ss = star_system_table.Get(system);

  if (!ss) {
    ss = star_system_table.Get (system+".system");
    if (!ss) {
      std::string ssys (system);
      FILE * fp = fopen (ssys.c_str(),"r");
      if (!fp) {
	ssys+=".system";
	fp = fopen (ssys.c_str(),"r");
      }
      if (!fp)
	return false;
      fclose (fp);
      ss = new StarSystem (ssys.c_str(),Vector (0,0,0),un->name);
      _Universe->LoadStarSystem (ss);
      ss->SwapOut();
      _Universe->activeStarSystem()->SwapIn();
    }
  }
  if(ss) {
    Vector p,q,r;
    un->GetOrientation (p,q,r);
    pendingjump.push_back (new unorigdest (un,jumppoint, this,ss,un->GetJumpStatus().delay,    AddJumpAnimation (un->Position()+r*un->rSize()*un->GetJumpStatus().delay, 10*un->rSize())));
  } else {
    return false;
  }
  return true;
}
