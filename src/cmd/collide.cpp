#include "cmd/collide.h"
#include "vegastrike.h"
#include "unit_generic.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "gfx/bsp.h"
#include "collide/rapcol.h"
#include "collide/csgeom/transfrm.h"
#include "collide/collider.h"
#include "hashtable.h"
#include <string>
#include "vs_globals.h"
#include "configxml.h"
static Hashtable <std::string,collideTrees,127> unitColliders;
collideTrees::collideTrees (const std::string &hk, BSPTree *bT, BSPTree *bS, csRapidCollider *cT, csRapidCollider *cS): hash_key(hk),bspTree(bT), bspShield(bS), colShield(cS) {
	for (int i=0;i<collideTreesMaxTrees;++i) {
		rapidColliders[i]=NULL;
	}
	rapidColliders[0]=cT;
	
	refcount=1;
	unitColliders.Put (hash_key,this);
}
float loge2 = log(2.f);
csRapidCollider * collideTrees::colTree(Unit * un, const Vector & othervelocity) {
	const float const_factor=1;
	float magsqr = un->GetVelocity().MagnitudeSquared();
	float newmagsqr = (un->GetVelocity()-othervelocity).MagnitudeSquared();	
	float speedsquared =const_factor*const_factor*(magsqr>newmagsqr?newmagsqr:magsqr);
        static int max_collide_trees=XMLSupport::parse_int(vs_config->getVariable("physics","max_collide_trees","16384"));

	if (un->rSize()*un->rSize()>SIMULATION_ATOM*SIMULATION_ATOM*speedsquared||max_collide_trees==1) {
		return rapidColliders[0];
	}
	if (rapidColliders[0]==NULL)
		return NULL;
	float movement = sqrtf (speedsquared)*SIMULATION_ATOM;
	int pow =(int)ceil(log (movement/un->rSize())/loge2);
//	pow=collideTreesMaxTrees-1;
	if (pow<0)
		pow=0;

	if (pow>=collideTreesMaxTrees||pow>=max_collide_trees)
		pow=collideTreesMaxTrees-1;
	
	int val = 1<<pow;
	//VSFileSystem::vs_fprintf (stderr,"%s %d %d\n",un->name.c_str(),pow,val);

	if (rapidColliders[pow]==NULL) {
		rapidColliders[pow]=un->getCollideTree(Vector(1,1,val));
	}
	return rapidColliders[pow];
}
collideTrees* collideTrees::Get(const std::string &hash_key) {
  return unitColliders.Get(hash_key);
}
void collideTrees::Dec() {
  refcount--;
  if (refcount==0) {
    unitColliders.Delete (hash_key);
    if (bspTree)
      delete bspTree;
	for (int i=0;i<collideTreesMaxTrees;++i) {
		if (rapidColliders[i])
			delete rapidColliders[i];
	}
    if (bspShield)
      delete bspShield;
    if (colShield)
      delete colShield;
    delete this;
    return;
  }
}

bool TableLocationChanged (const QVector & Mini,const QVector & minz) { 
  return (_Universe->activeStarSystem()->collidetable->c.hash_int (Mini.i)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.i) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.j)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.j) ||
	  _Universe->activeStarSystem()->collidetable->c.hash_int (Mini.k)!=_Universe->activeStarSystem()->collidetable->c.hash_int (minz.k));
}
bool TableLocationChanged (const LineCollide &lc, const QVector &minx, const QVector & maxx) {
  return TableLocationChanged (lc.Mini,minx) || TableLocationChanged (lc.Maxi,maxx);
}
void KillCollideTable (LineCollide * lc,StarSystem * ss) {
  if (lc->type==LineCollide::UNIT) {
    ss->collidetable->c.Remove ( lc,lc->object.u);
  } else {
    printf ("such collide types as %d not allowed",lc->type);
  }
}
bool EradicateCollideTable (LineCollide * lc, StarSystem * ss) {
  if (lc->type==LineCollide::UNIT) { 
    return ss->collidetable->c.Eradicate (lc->object.u);
  } else {
    printf ("such collide types as %d not allowed",lc->type);
    return false;
  }
}
void AddCollideQueue (LineCollide &tmp,StarSystem * ss) {
  if (tmp.type==LineCollide::UNIT) { 
    ss->collidetable->c.Put (&tmp,tmp.object.u);
  } else {
    printf ("such collide types as %d not allowed",tmp.type);
  }
}

bool lcwithin (const LineCollide & lc, const LineCollide&tmp) {
  return (lc.Mini.i< tmp.Maxi.i&&
	  lc.Mini.j< tmp.Maxi.j&&
	  lc.Mini.k< tmp.Maxi.k&&
	  lc.Maxi.i> tmp.Mini.i&&
	  lc.Maxi.j> tmp.Mini.j&&
	  lc.Maxi.k> tmp.Mini.k);
}
bool usehuge_table() {
  const unsigned int A = 9301;
  const unsigned int C = 49297;
  const unsigned int M = 233280;
  static unsigned int seed=3259235;
  seed = (seed * A + C) % M;
  return seed<(M/100);
}
bool Bolt::Collide (int index) {
  static bool New_Collide_System=XMLSupport::parse_bool(vs_config->getVariable("physics","new_collisions","true"));
  if (New_Collide_System) {
    //Collidable updated(**location);
    //updated.SetPosition(.5*(prev_position+cur_position));
    return _Universe->activeStarSystem()->collidemap[Unit::UNIT_BOLT]->CheckCollisions(this,**location);
  }else {
#ifdef OLD_COLLIDE_SYSTEM
    UnitCollection *candidates[2];  
    bool use_huge_list=usehuge_table();
    _Universe->activeStarSystem()->collidetable->c.Get (cur_position,candidates,use_huge_list);
    LineCollide minimaxi;//might as well have this so we can utilize common function
    minimaxi.Mini= ( prev_position.Min (cur_position));
    minimaxi.Maxi= ( prev_position.Max (cur_position));
    for (unsigned int j=0;j<2;j++) {
      Unit * un;
      for (un_iter i=candidates[j]->createIterator();(un=*i)!=NULL;++i) {
        
        if (lcwithin (minimaxi,(un)->GetCollideInfo ())) {
          if (this->Collide (un)) {
            if (j==0&&use_huge_list) {
              _Universe->activeStarSystem()->collidetable->c.AddHugeToActive(un);
            }
            Destroy(index);
            return true;
          }
          
        }
      }

    }
#endif
  }
  return false;
}
static bool beamCheckCollision (QVector pos, float len, const Collidable & un) {
  
  return (un.GetPosition()-pos).MagnitudeSquared()<=len*len+2*len*un.radius+un.radius*un.radius;
}
void Beam::CollideHuge (const LineCollide & lc, Unit * targetToCollideWith, Unit * firer, Unit * superunit) {
  static bool newUnitCollisions=XMLSupport::parse_bool(vs_config->getVariable("physics","new_collisions","true"));  



  if (newUnitCollisions) {
    QVector x0=center;
    QVector v=direction*curlength;
    if (is_null(superunit->location[Unit::UNIT_ONLY])&&curlength) {
      if (targetToCollideWith){
        this->Collide(targetToCollideWith,firer,superunit);
      }
    }else if (curlength) {
      CollideMap * cm=_Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY];

      CollideMap::iterator superloc=superunit->location[Unit::UNIT_ONLY];
      CollideMap::iterator tmore=superloc;
      if (!cm->Iterable(superloc)) {
        //fprintf (stderr,"ERROR: New collide map entry checked for collision\n Aborting collide\n");
        CollideArray::CollidableBackref * br=static_cast<CollideArray::CollidableBackref*>(superloc);    
        CollideMap::iterator tmploc=cm->begin()+br->toflattenhints_offset;
        if (tmploc==cm->end())
          tmploc--;
        tmore=superloc=tmploc;//don't decrease tless
      }else {
        ++tmore;
      }
      double t = v.Dot(x0)/v.Dot(v);//find where derivative of radius is zero
      double r0= x0.i;
      double r1= x0.i+v.i;
      double minlook=r0<r1?r0:r1;
      double maxlook=r0<r1?r1:r0;
      bool targcheck=false;
    
      maxlook+=(maxlook-(*superunit->location[Unit::UNIT_ONLY])->getKey())+2*curlength;//double damage, yo
      minlook+=(minlook-(*superunit->location[Unit::UNIT_ONLY])->getKey())-2*curlength*curlength;
      // (a+2*b)^2-(a+b)^2 = 3b^2+2ab = 2b^2+(a+b)^2-a^2
      if (superloc!=cm->begin()&&
          minlook<(*superunit->location[Unit::UNIT_ONLY])->getKey()){
        //less traversal
        CollideMap::iterator tless=superloc;
        --tless;
        while((*tless)->getKey()>=minlook) {
          CollideMap::iterator curcheck=tless;          
          bool breakit=false;
          if (tless!=cm->begin()) {
            --tless;            
          }else {
            breakit=true;
          }
          if ((*curcheck)->radius>0) {
            if (beamCheckCollision(center,curlength,(**curcheck))) {
              Unit * tmp=(**curcheck).ref.unit;
              this->Collide(tmp,firer,superunit);
              targcheck=(targcheck||tmp==targetToCollideWith);
            }
          }
          if (breakit)
            break;
          
        }
      }
      if (maxlook>(*superunit->location[Unit::UNIT_ONLY])->getKey()) {
        //greater traversal
        while (tmore!=cm->end()&&(*tmore)->getKey()<=maxlook){        
          if ((*tmore)->radius>0) {
            Unit *un=(*tmore)->ref.unit;
            if (beamCheckCollision(center,curlength,**tmore++)) {
              this->Collide(un,firer,superunit);
              targcheck=(targcheck||un==targetToCollideWith);
            }
          }else ++tmore;
        }        
      }
      if (targetToCollideWith&&!targcheck){
        this->Collide(targetToCollideWith,firer,superunit);
      }
    
    }
  }else {
#ifdef OLD_COLLIDE_SYSTEM
  UnitCollection *colQ [tablehuge+1];
  bool use_huge_list = usehuge_table();
  if (!lc.hhuge) {
    int sizecolq = _Universe->activeStarSystem()->collidetable->c.Get (&lc,colQ,use_huge_list);
    for (int j=0;j<sizecolq;j++) {
      Unit *un;
      for (un_iter i=colQ[j]->createIterator();(un=(*i))!=NULL;++i) {

	if (lcwithin(lc,(un)->GetCollideInfo())) {
	  if (this->Collide (un,firer,superunit)) {
	    if (j==0&&use_huge_list) {
	      _Universe->activeStarSystem()->collidetable->c.AddHugeToActive(un);
	    }
	  }
	}
      }
    }
  }else {
    if (targetToCollideWith&&(!use_huge_list)) {
      this->Collide(targetToCollideWith,firer,superunit);
    }else {
      un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
      Unit *un;
      for (;(un=*i)!=NULL;++i) {
	if (lcwithin (lc,(un)->GetCollideInfo())) {
	  this->Collide(un,firer,superunit);
	  if ((un!=targetToCollideWith)&&targetToCollideWith!=NULL) {
	    ListenToOwner(false);
	  }
	}
      }
    }
  }
#endif
  }

}


