#ifndef PYTHON_STUB
#include "python_class.h"
#include "cmd/briefing.h"
#include "cmd/script/mission.h"

namespace BriefingUtil {
	int addShip(string name, string faction, Vector vec){
		return mission->briefing->AddStarship(name.c_str(),
			_Universe->GetFaction(faction.c_str()),
			vec);
	}
	void removeShip(int whichship) {
		mission->briefing->RemoveStarship(whichship);
	}
	void enqueueOrder(int whichship,Vector vec,float time){
		mission->briefing->EnqueueOrder(whichship,vec,time);
	}
	void replaceOrder(int whichship,Vector vec,float time){
		mission->briefing->OverrideOrder(whichship,vec,time);
	}
	Vector getShipPosition(int whichship) {
		return mission->briefing->GetPosition(whichship);
	}
	void setShipPosition(int whichship,Vector vec) {
		mission->briefing->SetPosition(whichship,vec);
	}
	void terminate() {
		mission->BriefingEnd();
	}
	void setCamPosition(QVector p) {
		mission->briefing->cam.SetPosition(p);
	}
	void setCamOrientation(Vector p, Vector q, Vector r) {
		mission->briefing->cam.SetOrientation(p,q,r);
	}
	void setCloak(int whichship,float cloak) {
		mission->briefing->SetCloak(whichship,cloak);
	}
}
PYTHON_BEGIN_MODULE(Briefing)
Briefing.def(&BriefingUtil::addShip,"addShip");
Briefing.def(&BriefingUtil::removeShip,"removeShip");
Briefing.def(&BriefingUtil::enqueueOrder,"enqueueOrder");
Briefing.def(&BriefingUtil::replaceOrder,"replaceOrder");
Briefing.def(&BriefingUtil::getShipPosition,"getShipPosition");
Briefing.def(&BriefingUtil::setShipPosition,"setShipPosition");
Briefing.def(&BriefingUtil::terminate,"terminate");
Briefing.def(&BriefingUtil::setCamPosition,"setCamPosition");
Briefing.def(&BriefingUtil::setCamOrientation,"setCamOrientation");
Briefing.def(&BriefingUtil::setCloak,"setCloak");
PYTHON_END_MODULE(Briefing)

void InitBriefing() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Briefing);
	Python::reseterrors();
}









///////////////////////////////////////////////
// END C++
///////////////////////////////////////////////
//below replace ~ with enter
#else
#define MYPRINT(nam) print #nam
#define WRAPPED0(type,name,aff) def name(self):~    MYPRINT(name) ~    return aff
#define WRAPPED1(type,name,atype,a,aff) def name(self,a): ~    MYPRINT(name)  ~    return aff
#define WRAPPED2(type,name,atype,a,btype,b,aff) def name(self,a,b): ~    MYPRINT(name) ~    return aff
#define WRAPPED3(type,name,atype,a,btype,b,ctype,c,aff) def name(self,a,b,c): ~    MYPRINT(name) ~    return aff
#define voidWRAPPED0(name) def name(self): ~    MYPRINT(name)
#define voidWRAPPED1(name,atype,a) def name(self,a): ~    MYPRINT(name)
#define voidWRAPPED2(name,atype,a,btype,b) def name(self,a,b): ~    MYPRINT(name)
#define voidWRAPPED3(name,atype,a,btype,b,ctype,c) def name(self,a,b,c): ~    MYPRINT(name)

WRAPPED3(int,addShip,string,name,string,faction,Vector,vec,0)
voidWRAPPED1(removeShip,int,ship)
voidWRAPPED3(enqueueOrder,int,ship,Vector,dest,float,time)
voidWRAPPED3(replaceOrder,int,ship,Vector,dest,float,time)
WRAPPED1(Vector,getShipPosition,int,ship,(50,75,100))
voidWRAPPED2(setShipPosition,int,ship,Vector,vec)
voidWRAPPED0(terminate)
voidWRAPPED1(setCamPosition,Vector,pos)
voidWRAPPED3(setCamOrientation,Vector,p,Vector,q,Vector,r)
voidWRAPPED2(setCloak,int,ship,float,amount)

#endif
