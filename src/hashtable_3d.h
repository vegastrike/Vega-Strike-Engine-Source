#include "cmd_collide.h"
#include <algorithm>
#ifndef _HASHTABLE_3D_H_
#define _HASHTABLE_3D_H_
#define COLLIDETABLESIZE sizeof(CTSIZ)
#define COLLIDETABLEACCURACY sizeof (CTACCURACY)
const int HUGEOBJECT=16; //objects that go over 16 sectors are considered huge and better to check against everything.
struct LineCollide {
  void * object;
  enum collidables {UNIT, BEAM,BALL,BOLT,PROJECTILE} type;
  Vector Mini;
  Vector Maxi;
  LineCollide (void * objec, enum collidables typ,const Vector &st, const Vector &en) {this->object=objec;this->type=typ;this->Mini=st;this->Maxi=en;}
  LineCollide (const LineCollide &l) {object=l.object; type=l.type; Mini=l.Mini;Maxi=l.Maxi;}      
};

template <class T, class CTSIZ, class CTACCURACY> class Hashtable3d {
  int minaccessx,minaccessy,minaccessz,maxaccessx,maxaccessy,maxaccessz;
  vector <T> hugeobjects;
  vector <T> table [COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];
  static int hash_int (float i) {
    return (((int)(i<0?(i-COLLIDETABLEACCURACY):i))/COLLIDETABLEACCURACY)%(COLLIDETABLESIZE/2)+(COLLIDETABLESIZE/2); 
  }
  static void hash_vec (float i, float j, float k, int &x, int &y, int &z) {
    x = hash_int(i);
    y = hash_int(j);
    z = hash_int(k);
  }
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
  void Clear () {
    hugeobjects = vector <T>();
    for (int i=minaccessx;i<=maxaccessx;i++) {
    for (int j=minaccessy;j<=maxaccessy;j++) {
    for (int k=minaccessz;k<=maxaccessz;k++) {
      if (table[i][j][k].size())
	table[i][j][k]=vector <T>();
    }
    }
    }
    minaccessx=COLLIDETABLESIZE-1;
    minaccessy=COLLIDETABLESIZE-1;
    minaccessz=COLLIDETABLESIZE-1;
    maxaccessx=0;
    maxaccessy=0;
    maxaccessz=0;
    /*
    for (int l=0;l<COLLIDETABLESIZE;l++) {
      for (int m=0;m<COLLIDETABLESIZE;m++) {
	for (int n=0;n<COLLIDETABLESIZE;n++) {
	  if (table[l][m][n].size()!=0)
	    fprintf (stderr,"ERROR!!! SIZE: %d\n\n\n",table[l][m][n].size());
	}
      }

      }*/
  }
  void Get (const Vector &Exact, vector <T> &retval) {
    retval = table[hash_int(Exact.i)][hash_int(Exact.j)][hash_int(Exact.k)];
    //retval+=hugeobjects;
    //blah = blooh;
    retval.insert (retval.end(),hugeobjects.begin(),hugeobjects.end());
  }
  bool Get (const Vector &Min, const Vector & Max, vector <T> &retval) {    
    //int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(Min,minx,miny,minz);
    //    hash_vec(Max,maxx,maxy,maxz);
    float maxx= (ceil(Max.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(Max.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(Max.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    int x,y,z;
    if (Min.i==maxx) maxx+=COLLIDETABLEACCURACY/2;
    if (Min.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (Min.k==maxz) maxz+=COLLIDETABLEACCURACY/2;
    if (fabs((maxx-Min.i)*(maxy-Min.j)*(maxz-Min.k))>COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*HUGEOBJECT) {
      retval = collidequeue;
      return true;
    } else {
      retval = hugeobjects;
    }
    for (float i=Min.i;i<maxx;i+=COLLIDETABLEACCURACY) {
      x = hash_int (i);
      for (float j=Min.j;j<maxy;j+=COLLIDETABLEACCURACY) {   
	y = hash_int(j);
	for (float k=Min.k;k<maxz;k+=COLLIDETABLEACCURACY) {
	  z = hash_int(k);
	  //	  table[i][j][k].push_back(target);
	  for (unsigned int l=0;l<table[x][y][z].size();l++) {
	    unsigned int m=0;
	    for (;m<retval.size();m++) {
	      if (retval[m]==table[x][y][z][l])
		//make sure we're not pushing back duplicates;
		break;
	    }
	    if (m==retval.size())
	      retval.push_back (table[x][y][z][l]);
	  }
	}
      }
    }
    return false;
  }
  void Put(const LineCollide* target,const T objectToPut) {
    //    int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(target->Mini,minx,miny,minz);
    //    hash_vec(target->Maxi,maxx,maxy,maxz);
    int x,y,z;
    float maxx= (ceil(target->Maxi.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(target->Maxi.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(target->Maxi.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    if (target->Mini.i==maxx) maxx+=COLLIDETABLEACCURACY/2;
    if (target->Mini.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (target->Mini.k==maxz) maxz+=COLLIDETABLEACCURACY/2;
    if (fabs((maxx-target->Mini.i)*(maxy-target->Mini.j)*(maxz-target->Mini.k))>COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*HUGEOBJECT) {
      hugeobjects.push_back(objectToPut);
      return;
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
  void Remove(const LineCollide* target,const T objectToKill) {
    //    int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(target->Mini,minx,miny,minz);
    //    hash_vec(target->Maxi,maxx,maxy,maxz);
    vector <T>::iterator removal= hugeobjects.begin();
    int x,y,z;
    float maxx= (ceil(target->Maxi.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(target->Maxi.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(target->Maxi.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    if (target->Mini.i==maxx) maxx+=COLLIDETABLEACCURACY/2;
    if (target->Mini.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (target->Mini.k==maxz) maxz+=COLLIDETABLEACCURACY/2;

    if (fabs((maxx-target->Mini.i)*(maxy-target->Mini.j)*(maxz-target->Mini.k))>COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*HUGEOBJECT) {
      while (removal!=hugeobjects.end()) {
	removal = find (hugeobjects.begin(),hugeobjects.end(),objectToKill);
	if (removal!=hugeobjects.end()) {
	  hugeobjects.erase(removal);
	}
      }
      return;
    }
    for (float i=target->Mini.i;i<maxx;i+=COLLIDETABLEACCURACY) {
      x = hash_int(i);
      for (float j=target->Mini.j;j<maxy;j+=COLLIDETABLEACCURACY) {    
	y = hash_int(j);
	for (float k=target->Mini.k;k<maxz;k+=COLLIDETABLEACCURACY) {
	  z = hash_int(k);
	  removal = table[x][y][z].begin();
	  while (removal!=table[x][y][z].end()) {
	    removal = find (table[x][y][z].begin(),table[x][y][z].end(),objectToKill);
	    if (removal!=table[x][y][z].end()) {
	      table[x][y][z].erase(removal);
	    }
	  }
	}
      }
    }
  }
};

#endif
