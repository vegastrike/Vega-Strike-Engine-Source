#ifndef _CMD_COLLIDE_H_
#define _CMD_COLLIDE_H_
#include "gfx/vec.h"
#include "hashtable_3d.h"

void AddCollideQueue(const LineCollide & );
bool TableLocationChanged (const Vector &, const Vector &);
bool TableLocationChanged (const LineCollide &, const Vector &, const Vector &);
void KillCollideTable (LineCollide* lc);




#endif
