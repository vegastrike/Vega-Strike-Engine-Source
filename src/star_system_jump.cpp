#include "vegastrike.h"
#include "star_system.h"
#include "cmd/planet.h"
#include "lin_time.h"
#include "hashtable.h"
#include "gfx/animation.h"
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
  Unit * un;
  Planet * jumppoint;
  StarSystem * orig;
  StarSystem * dest;
  float delay;
  unorigdest (Unit * un, Planet * jumppoint, StarSystem * orig, StarSystem * dest, float delay):un(un),jumppoint(jumppoint),orig(orig),dest(dest), delay(delay){}
};


static std::vector <unorigdest> pendingjump;
static std::vector <Animation *>JumpAnimations;
static void AddJumpAnimation (const Vector & pos, const float size ) {
  JumpAnimations.push_back (new Animation ("explosion_orange.ani",false,.1,MIPMAP,true));
  JumpAnimations.back()->SetPosition (pos);

}
void StarSystem::AddStarsystemToUniverse(const string &mname) {
  star_system_table.Put (mname,this);
}
void StarSystem::DrawJumpStars() {
  for (unsigned int i=0;i<JumpAnimations.size();i++) {
    JumpAnimations[i]->Draw();
    if (JumpAnimations[i]->Done()) {
      delete JumpAnimations[i];
      JumpAnimations.erase (JumpAnimations.begin()+i);
      i--;
    }
  }
}

void StarSystem::ProcessPendingJumps() {
  for (unsigned int kk=0;kk<pendingjump.size();kk++) {
    if (pendingjump[kk].delay>=0) {
      pendingjump[kk].delay-=GetElapsedTime();
      continue;
    }
    if (pendingjump[kk].orig->RemoveUnit (pendingjump[kk].un)) {
      pendingjump[kk].un->RemoveFromSystem();
      pendingjump[kk].dest->AddUnit (pendingjump[kk].un);
      pendingjump[kk].un->Target(NULL);
      Iterator * iter = pendingjump[kk].orig->drawList->createIterator();
      Unit * unit;
      while((unit = iter->current())!=NULL) {
	if (unit->Target()==pendingjump[kk].un) {
	  unit->Target (pendingjump[kk].jumppoint);
	  unit->ActivateJumpDrive (0);
	}
	iter->advance();
      }
      delete iter;
      if (pendingjump[kk].un==fighters[0]) {
	_Universe->activeStarSystem()->SwapOut();
	_Universe->popActiveStarSystem();
	_Universe->pushActiveStarSystem(pendingjump[kk].dest);
	pendingjump[kk].dest->SwapIn();
      }
      vector <Unit *> possibilities;
      for (int i=0;i<pendingjump[kk].dest->numprimaries;i++) {
	vector <Unit *> tmp;
	tmp = ComparePrimaries (pendingjump[kk].dest->primaries[i],pendingjump[kk].orig);
	if (!tmp.empty()) {
	  possibilities.insert (possibilities.end(),tmp.begin(), tmp.end());
	}
      }
      if (!possibilities.empty()) {
	static int jumpdest=235034;
	pendingjump[kk].un->SetCurPosition(possibilities[jumpdest%possibilities.size()]->Position());
	jumpdest+=23231;
      }
    }
    Vector p,q,r;
    pendingjump[kk].un->GetOrientation(p,q,r);
    AddJumpAnimation (pendingjump[kk].un->Position()+pendingjump[kk].un->rSize()*r+pendingjump[kk].un->GetJumpStatus().delay*.5*pendingjump[kk].un->GetVelocity(), pendingjump[kk].un->rSize()*10);
    pendingjump.erase (pendingjump.begin()+kk);
    kk--;
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
    }
  }
  if(ss) {
    AddJumpAnimation (un->Position()+un->GetVelocity()*un->GetJumpStatus().delay, un->rSize()*5);
    pendingjump.push_back (unorigdest (un,jumppoint, this,ss,un->GetJumpStatus().delay));
  } else {
    return false;
  }
  return true;
}
