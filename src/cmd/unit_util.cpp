#include "planet.h"
#include <string>
#include "gfx/animation.h"
#include "unit_interface.h"
#include "config_xml.h"
#include "vs_globals.h"
using std::string;

namespace UnitUtil {
	string getFactionName (Unit *my_unit) {
		if (!my_unit)return "";
		return _Universe->GetFaction(my_unit->faction);
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
		my_unit->SetFaction(_Universe->GetFaction(factionname.c_str()));
    }
	string getName(Unit *my_unit){
		if (!my_unit)return "";
		return my_unit->name;
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
		if (!my_unit)return false;
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
	bool isSignificant(Unit *my_unit){
		if (!my_unit)return false;
		bool res=false;
		clsptr typ = my_unit->isUnit();
		string s=getFlightgroupName(my_unit);
		res=((typ==PLANETPTR&&!((Planet *)my_unit)->hasLights())||typ==ASTEROIDPTR||typ==NEBULAPTR||s=="Base");
		return res;
	}
	bool isSun(Unit *my_unit){
		if (!my_unit)return false;
		bool res=false;
		res=my_unit->isPlanet();
		if (res) {
			res = ((Planet *)my_unit)->hasLights();
		}
		return res;
	}
	void switchFg(Unit *my_unit,string arg){
		if (!my_unit)return;
		string type= my_unit->name;
		int nr_waves_left=0;
		int nr_ships=1;
		string order("default");
		Flightgroup *fg = my_unit->getFlightgroup();
		if (fg) {
			type = fg->type;
			nr_waves_left = fg->nr_waves_left;
			nr_ships =1;
			fg->Decrement(my_unit);
			order = fg->ainame;
		}
		fg = Flightgroup::newFlightgroup (arg,type,_Universe->GetFaction(my_unit->faction),order,nr_ships,nr_waves_left,"","",mission);
		my_unit->SetFg (fg,fg->nr_ships_left-1);
	}
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood){
		if (!my_unit)return 0;
		unsigned char sex=0;
		Cockpit * tmp;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			Order * ord=other_unit->getAIState();
			if (ord) {
				Animation * ani= ord->getCommFace (mood,sex);
				if (NULL!=ani) {
					tmp->SetCommAnimation (ani);
				}
			}
		}
		return sex;
	}
	bool commAnimation(Unit *my_unit,string anim){
		if (!my_unit)return false;
		Cockpit * tmp;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			Hashtable <std::string, Animation, char [63]> AniHashTable;
			Animation * ani= AniHashTable.Get(anim);
			if (NULL==ani) {
				ani = new Animation (anim.c_str());
				AniHashTable.Put(anim,ani);
			}
			tmp->SetCommAnimation (ani);
			return true;
		} else {
			return false;
		}
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
	float upgrade(Unit *my_unit, string file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts) {
		if (!my_unit)return 0;
		double percentage=0;
		printf ("upgrading %s %s %d %d %s\n",my_unit->name.c_str(),file.c_str(),mountoffset, subunitoffset,loop_through_mounts?"true":"false");
		fflush (stdout);
		percentage=my_unit->Upgrade (file,mountoffset,subunitoffset,force,loop_through_mounts);
		my_unit->SetTurretAI();
		return percentage;
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

	bool JumpTo (Unit * unit, string system) {
		if (unit!=NULL)
			return _Universe->activeStarSystem()->JumpTo(unit,NULL,system);
		else
			return false;
	}
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity){
		if (!my_unit)return false;
		if (my_unit->numCargo()>0) {
			unsigned int index;
			index = rand()%my_unit->numCargo();
			Cargo c(my_unit->GetCargo(index));	  
			c.quantity=quantity;
			if (my_unit->CanAddCargo(c)) {
				my_unit->AddCargo(c);
				my_unit->GetCargo(index).price*=percentagechange;
				return true;
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
	float getDistance(Unit *my_unit,Unit *un){
		return (my_unit->Position()-un->Position()).Magnitude()-my_unit->rSize()-un->rSize();
	}
	inline float getPlanetRadiusPercent () {
		static float planet_rad_percent =  XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_radius_percent",".75"));
		return planet_rad_percent;
	}
	float getSignificantDistance (Unit *un, Unit *sig) {
		float dist = getDistance (un,sig);
		float planetpct=getPlanetRadiusPercent();
		if (sig->isPlanet ())
			dist = dist - (sig->rSize()*planetpct);
		if (un->isPlanet ())
			dist = dist - (un->rSize()*planetpct);
		return dist;
	}
}

