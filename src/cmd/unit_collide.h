#ifndef _CMD_COLLIDE_H_
#define _CMD_COLLIDE_H_
#include "gfx/vec.h"
#include "hashtable_3d.h"

#define SAFE_COLLIDE_DEBUG

const int tablehuge=10;
const int coltableacc = 200;
const int coltablesize=20;
class CollideTable {
 public:
  Hashtable3d <LineCollide*, char[coltablesize],char[coltableacc], char [tablehuge]> c;
};

void AddCollideQueue(LineCollide &,StarSystem * ss);
bool TableLocationChanged (const Vector &, const Vector &);
bool TableLocationChanged (const LineCollide &, const Vector &, const Vector &);
void KillCollideTable (LineCollide* lc, StarSystem * ss);
bool EradicateCollideTable (LineCollide* lc, StarSystem * ss);

class csRapidCollider;
class BSPTree;

struct collideTrees {
  std::string hash_key;
  ///The bsp tree of this unit (used when shields down/unit-unit collisions)
  BSPTree *bspTree;
  csRapidCollider *colTree;
  ///The bsp tree of the shields of this unit (used for beams)
  BSPTree *bspShield;
  csRapidCollider *colShield;
  int refcount;
  collideTrees (const std::string &hk, BSPTree *bT, BSPTree *bS, csRapidCollider *cT, csRapidCollider *cS);
  void Inc () {refcount++;}
  void Dec ();
  static collideTrees * Get(const std::string &hash_key);
};



#endif
