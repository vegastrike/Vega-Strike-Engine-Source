#include <string>
#include "base.h"
#include "base_util.h"
namespace BaseUtil {
	inline Base::Room *CheckRoom (int room) {
		if (!Base::CurrentBase) return 0;
		if (room<0||room>=Base::CurrentBase->rooms.size()) return 0;
		return Base::CurrentBase->rooms[room];
	}
	int Room (std::string text) {
		if (!Base::CurrentBase) return -1;
		Base::CurrentBase->rooms.push_back(new Base::Room());
		return Base::CurrentBase->rooms.size()-1;
	}
	void Texture(int room, std::string index, std::string file, float x, float y) {
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->objs.push_back(new Base::Room::BaseSprite(file.c_str(),index));
#ifdef BASE_MAKER
		((Base::Room::BaseSprite*)newroom->objs.back())->texfile=file;
#endif
		((Base::Room::BaseSprite*)newroom->objs.back())->spr.SetPosition(x,y);
	}
	void Ship (int room, std::string index,QVector pos,Vector Q, Vector R) {
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		Vector P = R.Cross(Q);
		P.Normalize();
		newroom->objs.push_back(new Base::Room::BaseShip(P.i,P.j,P.k,Q.i,Q.j,Q.k,R.i,R.j,R.k,pos,index));
//		return Base::CurrentBase->rooms[Base::CurrentBase->curroom]->links.size()-1;
	}
	static void BaseLink (Base::Room *room,float x, float y, float wid, float hei, std::string text) {
		room->links.back()->x=x;
		room->links.back()->y=y;
		room->links.back()->wid=wid;
		room->links.back()->hei=hei;
		room->links.back()->text=text;
	}
	void Link (int room, std::string index, float x, float y, float wid, float hei, std::string text, int to) {
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->links.push_back(new Base::Room::Goto (index));
		BaseLink(newroom,x,y,wid,hei,text);
		((Base::Room::Goto*)newroom->links.back())->index=to;
	}
	void Launch (int room, std::string index, float x, float y, float wid, float hei, std::string text) {
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->links.push_back(new Base::Room::Launch (index));
		BaseLink(newroom,x,y,wid,hei,text);
	}
	void Comp(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes) { 
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		Base::Room::Comp *newcomp=new Base::Room::Comp (index);
		newroom->links.push_back(newcomp);
		BaseLink(newroom,x,y,wid,hei,text);
		static const EnumMap::Pair modelist [UpgradingInfo::MAXMODE+1] = {
			EnumMap::Pair ("NewsMode", UpgradingInfo::NEWSMODE), 
			EnumMap::Pair ("ShipMode", UpgradingInfo::SHIPDEALERMODE), 
			EnumMap::Pair ("UpgradeMode", UpgradingInfo::UPGRADEMODE), 
			EnumMap::Pair ("DowngradeMode", UpgradingInfo::DOWNGRADEMODE),
			EnumMap::Pair ("BriefingMode", UpgradingInfo::BRIEFINGMODE), 
			EnumMap::Pair ("MissionMode", UpgradingInfo::MISSIONMODE), 
			EnumMap::Pair ("SellMode", UpgradingInfo::SELLMODE), 
			EnumMap::Pair ("BuyMode", UpgradingInfo::BUYMODE),
			EnumMap::Pair (" SaveMode", UpgradingInfo::SAVEMODE), //for these two strings, I am not sure if we
			EnumMap::Pair (" AddMode", UpgradingInfo::ADDMODE), //should allow them (the ' ' at the beginning)...
			EnumMap::Pair ("UNKNOWN", UpgradingInfo::MAXMODE)
		};
		static const EnumMap modemap (modelist,UpgradingInfo::MAXMODE);
		const char *newmode=modes.c_str();
		int newlen=modes.size();
		char *curmode=new char [newlen+1];
		for (int i=0;i<newlen;) {
			int j;
			for (j=0;newmode[i]!=' '&&newmode[i]!='\0';i++,j++) {
				curmode[j]=newmode[i];
			}
			while(newmode[i]==' ')
				i++;
			if (j==0)
				continue;
			//in otherwords, if j is 0 then the 0th index will become null
			//EnumMap crashes if the string is empty.
			curmode[j]='\0';
			int modearg = modemap.lookup(curmode);
			if (modearg<UpgradingInfo::MAXMODE) {
				newcomp->modes.push_back((UpgradingInfo::BaseMode)(modearg));
			} else {
				fprintf(stderr,"WARNING: Computer mode %s not found in python script...\n",curmode);
			}
		}
		delete [] curmode;
	}
	void Python(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string pythonfile) {
		//instead of "Talk"/"Say" tags
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		newroom->links.push_back(new Base::Room::Python (pythonfile,index));
		BaseLink(newroom,x,y,wid,hei,text);
	}
	void Message(std::string text) {
		if (!Base::CurrentBase) return;
		Base::CurrentBase->rooms[Base::CurrentBase->curroom]->objs.push_back(new Base::Room::BaseTalk(text,"currentmsg"));
	}
	void EraseLink (int room, std::string index) {
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (int i=0;i<newroom->links.size();i++) {
			if (newroom->links[i]) {
				if (newroom->links[i]->index==index) {
					newroom->links.erase(newroom->links.begin()+i);
					break;
				}
			}
		}
	}
	void EraseObj (int room, std::string index) {
		Base::Room *newroom=CheckRoom(room);
		if (!newroom) return;
		for (int i=0;i<newroom->objs.size();i++) {
			if (newroom->objs[i]) {
				if (newroom->objs[i]->index==index) {
					newroom->objs.erase(newroom->objs.begin()+i);
					break;
				}
			}
		}
	}
	int GetCurRoom () {
		if (!Base::CurrentBase) return -1;
		return Base::CurrentBase->curroom;
	}
	int GetNumRoom () {
		if (!Base::CurrentBase) return -1;
		return Base::CurrentBase->rooms.size();
	}
}
