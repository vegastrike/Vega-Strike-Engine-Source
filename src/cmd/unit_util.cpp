#include <string>
#include "unit_generic.h"
#include "gfx/cockpit.h"
#include "planet_generic.h"
#include "gfx/animation.h"
#include "config_xml.h"
#include "unit_util.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "pilot.h"
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
	int communicateTo(Unit *my_unit,Unit *other_unit,float mood){
		if (!my_unit)return 0;
		unsigned char sex=0;
		Cockpit * tmp;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
                  if (other_unit) {
                    Animation * ani= other_unit->pilot->getCommFace (other_unit,mood,sex);
                    if (NULL!=ani) {
                      tmp->SetCommAnimation (ani,other_unit);
                    }
                  }
		}
		return sex;
	}
	bool commAnimation(Unit *my_unit,string anim){
		if (!my_unit)return false;
		Cockpit * tmp;
		if ((tmp=_Universe->isPlayerStarship (my_unit))) {
			static Hashtable <std::string, Animation, 63> AniHashTable;
			Animation * vid= AniHashTable.Get(anim);
			if (NULL==vid) {
				vid = new Animation (anim.c_str());
				AniHashTable.Put(anim,vid);
			}
			tmp->SetCommAnimation (vid,NULL);
			return true;
		} else {
			return false;
		}
	}
}

