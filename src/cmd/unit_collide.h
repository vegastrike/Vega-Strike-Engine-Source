#ifndef _CMD_COLLIDE_H_
#define _CMD_COLLIDE_H_

#define SAFE_COLLIDE_DEBUG
#include "gfx/vec.h"
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include "linecollide.h"
#define COLLIDETABLESIZE sizeof(CTSIZ)
#define COLLIDETABLEACCURACY sizeof (CTACCURACY)
///objects that go over 16 sectors are considered huge and better to check against everything.
#define HUGEOBJECT sizeof (CTHUGE)
//const int HUGEOBJECT=12; 

/**
 * Hashtable3d is a 3d datastructure that holds various starships that are
 * near enough to crash into each other (or also lights that are big enough
 * to shine on nearby units.
 */
template <class CTSIZ, class CTACCURACY, class CTHUGE> class UnitHash3d {
  ///All objects that are too large to fit (fastly) in the collide table
  UnitCollection hugeobjects;
  ///The hash table itself. Holds most units to be collided with
  UnitCollection table [COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];
  ///hashes 3 values into the appropriate spot in the hash table
  static void hash_vec (float i, float j, float k, int &x, int &y, int &z) {
    x = hash_int(i);
    y = hash_int(j);
    z = hash_int(k);
  }
  ///hashes 3 vals into the appropriate place in hash table
  static void hash_vec (const Vector & t,int &x, int&y,int&z) {
    hash_vec(t.i,t.j,t.k,x,y,z);
  }
public:
  void updateBloc (unsigned int whichblock) {
    un_iter ui =table  [whichblock%COLLIDETABLESIZE][(whichblock/COLLIDETABLESIZE)%COLLIDETABLESIZE][((whichblock/COLLIDETABLESIZE)/COLLIDETABLESIZE)%COLLIDETABLESIZE].createIterator();
    while (*ui) {
      ++ui;
    }
  }
  ///Hashes a single value to a value on the collide table truncated to all 3d constraints.  Consider using a swizzle
  int hash_int (const float aye) {
    return ((int)(((aye<0)?(aye-COLLIDETABLEACCURACY):aye)/COLLIDETABLEACCURACY))%(COLLIDETABLESIZE/2)+(COLLIDETABLESIZE/2); 
  }
  ///clears entire table
  void Clear () {
    if (!hugeobjects.empty()) {
      hugeobjects.clear();
    }
    for (int i=0;i<=COLLIDETABLESIZE-1;i++) {
    for (int j=0;j<=COLLIDETABLESIZE-1;j++) {
    for (int k=0;k<=COLLIDETABLESIZE-1;k++) {
      if (!table[i][j][k].empty())
	table[i][j][k].clear();
    }
    }
    }
  }
  ///returns any objects residing in the sector occupied by Exact
  int Get (const Vector &Exact, UnitCollection  *retval[]) {
    retval[1]=&table[hash_int(Exact.i)][hash_int(Exact.j)][hash_int(Exact.k)];
    //retval+=hugeobjects;
    //blah = blooh;
    retval[0]=&hugeobjects;
    return 2;
  }
  ///Returns all objects too big to be conveniently fit in the array
  UnitCollection & GetHuge () {
    return hugeobjects;
  }
  ///Returns all objects within sector(s) occupied by target
  int Get (const LineCollide* target, UnitCollection *retval[]) {    
    unsigned int sizer =1;
    //int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(Min,minx,miny,minz);
    //    hash_vec(Max,maxx,maxy,maxz);
    float maxx= (ceil(target->Maxi.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(target->Maxi.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(target->Maxi.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    int x,y,z;
    if (target->Mini.i==maxx) maxx+=COLLIDETABLEACCURACY/2;
    if (target->Mini.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (target->Mini.k==maxz) maxz+=COLLIDETABLEACCURACY/2;
    retval[0] = &hugeobjects;
    if (target->hhuge) {
      return sizer;//we can't get _everything
    } 
    for (float i=target->Mini.i;i<maxx;i+=COLLIDETABLEACCURACY) {
      x = hash_int (i);
      for (float j=target->Mini.j;j<maxy;j+=COLLIDETABLEACCURACY) {   
	y = hash_int(j);
	for (float k=target->Mini.k;k<maxz;k+=COLLIDETABLEACCURACY) {
	  z = hash_int(k);
	  if (!table[x][y][z].empty()) {
	    retval[sizer] = &table[x][y][z];
	    sizer++;
	    if (sizer>=HUGEOBJECT+1)
	      return sizer;
	  }
	}
      }
    }
    assert (sizer<=HUGEOBJECT+1);//make sure we didn't overrun our array
    return sizer;
  }
  ///Adds objectToPut into collide table with limits specified by target.
  void Put(LineCollide* target,Unit * objectToPut) {
    int x,y,z;
    float maxx= (ceil(target->Maxi.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(target->Maxi.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(target->Maxi.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    //for huge calculation...not sure it's necessary
    float minx= (floor(target->Mini.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float miny= (floor(target->Mini.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float minz= (floor(target->Mini.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    if (target->Mini.i==maxx) 
      maxx+=COLLIDETABLEACCURACY/2;
    if (target->Mini.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (target->Mini.k==maxz) maxz+=COLLIDETABLEACCURACY/2;
    if (fabs((maxx-minx)*(maxy-miny)*(maxz-minz))>((float)COLLIDETABLEACCURACY)*((float)COLLIDETABLEACCURACY)*((float)COLLIDETABLEACCURACY)*((float)HUGEOBJECT)) {
      target->hhuge = true;
      hugeobjects.prepend(objectToPut);
      return;
    }else {
      target->hhuge=false;
    }
    for (float i=target->Mini.i;i<maxx;i+=COLLIDETABLEACCURACY) {
      x = hash_int(i);
      for (float j=target->Mini.j;j<maxy;j+=COLLIDETABLEACCURACY) {    
	y = hash_int(j);
	for (float k=target->Mini.k;k<maxz;k+=COLLIDETABLEACCURACY) {
	  z = hash_int(k);
	  table[x][y][z].prepend(objectToPut);
	}
      }
    }
  }
  static bool removeFromVector (UnitCollection &myvector, Unit * objectToKill) {
    bool ret=false;
    un_iter removal = myvector.createIterator();
    Unit * un;
    while ((un=removal.current())) {
      if (un==objectToKill) {
	ret = true;
	removal.remove();
      } else {
	removal.advance();
      }
    }
    return ret;
  }
  bool Eradicate (Unit * objectToKill) {
    bool ret=removeFromVector (hugeobjects,objectToKill);
    for (unsigned int i=0;i<=COLLIDETABLESIZE-1;i++) {
      for (unsigned int j=0;j<=COLLIDETABLESIZE-1;j++) {
	for (unsigned int k=0;k<=COLLIDETABLESIZE-1;k++) {
	  ret |= removeFromVector (table[i][j][k],objectToKill);
	}
      }
    }
    return ret;
  }
  ///Removes objectToKill from collide table with span of Target
  bool Remove(const LineCollide* target,Unit *objectToKill) {
    //    int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(target->Mini,minx,miny,minz);
    //    hash_vec(target->Maxi,maxx,maxy,maxz);
    bool ret=false;
    int x,y,z;
    float maxx= (ceil(target->Maxi.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(target->Maxi.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(target->Maxi.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    if (target->Mini.i==maxx) maxx+=COLLIDETABLEACCURACY/2;
    if (target->Mini.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (target->Mini.k==maxz) maxz+=COLLIDETABLEACCURACY/2;
    if (!target->hhuge) {
      for (float i=target->Mini.i;i<maxx;i+=COLLIDETABLEACCURACY) {
	x = hash_int(i);
	for (float j=target->Mini.j;j<maxy;j+=COLLIDETABLEACCURACY) {    
	  y = hash_int(j);
	  for (float k=target->Mini.k;k<maxz;k+=COLLIDETABLEACCURACY) {
	    z = hash_int(k);
	    ret |= removeFromVector (table[x][y][z],objectToKill);
	  }
	}
      }
    }
    if (!ret&&!target->hhuge)
      {
	fprintf (stderr, "Nonfatal Collide Error\n");
      }
    if (!ret||target->hhuge) {
      ret|=removeFromVector (hugeobjects, objectToKill);
    }
    return ret;
  }
};

const int tablehuge=10;
const int coltableacc = 200;
const int coltablesize=20;
class CollideTable {
  unsigned int blocupdate;
 public:
  CollideTable ():blocupdate(0) {}
  void Update () {c.updateBloc(blocupdate++);}
  UnitHash3d <char[coltablesize],char[coltableacc], char [tablehuge]> c;
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
