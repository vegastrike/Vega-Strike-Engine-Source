#include "cmd_unit.h"
#include "cmd_beam.h"
#include "gfx_mesh.h"
#include "cmd_collide.h"
vector <LineCollide*> collidequeue;
const int COLLIDETABLESIZE=20;//cube root of entries
const int COLLIDETABLEACCURACY=200;// "1/largeness of sectors"
#define _USE_COLLIDE_TABLE
class CollideTable {
  int minaccessx,minaccessy,minaccessz,maxaccessx,maxaccessy,maxaccessz;
  vector <LineCollide*> table [COLLIDETABLESIZE][COLLIDETABLESIZE][COLLIDETABLESIZE];
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
  CollideTable() {
    minaccessx=COLLIDETABLESIZE-1;
    minaccessy=COLLIDETABLESIZE-1;
    minaccessz=COLLIDETABLESIZE-1;
    maxaccessx=0;
    maxaccessy=0;
    maxaccessz=0;    
  }
  void Clear () {
    int tminx = minaccessx;
    int tminy = minaccessy;
    int tminz = minaccessz;

    int tmaxx = maxaccessx;
    int tmaxy = maxaccessy;
    int tmaxz = maxaccessz;

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
	    fprintf (stderr,"ERROR!!! SIZE: %d\n\n\n",table[l][m][n].size());
	}
      }

    }

  }
  void Get (const Vector &Min, const Vector & Max, vector <LineCollide*> &retval) {    

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
    
  }
  
  void Put(LineCollide* target) {
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
	  table[x][y][z].push_back(target);
	}
      }
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
  for (unsigned int i=0;i<collidequeue.size();i++) {
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
  fprintf (stderr,"***MESH %s DELIVERS DAMAGE TO %s\n",name.c_str(),target->name.c_str());
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
  fprintf (stderr,"BEAM DELIVERS DAMAGE TO \n");//%s",target->name.c_str());
  //deliver float tmp=(curlength/range)); (damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
  return true;
}
