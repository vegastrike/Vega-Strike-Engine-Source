#ifndef _CMD_COLLIDE_H_
#define _CMD_COLLIDE_H_
#include "gfx/vec.h"
#include "hashtable_3d.h"

#define SAFE_COLLIDE_DEBUG

const int tablehuge=12;
class CollideTable {
 public:
  Hashtable3d <LineCollide*, char[20],char[200], char [tablehuge]> c;
};

void AddCollideQueue(LineCollide & );
bool TableLocationChanged (const Vector &, const Vector &);
bool TableLocationChanged (const LineCollide &, const Vector &, const Vector &);
void KillCollideTable (LineCollide* lc);
bool EradicateCollideTable (LineCollide* lc);




#endif
