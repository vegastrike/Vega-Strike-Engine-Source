
#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet.h"
using Orders::FireAt;

FireAt::FireAt (float reaction_time, float aggressivitylevel): Order (WEAPON|TARGET),  rxntime (reaction_time), delay(0), agg (aggressivitylevel), distance(1){
  gunspeed=float(.0001);
  gunrange=float(.0001);
  missileprobability=.001;
  
}
//temporary way of choosing
void FireAt::ChooseTargets (int ) {
  UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un ;
  float relation=1;
  float range=0;
  float worstrelation=0;
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    relation = _Universe->GetRelation (parent->faction, un->faction);
    Vector t;
    bool tmp = parent->InRange (un,t);
    if (tmp&&((relation<worstrelation||(relation==worstrelation&&t.Dot(t)<range)))) {
      worstrelation = relation;
      range = t.Dot(t);
      parent->Target (un);
    }
    iter->advance();
  }
#ifdef ORDERDEBUG
  fprintf (stderr,"it%x");
  fflush (stderr);
#endif
  delete iter;
#ifdef ORDERDEBUG
  fprintf (stderr,"it");
  fflush (stderr);
#endif

  if (worstrelation>0) {
    parent->Target (NULL);
  }
}
/* Proper choosing of targets
void FireAt::ChooseTargets (int num) {
  UnitCollection tmp;
  UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un ;
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    iter->advance();
  }
  delete iter;
  AttachOrder (&tmp);
}

*/
bool FireAt::ShouldFire(Unit * targ) {
  float dist;
  float angle = parent->cosAngleTo (targ, dist,parent->GetComputerData().itts?gunspeed:FLT_MAX,gunrange);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<agg&&angle>1/agg);
}

FireAt::~FireAt() {
#ifdef ORDERDEBUG
  fprintf (stderr,"fire%x\n",this);
  fflush (stderr);
#endif

}
void FireAt::Execute () {
  bool tmp = done;
#ifdef AGGDEBUG
  fprintf (stderr,"ord");
  fflush (stderr);
#endif
  Order::Execute();	
#ifdef AGGDEBUG
  fprintf (stderr,"eord ");
  fflush (stderr);
#endif
  if (gunspeed==float(.0001)) {
#ifdef AGGDEBUG
  fprintf (stderr,"gr ");
  fflush (stderr);
#endif
    parent->getAverageGunSpeed (gunspeed,gunrange);  
#ifdef AGGDEBUG
  fprintf (stderr,"egr ");
  fflush (stderr);
#endif

  }
  done = tmp;
  Unit * targ;
  bool shouldfire=false;
  if (targets) {
#ifdef AGGDEBUG
  fprintf (stderr,"trg ");
  fflush (stderr);
#endif
    UnitCollection::UnitIterator *iter = targets->createIterator();    
  
    if (!iter->current()) {
      ChooseTargets(2);
#ifdef AGGDEBUG
  fprintf (stderr,"i1%x",iter);
  fflush (stderr);
#endif
      delete iter;
#ifdef AGGDEBUG
  fprintf (stderr,"i1\n");
  fflush (stderr);
#endif

      iter = targets->createIterator();
    }
#ifdef AGGDEBUG
  fprintf (stderr,"shudfir ");
  fflush (stderr);
#endif
    while ((targ = iter->current())!=NULL) {
      shouldfire|=ShouldFire(targ);
      iter->advance();
    }
#ifdef AGGDEBUG
  fprintf (stderr,"i2%x",iter);
  fflush (stderr);
#endif
    delete iter;  
#ifdef AGGDEBUG
  fprintf (stderr,"i2");
  fflush (stderr);
#endif

  } else {

  }
  if ((targ = parent->Target())) {
#ifdef AGGDEBUG
  fprintf (stderr,"part ");
  fflush (stderr);
#endif
    bool istargetjumpableplanet = targ->isUnit()==PLANETPTR;
    if (istargetjumpableplanet) {
#ifdef AGGDEBUG
  fprintf (stderr,"jumpme ");
  fflush (stderr);
#endif
      istargetjumpableplanet=(!((Planet*)targ)->GetDestinations().empty())&&(parent->GetJumpStatus().drive>=0);
#ifdef AGGDEBUG
  fprintf (stderr,"noj ");
  fflush (stderr);
#endif
      if (!istargetjumpableplanet) {
#ifdef AGGDEBUG
  fprintf (stderr,"chosy ");
  fflush (stderr);
#endif
	ChooseTargets(1);
      }
    }
    
    if (targ->CloakVisible()>.8) {
#ifdef AGGDEBUG
  fprintf (stderr,"cv ");
  fflush (stderr);
#endif
      if (!istargetjumpableplanet)
#ifdef AGGDEBUG
  fprintf (stderr,"f");
  fflush (stderr);
#endif
	shouldfire |= ShouldFire (targ);
#ifdef AGGDEBUG
  fprintf (stderr,"ef ");
  fflush (stderr);
#endif

      if (targ->GetHull()<0) {
#ifdef AGGDEBUG
  fprintf (stderr,"huls ");
  fflush (stderr);
#endif
	ChooseTargets(1);
#ifdef AGGDEBUG
  fprintf (stderr,"ehuls ");
  fflush (stderr);
#endif
      }
    }else {
#ifdef AGGDEBUG
  fprintf (stderr,"cta ");
  fflush (stderr);
#endif
      ChooseTargets(1);
#ifdef AGGDEBUG
  fprintf (stderr,"ecta ");
  fflush (stderr);
#endif
    }
    if ((!istargetjumpableplanet)&&(float(rand())/RAND_MAX)<.5*missileprobability*SIMULATION_ATOM) {
#ifdef AGGDEBUG
  fprintf (stderr,"ctabl ");
  fflush (stderr);
#endif
      ChooseTargets(1);
#ifdef AGGDEBUG
  fprintf (stderr,"ectcpbla ");
  fflush (stderr);
#endif

    }
#ifdef AGGDEBUG
  fprintf (stderr,"endpart ");
  fflush (stderr);
#endif
  } else {
#ifdef AGGDEBUG
  fprintf (stderr,"juschoo ");
  fflush (stderr);
#endif
    ChooseTargets(1);
#ifdef AGGDEBUG
  fprintf (stderr,"ejuschoo ");
  fflush (stderr);
#endif

  }
  if (shouldfire) {
#ifdef AGGDEBUG
  fprintf (stderr,"blo ");
  fflush (stderr);
#endif
    if ((float(rand())/RAND_MAX)<missileprobability*SIMULATION_ATOM) {
#ifdef AGGDEBUG
  fprintf (stderr,"bb ");
  fflush (stderr);
#endif
      parent->Fire(true);
      parent->ToggleWeapon(true);//change missiles to only fire 1
#ifdef AGGDEBUG
  fprintf (stderr,"ebb ");
  fflush (stderr);
#endif

    }

    if (delay>rxntime) {
#ifdef AGGDEBUG
  fprintf (stderr,"rxn ");
  fflush (stderr);
#endif
      parent->Fire(false);
#ifdef AGGDEBUG
  fprintf (stderr,"erxn ");
  fflush (stderr);
#endif


    } else {
      delay +=SIMULATION_ATOM;
    }
  } else {
    delay =0;
#ifdef AGGDEBUG
  fprintf (stderr,"unfir ");
  fflush (stderr);
#endif
    parent->UnFire();
#ifdef AGGDEBUG
  fprintf (stderr,"eunfir ");
  fflush (stderr);
#endif

  }
#ifdef AGGDEBUG
  fprintf (stderr,"firsuc ");
  fflush (stderr);
#endif

  
}

