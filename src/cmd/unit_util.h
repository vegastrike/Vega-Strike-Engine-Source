#include "unit.h"
#include <string>
using std::string;

namespace UnitUtil {
	string getFactionName (Unit *my_unit) {
		return _Universe->GetFaction(my_unit->faction);
    }
	string getName(Unit *my_unit){
		return my_unit->name;
	}
	float getCredits(Unit *my_unit) {
		Cockpit * tmp;
		float viret=0;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			viret=tmp->credits;
		}
		return viret;
	}
	void addCredits(float credits) {
		Cockpit * tmp;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			tmp->credits+=credits;
		}
	}
    string getFlightgroupName(){
		string fgname;
		Flightgroup *fg=my_unit->getFlightgroup();
		if(fg){
			fgname=fg->name;
		}
		return fgname;
	}
	Unit *getFlightgroupLeader (Unit *my_unit) {
		class Flightgroup * fg=my_unit->getFlightgroup()
		Unit *ret_unit = fg?fg->getFlightgroup()->leader.GetUnit():my_unit;
		if (!ret_unit)
			ret_unit=0;
		return ret_unit;
	}
	bool setFlightgroupLeader (Unit *my_unit, Unit *oth_unit) {
		if (my_unit->getFlightgroup()) {
			my_unit->getFlightgroup()->leader.SetUnit(un);
			return true;
		} else {
			return false;
		}
	}
	string getFgDirective(Unit *my_unit){
		string fgdir ("b");
		if (my_unit->getFlightGroup)
			fgdir = my_unit->getFlightgroup()->directive;
		return fgdir;
	}
	bool setFgDirective(Unit *my_unit,string inp){
		string inp = getStringArgument (node,mode,1);
		if (my_unit->getFlightgroup()!=NULL) {
			my_unit->getFlightgroup()->directive = inp;
			return true;
		}
		return false;
	}
	int getFgSubnumber(Unit *my_unit){
		return my_unit->getFgSubnumber();
	}
	bool isSignificant(Unit *my_unit){
		bool res=false;
		clsptr typ = my_unit->isUnit();
		string s=getFlightgroupName(my_unit);
		res=((typ==PLANETPTR&&!((Planet *)my_unit)->hasLights())||typ==ASTEROIDPTR||typ==NEBULAPTR||s=="Base");
		return res;
	}
	bool isSun(Unit *my_unit){
		bool res=false;
		res=my_unit->isPlanet();
		if (res) {
			res = ((Planet *)my_unit)->hasLights();
		}
		return res;
	}
	void switchFg(Unit *my_unit,string arg){
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
		fg = Flightgroup::newFlightgroup (arg,type,_Universe->GetFaction(my_unit->faction),order,nr_ships,nr_waves_left,"","",this);
		my_unit->SetFg (fg,fg->nr_ships_left-1);
	}
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood){
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
		string anim =getStringArgument (node,mode,1);
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
	Unit* launch (string name_string,string faction_string,string shiptype, string unittype, string ai_string,int nr_of_ships,int nr_of_waves, QVector pos, string squadlogo){
		launchJumppoint(name_string,faction_string,type_string,type_string,ai_string,nr_of_ships,nr_of_waves,pos,sqadlogo,"");
	}
	Unit *launchJumppoint(string name_string,
			string faction_string,
			string type_string,
			string unittype_string;
			string ai_string,
			int nr_of_ships,
			int nr_of_waves, 
			QVector pos, 
			string squadlogo, 
			string destinations){
		int clstype=Unit::UNITPTR;
		if (unittype_string=="planet") {
			classtype =Unit::PLANETPTR;			
		}else if (unittype_string=="asteroid") {
			classtype = Unit::ASTEROIDPTR;
		}else if (unittype_string=="nebula") {
			classtype = Unit::NEBULAPTR;
		}
		CreateFlightgroup cf;
		cf.fg = Flightgroup::newFlightgroup (name_string,type_string,faction_string,ai_string,nr_of_ships,nr_of_waves,logo_tex,"",mission);
		cf.unittype=CreateFlightgroup::UNIT;
		cf.terrain_nr=-1;
		cf.waves=nr_of_waves;
		cf.nr_ships=nr_of_ships;
		cf.fg->pos=pos;
		for(int i=0;i<3;i++){
			cf.rot[i]=0.0;
		}
		Unit *tmp= mission->call_unit_launch(&cf,clstyp,destinations);
		mission->number_of_ships+=nr_of_ships;
		return tmp;
	}
	int removeCargo(Unit *my_unit,string s, int quantity, bool erasezero){
		int numret=0;
		unsigned int index;
		if (my_unit->GetCargo(s,index)) {
			quantity = my_unit->RemoveCargo (index,quantity,erasezero);
		}else {
			quantity=0;
		}
		return quantity;
	}
	float upgrade(Unit *my_unit, string file,int mountoffset,int subunitoffset, bool force,bool loop_through_mounts) {
		double percentage=0;
		printf ("upgrading %s %s %d %d %s\n",my_unit->name.c_str(),file.c_str(),mountoffset, subunitoffset,loop_through_mounts?"true":"false");
		fflush (stdout);
		percentage=my_unit->Upgrade (file,mountoffset,subunitoffset,force,loop_through_mounts);
		my_unit->SetTurretAI();
		return percentage;
	}
	bool incrementCargo(Unit *my_unit,float percentagechange,int quantity){
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
}