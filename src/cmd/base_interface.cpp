#include "base.h"
#include "gldrv/winsys.h"

Base::Room::~Room () {
	int i;
	for (i=0;i<links.size();i++) {
		delete links[i];
	}
	for (i=0;i<texes.size();i++) {
		delete texes[i];
	}
}

Base::Room::Room () {
	//Do Nothing!
}

void Base::Room::Draw () {
//	for (int i=0;i<texes.size();i++) {
		texes[0]->Draw();
//	}
}

int Base::Room::MouseOver (float x, float y) {
	for (int i=0;i<links.size();i++) {
		if (x>=links[i]->x&&
				x<=(links[i]->x+links[i]->wid)&&
				y>=links[i]->y&&
				y<=(links[i]->y+links[i]->hei)) {
			return i;
		}
	}
	return -1;
}

Base *Base::CurrentBase=0;
bool Base::CallComp=false;

bool RefreshGUI(void) {
	bool retval=false;
	if (_Universe->AccessCockpit()) {
		if (Base::CurrentBase) {
			if (_Universe->AccessCockpit()->GetParent()==Base::CurrentBase->caller){
				if (Base::CallComp) {
					return RefreshInterface ();	
				} else {
					Base::CurrentBase->Draw();
				}
				retval=true;
			}
		}
	}
	return retval;
}

void Base::Room::Click (::Base* base,float x, float y, int button, int state) {
	if (button==WS_LEFT_BUTTON) {
		int linknum=MouseOver (x,y);
		if (linknum>=0) {
			Link * link=links[linknum];
			if (link) {
				link->Click(base,x,y,button,state);
			}
		}
	} else {
		if (state==WS_MOUSE_UP) {
			Link *curlink=links[base->curlinkindex%links.size()];
			base->curlinkindex++;
			int x=(((curlink->x+(curlink->wid/2))+1)/2)*g_game.x_resolution;
			int y=-(((curlink->y+(curlink->hei/2))-1)/2)*g_game.y_resolution;
			winsys_warp_pointer(x,y);
			MouseOverWin(x,y);
		}
	}
}

void Base::MouseOver (float x, float y) {
	int i=rooms[curroom]->MouseOver(x,y);
	Room::Link *link=0;
	if (i<0) {
		link=0;
	} else {
		link=rooms[curroom]->links[i];
	}
	if (link) {
		curtext.SetText(link->text);
		drawlinkcursor=true;
	} else {
		curtext.SetText(rooms[curroom]->deftext);
		drawlinkcursor=false;
	}
}

void Base::Click (float x, float y, int button, int state) {
	rooms[curroom]->Click(this,x,y,button,state);
}

void Base::ClickWin (int button, int state, int x, int y) {
	if (CurrentBase) {
		CurrentBase->Click((((float)(x*2))/g_game.x_resolution)-1,-(((float)(y*2))/g_game.y_resolution)+1,button,state);
	}
}

void Base::MouseOverWin (int x, int y) {
	SetSoftwareMousePosition(x,y);
	if (CurrentBase)
		CurrentBase->MouseOver((((float)(x*2))/g_game.x_resolution)-1,-(((float)(y*2))/g_game.y_resolution)+1);
}

void Base::GotoLink (int linknum) {
	curlinkindex=0;
	curroom=linknum;
	curtext.SetText(rooms[curroom]->deftext);
	drawlinkcursor=false;
}

Base::~Base () {
	CurrentBase=0;
	restore_main_loop();
	for (int i=0;i<rooms.size();i++) {
		delete rooms[i];
	}
}

void Base::InitCallbacks () {
	winsys_set_mouse_func(ClickWin);
	winsys_set_motion_func(MouseOverWin);
	winsys_set_passive_motion_func(MouseOverWin);
	CurrentBase=this;
//	UpgradeCompInterface(caller,baseun);
	CallComp=false;
}

extern string getCargoUnitName (const char *name);

void Unit::UpgradeInterface(Unit * baseun) {
	if (!Base::CurrentBase) {
		Base *base=new Base ((getCargoUnitName(baseun->getFullname().c_str())+".xbase").c_str(),baseun,this);
		base->InitCallbacks();
		SetSoftwareMousePosition(0,0);
	}
}

Base::Base (const char *basefile, Unit *base, Unit*un) {
	caller=un;
	curlinkindex=0;
	this->baseun=base;
	float x,y;
	curtext.GetCharSize(x,y);
	curtext.SetCharSize(x*2,y*2);
	LoadXML(basefile);
	GotoLink(0);
}

void Base::Room::Link::Click (Base *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
//		Do nothing...
	}
}

void Base::Room::Comp::Click (Base *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Base::CallComp=true;
		UpgradeCompInterface(base->caller,base->baseun,modes);
	}
}

void Base::Room::Launch::Click (Base *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Base::CurrentBase=0;
		delete base;
		restore_main_loop();
	}
}

void Base::Room::Goto::Click (Base *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		base->GotoLink(index);
	}
}

void Base::Draw () {
	GFXColor(0,0,0,0);
	StartGUIFrame(GFXTRUE);
	rooms[curroom]->Draw();
	float x,y;
	curtext.GetCharSize(x,y);
	curtext.SetPos(-1+(1.5*x),-1+(1.5*y));
	GFXColor(0,1,0,1);
	curtext.Draw();
	EndGUIFrame (drawlinkcursor);
}

