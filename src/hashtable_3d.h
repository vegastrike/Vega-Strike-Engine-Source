#ifndef _HASHTABLE_3D_H_
#define _HASHTABLE_3D_H_
#include "gfx/vec.h"
#include <algorithm>
#include <vector>
#define COLLIDETABLESIZE sizeof(CTSIZ)
#define COLLIDETABLEACCURACY sizeof (CTACCURACY)

const int HUGEOBJECT=12; //objects that go over 16 sectors are considered huge and better to check against everything.

struct LineCollide {
  void * object;
  Vector Mini;
  Vector Maxi;
  enum collidables {UNIT, BEAM,BALL,BOLT,PROJECTILE} type;
  bool hhuge;
  LineCollide(){hhuge=false;}
  LineCollide (void * objec, enum collidables typ,const Vector &st, const Vector &en) {this->object=objec;this->type=typ;this->Mini=st;this->Maxi=en;hhuge=false;}
  LineCollide (const LineCollide &l) {object=l.object; type=l.type; Mini=l.Mini;Maxi=l.Maxi;hhuge=l.hhuge;}      
};


template <class T, class CTSIZ, class CTACCURACY> class Hashtable3d {
  int minaccessx,minaccessy,minaccessz,maxaccessx,maxaccessy,maxaccessz;
  std::vector <T> hugeobjects;
  std::vector <T> table [COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];
  void hash_vec (float i, float j, float k, int &x, int &y, int &z) {
    x = hash_int(i);
    y = hash_int(j);
    z = hash_int(k);
  }
  void hash_vec (const Vector & t,int &x, int&y,int&z) {
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
  int hash_int (const float aye) {
    return ((int)(((aye<0)?(aye-COLLIDETABLEACCURACY):aye)/COLLIDETABLEACCURACY))%(COLLIDETABLESIZE/2)+(COLLIDETABLESIZE/2); 
  }
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
  void Get (const Vector &Exact, std::vector <T> &retval) {
    retval = table[hash_int(Exact.i)][hash_int(Exact.j)][hash_int(Exact.k)];
    //retval+=hugeobjects;
    //blah = blooh;
    retval.insert (retval.end(),hugeobjects.begin(),hugeobjects.end());
  }
  std::vector <T>& GetHuge () {
    return hugeobjects;
  }
  void Get (const LineCollide* target, std::vector <T> &retval) {    
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
    retval = hugeobjects;
    if (target->hhuge) {
      return;//we can't get _everything
    } 
    for (float i=target->Mini.i;i<maxx;i+=COLLIDETABLEACCURACY) {
      x = hash_int (i);
      for (float j=target->Mini.j;j<maxy;j+=COLLIDETABLEACCURACY) {   
	y = hash_int(j);
	for (float k=target->Mini.k;k<maxz;k+=COLLIDETABLEACCURACY) {
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
  }
  void Put(LineCollide* target,const T objectToPut) {
    //    int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(target->Mini,minx,miny,minz);
    //    hash_vec(target->Maxi,maxx,maxy,maxz);
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
    if (fabs((maxx-minx)*(maxy-miny)*(maxz-minz))>COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*COLLIDETABLEACCURACY*HUGEOBJECT) {
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
  T Remove(const LineCollide* target,const T objectToKill) {
    //    int minx,miny,minz,maxx,maxy,maxz;
    //    hash_vec(target->Mini,minx,miny,minz);
    //    hash_vec(target->Maxi,maxx,maxy,maxz);
    T retval;
    std::vector <T>::iterator removal= hugeobjects.begin();
    int x,y,z;
    float maxx= (ceil(target->Maxi.i/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxy= (ceil(target->Maxi.j/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    float maxz= (ceil(target->Maxi.k/COLLIDETABLEACCURACY))*COLLIDETABLEACCURACY;
    if (target->Mini.i==maxx) maxx+=COLLIDETABLEACCURACY/2;
    if (target->Mini.j==maxy) maxy+=COLLIDETABLEACCURACY/2;
    if (target->Mini.k==maxz) maxz+=COLLIDETABLEACCURACY/2;

    if (target->hhuge) {
      while (removal!=hugeobjects.end()) {
		  removal = std::find (hugeobjects.begin(),hugeobjects.end(),objectToKill);
	if (removal!=hugeobjects.end()) {
	  retval = *removal;
	  hugeobjects.erase(removal);
	}
      }
      return retval;
    }
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
	      retval = *removal;
	      table[x][y][z].erase(removal);
	    }
	  }
	}
      }
    }
    return retval;
  }
};

#endif
