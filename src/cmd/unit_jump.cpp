//#include "unit_template.h"
#include "audiolib.h"
#include "star_system_generic.h"
#include "cmd/images.h"
// From star_system_jump.cpp
extern Hashtable<std::string, StarSystem, 127> star_system_table;
extern std::vector <unorigdest *> pendingjump;

// From star_system_jump.cpp
inline bool CompareDest (Unit * un, StarSystem * origin) {
  for (unsigned int i=0;i<un->GetDestinations().size();i++) {
	  if (std::string(origin->getFileName())==std::string(un->GetDestinations()[i]))
		  return true;
  }
  return false;
}
inline std::vector <Unit *> ComparePrimaries (Unit * primary, StarSystem *origin) {
  std::vector <Unit *> myvec;
  if (CompareDest (primary, origin))
    myvec.push_back (primary);
  /*
  if (primary->isUnit()==PLANETPTR) {
    Iterator *iter = ((Planet *)primary)->createIterator();
    Unit * unit;
    while((unit = iter->current())!=NULL) {
      if (unit->isUnit()==PLANETPTR)
	if (CompareDest ((Planet*)unit,origin)) {
	  myvec.push_back (unit);
	}
      iter->advance();
    }
    delete iter;
  }
  */
  return myvec;
}
extern void DealPossibleJumpDamage (Unit *un);
extern void ActivateAnimation(Unit *);
template <class UnitType>
void GameUnit<UnitType>::TransferUnitToSystem (unsigned int kk, StarSystem * &savedStarSystem, bool dosightandsound) {
  if (pendingjump[kk]->orig==this->activeStarSystem||this->activeStarSystem==NULL) {
	  if (Unit::TransferUnitToSystem (pendingjump[kk]->dest)) {
#ifdef JUMP_DEBUG
      VSFileSystem::vs_fprintf (stderr,"Unit removed from star system\n");
#endif

      ///eradicating from system, leaving no trace
	  Unit::TransferUnitToSystem(pendingjump[kk]->dest);


      UnitCollection::UnitIterator iter = pendingjump[kk]->orig->getUnitList().createIterator();
      Unit * unit;
      while((unit = iter.current())!=NULL) {
	if (unit->Threat()==this) {
	  unit->Threaten (NULL,0);
	}
	if (unit->VelocityReference()==this) {
	  unit->VelocityReference(NULL);
	}
	if (unit->Target()==this) {
	  unit->Target (pendingjump[kk]->jumppoint.GetUnit());
	  unit->ActivateJumpDrive (0);
	}else {
	  Flightgroup * ff = unit->getFlightgroup();
	  if (ff) {
		  if (this==ff->leader.GetUnit()&&(ff->directive=="f"||ff->directive=="F")) {
			unit->Target (pendingjump[kk]->jumppoint.GetUnit());
			unit->getFlightgroup()->directive="F";
			unit->ActivateJumpDrive (0);
		  }
	  }
	}
	iter.advance();
      }
      Cockpit * an_active_cockpit = _Universe->isPlayerStarship(this);
      if (an_active_cockpit!=NULL) {
	an_active_cockpit->activeStarSystem=pendingjump[kk]->dest;
	visitSystem (an_active_cockpit,pendingjump[kk]->dest->getFileName());
//	vector<float> *v = &an_active_cockpit->savegame->getMissionData(string("visited_")+pendingjump[kk]->dest->getFileName());
//	if (v->empty())v->push_back (1.0);else (*v)[0]=1.0;
      }
      if (this==_Universe->AccessCockpit()->GetParent()) {
	VSFileSystem::vs_fprintf (stderr,"Unit is the active player character...changing scene graph\n");
	savedStarSystem->SwapOut();
	AUDStopAllSounds();
	savedStarSystem = pendingjump[kk]->dest;
	pendingjump[kk]->dest->SwapIn();

	
      }
      
      _Universe->setActiveStarSystem(pendingjump[kk]->dest);
      vector <Unit *> possibilities;
      iter = pendingjump[kk]->dest->getUnitList().createIterator();
      Unit * primary;
      while ((primary = iter.current())!=NULL) {
	vector <Unit *> tmp;
	tmp = ComparePrimaries (primary,pendingjump[kk]->orig);
	if (!tmp.empty()) {
	  possibilities.insert (possibilities.end(),tmp.begin(), tmp.end());
	}
	iter.advance();
      }
      if (!possibilities.empty()) {
		  static int jumpdest=235034;
		  Unit * jumpnode = possibilities[jumpdest%possibilities.size()];
		  this->SetCurPosition(jumpnode->Position());
		  ActivateAnimation(jumpnode);
		  if (jumpnode->isUnit()==UNITPTR) {
			  QVector Offset (jumpnode->Position().i<0?1:-1,
							  jumpnode->Position().j<0?1:-1,
							  jumpnode->Position().k<0?1:-1);
			  Offset*=jumpnode->rSize()*2+this->rSize()*2;
			  this->SetPosAndCumPos(jumpnode->Position()+Offset);
          }
		  Unit * tester;
		  for (unsigned int jjj=0;jjj<2;++jjj) {
		  for (un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
			   (tester=*i)!=NULL; ++i){

			  if (tester->isUnit()==UNITPTR && tester!=this){
			  if ((this->LocalPosition()-tester->LocalPosition()).Magnitude()<this->rSize()+tester->rSize()) {
				  SetCurPosition(this->LocalPosition()+this->cumulative_transformation_matrix.getR()*(4*(this->rSize()+tester->rSize())));
			  }
			  }
		  }
		  }
          jumpdest+=23231;
      }
      DealPossibleJumpDamage (this);
      static int jumparrive=AUDCreateSound(vs_config->getVariable ("unitaudio","jumparrive", "sfx43.wav"),false);
      if (dosightandsound)
	AUDPlay (jumparrive,this->LocalPosition(),this->GetVelocity(),1);
    } else {
#ifdef JUMP_DEBUG
      VSFileSystem::vs_fprintf (stderr,"Unit FAILED remove from star system\n");
#endif
    }
	  if (this->docked&UnitType::DOCKING_UNITS) {
      for (unsigned int i=0;i<this->image->dockedunits.size();i++) {
	Unit * unut;
	if (NULL!=(unut=this->image->dockedunits[i]->uc.GetUnit())) {
	  unut->TransferUnitToSystem (kk,savedStarSystem,dosightandsound);
	}
      }
    }
	if (this->docked&(UnitType::DOCKED|UnitType::DOCKED_INSIDE)) {
      Unit * un = this->image->DockedTo.GetUnit();
      if (!un) {
	this->docked &= (~(UnitType::DOCKED|UnitType::DOCKED_INSIDE));
      }else {
	Unit * targ=NULL;
	for (un_iter i=pendingjump[kk]->dest->getUnitList().createIterator();
	     (targ = (*i));
	     ++i) {
	  if (targ==un) {
	    break;
	  }
	}
	if (targ!=un)
	  this->UnDock (un);
      }
    }
  }else {
    VSFileSystem::vs_fprintf (stderr,"Already jumped\n");
  }
}
