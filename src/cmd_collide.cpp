#include "cmd_unit.h"
#include "cmd_beam.h"
#include "gfx_mesh.h"
#include "cmd_collide.h"
vector <LineCollide*> collidequeue;
const int COLLIDETABLESIZE=20;//cube root of entries
const float COLLIDETABLEACCURACY=.005;// "1/largeness of sectors"
#define _USE_COLLIDE_TABLE
class CollideTable {
  int minaccessx,minaccessy,minaccessz,maxaccessx,maxaccessy,maxaccessz;
  vector <LineCollide*> table [COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];

  inline void hash_vec (const Vector & tmp, int &x, int &y, int &z) {
    x = ((int)(tmp.i*COLLIDETABLEACCURACY));
    y = ((int)(tmp.j*COLLIDETABLEACCURACY));
    z = ((int)(tmp.k*COLLIDETABLEACCURACY));
  }
  inline void get_max_min (int &minx,int &maxx, int &miny, int &maxy, int &minz, int &maxz) {
    if (maxx-minx>=COLLIDETABLESIZE) {
      minx = 0;
      maxx = COLLIDETABLESIZE-1;
    }
    if (maxy-miny>=COLLIDETABLESIZE) {
      miny = 0;
      maxy = COLLIDETABLESIZE-1;    
    }
    if (maxz-minz>=COLLIDETABLESIZE) {
      minz = 0;
      maxz = COLLIDETABLESIZE-1; 
    }
    minx = minx%COLLIDETABLESIZE/2+COLLIDETABLESIZE/2;
    miny = miny%COLLIDETABLESIZE/2+COLLIDETABLESIZE/2;
    minz = minz%COLLIDETABLESIZE/2+COLLIDETABLESIZE/2;
    maxx = maxx%COLLIDETABLESIZE/2+COLLIDETABLESIZE/2;
    maxy = maxy%COLLIDETABLESIZE/2+COLLIDETABLESIZE/2;
    maxz = maxz%COLLIDETABLESIZE/2+COLLIDETABLESIZE/2;
  }
public:
  CollideTable() {
    minaccessx=COLLIDETABLESIZE-1;
    minaccessy=COLLIDETABLESIZE-1;
    minaccessz=COLLIDETABLESIZE-1;
    maxaccessx=0;
    maxaccessy=0;
    maxaccessz=0;    
  }
  void Clear () {
    for (int i=minaccessx;i<=maxaccessx;i++) {
    for (int j=minaccessy;j<=maxaccessy;j++) {
    for (int k=minaccessz;k<=maxaccessz;k++) {
      if (table[i][j][k].size())
	table[i][j][k]=vector <LineCollide*>();
    }
    }
    }
    minaccessx=COLLIDETABLESIZE-1;
    minaccessy=COLLIDETABLESIZE-1;
    minaccessz=COLLIDETABLESIZE-1;
    maxaccessx=0;
    maxaccessy=0;
    maxaccessz=0;
    for (int l=0;l<COLLIDETABLESIZE;l++) {
      for (int m=0;m<COLLIDETABLESIZE;m++) {
	for (int n=0;n<COLLIDETABLESIZE;n++) {
	  if (table[l][m][n].size()!=0)
	    int wahooo=1;
	}
      }

    }

  }
  void Get (const Vector &Min, const Vector & Max, vector <LineCollide*> &retval) {    

    int minx,miny,minz,maxx,maxy,maxz;
    hash_vec(Min,minx,miny,minz);
    hash_vec(Max,maxx,maxy,maxz);
    get_max_min (minx,maxx,miny,maxy,minz,maxz);
    for (int i=minx;;i++) {//reverse??????????????? which is fast
      if (i==COLLIDETABLESIZE) i=0;
      for (int j=miny;;j++) {      
	if (j==COLLIDETABLESIZE) j=0;
	for (int k=minz;;k++) {
	  if (k==COLLIDETABLESIZE) k=0;	  
	  //	  table[i][j][k].push_back(target);
	  for (unsigned int l=0;l<table[i][j][k].size();l++) {
	    retval.push_back ((table[i][j][k])[l]);
	  }
	  if (k==maxz) 
	    break;
	}
	if (j==maxy) 
	  break;
      }
      if (i==maxx)
	break;
    }
    
  }
  
  void Put(LineCollide* target) {
    int minx,miny,minz,maxx,maxy,maxz;
    hash_vec(target->Mini,minx,miny,minz);
    hash_vec(target->Maxi,maxx,maxy,maxz);
    get_max_min (minx,maxx,miny,maxy,minz,maxz);
    if (maxx>maxaccessx) maxaccessx=maxx;
    if (maxy>maxaccessy) maxaccessy=maxy;
    if (maxz>maxaccessz) maxaccessz=maxz;
    if (minx<minaccessx) minaccessx=minx;
    if (miny<minaccessy) minaccessy=miny;
    if (minz<minaccessz) minaccessz=minz;

    for (int i=minx;;i++) {//reverse??????????????? which is fast
      if (i==COLLIDETABLESIZE) i=0;
      for (int j=miny;;j++) {      
	if (j==COLLIDETABLESIZE) j=0;
	for (int k=minz;;k++) {
	  if (k==COLLIDETABLESIZE) k=0;	  
	  table[i][j][k].push_back(target);
	  if (k==maxz) 
	    break;
	}
	if (j==maxy) 
	  break;
      }
      if (i==maxx)
	break;
    }

  }
} collidetable;

void AddCollideQueue (const LineCollide &tmp) {
  int size = collidequeue.size();
  collidequeue.push_back (new LineCollide(tmp));
#ifdef _USE_COLLIDE_TABLE
  collidetable.Put (collidequeue[size]);
#endif

  
}
void ClearCollideQueue() {
#ifdef _USE_COLLIDE_TABLE
  collidetable.Clear();//blah might take some time
#endif
  for (int i=0;i<collidequeue.size();i++) {
    delete collidequeue[i];
  }
  collidequeue = vector<LineCollide*>();
}

void Unit::CollideAll() {
  unsigned int i;
  Vector minx (Position().i-radial_size,Position().j-radial_size,Position().k-radial_size);
  Vector maxx(Position().i+radial_size,Position().j+radial_size,Position().k+radial_size);
  //target->curr_physical_state.position;, rSize();
#ifdef _USE_COLLIDE_TABLE
  #define COLQ colQ
  vector <LineCollide*> colQ;
  collidetable.Get (minx,maxx,colQ);
#else
  #define COLQ collidequeue
#endif
  for (i=0;i<COLQ.size();i++) {
    if (Position().i+radial_size>COLQ[i]->Mini.i&&
	Position().i-radial_size<COLQ[i]->Maxi.i&&
	Position().j+radial_size>COLQ[i]->Mini.j&&
	Position().j-radial_size<COLQ[i]->Maxi.j&&
	Position().k+radial_size>COLQ[i]->Mini.k&&
	Position().k-radial_size<COLQ[i]->Maxi.k) {
      //continue;
      switch (COLQ[i]->type) {
      case LineCollide::UNIT://other units!!!
	((Unit*)COLQ[i]->object)->Collide(this);
	break;
      case LineCollide::BEAM:
	((Beam*)COLQ[i]->object)->Collide(this);
	break;
      case LineCollide::BALL:
	break;
      case LineCollide::BOLT:
	break;
      case LineCollide::PROJECTILE:
	break;
      }
    }
  }
  //add self to the queue??? using prev and cur physical state as an UNKNOWN
  AddCollideQueue (LineCollide(this,LineCollide::UNIT,minx,maxx));
#undef COLQ
}

bool Mesh::Collide (Unit * target, const Transformation &cumtrans, Matrix cumtransmat) {
  Transformation cumulative_transformation = local_transformation;
  cumulative_transformation.Compose(cumtrans, cumtransmat);
  //cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  //cumulative_transformation.position//rSize()
  if (target->querySphere (cumulative_transformation.position,rSize())&&
      target->queryBoundingBox (cumulative_transformation.position,rSize())
      //&&IntersectBSP (cumulative_transformation.position,rSize())//bsp
      )
    return true;
  //
  return false;
}

bool Unit::OneWayCollide (Unit * target) {//do each of these bubbled subunits collide with the other unit?
  int i;
  for (i=0;i<nummesh;i++) {
    if (meshdata[i]->Collide(target,cumulative_transformation,cumulative_transformation_matrix))
      return true;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->OneWayCollide(target))
      return true;
  }
  return false;
}

bool Unit::Collide (Unit * target) {
  if (target==this) 
    return false;
  //unit v unit? use point sampling?
  //now first make sure they're within bubbles of each other...
  if ((Position()-target->Position()).Magnitude()>radial_size+target->radial_size)
    return false;
  //now do some serious checks
  if (radial_size>target->radial_size) {
    if ((!target->OneWayCollide(this))||(!OneWayCollide(target)))
      return false;
  }else {
    if ((!OneWayCollide(target))||(!target->OneWayCollide(this)))
      return false;
  }
  //deal damage similarly to beam damage!!  Apply some sort of repel force

  //each mesh with each mesh? naw that should be in one way collide
  return true;
}


bool Beam::Collide (Unit * target) {
  if (target==owner) 
    return false;
  float distance = target->querySphere (center,direction,0);
  if (distance==0||distance>curlength) {
    return false;
  }
  if (target->queryBoundingBox(center,direction,0)==0)
    return false;
  curlength = distance;
  impact=IMPACT;
  //deliver float tmp=(curlength/range)); (damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
  return true;
}
