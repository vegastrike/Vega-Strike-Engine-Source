#include "base.h"
#include "gldrv/winsys.h"
#include "vs_path.h"
#include "lin_time.h"
#include "audiolib.h"
#include "gfx/camera.h"
#include "gfx/cockpit_generic.h"
#include "python/init.h"
#include "planet_generic.h"
#include <Python.h>
#include <algorithm>
#include "base_util.h"
#include "config_xml.h"
#include "save_util.h"
#include "unit_util.h"
#ifdef BASE_MAKER
 #include <stdio.h>
 #ifdef _WIN32
  #include <windows.h>
 #endif
static char makingstate=0;
#endif
extern const char *mission_key; //defined in main.cpp
bool BaseInterface::Room::BaseTalk::hastalked=false;

BaseInterface::Room::~Room () {
	int i;
	for (i=0;i<links.size();i++) {
		if (links[i])
			delete links[i];
	}
	for (i=0;i<objs.size();i++) {
		if (objs[i])
			delete objs[i];
	}
}

BaseInterface::Room::Room () {
//		Do nothing...
}

void BaseInterface::Room::BaseObj::Draw (BaseInterface *base) {
//		Do nothing...
}

void BaseInterface::Room::BaseSprite::Draw (BaseInterface *base) {
	GFXBlendMode(SRCALPHA,INVSRCALPHA);
	GFXEnable(TEXTURE0);
	spr.Draw();
}

void BaseInterface::Room::BaseShip::Draw (BaseInterface *base) {
	Unit *un=base->caller.GetUnit();
	if (un) {
		GFXHudMode (GFXFALSE);
		GFXEnable (DEPTHTEST);
		GFXEnable (DEPTHWRITE);
		Vector p,q,r;
		_Universe->AccessCamera()->GetOrientation (p,q,r);
		int co=_Universe->AccessCamera()->getCockpitOffset();
		_Universe->AccessCamera()->setCockpitOffset(0);
		_Universe->AccessCamera()->UpdateGFX();
		QVector pos =  _Universe->AccessCamera ()->GetPosition();
		Matrix cam (p.i,p.j,p.k,q.i,q.j,q.k,r.i,r.j,r.k,pos);
		Matrix final;
		Matrix newmat = mat;
		newmat.p.k*=un->rSize();
		newmat.p+=QVector(0,0,g_game.znear);
		newmat.p.i*=newmat.p.k;
		newmat.p.j*=newmat.p.k;
		MultMatrix (final,cam,newmat);
		(un)->DrawNow(final);
		GFXDisable (DEPTHTEST);
		GFXDisable (DEPTHWRITE);
		_Universe->AccessCamera()->setCockpitOffset(co);
		_Universe->AccessCamera()->UpdateGFX();
//		_Universe->AccessCockpit()->SetView (CP_PAN);
		GFXHudMode (GFXTRUE);
	}
}

void BaseInterface::Room::Draw (BaseInterface *base) {
	int i;
	for (i=0;i<objs.size();i++) {
		if (objs[i])
			objs[i]->Draw(base);
	}
}
static BaseInterface::Room::BaseTalk * only_one_talk=NULL;

BaseInterface::Room::BaseTalk::BaseTalk (std::string msg,std::string ind, bool only_one) :BaseObj(ind), curchar (0), curtime (0), message(msg) {
	if (only_one) {
		only_one_talk=this;
	}
}

void BaseInterface::Room::BaseTalk::Draw (BaseInterface *base) {
/*	GFXColor4f(1,1,1,1);
	GFXBegin(GFXLINESTRIP);
		GFXVertex3f(caller->x,caller->y,0);
		GFXVertex3f(caller->x+caller->wid,caller->y,0);
		GFXVertex3f(caller->x+caller->wid,caller->y+caller->hei,0);
		GFXVertex3f(caller->x,caller->y+caller->hei,0);
		GFXVertex3f(caller->x,caller->y,0);
	GFXEnd();*/
	if (only_one_talk!=NULL&&only_one_talk!=this) {
		curchar = message.size();
		curtime=999999;
	}
	if (hastalked) return;
	if (curchar<message.size()) {
		curtime+=GetElapsedTime()/getTimeCompression();
		static float inbetween=XMLSupport::parse_float(vs_config->getVariable("graphics","text_speed",".025"));
		if (curtime>inbetween) {
			base->othtext.SetText(message.substr(0,++curchar));
			curtime=0;
		}
	} else {
		curtime+=GetElapsedTime()/getTimeCompression();
		static float delay=XMLSupport::parse_float(vs_config->getVariable("graphics","text_delay",".05"));
		if (curtime>((delay*message.size())+2)) {
			curtime=0;
			BaseObj * self=this;
			std::vector<BaseObj *>::iterator ind=std::find(base->rooms[base->curroom]->objs.begin(),
					base->rooms[base->curroom]->objs.end(),
					this);
			if (ind<base->rooms[base->curroom]->objs.end()) {
				*ind=NULL;
				base->othtext.SetText("");
				delete this;
				return; //do not do ANYTHING with 'this' after the previous statement...
			}
		}
	}
	hastalked=true;
}

int BaseInterface::Room::MouseOver (BaseInterface *base,float x, float y) {
	for (int i=0;i<links.size();i++) {
		if (links[i]) {
			if (x>=links[i]->x&&
					x<=(links[i]->x+links[i]->wid)&&
					y>=links[i]->y&&
					y<=(links[i]->y+links[i]->hei)) {
				return i;
			}
		}
	}
	return -1;
}

BaseInterface *BaseInterface::CurrentBase=0;
bool BaseInterface::CallComp=false;

bool RefreshGUI(void) {
	bool retval=false;
	if (_Universe->AccessCockpit()) {
		if (BaseInterface::CurrentBase) {
			if (_Universe->AccessCockpit()->GetParent()==BaseInterface::CurrentBase->caller.GetUnit()){
				if (BaseInterface::CallComp) {
					return RefreshInterface ();	
				} else {
					BaseInterface::CurrentBase->Draw();
				}
				retval=true;
			}
		}
	}
	return retval;
}

void BaseInterface::Room::Click (BaseInterface* base,float x, float y, int button, int state) {
	if (button==WS_LEFT_BUTTON) {
		int linknum=MouseOver (base,x,y);
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
#else
			printf("\n%s\n",str);
			int ret=1;
#endif
			int index;
			int rmtyp;
			if (ret==1) {
				if (button==WS_RIGHT_BUTTON) {
#ifdef _WIN32
					FILE *fp=fopen("stdin.txt","rt");
#else
					FILE *fp=stdin;
#endif
					fscanf(fp,"%200s",input);
#ifdef _WIN32
					fclose(fp);
#endif
				} else if (button==WS_MIDDLE_BUTTON&&makingstate==0) {
					int i;
#ifdef _WIN32
					FILE *fp=fopen("stdin.txt","rt");
#else
					FILE *fp=stdin;
#endif
	 				fscanf(fp,"%d",&rmtyp);
					switch(rmtyp) {
					case 1:
						links.push_back(new Goto("linkind","link"));
						fscanf(fp,"%d",&((Goto*)links.back())->index);
						break;
					case 2:
						links.push_back(new Launch("launchind","launch"));
						break;
					case 3:
						links.push_back(new Comp("compind","comp"));
						fscanf(fp,"%d",&index);
						for (i=0;i<index&&(!feof(fp));i++) {
							fscanf(fp,"%d",&ret);
							((Comp*)links.back())->modes.push_back((UpgradingInfo::BaseMode)ret);
						}
						break;
					default:
#ifdef _WIN32
						fclose(fp);
						MessageBox(NULL,"warning: invalid basemaker option","Error",MB_OK);
#endif
						printf("warning: invalid basemaker option: %d",rmtyp);
						return;
					}
					fscanf(fp,"%200s",input);
					input[200]=input[199]='\0';
					links.back()->text=string(input);
#ifdef _WIN32
					fclose(fp);
#endif
				}
				if (button==WS_RIGHT_BUTTON) {
					input[200]=input[199]='\0';
					objs.push_back(new BaseSprite(input,"tex"));
					((BaseSprite*)objs.back())->texfile=string(input);
					((BaseSprite*)objs.back())->spr.SetPosition(x,y);
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
		if (state==WS_MOUSE_UP&&links.size()) {
			int count=0;
			while (count++<links.size()) { 
				Link *curlink=links[base->curlinkindex++%links.size()];
				if (curlink) {
					int x=(((curlink->x+(curlink->wid/2))+1)/2)*g_game.x_resolution;
					int y=-(((curlink->y+(curlink->hei/2))-1)/2)*g_game.y_resolution;
					winsys_warp_pointer(x,y);
					MouseOverWin(x,y);
					break;
				}
			}
		}
#endif
	}
}

void BaseInterface::MouseOver (float x, float y) {
	int i=rooms[curroom]->MouseOver(this,x,y);
	Room::Link *link=0;
	if (i<0) {
		link=0;
	} else {
		link=rooms[curroom]->links[i];
	}
	if (link) {
		curtext.SetText(link->text);
		curtext.col=GFXColor(1,.666667,0,1);
		drawlinkcursor=true;
	} else {
		curtext.SetText(rooms[curroom]->deftext);
		curtext.col=GFXColor(0,1,0,1);
		drawlinkcursor=false;
	}
}

void BaseInterface::Click (float x, float y, int button, int state) {
	rooms[curroom]->Click(this,x,y,button,state);
}

void BaseInterface::ClickWin (int button, int state, int x, int y) {
	if (CurrentBase) {
		CurrentBase->Click((((float)(x*2))/g_game.x_resolution)-1,-(((float)(y*2))/g_game.y_resolution)+1,button,state);
	}
}

void BaseInterface::MouseOverWin (int x, int y) {
	SetSoftwareMousePosition(x,y);
	if (CurrentBase)
		CurrentBase->MouseOver((((float)(x*2))/g_game.x_resolution)-1,-(((float)(y*2))/g_game.y_resolution)+1);
}

void BaseInterface::GotoLink (int linknum) {
	othtext.SetText("");
	if (rooms.size()>linknum&&linknum>=0) {
		curlinkindex=0;
		curroom=linknum;
		curtext.SetText(rooms[curroom]->deftext);
		drawlinkcursor=false;
	} else {
#ifndef BASE_MAKER
		fprintf(stderr,"\nWARNING: base room #%d tried to go to an invalid index: #%d",curroom,linknum);
		assert(0);
#else
		while(rooms.size()<=linknum) {
			rooms.push_back(new Room());
			char roomnum [50];
			sprintf(roomnum,"Room #%d",linknum);
			rooms.back()->deftext=roomnum;
		}
		GotoLink(linknum);
#endif
	}
}

BaseInterface::~BaseInterface () {
#ifdef BASE_MAKER
	FILE *fp=fopen("bases/NEW_BASE"BASE_EXTENSION,"wt");
	if (fp) {
		EndXML(fp);
		fclose(fp);
	}
#endif
	CurrentBase=0;
	restore_main_loop();
	for (int i=0;i<rooms.size();i++) {
		delete rooms[i];
	}
}

void BaseInterface::InitCallbacks () {
	winsys_set_mouse_func(ClickWin);
	winsys_set_motion_func(MouseOverWin);
	winsys_set_passive_motion_func(MouseOverWin);
	CurrentBase=this;
//	UpgradeCompInterface(caller,baseun);
	CallComp=false;
}

BaseInterface::Room::Talk::Talk (std::string ind,std::string pythonfile)
		: BaseInterface::Room::Link(ind,pythonfile) {
	index=-1;
#ifndef BASE_MAKER
	gameMessage * last;
	int i=0;
	vector <std::string> who;
	string newmsg;
	string newsound;
	who.push_back ("bar");
	while ((last= mission->msgcenter->last(i++,who))!=NULL) {
		newmsg=last->message;
		newsound="";
		int first=newmsg.find_first_of("[");
		int last=newmsg.find_first_of("]");
		if (first!=string::npos&&(first+1)<newmsg.size()) {
			newsound=newmsg.substr(first+1,last-first-1);
			newmsg=newmsg.substr(0,first);
		}
		this->say.push_back(newmsg);
		this->soundfiles.push_back(newsound);
	}
#endif
}
BaseInterface::Room::Python::Python (std::string ind,std::string pythonfile)
		: BaseInterface::Room::Link(ind,pythonfile) {
}
double compute_light_dot (Unit * base,Unit *un) {
  StarSystem * ss =base->getStarSystem ();
  double ret=-1;
  Unit * st;
  Unit * base_owner=NULL;
  if (ss) {
    _Universe->pushActiveStarSystem (ss);
    un_iter ui = ss->getUnitList().createIterator();
    for (;(st = *ui);++ui) {
      if (st->isPlanet()) {
	if (((Planet *)st)->hasLights()) {
	  QVector v1 = (un->Position()-base->Position()).Normalize();
	  QVector v2 = (st->Position()-base->Position()).Normalize();
	  double dot = v1.Dot(v2);
	  if (dot>ret) {
	    fprintf (stderr,"dot %lf",dot);
	    ret=dot;
	  }
	} else {
	  un_iter ui = ((Planet *)st)->satellites.createIterator();
	  Unit * ownz=NULL;
	  for (;(ownz=*ui);++ui) {
	    if (ownz==base) {
	      base_owner = st;
	    }
	  }
	}
      }
    }
    _Universe->popActiveStarSystem();
  }else return 1;
  if (base_owner==NULL||base->isUnit()==PLANETPTR) {
    return ret;
  }else {
    return compute_light_dot(base_owner,un);
  }
}

const char * compute_time_of_day (Unit * base,Unit *un) {
  float rez= compute_light_dot (base,un);
  if (rez>.2) 
    return "day";
  if (rez <-.1)
    return "night";
  return "sunset";
}
BaseInterface::BaseInterface (const char *basefile, Unit *base, Unit*un)
		: curtext(GFXColor(0,1,0,1),GFXColor(0,0,0,1)) , othtext(GFXColor(1,1,.5,1),GFXColor(0,0,0,1)) {
	CurrentBase=this;
	caller=un;
	curlinkindex=0;
	this->baseun=base;
	float x,y;
	curtext.GetCharSize(x,y);
	curtext.SetCharSize(x*2,y*2);
	//	curtext.SetSize(2-(x*4 ),-2);
	curtext.SetSize(1-.01,-2);
	othtext.GetCharSize(x,y);
	othtext.SetCharSize(x*2,y*2);
	//	othtext.SetSize(2-(x*4),-.75);
	othtext.SetSize(1-.01,-.75);
	Load(basefile, compute_time_of_day(base,un),FactionUtil::GetFaction(base->faction));
	vector <string> vec;
	vec.push_back(base->name);
	if (un) {
	    int cpt=UnitUtil::isPlayerStarship(un);
		if (cpt>=0) {
			saveStringList(UnitUtil::isPlayerStarship(un),mission_key,vec);
		}
	}
	if (!rooms.size()) {
		fprintf(stderr,"ERROR: there are no rooms in basefile \"%s%s%s\" ...\n",basefile,compute_time_of_day(base,un),BASE_EXTENSION);
		rooms.push_back(new Room ());
		rooms.back()->deftext="ERROR: No rooms specified...";
#ifndef BASE_MAKER
		rooms.back()->objs.push_back(new Room::BaseShip (-1,0,0,0,0,-1,0,1,0,QVector(0,0,2),"default room"));
		BaseUtil::Launch(0,"default room",-1,-1,1,2,"ERROR: No rooms specified... - Launch");
		BaseUtil::Comp(0,"default room",0,-1,1,2,"ERROR: No rooms specified... - Computer",
				"BUYMODE SELLMODE UPGRADEMODE DOWNGRADEMODE NEWSMODE SHIPMODE MISSIONMODE BRIEFINGMODE");
#endif
	}
	GotoLink(0);
}

void BaseInterface::Room::Python::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
//		Do nothing...

	}
}

void BaseInterface::Room::Comp::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
		Unit *un=base->caller.GetUnit();
		Unit *baseun=base->baseun.GetUnit();
		if (un&&baseun) {
			BaseInterface::CallComp=true;
			UpgradeCompInterface(un,baseun,modes);
		}
	}
}
void BaseInterface::Terminate() {
  Unit *un=caller.GetUnit();
  int cpt=UnitUtil::isPlayerStarship(un);
  if (un&&cpt>=0) {
    vector <string> vec;
    vec.push_back(string());
    saveStringList(cpt,mission_key,vec);
  }
  BaseInterface::CurrentBase=NULL;
  restore_main_loop();
  delete this;
}
extern void abletodock(int dock);
#include "ai/communication.h"
void BaseInterface::Room::Launch::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
	  Link::Click(base,x,y,button,state);
	  static bool auto_undock = XMLSupport::parse_bool(vs_config->getVariable("physics","AutomaticUnDock","true"));
	  if (auto_undock) {
	  Unit * bas = base->baseun.GetUnit();
	  Unit * playa = base->caller.GetUnit();
	  if (playa &&bas) {
	    playa->UnDock (bas);
	    CommunicationMessage c(bas,playa,NULL,0);
	    c.SetCurrentState (c.fsm->GetUnDockNode(),NULL,0);
	    playa->getAIState()->Communicate (c);
	    abletodock(5);
	  }
	  }
	  base->Terminate();
	}
}

void BaseInterface::Room::Goto::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
		base->GotoLink(index);
	}
}

void BaseInterface::Room::Talk::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		Link::Click(base,x,y,button,state);
		if (index>=0) {
			delete base->rooms[curroom]->objs[index];
			base->rooms[curroom]->objs[index]=NULL;
			index=-1;
			base->othtext.SetText("");
		} else if (say.size()) {
			curroom=base->curroom;
//			index=base->rooms[curroom]->objs.size();
			int sayindex=rand()%say.size();
			base->rooms[curroom]->objs.push_back(new Room::BaseTalk(say[sayindex],"currentmsg",true));
//			((Room::BaseTalk*)(base->rooms[curroom]->objs.back()))->sayindex=(sayindex);
//			((Room::BaseTalk*)(base->rooms[curroom]->objs.back()))->curtime=0;
			if (soundfiles[sayindex].size()>0) {
				int sound = AUDCreateSoundWAV (soundfiles[sayindex],false);
				if (sound==-1) {
					fprintf(stderr,"\nCan't find the sound file %s\n",soundfiles[sayindex].c_str());
				} else {
//					AUDAdjustSound (sound,_Universe->AccessCamera ()->GetPosition(),Vector(0,0,0));
					AUDStartPlaying (sound);
					AUDDeleteSound(sound);//won't actually toast it until it stops
				}
			}
		} else {
			fprintf(stderr,"\nThere are no things to say...\n");
			assert(0);
		}
	}
}

void BaseInterface::Room::Link::Click (BaseInterface *base,float x, float y, int button, int state) {
	if (state==WS_MOUSE_UP) {
		const char * filnam=this->pythonfile.c_str();
		if (filnam[0]) {
			FILE *fp=fopen(filnam,"r");
			if (fp) {
				int length=strlen(filnam);
				char *newfile=new char[length+1];
				strncpy(newfile,filnam,length);
				newfile[length]='\0';
				::Python::reseterrors();
				PyRun_SimpleFile(fp,newfile);
				::Python::reseterrors();
				fclose(fp);
			} else {
				fprintf(stderr,"Warning:python link file '%s' not found\n",filnam);
			}
		}
	}
}

void BaseInterface::Draw () {
	GFXColor(0,0,0,0);
	StartGUIFrame(GFXTRUE);
	Room::BaseTalk::hastalked=false;
	rooms[curroom]->Draw(this);
	float x,y;
	curtext.GetCharSize(x,y);
	curtext.SetPos(-.99,-1+(y*1.5));
//	if (!drawlinkcursor)
//		GFXColor4f(0,1,0,1);
//	else
//		GFXColor4f(1,.333333,0,1);
	curtext.Draw();
	othtext.SetPos(-.99,1);
//	GFXColor4f(0,.5,1,1);
	othtext.Draw();
	EndGUIFrame (drawlinkcursor);
	Unit *un=caller.GetUnit();
	Unit *base=baseun.GetUnit();
	if (un&&(!base)) {
		fprintf(stderr,"Error: Base NULL");
		mission->msgcenter->add("game","all","[Computer] Docking unit destroyed. Emergency launch initiated.");
		for (int i=0;i<un->image->dockedunits.size();i++) {
			if (un->image->dockedunits[i]->uc.GetUnit()==base) {
				un->FreeDockingPort (i);
			}
		}
		Terminate();
	}
}

