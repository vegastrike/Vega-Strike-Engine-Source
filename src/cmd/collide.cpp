#include "cmd/collide.h"
#include "vegastrike.h"
#include "unit_generic.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "gfx/bsp.h"
#include "collide/rapcol.h"
#include "collide/csgeom/transfrm.h"
#include "collide/collider.h"
#include "hashtable.h"
#include <string>
#include "vs_globals.h"
#include "configxml.h"
static Hashtable <std::string,collideTrees,char[127]> unitColliders;
collideTrees::collideTrees (const std::string &hk, BSPTree *bT, BSPTree *bS, csRapidCollider *cT, csRapidCollider *cS): hash_key(hk),bspTree(bT), colTree(cT), bspShield(bS), colShield(cS) {
  refcount=1;
  unitColliders.Put (hash_key,this);
}
collideTrees* collideTrees::Get(const std::string &hash_key) {
  return unitColliders.Get(hash_key);
}
void collideTrees::Dec() {
  refcount--;
  if (refcount==0) {
    unitColliders.Delete (hash_key);
    if (bspTree)
      delete bspTree;
    if (colTree) 
      delete colTree;
    if (bspShield)
      delete bspShield;
    if (colShield)
      delete colShield;
    delete this;
    return;
  }
}

bool TableLocationChanged (const QVector & Mini,const QVector & minz) { 
  return (_Universe->activeStarSystem()->collidetable->c.hash_int (Mini.i)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.i) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.j)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.j) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.k)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.k));
}
bool TableLocationChanged (const LineCollide &lc, const QVector &minx, const QVector & maxx) {
  return TableLocationChanged (lc.Mini,minx) || TableLocationChanged (lc.Maxi,maxx);
}
void KillCollideTable (LineCollide * lc,StarSystem * ss) {
  if (lc->type==LineCollide::UNIT) {
    ss->collidetable->c.Remove ( lc,lc->object.u);
  } else {
    printf ("such collide types as %d not allowed",lc->type);
  }
}
bool EradicateCollideTable (LineCollide * lc, StarSystem * ss) {
  if (lc->type==LineCollide::UNIT) { 
    return ss->collidetable->c.Eradicate (lc->object.u);
  } else {
    printf ("such collide types as %d not allowed",lc->type);
    return false;
  }
}
void AddCollideQueue (LineCollide &tmp,StarSystem * ss) {
  if (tmp.type==LineCollide::UNIT) { 
    ss->collidetable->c.Put (&tmp,tmp.object.u);
  } else {
    printf ("such collide types as %d not allowed",tmp.type);
  }
}

bool lcwithin (const LineCollide & lc, const LineCollide&tmp) {
  return (lc.Mini.i< tmp.Maxi.i&&
	  lc.Mini.j< tmp.Maxi.j&&
	  lc.Mini.k< tmp.Maxi.k&&
	  lc.Maxi.i> tmp.Mini.i&&
	  lc.Maxi.j> tmp.Mini.j&&
	  lc.Maxi.k> tmp.Mini.k);
}
bool usehuge_table() {
  const unsigned int A = 9301;
  const unsigned int C = 49297;
  const unsigned int M = 233280;
  static unsigned int seed=3259235;
  seed = (seed * A + C) % M;
  return seed<(M/100);
}
bool Bolt::Collide () {
  UnitCollection *candidates[2];  
  bool use_huge_list=usehuge_table();
  _Universe->activeStarSystem()->collidetable->c.Get (cur_position,candidates,use_huge_list);
  LineCollide minimaxi;//might as well have this so we can utilize common function
  minimaxi.Mini= ( prev_position.Min (cur_position));
  minimaxi.Maxi= ( prev_position.Max (cur_position));
  for (unsigned int j=0;j<2;j++) {
    Unit * un;
    for (un_iter i=candidates[j]->createIterator();(un=*i)!=NULL;++i) {
      
      if (lcwithin (minimaxi,(un)->GetCollideInfo ())) {
	if (this->Collide (un)) {
	  if (j==0&&use_huge_list) {
	    _Universe->activeStarSystem()->collidetable->c.AddHugeToActive(un);
	  }
	  delete this;
	  return true;
	}
	
      }
    }
  }
  return false;
}

void Beam::CollideHuge (const LineCollide & lc, Unit * targetToCollideWith) {
  UnitCollection *colQ [tablehuge+1];
  bool use_huge_list = usehuge_table();
  if (!lc.hhuge) {
    int sizecolq = _Universe->activeStarSystem()->collidetable->c.Get (&lc,colQ,use_huge_list);
    for (int j=0;j<sizecolq;j++) {
      Unit *un;
      for (un_iter i=colQ[j]->createIterator();(un=(*i))!=NULL;++i) {

	if (lcwithin(lc,(un)->GetCollideInfo())) {
	  if (this->Collide (un)) {
	    if (j==0&&use_huge_list) {
	      _Universe->activeStarSystem()->collidetable->c.AddHugeToActive(un);
	    }
	  }
	}
      }
    }
  }else {
    if (targetToCollideWith&&(!use_huge_list)) {
      this->Collide(targetToCollideWith);
    }else {
      un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
      Unit *un;
      for (;(un=*i)!=NULL;++i) {
	if (lcwithin (lc,(un)->GetCollideInfo())) {
	  this->Collide(un);
	  if ((un!=targetToCollideWith)&&targetToCollideWith!=NULL) {
	    ListenToOwner(false);
	  }
	}
      }
    }
  }

}


