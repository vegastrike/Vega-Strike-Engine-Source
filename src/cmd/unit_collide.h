#ifndef _CMD_COLLIDE_H_
#define _CMD_COLLIDE_H_
#include "gfx/vec.h"
#include "hashtable_3d.h"

#define SAFE_COLLIDE_DEBUG

const int tablehuge=10;
class CollideTable {
 public:
  Hashtable3d <LineCollide*, char[20],char[200], char [tablehuge]> c;
};

void AddCollideQueue(LineCollide & );
bool TableLocationChanged (const Vector &, const Vector &);
bool TableLocationChanged (const LineCollide &, const Vector &, const Vector &);
void KillCollideTable (LineCollide* lc);
bool EradicateCollideTable (LineCollide* lc);

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
