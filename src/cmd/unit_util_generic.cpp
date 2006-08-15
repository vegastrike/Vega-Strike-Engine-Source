#include <string>
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "configxml.h"
#include "vs_globals.h"
#include "gfx/cockpit_generic.h"
#include "universe_util.h"
#include "cmd/ai/docking.h"
#include "savegame.h"
#include "cmd/planet_generic.h"
#include "faction_generic.h"
#include "cmd/ai/fire.h"
#include "gfx/cockpit_generic.h"
#include "role_bitmask.h"
#ifndef NO_GFX
#include "gfx/cockpit.h"
#endif
using std::string;
extern Unit * getTopLevelOwner();
static bool nameIsAsteroid(std::string name){
  if (name.length()<8) return false;
  return (name[0]=='A'||name[0]=='a')&&
    name[1]=='s'&&
    name[2]=='t'&&
    name[3]=='e'&&
    name[4]=='r'&&
    name[5]=='o'&&
    name[6]=='i'&&
    name[7]=='d'
    ;
}

namespace UnitUtil {
  bool isAsteroid(Unit * my_unit) {
		if (!my_unit)return false;
                return (my_unit->isUnit()==ASTEROIDPTR||nameIsAsteroid(my_unit->name));
  }
	bool isCapitalShip(Unit *my_unit){
		if (!my_unit)return false;
		bool res=false;
                static unsigned int capitaltypes=ROLES::getCapitalRoles();
                return ((1<<(unsigned int)my_unit->combatRole())&capitaltypes)!=0;
	}
	int getPhysicsPriority (Unit*  un) {
		static const bool FORCE_TOP_PRIORITY=XMLSupport::parse_bool(
			vs_config->getVariable("physics","priorities","force_top_priority","false") );
		if (FORCE_TOP_PRIORITY)
			return 1;

        //Some other comment mentions these need special treatment for subunit scheduling
		static const int PLAYER_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","player","1") );
		static const int MISSILE_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","missile","1") );

		float rad= un->rSize();
		clsptr untype=un->isUnit();
		float cpdist=FLT_MAX;
		float tooclose=0;
        unsigned int np = _Universe->numPlayers();
		Cockpit* cockpit=_Universe->AccessCockpit();
        for (unsigned int i=0;i<np;++i) {
			
			Unit * player=_Universe->AccessCockpit(i)->GetParent();
			if (player) {
				if (un==player->Target())
					return PLAYER_PRIORITY;
				float tmpdist = UnitUtil::getDistance(un,player);
				if (tmpdist<cpdist) {
					cockpit=_Universe->AccessCockpit(i);
					cpdist=tmpdist;
					tooclose = 
						2*(un->radial_size+player->radial_size)
						+ (player->Velocity - un->GetVelocity()).Magnitude();
				}
			}
#ifndef NO_GFX
			Camera * cam = _Universe->AccessCockpit(i)->AccessCamera();
				if (cam) {
					QVector campos = cam->GetPosition();
					double dist =(campos-un->Position()).Magnitude()-rad;
					if (dist<cpdist) {
						cpdist=dist;
						Unit * parent=_Universe->AccessCockpit(i)->GetParent();
						tooclose = 
							2*(un->radial_size+(parent?parent->radial_size:0)) 
							+ (cam->GetVelocity() - un->GetVelocity()).Magnitude();
					}
				}
#endif
            if (player==un||
               _Universe->AccessCockpit(i)->GetSaveParent()==un) {                    
               return PLAYER_PRIORITY;                    
            }
		}
		if (untype==MISSILEPTR)
			return MISSILE_PRIORITY;

		static const int ASTEROID_PARENT_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","asteroid_parent","1") );
        static const int ASTEROID_HIGH_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","asteroid_high","2") );
        static const int ASTEROID_LOW_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","asteroid.low","32") );
		
		static const int TOP_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","top","1") );
		static const int HIGH_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","high",SERVER?"1":"2") );
		static const int MEDIUM_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","medium","8") );
		static const int LOW_PRIORITY=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","low","32") );
        
		static const int NOT_VISIBLE_COMBAT_HIGH=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","notvisible_combat_high","10") );
		static const int NOT_VISIBLE_COMBAT_MEDIUM=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","notvisible_combat_medium","20") );
		static const int NOT_VISIBLE_COMBAT_LOW=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","notvisible_combat_low","40") );

		static const int NO_ENEMIES=XMLSupport::parse_int(
			vs_config->getVariable("physics","priorities","no_enemies","64") );

		// Here we assume that SIM_QUEUE_SIZE is >=64
        const int LOWEST_PRIORITY=SIM_QUEUE_SIZE;
		
		

		Unit * parent=cockpit->GetParent();


		float gun_range=0;
		float missile_range=0;
		float dist=cpdist;
		if (parent) {
			float speed=0;
			parent->getAverageGunSpeed(speed,gun_range,missile_range);
		}
		static int cargofac=FactionUtil::GetFaction("cargo");
		static int upfac=FactionUtil::GetFaction("upgrades");
		static int neutral=FactionUtil::GetFaction("neutral");

        if (un->schedule_priority != Unit::scheduleDefault) {
            //Asteroids do scheduling themselves within subunits, so...
            //...only one caveat: units with their own internal scheduling
            //must have constant priority... otherwise, big mess when changing
            //priorities. I'll think of one way to overcome this...
            switch (un->schedule_priority) {
            case Unit::scheduleAField:
                return ASTEROID_PARENT_PRIORITY;
            case Unit::scheduleRoid:
                if (dist<tooclose)
                    return ASTEROID_HIGH_PRIORITY; else
                    return ASTEROID_LOW_PRIORITY;
            }
        }

		if (un->owner==getTopLevelOwner()||un->faction==cargofac||un->faction==upfac||un->faction==neutral) {
            if (dist<tooclose)
                return LOW_PRIORITY; else
                return LOWEST_PRIORITY;
		}
		Unit * targ = un->Target();
		if (_Universe->isPlayerStarship(targ)) {
			return HIGH_PRIORITY;
		}
		string obj = UnitUtil::getFgDirective(un);
		if (!(obj.length()==0||(obj.length()>=1&&obj[0]=='b'))) {
			return MEDIUM_PRIORITY;
		}
		if (dist<gun_range)
			return MEDIUM_PRIORITY;
		if (dist<missile_range)
			return LOW_PRIORITY;
		if (targ){
			float speed;
			un->getAverageGunSpeed(speed,gun_range,missile_range);
			double distance=UnitUtil::getDistance(un,targ);
			if (distance<=gun_range)
				return NOT_VISIBLE_COMBAT_HIGH;
			if (distance<missile_range)
				return NOT_VISIBLE_COMBAT_MEDIUM;
			return NOT_VISIBLE_COMBAT_LOW;
		}
		return NO_ENEMIES;
	}

	void orbit (Unit * my_unit, Unit * orbitee, float speed, QVector R, QVector S, QVector center) {
		if (my_unit) {
			my_unit->PrimeOrders (new PlanetaryOrbit (my_unit,speed/(3.1415926536*(S.Magnitude()+R.Magnitude())),0,R,S,center,orbitee));
			if (orbitee){
				if (orbitee->isUnit()==PLANETPTR) {
					((Planet *)orbitee)->AddSatellite (my_unit);
				}
			}
			if (my_unit->faction!=FactionUtil::GetFactionIndex ("neutral")) {
				Order * tmp = new Orders::FireAt (0.2,15.0);
				my_unit->EnqueueAI (tmp);
				my_unit->SetTurretAI();
			}
			my_unit->SetOwner(getTopLevelOwner());
		}
	}
	string getFactionName (Unit *my_unit) {
		if (!my_unit)return "";
		return FactionUtil::GetFaction(my_unit->faction);
    }
	int getFactionIndex (Unit *my_unit) {
		if (!my_unit)return 0;
		return my_unit->faction;
    }
	void setFactionIndex (Unit *my_unit,int factionname) {
		if (!my_unit)return;
		my_unit->SetFaction(factionname);
    }
	void setFactionName (Unit *my_unit,string factionname) {
		if (!my_unit)return;
		my_unit->SetFaction(FactionUtil::GetFactionIndex(factionname));
    }
	string getName(Unit *my_unit){
		if (!my_unit)return "";
		return my_unit->name;
	}
	void setName(Unit *my_unit,string name){
		if (!my_unit)return;
		my_unit->name=name;
	}
	void SetHull(Unit *my_unit,float newhull){
		if (!my_unit)return;
		my_unit->hull=newhull;
	}
	float getCredits(Unit *my_unit) {
		if (!my_unit)return 0;
		Cockpit * tmp;
		float viret=0;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			viret=tmp->credits;
		}
		return viret;
	}
	void addCredits(Unit *my_unit,float credits) {
		if (!my_unit)return;
		Cockpit * tmp;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			tmp->credits+=credits;
		}
	}
    string getFlightgroupName(Unit *my_unit){
		if (!my_unit)return "";
		string fgname;
		Flightgroup *fg=my_unit->getFlightgroup();
		if(fg){
			fgname=fg->name;
		}
		return fgname;
	}
	Unit *getFlightgroupLeader (Unit *my_unit) {
		if (!my_unit)return 0;
		class Flightgroup * fg=my_unit->getFlightgroup();
		Unit *ret_unit = fg?fg->leader.GetUnit():my_unit;
		if (!ret_unit)
			ret_unit=0;
		return ret_unit;
	}
	bool setFlightgroupLeader (Unit *my_unit, Unit *un) {
		if (!my_unit||!un)return false;		 
		if (my_unit->getFlightgroup()) {
			my_unit->getFlightgroup()->leader.SetUnit(un);
			return true;
		} else {
			return false;
		}
	}
	string getFgDirective(Unit *my_unit){
		if (!my_unit)return "";
		string fgdir ("b");
		if (my_unit->getFlightgroup())
			fgdir = my_unit->getFlightgroup()->directive;
		return fgdir;
	}
	bool setFgDirective(Unit *my_unit,string inp){
		if (!my_unit)return false;
		if (my_unit->getFlightgroup()!=NULL) {
			my_unit->getFlightgroup()->directive = inp;
			return true;
		}
		return false;
	}
	int getFgSubnumber(Unit *my_unit){
		if (!my_unit)return -1;
		return my_unit->getFgSubnumber();
	}
	int removeCargo(Unit *my_unit,string s, int quantity, bool erasezero){
		if (!my_unit)return 0;
		unsigned int index;
		if (my_unit->GetCargo(s,index)) {
			quantity = my_unit->RemoveCargo (index,quantity,erasezero);
		}else {
			quantity=0;
		}
		return quantity;
	}
        bool repair(Unit *my_unit) {
            if (!my_unit) {
                return false;
            }
            return my_unit->RepairUpgrade();
        }
	float upgrade(Unit *my_unit, string file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts) {
		if (!my_unit)return 0;
		double percentage=0;
		printf ("upgrading %s %s %d %d %s\n",my_unit->name.c_str(),file.c_str(),mountoffset, subunitoffset,loop_through_mounts?"true":"false");
		fflush (stdout);
		percentage=my_unit->Upgrade (file,mountoffset,subunitoffset,force,loop_through_mounts);
		my_unit->SetTurretAI();
		return percentage;
	}
	int removeWeapon(Unit * my_unit, string weapon_name, int mountoffset, bool loop) {
          if (!my_unit)return -1;
          int maxmount=my_unit->mounts.size();
          int max=maxmount+mountoffset;
          for (int i=mountoffset;i<max;++i) {
            int index=i%maxmount;
            if (my_unit->mounts[i].type->weapon_name==weapon_name&&(my_unit->mounts[i].status==Mount::ACTIVE||my_unit->mounts[i].status==Mount::INACTIVE)) {
              my_unit->mounts[i].status=Mount::UNCHOSEN;
              return i;
            }
          }
          return -1;
	}
	int addCargo (Unit *my_unit,Cargo carg) {
	  if (!my_unit)return 0;
	  int i;
	  for (i=carg.quantity;i>0&&!my_unit->CanAddCargo(carg);i--) {
	    carg.quantity=i;
	  }
	  if (i>0) {
	    carg.quantity=i;
	    my_unit->AddCargo(carg);
	  }else {
	    carg.quantity=0;
	  }
	  return carg.quantity; 
	}
	int forceAddCargo (Unit *my_unit,Cargo carg) {
	  if (!my_unit)return 0;
          my_unit->AddCargo(carg);
	  return carg.quantity; 
	}
	int hasCargo (Unit * my_unit, string mycarg) {
	  if (!my_unit) return 0;
		unsigned int i;
		Cargo * c = my_unit->GetCargo (mycarg,i);
		if (c==NULL)
			return 0;
		return c->quantity;
	}
	bool JumpTo (Unit * unit, string system) {
		if (unit!=NULL)
			return unit->getStarSystem()->JumpTo(unit,NULL,system);
		else
			return false;
	}
        string getUnitSystemFile (Unit * un) {
	  if (!un) {
	    return _Universe->activeStarSystem()->getFileName();
	  }
	  StarSystem * ss = un->getStarSystem();
	  return ss->getFileName();
        }
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity){
		if (!my_unit)return false;
		if (my_unit->numCargo()>0) {
			unsigned int index;
			index = rand()%my_unit->numCargo();
			Cargo c(my_unit->GetCargo(index));	  
			c.quantity=quantity;
                        if (c.price!=0) {
                          if (my_unit->CanAddCargo(c)) {
                            my_unit->AddCargo(c);
                            my_unit->GetCargo(index).price*=percentagechange;
                            return true;
                          }
                        }
		}
		return false;
	}
	bool decrementCargo(Unit *my_unit,float percentagechange){
		if (!my_unit)return false;
		if (my_unit->numCargo()>0) {
			unsigned int index;
			index = rand()%my_unit->numCargo();
			if (my_unit->RemoveCargo(index,1,false)) {
				my_unit->GetCargo(index).price*=percentagechange;
			}
			return true;
		}
		return false;
	}


  
	Cargo GetCargoIndex (Unit *my_unit, int index) {
	  if (my_unit) {
	    if (index>=0&&(unsigned int)index<my_unit->numCargo()) {
	      return my_unit->GetCargo(index);
	    }
	  }
	  Cargo ret;
	  ret.quantity=0;
	  return ret;
	}

	Cargo GetCargo (Unit *my_unit, std::string cargname) {
		if (my_unit) {
			unsigned int indx=0;
			Cargo *cargptr=my_unit->GetCargo(cargname,indx);
			if (cargptr&&indx>=0) {
				return *cargptr;
			}
		}
		Cargo ret;
		ret.quantity=0;
		return ret;
	}
	bool isDockableUnit(Unit *my_unit) {
		if (!my_unit) return false;
		std::string unit_fgid = getFlightgroupName(my_unit);
		return ((((my_unit->isPlanet ())&&(!isSun(my_unit))&&isSignificant(my_unit)&&(!my_unit->isJumppoint()))||unit_fgid=="Base"||my_unit->isUnit()==UNITPTR)&&my_unit->DockingPortLocations().size()>0);
	}
	float getDistance(Unit *my_unit,Unit *un){
	  if (my_unit==NULL||un==NULL)
	    return FLT_MAX;
	  return (my_unit->Position()-un->Position()).Magnitude()-my_unit->rSize()-un->rSize();
	}
	float getSignificantDistance (Unit *un, Unit *sig) {
	  if (un==NULL||sig==NULL)
	    return FLT_MAX;
	  
		float dist = getDistance (un,sig);
		
		float planetpct=UniverseUtil::getPlanetRadiusPercent();
		if (sig->isPlanet ())
			dist = dist - (sig->rSize()*planetpct);
		if (un->isPlanet ())
			dist = dist - (un->rSize()*planetpct);
		return dist;
	}
        int isPlayerStarship (Unit * un) {
                Cockpit * cp = _Universe->isPlayerStarship (un);
                if (cp==NULL) {
                        return -1;
                }
                return (cp - _Universe->AccessCockpit(0));
        }
  void setSpeed (Unit * my_unit, float speed) {
    if (my_unit) {
      my_unit->GetComputerData().set_speed = speed;
    }
  }
	float maxSpeed (Unit * my_unit) {
		if (!my_unit) {
			return 0;
		}
		return my_unit->GetComputerData().max_speed();
	}
	float maxAfterburnerSpeed (Unit * my_unit) {
		if (!my_unit) {
			return 0;
		}
		return my_unit->GetComputerData().max_ab_speed();
	}
	void setECM (Unit * my_unit, int NewECM) { //short fix
    if (!my_unit)
      return;
    my_unit->GetImageInformation().ecm = NewECM;
  }
	int getECM (Unit * my_unit) { //short fix
    if (!my_unit)
      return 0;
    return my_unit->GetImageInformation().ecm;
  }
	static bool ishere (Unit *par,Unit *look) {
		for (un_iter uniter=par->getSubUnits();uniter.current();uniter.advance()) {
			if (uniter.current()==look) {
				return true;
			}
			if (uniter.current()!=par&&ishere(uniter.current(),look)) {
				return true;
			}
		}
		return false;
	}
	Unit *owner (Unit *un) {
		Unit *found=NULL;
		for (UniverseUtil::PythonUnitIter uniter=UniverseUtil::getUnitList();uniter.current();uniter.advance()) {
			if (uniter.current()==un||ishere(uniter.current(),un)) {
				found=uniter.current();
				break;
			}
		}
		return found;
	}
        void performDockingOperations (Unit * un, Unit * unitToDockWith,int actually_dock) {
	  if (un && unitToDockWith) {
	    Order * ai = un->aistate;
	    un->aistate = NULL;
	    un->PrimeOrders (new Orders::DockingOps (unitToDockWith, ai,actually_dock!=0));
	  }
	}
}
