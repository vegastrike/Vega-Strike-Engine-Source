#include <string>
#include "unit_generic.h"
#include "configxml.h"
#include "unit_util.h"
#include "vs_globals.h"
using std::string;

namespace UnitUtil {
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
		fg = Flightgroup::newFlightgroup (arg,type,FactionUtil::GetFaction(my_unit->faction),order,nr_ships,nr_waves_left,"","",mission);
		my_unit->SetFg (fg,fg->nr_ships_left-1);
	}
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood){return 0;}
	bool commAnimation(Unit *my_unit,string anim){ return false;}
}
