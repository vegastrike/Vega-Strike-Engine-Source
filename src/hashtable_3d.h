#ifndef _HASHTABLE_3D_H_
#define _HASHTABLE_3D_H_
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
template <class T, class CTSIZ, class CTACCURACY, class CTHUGE> class Hashtable3d {
  ///Keeps track of the minimum and maximum write of the table since the last clear
  int minaccessx,minaccessy,minaccessz,maxaccessx,maxaccessy,maxaccessz;
  ///All objects that are too large to fit (fastly) in the collide table
  std::vector <T> hugeobjects;
  ///The hash table itself. Holds most units to be collided with
  std::vector <T> table [COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];
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
  Hashtable3d() {
    minaccessx=COLLIDETABLESIZE-1;
    minaccessy=COLLIDETABLESIZE-1;
    minaccessz=COLLIDETABLESIZE-1;
    maxaccessx=0;
    maxaccessy=0;
    maxaccessz=0;    
  }
  ///Hashes a single value to a value on the collide table truncated to all 3d constraints.  Consider using a swizzle
  int hash_int (const float aye) {
    return ((int)(((aye<0)?(aye-COLLIDETABLEACCURACY):aye)/COLLIDETABLEACCURACY))%(COLLIDETABLESIZE/2)+(COLLIDETABLESIZE/2); 
  }
  ///clears entire table
  void Clear () {
    hugeobjects.clear();
    for (int i=minaccessx;i<=maxaccessx;i++) {
    for (int j=minaccessy;j<=maxaccessy;j++) {
    for (int k=minaccessz;k<=maxaccessz;k++) {
      if (table[i][j][k].size())
	table[i][j][k].clear();
    }
    }
    }
    minaccessx=COLLIDETABLESIZE-1;
    minaccessy=COLLIDETABLESIZE-1;
    minaccessz=COLLIDETABLESIZE-1;
    maxaccessx=0;
    maxaccessy=0;
    maxaccessz=0;
  }
  ///returns any objects residing in the sector occupied by Exact
  int Get (const Vector &Exact, std::vector <T> *retval[]) {
    retval[1]=&table[hash_int(Exact.i)][hash_int(Exact.j)][hash_int(Exact.k)];
    //retval+=hugeobjects;
    //blah = blooh;
    retval[0]=&hugeobjects;
    return 2;
  }
  ///Returns all objects too big to be conveniently fit in the array
  std::vector <T> & GetHuge () {
    return hugeobjects;
  }
  ///Returns all objects within sector(s) occupied by target
  int Get (const LineCollide* target, std::vector <T> *retval[]) {    
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
  void Put(LineCollide* target,const T objectToPut) {
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
      hugeobjects.push_back(objectToPut);
      return;
    }else {
      target->hhuge=false;
    }
    for (float i=target->Mini.i;i<maxx;i+=COLLIDETABLEACCURACY) {
      x = hash_int(i);
      if (x<minaccessx) minaccessx=x;
      if (x>maxaccessx) maxaccessx=x;
      for (float j=target->Mini.j;j<maxy;j+=COLLIDETABLEACCURACY) {    
	y = hash_int(j);
	if (y<minaccessy) minaccessy=y;
	if (y>maxaccessy) maxaccessy=y;
	for (float k=target->Mini.k;k<maxz;k+=COLLIDETABLEACCURACY) {
	  z = hash_int(k);
	  if (z<minaccessz) minaccessz=z;
	  if (z>maxaccessz) maxaccessz=z;
	  table[x][y][z].push_back(objectToPut);
	}
      }
    }
  }
  ///Removes objectToKill from collide table with span of Target
  T Remove(const LineCollide* target,const T objectToKill) {
    //    int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(target->Mini,minx,miny,minz);
    //    hash_vec(target->Maxi,maxx,maxy,maxz);
    T *retval=NULL;
    std::vector <T>::iterator removal= hugeobjects.begin();
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
	    removal = table[x][y][z].begin();
	    while (removal!=table[x][y][z].end()) {
	      removal = std::find (table[x][y][z].begin(),table[x][y][z].end(),objectToKill);
	      if (removal!=table[x][y][z].end()) {
		retval = removal;
		table[x][y][z].erase(removal);
	      }
	    }
	  }
	}
      }
    }
    if (!retval&&!target->hhuge)
      {
	fprintf (stderr, "bad things");
      }
    if (!retval||target->hhuge) {
      while (removal!=hugeobjects.end()) {
		  removal = std::find (hugeobjects.begin(),hugeobjects.end(),objectToKill);
	if (removal!=hugeobjects.end()) {
	  retval = removal;
	  hugeobjects.erase(removal);
	}
      }
    }

    if (retval)
      return *retval;
    else 
      return T();
  }
};

#endif
