#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include <vector>

#include <string>
#include <algorithm>
#include "unit_generic.h"
#include "configxml.h"
GFXVertexList * bolt_draw::boltmesh=NULL;

extern void AUDAdjustSound(int i, QVector const & qv, Vector const & vv);
extern bool AUDIsPlaying(int snd);
bolt_draw::~bolt_draw () {

  unsigned int i;
  for (i=0;i<balls.size();i++) {
    for (unsigned int j=0;j<balls[i].size();j++) {
      delete balls[i][j];
    }
  }
  for (i=0;i<bolts.size();i++) {
    for (unsigned int j=0;j<bolts[i].size();j++) {
      delete bolts[i][j];
    }
  }
}

bolt_draw::bolt_draw () {
  if (!boltmesh) {
    GFXVertex vtx[12];
#define V(ii,xx,yy,zz,ss,tt) vtx[ii].x=xx;vtx[ii].y=yy;vtx[ii].z=zz;vtx[ii].i=0;vtx[ii].j=0;vtx[ii].k=1;vtx[ii].s=ss;vtx[ii].t=tt;
    V(0,0,0,-.875,0,.5);
    V(1,0,-1,0,.875,1);
    V(2,0,0,.125,1,.5);
    V(3,0,1,0,.875,0);
    V(4,0,0,-.875,0,.5);
    V(5,-1,0,0,.875,1);
    V(6,0,0,.125,1,.5);
    V(7,1,0,0,.875,0);
    V(8,1,0,0,.1875,0);
    V(9,0,1,0,.375,.1875);
    V(10,-1,0,0,.1875,.375);
    V(11,0,-1,0,0,.1875);
    boltmesh = new GFXVertexList (GFXQUAD,12,vtx,12,false);//not mutable;
  }
}

Bolt::Bolt (const weapon_info & typ, const Matrix &orientationpos,  const Vector & shipspeed, Unit * owner): col (typ.r,typ.g,typ.b,typ.a), cur_position (orientationpos.p), ShipSpeed (shipspeed) {
  VSCONSTRUCT2('t')
  bolt_draw *q= _Universe->activeStarSystem()->bolts;
  prev_position= cur_position;
  this->owner = owner;
  this->decal = 0;
  type = typ.type;
  damage = typ.Damage+typ.PhaseDamage;
  if (damage) 
    percentphase=(unsigned char) (255.*typ.PhaseDamage/damage);
  else
    percentphase=0;
  longrange = typ.Longrange;
  radius = typ.Radius;
  speed = typ.Speed/(type==weapon_info::BOLT?typ.Length:typ.Radius);//will scale it by length long!
  range = typ.Range/(type==weapon_info::BOLT?typ.Length:typ.Radius);
  curdist = 0;
  CopyMatrix (drawmat,orientationpos);
  
  if (type==weapon_info::BOLT) {
    ScaleMatrix (drawmat,Vector (typ.Radius,typ.Radius,typ.Length));
    //    if (q->boltmesh==NULL) {
    //      CreateBoltMesh();
    //    }
	/*
    decal = q->boltdecals->AddTexture (typ.file.c_str(),MIPMAP);
    if (decal>=(int)q->bolts.size()) {
      q->bolts.push_back (vector <Bolt *>());
      int blargh = q->boltdecals->AddTexture (typ.file.c_str(),MIPMAP);
      if (blargh>=(int)q->bolts.size()) {
	q->bolts.push_back (vector <Bolt *>());	
      }
      q->cachedecals.push_back (blargh);
    }
    q->bolts[decal].push_back (this);
	*/
  } else {
    ScaleMatrix (drawmat,Vector (typ.Radius,typ.Radius,typ.Radius));
	/*
    decal=-1;
    for (unsigned int i=0;i<q->animationname.size();i++) {
      if (typ.file==q->animationname[i]) {
	decal=i;
      }
    }
    if (decal==-1) {
      decal = q->animations.size();
      q->animationname.push_back (typ.file);
      q->animations.push_back (new Animation (typ.file.c_str(), true,.1,MIPMAP,false));//balls have their own orientation
      q->animations.back()->SetPosition (cur_position);
      q->balls.push_back (vector <Bolt *> ());
    }
    q->balls[decal].push_back (this);
	*/
  }
}

void Bolt::Draw () {
}
Bolt::~Bolt () {
  VSDESTRUCT2
  bolt_draw *q = _Universe->activeStarSystem()->bolts;
  vector <vector <Bolt *> > *target;
  if (type==weapon_info::BOLT) { 
    target = &q->bolts;
    //q->boltdecals->DelTexture (decal);
  } else {
    target = &q->balls;
  }
  vector <Bolt *>::iterator tmp= std::find ((*target)[decal].begin(),(*target)[decal].end(),this); 
  if (tmp!=(*target)[decal].end()) {
    (*target)[decal].erase(tmp);
  } else {
    //might as well look for potential faults! Doesn't cost us time
    fprintf (stderr,"Bolt Fault! Not found in draw queue! Attempting to recover\n");
    for (vector <vector <Bolt *> > *srch = &q->bolts;srch!=NULL;srch=&q->balls) {
      for (unsigned int mdecal=0;mdecal<(*srch).size();mdecal++) {
	vector <Bolt *>::iterator mtmp= (*srch)[mdecal].begin();
	while (mtmp!=(*srch)[mdecal].end()) {
	  std::find ((*srch)[mdecal].begin(),(*srch)[mdecal].end(),this);       
	  if (mtmp!=(*srch)[mdecal].end()) {
	    (*srch)[mdecal].erase (mtmp);
	    fprintf (stderr,"Bolt Fault Recovered\n");
	  }
	}
      }
      if (srch==&q->balls) {
	break;
      }
    }

  }
}
