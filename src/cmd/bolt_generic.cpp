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
Bolt::Bolt (const weapon_info * typ, const Matrix &orientationpos,  const Vector & shipspeed, void * owner, CollideMap::iterator hint):  cur_position (orientationpos.p), ShipSpeed (shipspeed) {
  VSCONSTRUCT2('t')
  bolt_draw *q= _Universe->activeStarSystem()->bolts;
  prev_position= cur_position;
  this->owner = owner;
  this->type = typ;
  curdist = 0;
  CopyMatrix (drawmat,orientationpos);
  Vector vel=shipspeed+orientationpos.getR()*typ->Speed;
  if (typ->type==weapon_info::BOLT) {
    ScaleMatrix (drawmat,Vector (typ->Radius,typ->Radius,typ->Length));
    decal = Bolt::AddTexture(q,typ->file);
    this->location=_Universe->activeStarSystem()->collidemap->insert(Collidable(Bolt::BoltIndex(q->bolts[decal].size(),decal,false).bolt_index,(shipspeed+orientationpos.getR()*typ->Speed).Magnitude()*.5,cur_position+vel*SIMULATION_ATOM*.5),hint);
    q->bolts[decal].push_back (*this);
  } else {
    ScaleMatrix (drawmat,Vector (typ->Radius,typ->Radius,typ->Radius));
    decal=Bolt::AddAnimation(q,typ->file,cur_position);

    this->location=_Universe->activeStarSystem()->collidemap->insert(Collidable(Bolt::BoltIndex(q->balls[decal].size(),decal,true).bolt_index,(shipspeed+orientationpos.getR()*typ->Speed).Magnitude()*.5,cur_position+vel*SIMULATION_ATOM*.5),hint);
    q->balls[decal].push_back (*this);
  }
}
 
bool Bolt::Update (int index) {
  const weapon_info *type=this->type;
  float speed=type->Speed;
  curdist +=speed*SIMULATION_ATOM;
  prev_position = cur_position;
  cur_position+=((ShipSpeed+drawmat.getR()*speed/((type->type==weapon_info::BALL)*type->Radius+(type->type!=weapon_info::BALL)*type->Length)).Cast()*SIMULATION_ATOM);
  if (curdist>type->Range) {
    this->Destroy(index);//risky
    return false;
  }
  Collidable updated(**location);
  updated.SetPosition(.5*(prev_position+cur_position));
  //location=_Universe->activeStarSystem()->collidemap->changeKey(location,updated);
  return true;
}

void bolt_draw::UpdatePhysics () {
  vector <vector <Bolt> > *tmp = &bolts;
  for (int l=0;l<2;l++) {    
    for (vector <vector <Bolt> >::iterator i= tmp->begin();i!=tmp->end();i++) {
      for (int j=0;j<i->size();j++) {
        int size=i->size();
        Bolt * b=&((*i)[j]);
	///warning these require active star system to be set appropriately
	if (!b->Update(j)) {
	  j--;//deleted itself
	} else if (b->Collide(j)) {
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
  static bool collidejump = XMLSupport::parse_bool(vs_config->getVariable("physics","JumpWeaponCollision","false"));
  if (type==PLANETPTR&&(!collidejump)&&!target->GetDestinations().empty()) {
    return false;
  }

  Vector normal;
  float distance;
  Unit * affectedSubUnit;
  if ((affectedSubUnit =target->queryBSP (prev_position,cur_position,normal,distance))) {//ignore return
    QVector tmp = (cur_position-prev_position).Normalize();
    tmp = tmp.Scale(distance);
    distance = curdist/this->type->Range;
    GFXColor coltmp (this->type->r,this->type->g,this->type->b,this->type->a);
    /*    coltmp.r+=.5;
    coltmp.g+=.5;
    coltmp.b+=.5;
    if (coltmp.r>1)coltmp.r=1;
    if (coltmp.g>1)coltmp.g=1;
    if (coltmp.b>1)coltmp.b=1;*/
    //float appldam = type->Damage;
    //float phasdam =type->PhaseDamage;
    //if (damage>0) { This was needed back in the "I don't share code" ApplyDamage days
      target->ApplyDamage ((prev_position+tmp).Cast(),
                           normal, 
                           this->type->Damage* ((1-distance)+distance*this->type->Longrange),
                           affectedSubUnit,
                           coltmp,
                           owner, 
                           this->type->PhaseDamage* ((1-distance)+distance*this->type->Longrange));
	  //}else if (damage<0) {
      //target->leach (1,phasdam<0?-phasdam:1,appldam<0?-appldam:1);      
	  //}
    return true;
  }
  return false;
}
size_t nondecal_index(Collidable::CollideRef b) {
  return b.bolt_index>>8;
}
Bolt * Bolt::BoltFromIndex(StarSystem * ss, Collidable::CollideRef b) {
  size_t ind =nondecal_index(b);
  if (b.bolt_index&128) {
    return &ss->bolts->balls[b.bolt_index&0x7f][ind];
  }else{
    return &ss->bolts->bolts[b.bolt_index&0x7f][ind];
  }

}
bool Bolt::CollideAnon(Collidable::CollideRef b, Unit *un) {
  Bolt * tmp=BoltFromIndex(_Universe->activeStarSystem(),b);
  if (tmp->Collide(un)) {
    tmp->Destroy(nondecal_index(b));
    return true;
  }
  return false;
}
union Collidable::CollideRef;
Collidable::CollideRef Bolt::BoltIndex(int index, int decal, bool isBall) {
  Collidable::CollideRef temp;
  temp.bolt_index=index;
  temp.bolt_index<<=8;
  temp.bolt_index|=decal;
  temp.bolt_index|=isBall?128:0;
  return temp;  
}

void BoltDestroyGeneric (Bolt * whichbolt, int index, int decal, bool isBall) {
  VSDESTRUCT2
  bolt_draw *q = _Universe->activeStarSystem()->bolts;
  vector <vector <Bolt> > *target;
  if (!isBall) { 
    target = &q->bolts;
  } else {
    target = &q->balls;
  }
  vector<Bolt> * vec=&(*target)[decal];
  int fsize=vec->size();
  if (&(*vec)[index]==whichbolt) {
    int tsize=vec->size();
    (*vec->back().location)->ref=(*(*vec)[index].location)->ref;
    assert (index<tsize);
    _Universe->activeStarSystem()->collidemap->erase((*vec)[index].location);
    (*vec)[index]=vec->back();//just a memcopy, yo    
    vec->pop_back();//pop that back up
  }else {
    VSFileSystem::vs_fprintf (stderr,"Bolt Fault Nouveau! Not found in draw queue! No Chance to recover\n");
    fflush(stderr);
    assert(0);
    /*
    vector <Bolt>::iterator tmp= std::find ((*target)[decal].begin(),(*target)[decal].end(),*whichbolt); 
    if (tmp!=(*target)[decal].end()) {
      (*target)[decal].erase(tmp);
    } else {
      //might as well look for potential faults! Doesn't cost us time
      VSFileSystem::vs_fprintf (stderr,"Bolt Fault! Not found in draw queue! Attempting to recover\n");
      for (vector <vector <Bolt> > *srch = &q->bolts;srch!=NULL;srch=&q->balls) {
        for (unsigned int mdecal=0;mdecal<(*srch).size();mdecal++) {
          vector <Bolt>::iterator mtmp= (*srch)[mdecal].begin();
          while (mtmp!=(*srch)[mdecal].end()) {
            std::find ((*srch)[mdecal].begin(),(*srch)[mdecal].end(),*whichbolt);       
            if (mtmp!=(*srch)[mdecal].end()) {
              (*srch)[mdecal].erase (mtmp);
              VSFileSystem::vs_fprintf (stderr,"Bolt Fault Recovered\n");
            }
          }
        }
        if (srch==&q->balls) {
          break;
        }
      }     
    }
    */
  }
}
