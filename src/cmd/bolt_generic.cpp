#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include <vector>

#include <string>
#include <algorithm>
#include "unit_generic.h"
#include "audiolib.h"
#include "config_xml.h"
using std::vector;
using std::string;

extern double interpolation_blend_factor;

bool Bolt::Update () {
  curdist +=speed*SIMULATION_ATOM;
  prev_position = cur_position;
  cur_position+=((ShipSpeed+drawmat.getR()*speed).Cast()*SIMULATION_ATOM);
  if (curdist>range) {
    delete this;//risky
    return false;
  }
#ifdef PERBOLTSOUND
  if (!AUDIsPlaying(sound)) {
    AUDDeleteSound (sound);
    sound=-1;
  } else {
#ifndef PERFRAMESOUND
    if ((*j)->sound!=-1)
      AUDAdjustSound ((*j)->sound,cur_position,(*j)->ShipSpeed+(*j)->speed*Vector ((*j)->drawmat[8],(*j)->drawmat[9],(*j)->drawmat[10]));
#endif

  }
#endif
  return true;
}

void bolt_draw::UpdatePhysics () {
  vector <vector <Bolt *> > *tmp = &bolts;
  for (int l=0;l<2;l++) {    
    for (vector <vector <Bolt *> >::iterator i= tmp->begin();i!=tmp->end();i++) {
      for (unsigned int j=0;j<i->size();j++) {
	///warning these require active star system to be set appropriately
	if (!(*i)[j]->Update()) {
	  j--;//deleted itself
	} else if ((*i)[j]->Collide()) {
	  j--;//deleted itself!
	}
      }
    }
    tmp = &balls;
  }
}

bool Bolt::Collide (Unit * target) {
  enum clsptr type = target->isUnit();
  if (target==owner||type==NEBULAPTR||type==ASTEROIDPTR) {
    static bool collideroids = XMLSupport::parse_bool(vs_config->getVariable("physics","AsteroidWeaponCollision","false"));
    if (type!=ASTEROIDPTR||(!collideroids)) {
      return false;
    }
  }
  Vector normal;
  float distance;
  Unit * affectedSubUnit;
  if ((affectedSubUnit =target->queryBSP (prev_position,cur_position,normal,distance))) {//ignore return
  //  QVector pos;
  //  double dis=distance;
  //  if ((affectedSubUnit = target->BeamInsideCollideTree(prev_position,cur_position,pos,normal,dis))) { 
  //    distance=dis;
    QVector tmp = (cur_position-prev_position).Normalize();
    tmp = tmp.Scale(distance);
    distance = curdist/range;
    GFXColor coltmp (col);
    /*    coltmp.r+=.5;
    coltmp.g+=.5;
    coltmp.b+=.5;
    if (coltmp.r>1)coltmp.r=1;
    if (coltmp.g>1)coltmp.g=1;
    if (coltmp.b>1)coltmp.b=1;*/
    float appldam = ((float(255-percentphase)/255)*damage);;
    float phasdam =((float(percentphase)/255)*damage);
    //if (damage>0) { This was needed back in the "I don't share code" ApplyDamage days
      target->ApplyDamage ((prev_position+tmp).Cast(),normal, appldam* ((1-distance)+distance*longrange),affectedSubUnit,coltmp,(Unit*)owner, phasdam* ((1-distance)+distance*longrange));
	  //}else if (damage<0) {
      //target->leach (1,phasdam<0?-phasdam:1,appldam<0?-appldam:1);      
	  //}
    return true;
  }
  return false;
}


