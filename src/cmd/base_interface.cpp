#include "base.h"
#include "gldrv/winsys.h"
#include "vs_path.h"
#ifdef BASE_MAKER
 #include <stdio.h>
 #ifdef _WIN32
  #include <windows.h>
 #endif
static char makingstate=0;
#endif


Base::Room::~Room () {
	int i;
	for (i=0;i<links.size();i++) {
		delete links[i];
	}
	for (i=0;i<texes.size();i++) {
		delete texes[i];
	}
	for (i=0;i<ships.size();i++) {
		delete ships[i];
	}
}

Base::Room::Room () {
//		Do nothing...
}

void Base::Room::Draw () {
	int i;
	for (i=0;i<texes.size();i++) {
		texes[i]->Draw();
	}
//	GFXHudMode (GFXFALSE);
	for (i=0;i<ships.size();i++) {
		CurrentBase->caller->DrawNow(*ships[i]);
	}
//	GFXHudMode (GFXTRUE);
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
#ifdef BASE_MAKER
		if (state==WS_MOUSE_UP) {
			char input [201];
			char *str;
			if (button==WS_RIGHT_BUTTON)
				str="Please create a file named stdin.txt and type\nin the sprite file that you wish to use.";
			else if (button==WS_MIDDLE_BUTTON)
				str="Please create a file named stdin.txt and type\nin the type of room followed by arguments for the room followed by text in quotations:\n1 ROOM# \"TEXT\"\n2 \"TEXT\"\n3 vector<MODES>.size vector<MODES> \"TEXT\"";
			else
				return;
 #ifdef _WIN32
			int ret=MessageBox(NULL,str,"Input",MB_OKCANCEL);
			int index;
			int rmtyp;
			if (ret==1) {
				if (button==WS_RIGHT_BUTTON) {
					FILE *fp=fopen("stdin.txt","rt");
					fscanf(fp,"%200s",input);
					fclose(fp);
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==0) {
					FILE *fp=fopen("stdin.txt","rt");
	 				fscanf(fp,"%d",&rmtyp);
					switch(rmtyp) {
					case 1:
						links.push_back(new Goto());
						fscanf(fp,"%d",&((Goto*)links.back())->index);
						break;
					case 2:
						links.push_back(new Launch());
						break;
					case 3:
						links.push_back(new Comp());
						fscanf(fp,"%d",&index);
						for (int i=0;i<index&&(!feof(fp));i++) {
							fscanf(fp,"%d",&ret);
							((Comp*)links.back())->modes.push_back((UpgradingInfo::BaseMode)ret);
						}
						break;
					}
					fscanf(fp,"%200s",input);
					input[200]=input[199]='\0';
					links.back()->text=string(input);
					fclose(fp);
				}
 #else
			{
				printf("\n%s\n",str);
				if (button==WS_RIGHT_BUTTON) {
	 				scanf("%200s",input);
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==0) {
	 				scanf("%d",rmtyp);
					switch(rmtyp) {
					case 1:
						links.push_back(new Goto());
						scanf("%d",&((Goto*)links.back())->index);
						break;
					case 2:
						links.push_back(new Launch());
						break;
					case 3:
						links.push_back(new Comp());
						scanf("%d",&index);
						for (int i=0;i<index;i++) {
							scanf("%d",&ret);
							((Comp*)links.back())->modes.push_back((UpgradingInfo::BaseMode)ret);
						}
						break;
					}
					scanf("%200s",input);
					input[200]=input[199]='\0';
					links.back()->text=string(input);
				}
 #endif
				if (button==WS_RIGHT_BUTTON) {
					input[200]=input[199]='\0';
					texfiles.push_back(string(input));
					texes.push_back(new Sprite(input));
					texes.back()->SetPosition(x,y);
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==0) {
					links.back()->x=x;
					links.back()->y=y;
					links.back()->wid=0;
					links.back()->hei=0;
					makingstate=1;
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==1) {
					links.back()->wid=x-links.back()->x;
					if (links.back()->wid<0)
						links.back()->wid=-links.back()->wid;
					links.back()->hei=y-links.back()->y;
					if (links.back()->hei<0)
						links.back()->hei=-links.back()->hei;
					makingstate=0;
				}
			}
		}
#else
		if (state==WS_MOUSE_UP) {
			Link *curlink=links[base->curlinkindex%links.size()];
			base->curlinkindex++;
			int x=(((curlink->x+(curlink->wid/2))+1)/2)*g_game.x_resolution;
			int y=-(((curlink->y+(curlink->hei/2))-1)/2)*g_game.y_resolution;
			winsys_warp_pointer(x,y);
			MouseOverWin(x,y);
		}
#endif
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
	if (rooms.size()>linknum) {
		curlinkindex=0;
		curroom=linknum;
		curtext.SetText(rooms[curroom]->deftext);
		drawlinkcursor=false;
	} else {
		fprintf(stderr,"\nWARNING: base room #%d tried to go to an invalid index: #%d",curroom,linknum);
	}
}

Base::~Base () {
#ifdef BASE_MAKER
	vschdir("bases");
	FILE *fp=fopen("NEW_BASE.xbase","wt");
	if (fp) {
		EndXML(fp);
		fclose(fp);
	}
	vscdup();
#endif
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
	  string basename = (getCargoUnitName(baseun->getFullname().c_str()));
	  if (baseun->isUnit()!=PLANETPTR) {
	    basename = baseun->name;
	  }
	  Base *base=new Base ((basename+".xbase").c_str(),baseun,this);
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
	if (!rooms.size()) {
		fprintf(stderr,"\nERROR: there are no rooms...");
		rooms.push_back(new Room ());
		rooms.back()->links.push_back(new Room::Launch ());
		rooms.back()->links.back()->x=rooms.back()->links.back()->y=-1;
		rooms.back()->links.back()->wid=rooms.back()->links.back()->hei=2;
		rooms.back()->deftext="ERROR: No rooms specified...";
		rooms.back()->links.back()->text="ERROR: No rooms specified...";
	}
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

