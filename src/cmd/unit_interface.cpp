#include "unit.h"
//#include "cmd/images.h"
#include "unit_factory.h"
#include "universe_util.h"
#include "gui/text_area.h"
#include "gui/button.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "main_loop.h"
#include <algorithm>
#include "gfx/cockpit.h"
#include "savegame.h"
#include "cmd/script/mission.h"
#include "unit_interface.h"
#include "config_xml.h"
#include "gldrv/winsys.h"
#include "base.h"
#include "music.h"
#include "unit_const_cache.h"
#include "configxml.h"
#include "unit_util.h"
#include "load_mission.h"
extern Music *muzak;
#ifdef _WIN32
#define strcasecmp stricmp
#endif
extern int GetModeFromName (const char *);
extern void ModifyMouseSensitivity(int&,int&);
extern std::set <std::string> GetListOfDowngrades();
extern void ClearDowngradeMap();
extern unsigned int getSaveStringLength (int whichcp, string key);
extern unsigned int eraseSaveString (int whichcp, string key, unsigned int num);
extern std::string getSaveString (int whichcp, string key, unsigned int num);
extern void putSaveString (int whichcp, string key, unsigned int num,std::string s);

static const char *miss_script="mission_scripts";
static const char *miss_name="mission_names";
static const char *miss_desc="mission_descriptions";
static const char *news_name="dynamic_newsnews";
using std::string;
extern const Unit * makeFinalBlankUpgrade (string name, int faction);
extern const Unit * makeTemplateUpgrade (string name, int faction);
extern const Unit * loadUnitByCache(std::string name,int faction);

static string beautify (const std::string &input) {
  string ret = input;
  string::iterator i=ret.begin();
  for (;i!=ret.end();i++) {
    if (i!=ret.begin()) {
      if ((*(i-1))==' ') {
	*i = toupper(*i);
      }
    }
    if (*i=='_') {
      *i=' ';
    }
  }
  return ret;
}

static float usedPrice (float percentage) {
  static float shipSellback=XMLSupport::parse_float(vs_config->getVariable("economics","upgrade_sellback_price",".5"));
  return shipSellback*percentage;
}
bool final_cat (const std::string &cat) {
  if (cat.empty())
    return false;
  return (std::find (cat.begin(),cat.end(),'*')==cat.end());
}
static string getLevel (const string &input, int level) {
  char * ret=strdup (input.c_str());
  int count=0;
  char * i =ret;
  for (;count!=level&&((*i)!='\0');i++) {
    if (*i=='/') {
      count++;
    }
  }
  char * retthis=i;
  for (;*i!='\0';i++) {
    if (*i=='/'){
      *i='\0';
    }
  }
  string retval (retthis);
  free (ret);
  return retval;
}
static bool match (vector <string>::const_iterator cat, vector<string>::const_iterator endcat, string::const_iterator item, string::const_iterator itemend, bool perfect_match) {
  string::const_iterator a;
  if (cat==endcat) {
    return (!perfect_match)||(item==itemend);
  }
  a = (*cat).begin();
  for (;a!=(*cat).end()&&item!=itemend;a++,item++) {
    if (*a!=*item) {
      return false;
    }
  }
  if (item!=itemend) {
    if (*item!='/') 
      return false;
    item++;
    cat++;
    return match (cat,endcat,item,itemend,perfect_match);
  }else {
    return endcat==(cat+1);
  }
}
const Unit * getUnitFromUpgradeName (string upgrade_name, int myunitfaction=0) {
    const char * input_buffer = upgrade_name.c_str();
    const Unit * NewPart = UnitConstCache::getCachedConst (StringIntKey (input_buffer,FactionUtil::GetFaction("upgrades")));
    if (!NewPart) {
	    NewPart = NewPart = UnitConstCache::setCachedConst (StringIntKey (input_buffer,
					  FactionUtil::GetFaction("upgrades")),
					  UnitFactory::createUnit (input_buffer,true,FactionUtil::GetFaction("upgrades")));
    }
    if (NewPart->name==string("LOAD_FAILED")) {
	    NewPart = UnitConstCache::getCachedConst (StringIntKey(input_buffer,myunitfaction));
	    if (!NewPart) {
            NewPart = UnitConstCache::setCachedConst (StringIntKey(input_buffer,
					    myunitfaction),
					    UnitFactory::createUnit (input_buffer,true,myunitfaction));
	    }
    }
    return NewPart;
}
UpgradingInfo::UpgradingInfo(Unit * un, Unit * base, vector<BaseMode> modes):base(base),buyer(un),mode(BUYMODE),title("Buy Cargo") {
	CargoList = new TextArea(-1, 0.9, 1, 1.7, 1);
	CargoInfo = new TextArea(0, 0.9, 1, 1.7, 0);
	Modes=new Button * [modes.size()+2];//save mode and null mode
	CargoInfo->DoMultiline(1);
	Cockpit * cp = _Universe->isPlayerStarship(un);
	briefingMission=NULL;
	Cockpit * tmpcockpit = _Universe->AccessCockpit();
	if (cp) {
	  _Universe->SetActiveCockpit(cp);
	  WriteSaveGame(cp,true);
	}
	NewPart=NULL;//no ship to upgrade
	downgradelimiter=templ=NULL;//no template
	//	CargoList->AddTextItem("a","Just a test item");
	//	CargoList->AddTextItem("b","And another just to be sure");
	CargoInfo->AddTextItem("name", "");
	CargoInfo->AddTextItem("price", "");
	CargoInfo->AddTextItem("mass", "");
	CargoInfo->AddTextItem("volume", "");
	CargoInfo->AddTextItem("description", "");
	OK = new Button(-0.94, -0.85, 0.15, 0.1, "Done");
	COMMIT = new Button(-0.75, -0.85, 0.25, 0.1, "Buy");
	const char  MyButtonModes[][128] = {"Merchant's Inventory","My Inventory","Mission BBS","My Ship Stats","GNN News", "Ship Dealer","Upgrade Ship","Unimplemented","Downgrade Ship","Save/Load"};
	float beginx = -.4;
	float lastx = beginx;
	float sizeb;
	float sizes;
	if (modes.size()>=4) {
		if (((((float)modes.size())/2)!=((float)(modes.size()/2)))) {
			sizes=(1.4/((modes.size()/2)+1))-.02;
			sizeb=(1.4/(modes.size()/2))-.02;
		} else {
			sizes=sizeb=(1.4/(modes.size()/2))-.02;
		}
	} else {
		sizes=sizeb=(1.4/modes.size())-.02;
	}
	int i;
	modes.push_back(SAVEMODE);
	availmodes=modes;
	for (i=0;(unsigned int)i<(modes.size());i++) {
		if (modes[i]!=SAVEMODE) {
			if (modes[i]!=ADDMODE) {
/*				if (i<(modes.size()-1)/2) {
					Modes[i]= new Button (lastx,-.82,size,0.07,MyButtonModes[modes[i]]);
				}else {
					Modes[i]= new Button (beginx,-.91,size,0.07,MyButtonModes[modes[i]]);
					beginx+=sizes+.04;
				}*/
				if ((modes.size())<=4) {
					Modes[i]= new Button (lastx,-0.85,sizes,0.1,MyButtonModes[modes[i]]);
					lastx+=sizes+.04;
				} else {
					if ((unsigned int)i<(modes.size()-1)/2) {
						Modes[i]= new Button (lastx,-.82,sizeb,0.07,MyButtonModes[modes[i]]);
						lastx+=sizeb+.04;
					}else {
						Modes[i]= new Button (beginx,-.91,sizes,0.07,MyButtonModes[modes[i]]);
						beginx+=sizes+.04;
					}
				}
			}else {
				Modes[i]=new Button (0,0,0,0,"Unimplemented");
			}
		} else {
			Modes[i]=new Button (.68,.98,.3,.07,MyButtonModes[modes[i]]);
		}
	}
	Modes[i]=0;
	readnews=false;
	if (modes.size()) {
		CargoList->RenderText();
		CargoInfo->RenderText();	
		SetMode (modes[0],NORMAL);
		_Universe->SetActiveCockpit(tmpcockpit);
	}
}
UpgradingInfo::~UpgradingInfo() {
  /*    if (templ){
      templ->Kill();
      templ=NULL;
    }
    if (NewPart) {
      NewPart->Kill();
      NewPart=NULL;
      }*/
    base.SetUnit(NULL);
    buyer.SetUnit(NULL);
    delete CargoList;
    delete CargoInfo;
    for (int i=0;Modes[i]!=0;i++) {
      delete Modes[i];
    }
    delete [] Modes;
}
void UpgradingInfo::Render() {
    //    GFXSubwindow (0,0,g_game.x_resolution,g_game.y_resolution);
    Unit * un = buyer.GetUnit(); 
    if (un) {
      Cockpit * cp = _Universe->isPlayerStarship(un);
      if (cp)
	_Universe->SetActiveCockpit(cp);
    }
    bool render=true;

    TextPlane * tp=NULL;
#ifdef USE_BRIEFINGS
    if (mode==BRIEFINGMODE&&submode==STOP_MODE) {
      for (unsigned int i=0;i<active_missions.size();i++) {
	if (active_missions[i]==briefingMission) {
	  if (briefingMission->BriefingInProgress()) {
	    render=false;

	    tp = briefingMission->BriefingRender();
	  }else {
	    SetMode(BRIEFINGMODE,NORMAL);
	  }
	}
      }
    }
#endif
    StartGUIFrame(
#ifdef USE_BRIEFINGS
		mode==BRIEFINGMODE?GFXFALSE:GFXTRUE
#else
		GFXTRUE
#endif
		);
    if (render) {
      // Black background
      ShowColor(-1,-1,2,2, 0,0,0,1);
      ShowColor(0,0,0,0, 1,1,1,1);
      char floatprice [100];
      sprintf(floatprice,"%.2f",_Universe->AccessCockpit()->credits);
      Unit * baseunit = this->base.GetUnit();
      string basename;
      if (baseunit) {
	basename = "";
	if (baseunit->isUnit()==PLANETPTR) {
	  
	  string temp = ((Planet *)baseunit)->getHumanReadablePlanetType()+" Planet";
	  basename +=temp;
	}else {
	  basename+= baseunit->name;
	}
      }
      ShowText(-0.98, 0.93, 2, 4, (title+basename+string("  Credits: ")+floatprice).c_str(), 0);
      CargoList->Refresh();
      CargoInfo->Refresh();
    }

    if (tp) {
      GFXDisable(TEXTURE0);
      GFXDisable(TEXTURE1);
      GFXColor (0,1,1,1);
      tp->Draw();
      GFXColor (1,1,1,1);
      GFXDisable(TEXTURE0);
      GFXEnable(TEXTURE1);
    }
    OK->Refresh();
    COMMIT->Refresh();
    for (unsigned int i=0;Modes[i]!=0;i++) {
      Modes[i]->Refresh();
    }
    EndGUIFrame(drawovermouse);
}
void UpgradingInfo::SetMode (enum BaseMode mod, enum SubMode smod) {
    bool resetcat=false;
    if (mod!=mode) {
      curcategory.clear();
      resetcat=true;
    }
    string ButtonText;
    switch (mod) {
    case SAVEMODE:
      title = "Save/Load ";
      ButtonText="";
      break;
    case BRIEFINGMODE: 
#ifdef USE_BRIEFINGS
      title="Briefings   ";
      ButtonText="End";
#else
	  title="View Ship Stats ";
	  ButtonText="";
#endif
      break;
    case NEWSMODE:
      title="GNN News  ";
      ButtonText="ReadNews";
      break;
    case BUYMODE:
      title = "Buy Cargo  ";
      ButtonText= "Buy";
      break;
    case SELLMODE:
      title = "Sell Cargo  ";
      ButtonText= "Sell";
      break;
    case UPGRADEMODE:
      title = "Upgrade Ship ";
      ButtonText="Upgrade";
      if (!beginswith (curcategory,"upgrades")) {
	curcategory.clear();
	curcategory.push_back("upgrades");
      }
      break;
    case ADDMODE:
      title = "Enhance Starship ";
      ButtonText="AddStats";
      if (!beginswith (curcategory,"upgrades")) {
	curcategory.clear();
	curcategory.push_back("upgrades");
      }
      break;
    case DOWNGRADEMODE:
      title = "Sell Upgrades  ";
      ButtonText= "SellPart";
      if (!beginswith (curcategory,"upgrades")) {
	curcategory.clear();
	curcategory.push_back("upgrades");
      }
      break;
    case MISSIONMODE:
      if (title.find ("Mission BBS")==string::npos) {
	title = "Mission BBS  ";
      }
      if (!beginswith (curcategory,"missions")) {
	curcategory.clear();
	curcategory.push_back("missions");
      }
      ButtonText="Accept";
      break;
    case SHIPDEALERMODE:
      title = "Buy Starship  ";
      ButtonText="BuyShip";
      if (!beginswith (curcategory,"starships")) {
	curcategory.clear();
	curcategory.push_back("starships");
      }
      break;
    }
	if (mode==NEWSMODE&&mod!=NEWSMODE&&readnews) {
		muzak->Skip();
		readnews=false;
	}
	static int where=0;
    if (smod!=NORMAL) {
      if (submode==NORMAL) {
	where=CargoList->GetSelectedItem();
      }
      switch (smod) {
      case MOUNT_MODE:
	title="Select On Which Mount To Place Your Weapon";
	break;
      case SUBUNIT_MODE:
	title="Select On Which Turret Mount To Place Your Turret";
	break;
      case CONFIRM_MODE:
	title="This may not entirely fit on your ship. Continue anyway?";
	break;
      case STOP_MODE:
	title="Viewing Mission Briefing. Press End to exit.";
	break;
      }
    }
    
    COMMIT->ModifyName (ButtonText.c_str());
	int tmpsubmode=submode;
    mode = mod;
    submode = smod;
    SetupCargoList();
    if (smod==NORMAL&&tmpsubmode!=NORMAL) {
      CargoList->SetSelectedItem(where);
	  lastselected.last=false;
      where=0;
    }
}
bool UpgradingInfo::beginswith (const vector <std::string> &cat, const std::string &s) {
    if (cat.empty()) {
      return false;
    }
    return cat.front()==s;
}
void UpgradingInfo::SetupCargoList () {
    CurrentList = &GetCargoList();
    //    std::sort (CurrentList->begin(),CurrentList->end());
    CargoList->ClearList();
#ifdef USE_BRIEFINGS
	if (mode==BRIEFINGMODE) {
		title="Briefings not implemented.";
		return;
	}
#endif
    if (submode==NORMAL) {
      if (mode==SAVEMODE) {
        CargoList->AddTextItem ("Save","Save");
        CargoList->AddTextItem ("Load","Load");
      }else
      if (mode==NEWSMODE) {
	static bool news_from_cargolist=XMLSupport::parse_bool(vs_config->getVariable("cargo","news_from_cargolist","false"));
	if (news_from_cargolist) {
		gameMessage last;
		int i=0;
		vector <std::string> who;
		who.push_back ("news");
		while ((mission->msgcenter->last(i++,last,who))) {
			CargoList->AddTextItem ((tostring(i-1)+" "+last.message).c_str(),last.message.c_str());
		}
	}else {
		int playernum=UnitUtil::isPlayerStarship(buyer.GetUnit());
		int len=getSaveStringLength(playernum,news_name);
		for (int i=len-1;i>=0;i--) {
			string tmp = tostring(len-i-1)+" "+getSaveString(playernum,news_name,i);
			CargoList->AddTextItem(tmp.c_str(),getSaveString(playernum,news_name,i).c_str());
			VSFileSystem::vs_fprintf (stderr,"<*>%s",tmp.c_str());
			
		}

	}

      }else {
	bool addedsomething=false;
	if (curcategory.empty()) {
	  string::size_type pos=title.find("Category: ",0);
	  if (pos!=string::npos) {
	    title=title.substr(0,pos);
	  }
	  if (mode==BRIEFINGMODE) {
#ifdef USE_BRIEFINGS
	    curcategory.push_back("briefings");
#else
		curcategory.push_back("mystats");
#endif
	  }
	}
	if (!curcategory.empty()) {
	  string::size_type pos=title.find("Category: ",0);
	  if (pos!=string::npos) {
	    title=title.substr(0,pos);
	  }
	  title+=string("Category: ")+curcategory.back()+"  ";
	  if (mode==BUYMODE||mode==SELLMODE||curcategory.size()>1) {
	    CargoList->AddTextItem ("[Back To Categories]","[Back To Categories]",NULL,GFXColor(0,1,.5,1));
	  }else {
	    //	    CargoList->AddTextItem ("","",NULL,GFXColor(0,0,0,1));
	  }

	  for (unsigned int i=0;i<CurrentList->size();i++) {
	    if (match(curcategory.begin(),curcategory.end(),(*CurrentList)[i].cargo.category.begin(),(*CurrentList)[i].cargo.category.end(),true)) {
		if (mode!=UPGRADEMODE&&mode!=DOWNGRADEMODE
#ifdef USE_BRIEFINGS
		  &&mode!=BRIEFINGMODE
#endif
		  ) {
		    (*CurrentList)[i].color=GFXColor(1,1,1,1);
		}
		  Unit *un=buyer.GetUnit();
		  Cockpit *cpt=NULL;
		  static bool gottencolor=false;
		  static GFXColor nomoney (0,0,0,-1);
		  if (nomoney.a<0) {
			  float color [4]={1,0,0,1};
			  vs_config->getColor(std::string("default"),"no_money",color,true);
			  nomoney=GFXColor(color[0],color[1],color[2],color[3]);
			  if (nomoney.a<0) nomoney.a=0;
		  }
		  if (un&&(cpt=_Universe->isPlayerStarship(un))) {
			  int tmpquan=(*CurrentList)[i].cargo.quantity;
			  (*CurrentList)[i].cargo.quantity=1;
			  Unit *bas;
			  bas=base.GetUnit();
			  if (mode==BUYMODE) {
				  if (((*CurrentList)[i].cargo.price>cpt->credits)||(!(un->CanAddCargo((*CurrentList)[i].cargo)))) {
					  (*CurrentList)[i].color=nomoney;
				  }
			  } else if (mode==SHIPDEALERMODE&&bas) {
				  if ((((*CurrentList)[i].cargo.price-(usedPrice(bas->PriceCargo (un->name))))>cpt->credits)) {
					  (*CurrentList)[i].color=nomoney;
				  }
			  } else if ((mode==SELLMODE)&&bas) {
				  if (!(bas->CanAddCargo((*CurrentList)[i].cargo))) {
					  (*CurrentList)[i].color=nomoney;
				  }
//			  } else if ((mode==DOWNGRADEMODE||mode==UPGRADEMODE||mode==BRIEFINGMODE||mode==SAVEMODE||mode==NEWSMODE) {
				  //do nothing (Upgrades,Downgrades,briefings,saves and news don't take up cargo space)
			  } else if (mode==MISSIONMODE) {
				  if (active_missions.size()>=UniverseUtil::maxMissions()) {
					  (*CurrentList)[i].color=nomoney;
				  }
			  }
			  (*CurrentList)[i].cargo.quantity=tmpquan;
		  } else {
			  (*CurrentList)[i].color=nomoney;
		  }
			  string printstr;
			  if (mode==MISSIONMODE) {
				  string tmpstr ((*CurrentList)[i].cargo.content);
				  string::size_type pos=tmpstr.rfind("/",tmpstr.size()-2);
				  if (pos!=string::npos) {
					  tmpstr=tmpstr.substr(pos+1,tmpstr.size()-(pos+1));
					  tmpstr[0]=toupper(tmpstr[0]);
				  }
				  printstr=beautify(tmpstr);
			  } else {
				  printstr=beautify((*CurrentList)[i].cargo.content);
			  }
			  if ((*CurrentList)[i].cargo.quantity>1)
				  printstr+="("+tostring((*CurrentList)[i].cargo.quantity)+")";
			  CargoList->AddTextItem ((tostring((int)i)+ string(" ")+(*CurrentList)[i].cargo.content).c_str() ,printstr.c_str(),NULL,(*CurrentList)[i].color);
			  addedsomething=true;
	    }
	  }
	}
	string curcat=("");
	CargoList->AddTextItem ("","");
	for (unsigned int i=0;i<CurrentList->size();i++) {
	  string curlist  ((*CurrentList)[i].cargo.category);
	  string lev=getLevel (curlist,curcategory.size());
	  if (match (curcategory.begin(),curcategory.end(),curlist.begin(),curlist.end(),false)&&
	      (!match (curcategory.begin(),curcategory.end(),curlist.begin(),curlist.end(),true))&&
	      lev!=curcat) {
	    GFXColor col=(mode!=SHIPDEALERMODE||lev!="My_Fleet")?GFXColor(0,1,1,1):GFXColor(0,.5,1,1);
	    CargoList->AddTextItem ((string("x")+lev).c_str(),beautify(lev).c_str(),NULL,col);
		addedsomething=true;
	    curcat =lev;
	  }
	}
	if (!addedsomething) {
	  if (!curcategory.empty()) {
	    curcategory.pop_back();		     
	    SetupCargoList();
	  }
        }
        
      }
      if (mode==ADDMODE||mode==UPGRADEMODE)
	CargoList->AddTextItem("Basic Repair","Basic Repair",NULL,GFXColor(0,1,0,1));

        
    }else {
      Unit * un = buyer.GetUnit();
      int i=0;
      un_iter ui;
      if (un) {
	switch (submode) {
	case MOUNT_MODE:
	  for (;i<un->GetNumMounts();i++) {
	    if (un->mounts[i].status==Mount::ACTIVE||un->mounts[i].status==Mount::INACTIVE)
	      CargoList->AddTextItem ((tostring(i)+un->mounts[i].type->weapon_name).c_str(),un->mounts[i].type->weapon_name.c_str());
	    else 
	      CargoList->AddTextItem ((tostring(i)+" [Empty]").c_str(),(std::string("[")+lookupMountSize(un->mounts[i].size)+std::string("]")).c_str());
	  }
	  break;
	case SUBUNIT_MODE:
	  for (ui=un->getSubUnits();(*ui)!=NULL;++ui,++i) {
	    CargoList->AddTextItem ((tostring(i)+(*ui)->name).c_str(),(*ui)->name.c_str());
	  }
	  break;
	case CONFIRM_MODE:
	  CargoList->AddTextItem ("Yes","Yes");
	  CargoList->AddTextItem ("No","No");
	  break;
	}
      }else {
	submode=NORMAL;
      }
    }
  }
UpgradingInfo *upgr;
unsigned int player_upgrading;

bool RefreshInterface(void) {
  bool retval=false;
  if (player_upgrading==_Universe->CurrentCockpit()){
    retval=true;
    upgr->Render();
  }
  return retval;
}

void UpgradingInfo::ProcessMouseClick(int button, int state, int x, int y) {
        ModifyMouseSensitivity(x,y);
  SetSoftwareMousePosition (x,y);
  if (upgr) {
	  int cur = _Universe->CurrentCockpit();
	  _Universe->SetActiveCockpit (_Universe->AccessCockpit(player_upgrading));
	  upgr->ProcessMouse(1, x, y, button, state);
	  _Universe->SetActiveCockpit(_Universe->AccessCockpit(cur));
  }else {
//		Navigationinfo::mouseClick(button,state,x,y);	  
  }
}

void UpgradingInfo::ProcessMouseActive(int x, int y) {
        ModifyMouseSensitivity(x,y);
  SetSoftwareMousePosition (x,y);
  if (upgr) {
	  int cur = _Universe->CurrentCockpit();
	  _Universe->SetActiveCockpit (_Universe->AccessCockpit(player_upgrading));
	  upgr->ProcessMouse(2, x, y, 0, 0);
	  _Universe->SetActiveCockpit(_Universe->AccessCockpit(cur));
  }else {
//		Navigationinfo::mouseDrag(x,y);
  }
}

void UpgradingInfo::ProcessMousePassive(int x, int y) {
        ModifyMouseSensitivity(x,y);
	SetSoftwareMousePosition(x,y);
	if (upgr) {
		int cur = _Universe->CurrentCockpit();
		_Universe->SetActiveCockpit (_Universe->AccessCockpit(player_upgrading));
		upgr->ProcessMouse(3, x, y, 0, 0);
		_Universe->SetActiveCockpit(_Universe->AccessCockpit(cur));
	}else {
//		Navigationinfo::mouseMotion(x,y);
	}
}
void UpgradeCompInterface(Unit *un,Unit * base, vector <UpgradingInfo::BaseMode> modes) {
  if (upgr) {
    if (upgr->buyer.GetUnit()==un) {
      return;//too rich for my blood...don't let 2 people buy cargo for 1
    }
  }
  printf("Starting docking\n");
  winsys_set_mouse_func(UpgradingInfo::ProcessMouseClick);
  winsys_set_motion_func(UpgradingInfo::ProcessMouseActive);
  winsys_set_passive_motion_func(UpgradingInfo::ProcessMousePassive);
  //(x, y, width, height, with scrollbar)
  upgr=( new UpgradingInfo (un,base,modes));
  player_upgrading=(_Universe->CurrentCockpit());
  
}

void CargoToMission (const char * item,TextArea * ta, Unit *buyer) {
	char * item1 = strdup (item);
	int tmp;
	sscanf (item,"%d %s",&tmp,item1);
	static bool miss_from_cargolist=XMLSupport::parse_bool(vs_config->getVariable("cargo","missions_from_cargolist","false"));
	if (!miss_from_cargolist) {
		std::string mydesc, myname;
		int playernum=UnitUtil::isPlayerStarship(buyer);
		int len=getSaveStringLength(playernum,miss_name);
		assert(len==getSaveStringLength(playernum,miss_desc));
		for (unsigned int i=0;i<len;i++) {
			if (getSaveString(playernum,miss_name,i)==item1) {
				mydesc=getSaveString(playernum,miss_desc,i);
				myname=beautify(getSaveString(playernum,miss_name,i));
				break;
			}
		}
		ta->ChangeTextItem ("name",myname.c_str());
		ta->ChangeTextItem ("price","");
		ta->ChangeTextItem ("mass","");
		ta->ChangeTextItem ("volume","");
		ta->ChangeTextItem ("description",mydesc.c_str());  
	} else {
		Mission temp (item1,false);
		free (item1);
		temp.initMission(false);
		ta->ChangeTextItem ("name",temp.getVariable ("mission_name","").c_str());
		ta->ChangeTextItem ("price","");
		ta->ChangeTextItem ("mass","");
		ta->ChangeTextItem ("volume","");
		ta->ChangeTextItem ("description",temp.getVariable("description","").c_str());  
	}
}
extern void RespawnNow (Cockpit * cp);
bool UpgradingInfo::SelectItem (const char *item, int button, int buttonstate) {
	char floatprice [640];
  switch (mode) {
#ifdef USE_BRIEFINGS
  case BRIEFINGMODE:
    switch (submode) {
    case NORMAL:
      {
	int cargonumber;
	if (sscanf (item,"%d",&cargonumber)==1) {
	  if (cargonumber<(int)active_missions.size()) {
	    active_missions[cargonumber]->BriefingStart();
	    briefingMission = active_missions[cargonumber];//never dereference
	    CargoList->ClearList();
	    SetMode(BRIEFINGMODE,STOP_MODE);
	  }
	}
      }
    }
    break;
#endif
  case SAVEMODE:
    
    if (item) {
     Unit * buy = buyer.GetUnit();
     if (buy!=NULL) {

      Cockpit * cp = _Universe->isPlayerStarship(buy);
      if (cp) {
       if (item[0]=='S') {
	 title="Game Saved Successfully. ";
	 WriteSaveGame(cp,false);
       }else if (item[0]=='L') {
	title="Game Loaded. Click Done to Return to ship. ";
        buy->Kill();
        RespawnNow(cp);
        DoDone();
	BaseInterface::CurrentBase->Terminate();
	return true;
      }
     }
    }
    }
    break;
#ifndef USE_BREFINGS
  case BRIEFINGMODE:
#endif
  case BUYMODE:
  case SELLMODE:
  case UPGRADEMODE:
  case ADDMODE:
  case DOWNGRADEMODE:    
  case SHIPDEALERMODE:
    if (submode==NORMAL) {
        Unit * bas = this->base.GetUnit();
        if (bas) {
	int cargonumber;
	sscanf (item,"%d",&cargonumber);
	if (cargonumber>=CurrentList->size()) {
		if (CurrentList->size()==1)
			cargonumber=0;
		else if (CurrentList->size()>1)
			cargonumber=1;
		else
			break;
	}
	CargoInfo->ChangeTextItem ("name",(*CurrentList)[cargonumber].cargo.content.c_str());
	float oldprice=(*CurrentList)[cargonumber].cargo.price;
	if ((*CurrentList)[cargonumber].cargo.category.find("My_Fleet")==std::string::npos) {
	  oldprice=bas->PriceCargo((*CurrentList)[cargonumber].cargo.content);
	}
        if (mode==DOWNGRADEMODE) {
	  oldprice =usedPrice(oldprice);
	}
	//        if ((*CurrentList)[cargonumber].content=="jump_drive") {
	//          oldprice/=3.0;
	//        }
	sprintf(floatprice,"Price: %.2f",oldprice);
	if (mode==SELLMODE) {
	  sprintf (floatprice,"Price %.2f Purchase@ %.2f",oldprice,(*CurrentList)[cargonumber].cargo.price);
	}
	CargoInfo->ChangeTextItem ("price",floatprice);
//	CargoInfo->ChangeTextItemColor("price",(*CurrentList)[cargonumber].color);
	sprintf(floatprice,"Mass: %.2f",(*CurrentList)[cargonumber].cargo.mass);
	CargoInfo->ChangeTextItem ("mass",floatprice);
//	CargoInfo->ChangeTextItemColor("price",(*CurrentList)[cargonumber].color);
	sprintf(floatprice,"Cargo Volume: %.2f",(*CurrentList)[cargonumber].cargo.volume);
	CargoInfo->ChangeTextItem ("volume",floatprice);
	CargoInfo->ChangeTextItem ("description",(*CurrentList)[cargonumber].cargo.description.c_str(),true);
      }
	} else {
      CommitItem (item,0,buttonstate);
    }
	break;
  case NEWSMODE:
    {
	int cargonumber;
	sscanf (item,"%d",&cargonumber);
     	gameMessage last;
	vector <std::string> who;
	CargoInfo->ChangeTextItem ("name","");
	static bool news_from_cargolist=XMLSupport::parse_bool(vs_config->getVariable("cargo","news_from_cargolist","false"));
	if (news_from_cargolist) {
		who.push_back ("news");
		if ((mission->msgcenter->last(cargonumber,last,who))) {
			CargoInfo->ChangeTextItem ("description",last.message.c_str(),true);
			static string newssong=vs_config->getVariable("audio","newssong","../music/news1.ogg");
			muzak->GotoSong(newssong);
			readnews=true;
		}
	}else {
		const char * temp = item;
		while (*temp!=' '&&*temp!='\0')
			temp++;
		if (*temp==' ')
			temp++;
		CargoInfo->ChangeTextItem ("description",temp,true);
		static string newssong=vs_config->getVariable("audio","newssong","../music/news1.ogg");
		muzak->GotoSong(newssong);
		readnews=true;
	}
	CargoInfo->ChangeTextItem ("price","");
	CargoInfo->ChangeTextItem ("mass","");
	CargoInfo->ChangeTextItem ("volume","");

    }
    break;
  case MISSIONMODE:
    CargoToMission (item,CargoInfo,this->buyer.GetUnit());
    break;
  }
  return false;
}
void UpgradingInfo::DoDone() {
	if (mode==NEWSMODE&&readnews) {
		muzak->Skip();
		readnews=false;
	}
	BaseInterface::CurrentBase->InitCallbacks();
	BaseInterface::CurrentBase->CallComp=false;
	if (upgr==this) {
		delete upgr;
		upgr=NULL;
	}
}

void UpgradingInfo::StopBriefing() {
    if (submode==STOP_MODE) {
      for (unsigned int i=0;i<active_missions.size();i++) {
	if (briefingMission==active_missions[i]) {
	  active_missions[i]->BriefingEnd();
	  briefingMission=NULL;
	  SetMode(BRIEFINGMODE,NORMAL);
	  break;
	}
      }
    }
}

extern char * GetUnitDir (const char *);
string BasicRepair (Unit * parent, string title) {
  if (parent) {
    string newtitle;
    static float repair_price = XMLSupport::parse_float(vs_config->getVariable("physics","repair_price","1000"));
    if (UnitUtil::getCredits(parent)>repair_price) {
      if (parent->RepairUpgrade()) {
	UnitUtil::addCredits (parent,-repair_price);
	newtitle= "Successfully Repaired Damage";
      }else {
	newtitle = "Nothing To Repair";
      }
    }else newtitle = "Not Enough Credits to Repair";
    return newtitle;
  }
  return title;
}
Cargo GetCargoForOwnerStarship (Cockpit * cp, int i);
Cargo GetCargoForOwnerStarshipName (Cockpit * cp, std::string nam, int & ind);



void SwapInNewShipName(Cockpit * cp,std::string newfilename,int SwappingShipsIndex) {
	  Unit * parent= cp->GetParent();
	  if (parent) {
	    if (SwappingShipsIndex!=-1) {
	      
	      while (cp->unitfilename.size()<=SwappingShipsIndex+1) {
		cp->unitfilename.push_back("");
	      } 
	      cp->unitfilename[SwappingShipsIndex]=parent->name;
	      cp->unitfilename[SwappingShipsIndex+1]=_Universe->activeStarSystem()->getFileName();
	      for (unsigned int i=1;i<cp->unitfilename.size();i+=2) {
		if(cp->unitfilename[i]==newfilename) {
		  cp->unitfilename.erase(cp->unitfilename.begin()+i);
		  if (i<cp->unitfilename.size()) {
		    cp->unitfilename.erase(cp->unitfilename.begin()+i);//get rid of system
		  }
		  i-=2;//then +=2;
		}
	      }
	    }else {
	      cp->unitfilename.push_back(parent->name);
	      cp->unitfilename.push_back(_Universe->activeStarSystem()->getFileName());
	    }
	  }else if (SwappingShipsIndex!=-1) {//if parent is dead
	    if (cp->unitfilename.size()>SwappingShipsIndex) {//erase the ship we have
	      cp->unitfilename.erase(cp->unitfilename.begin()+SwappingShipsIndex);
	    }
	    if (cp->unitfilename.size()>SwappingShipsIndex) {
	      cp->unitfilename.erase(cp->unitfilename.begin()+SwappingShipsIndex);
	    }
	  }
	  cp->unitfilename.front()= newfilename;
}


void UpgradingInfo::CommitItem (const char *inp_buf, int button, int state) {
  if (strcmp (inp_buf,"Basic Repair")==0) {
    string newtitle = BasicRepair( buyer.GetUnit(),title);
    title=newtitle;
    SetupCargoList();
    title=newtitle;
    return;
  }
  Unit * un;
  Unit * base;
  unsigned int offset;
  int quantity=(button==WS_LEFT_BUTTON)?1:(button==WS_MIDDLE_BUTTON?10000:10);
  int index;
  char * input_buffer = strdup (inp_buf);
  sscanf (inp_buf,"%d %s",&index,input_buffer);
  downgradelimiter=templ=NULL;
  if (state==0&&(un=buyer.GetUnit())&&(base=this->base.GetUnit())) {
  
  switch (mode) {
  case SHIPDEALERMODE:
    {
      Cargo *part = base->GetCargo (string(input_buffer), offset);
      Cargo my_fleet_part;
      int SwappingShipsIndex=-1;
      if (std::find (curcategory.begin(),curcategory.end(),string("My_Fleet"))!=curcategory.end()) {
	printf ("found my starship");
	part = &my_fleet_part;
	my_fleet_part = GetCargoForOwnerStarshipName(_Universe->AccessCockpit(),input_buffer,SwappingShipsIndex);
	if (part->content=="") {
	  part=NULL;
	  SwappingShipsIndex=-1;
	}
      }
      if (part) {
	float usedprice = usedPrice (base->PriceCargo (_Universe->AccessCockpit()->GetUnitFileName()));
	usedprice=0;//!!!!KEEP THE OLD SHIP NOW
	if (part->price<=usedprice+_Universe->AccessCockpit()->credits) {


	  Flightgroup * fg = un->getFlightgroup();
	  int fgsnumber=0;
	  if (fg!=NULL) {
	    fgsnumber=fg->nr_ships;
	    fg->nr_ships++;
	    fg->nr_ships_left++;
	  }
	  string newmodifications="";
	  if (SwappingShipsIndex!=-1) {//if we're swapping not buying load the olde one
	    newmodifications = _Universe->AccessCockpit()->GetUnitModifications();
	  }
	  Unit * NewPart = UnitFactory::createUnit (input_buffer,false,base->faction,newmodifications,fg,fgsnumber);
	  NewPart->SetFaction(un->faction);
	  if (NewPart->name!=string("LOAD_FAILED")) {
	    if (NewPart->nummesh()>0) {
	      WriteSaveGame(_Universe->AccessCockpit(),false);//oops saved game last time at wrong place
	      _Universe->AccessCockpit()->credits-=part->price-usedprice;
	      NewPart->curr_physical_state=un->curr_physical_state;
	      NewPart->SetPosAndCumPos(UniverseUtil::SafeEntrancePoint(un->Position(),NewPart->rSize()));
	      NewPart->prev_physical_state=un->prev_physical_state;
	      _Universe->activeStarSystem()->AddUnit (NewPart);
	      SwapInNewShipName(_Universe->AccessCockpit(),input_buffer,SwappingShipsIndex);
	      _Universe->AccessCockpit()->SetParent(NewPart,input_buffer,_Universe->AccessCockpit()->GetUnitModifications().c_str(),un->curr_physical_state.position);//absolutely NO NO NO modifications...you got this baby clean off the slate

	      SwitchUnits (NULL,NewPart);
	      un->UnDock (base);
	      //base->RequestClearance(NewPart);
	      buyer.SetUnit (NewPart);
	      //NewPart->Dock(base);
	      WriteSaveGame (_Universe->AccessCockpit(),true);
	      NewPart=NULL;
	      un->Kill();
	      DoDone();
	      BaseInterface::CurrentBase->Terminate();
	      return;
	    }
	  }
	  NewPart->Kill();
	  NewPart=NULL;
	}
      }
    }
    break;

  case UPGRADEMODE:
  case ADDMODE:
  case DOWNGRADEMODE:    

    {

      char *unitdir =GetUnitDir(un->name.c_str());
      std::string templnam = (string(unitdir)+string(".template"));
      std::string limiternam = (string(unitdir)+string(".blank"));
      const Unit * temprate= UnitConstCache::getCachedConst (StringIntKey(templnam,un->faction));
      if (!temprate)
	temprate = UnitConstCache::setCachedConst(StringIntKey(templnam,un->faction),UnitFactory::createUnit(templnam.c_str(),true,un->faction));
      free(unitdir);
      if (temprate->name!=string("LOAD_FAILED")) {
	templ=temprate;
      }else {
	templ=NULL;
      }
      const Unit * dglim = makeFinalBlankUpgrade (un->name,un->faction);
      if (dglim->name!=string("LOAD_FAILED")) {
          downgradelimiter= dglim;
      }else {
          downgradelimiter=NULL;
      }
      
    }
    switch(submode) {
    case NORMAL:
      {
	multiplicitive=false;
	int mod=GetModeFromName(input_buffer);
	if (mod==1&&mode==UPGRADEMODE)
	  mode=ADDMODE;
	if (mod==2) {
	  multiplicitive=true;
	}

	Cargo *part = base->GetCargo (string(input_buffer), offset);
       	if ((part?part->quantity:0) ||
	    (mode==DOWNGRADEMODE&&(part=GetMasterPartList(input_buffer))!=NULL)) {
	  this->part = *part;
	  if (NewPart) 
	    NewPart=NULL;
	  if (0==strcmp (input_buffer,"repair")) {
	    free (input_buffer);
	    char *unitdir =GetUnitDir(un->name.c_str());
	    VSFileSystem::vs_fprintf (stderr,"SOMETHING WENT WRONG WITH REPAIR UPGRADE");
	    input_buffer = strdup ((string(unitdir)+string(".blank")).c_str());
	    free(unitdir);
	  }
	  NewPart = getUnitFromUpgradeName (input_buffer,un->faction);
	  if (NewPart->name!=string("LOAD_FAILED")) {
   	    if (mode!=SHIPDEALERMODE) {
	      selectedmount=0;
	      selectedturret=0;
	      if (NewPart->GetNumMounts()) {
		SetMode(mode,MOUNT_MODE);
	      }else {
		CompleteTransactionAfterMountSelect();
	      }
	    }
	  } else {
	    NewPart=NULL;
	  }
	} else {
	  if (NewPart) {
	    if (NewPart->name==string("LOAD_FAILED")) {
	      //	      NewPart->Kill();
	      NewPart=NULL;
	    }
	  }
	}
      }
      break;
    case MOUNT_MODE:
      selectedmount = index;
      CompleteTransactionAfterMountSelect();
      break;
    case SUBUNIT_MODE:
      selectedturret = index;
      CompleteTransactionAfterTurretSelect();
      break;
    case CONFIRM_MODE:
      if (0==strcmp ("Yes",inp_buf)) {
	  CompleteTransactionConfirm();
      }else {
	SetMode(mode,NORMAL);
      }
    }
    break;
  case BUYMODE:
    if ((un=this->buyer.GetUnit())) {
      if ((base=this->base.GetUnit())) {
	un->BuyCargo (input_buffer,quantity,base,_Universe->AccessCockpit()->credits);
      }
      SetMode (mode,submode);
      SelectLastSelected();
    }
    break;
  case SELLMODE:
    if ((un=this->buyer.GetUnit())) {
      if ((base=this->base.GetUnit())) {
	Cargo sold;
	un->SellCargo (input_buffer,quantity,_Universe->AccessCockpit()->credits,sold,base);
      }
      SetMode (mode,submode);
      SelectLastSelected();
    }  
    break;
  case MISSIONMODE:

    if ((un=this->base.GetUnit())) {
      unsigned int index;
	static int max_missions = XMLSupport::parse_int (vs_config->getVariable ("physics","max_missions","4"));
	if (active_missions.size()<max_missions) {
	    std::string myscript;
	    title= ((string("Accepted Mission ")+input_buffer).c_str());
	    static bool miss_from_cargolist=XMLSupport::parse_bool(vs_config->getVariable("cargo","missions_from_cargolist","false"));
	    if (!miss_from_cargolist) {
		int playernum=UnitUtil::isPlayerStarship(this->buyer.GetUnit());
		int len=getSaveStringLength(playernum,miss_name);
		assert(len==getSaveStringLength(playernum,miss_script));
		for (unsigned int i=0;i<len;i++) {
		  if (getSaveString(playernum,miss_name,i)==input_buffer) {
			myscript=getSaveString(playernum,miss_script,i);
			eraseSaveString(playernum,miss_script,i);
			eraseSaveString(playernum,miss_name,i);
			eraseSaveString(playernum,miss_desc,i);
			break;
		  }
		}
		LoadMission ("",myscript,false);
		unsigned int leng= active_missions.size();
		if (leng>0) {
			active_missions[leng-1]->mission_name = input_buffer;
		}
	    } else if (NULL!=un->GetCargo(input_buffer,index)) {
		if (1==un->RemoveCargo(index,1,true)) {
		  LoadMission (input_buffer,false);
		}
	    }else {
		title=("Mission BBS::Error Accepting Mission");
	    }
	    SetMode (mode,submode);
	    SelectLastSelected();

	}else {
	 title= ("Mission BBS::Too Many Missions In Progress");
	}
    }
    break;

  }
  }
  free (input_buffer);
}


void UpgradingInfo::CompleteTransactionAfterMountSelect() {
    if (NewPart->viewSubUnits().current()!=NULL) {
      SetMode (mode,SUBUNIT_MODE);
    }else {
      selectedturret=0;
      CompleteTransactionAfterTurretSelect();
    }
}

void UpgradingInfo::CompleteTransactionAfterTurretSelect() {
  int mountoffset = selectedmount;
  int subunitoffset = selectedturret;
  bool canupgrade=false;
  double percentage;
  Unit * un;
  int addmultmode=0;
  if (mode==ADDMODE)
    addmultmode=1;
  if (multiplicitive==true)
    addmultmode=2;
  if ((un=buyer.GetUnit())) {
    switch (mode) {
    case UPGRADEMODE:
    case ADDMODE:
    
      canupgrade = un->canUpgrade (NewPart,mountoffset,subunitoffset,addmultmode,false,percentage,templ);
      break;
    case DOWNGRADEMODE:
      canupgrade = un->canDowngrade (NewPart,mountoffset,subunitoffset,percentage,downgradelimiter);
      break;
    }
    if (!canupgrade) {
      title=(mode==DOWNGRADEMODE)?string ("You do not have exactly what you wish to sell. Continue?"):string("The upgrade cannot fit the frame of your starship. Continue?");
      SetMode(mode,CONFIRM_MODE);
    }else {
      CompleteTransactionConfirm();
    }
  }
}
void UpgradingInfo::SelectLastSelected() {
	if (lastselected.last) {
    int ours = CargoList->DoMouse(lastselected.type, lastselected.x, lastselected.y, lastselected.button, lastselected.state);
    if (ours) {
      ProcessMouse(lastselected.type, lastselected.x, lastselected.y, lastselected.button, lastselected.state);
      char *buy_name = CargoList->GetSelectedItemName();
      
      if (buy_name) {
	if (buy_name[0]) {
	  SelectItem(buy_name,lastselected.button, lastselected.state);
	}
      }
    }
    }
}
void UpgradingInfo::CompleteTransactionConfirm () {
  double percentage;
  int mountoffset = selectedmount;
  int subunitoffset = selectedturret;
  bool canupgrade;
  float price;
  Unit * un;
  Unit * bas;
  int addmultmode = 0;
  if ((un=buyer.GetUnit())) {
    switch (mode) {
    case UPGRADEMODE:
    case ADDMODE:

      if (mode==ADDMODE)
	addmultmode=1;
      if (multiplicitive==true)
	addmultmode=2;
      
      canupgrade =un->canUpgrade (NewPart,mountoffset,subunitoffset,addmultmode,true,percentage,templ);
      price =(float)(part.price*(1-usedPrice(percentage)));
      if ((_Universe->AccessCockpit()->credits>price)) {
	_Universe->AccessCockpit()->credits-=price;

	un->Upgrade (NewPart,mountoffset,subunitoffset,addmultmode,true,percentage,templ);
	unsigned int removalindex;
	if ((bas=base.GetUnit())) {
	Cargo * tmp = bas->GetCargo (part.content,removalindex);
	bas->RemoveCargo(removalindex,1,false );
	}
      }
      if (mode==ADDMODE) {
	mode=UPGRADEMODE;
      }
      break;
    case DOWNGRADEMODE:
      canupgrade = un->canDowngrade (NewPart,mountoffset,subunitoffset,percentage,downgradelimiter);
      //      if (part.content=="jump_drive") {
      //        part.price/=3;
      //      }
      price =part.price*usedPrice(percentage);
      _Universe->AccessCockpit()->credits+=price;
      if (un->Downgrade (NewPart,mountoffset,subunitoffset,percentage,downgradelimiter)) {
      if ((bas=base.GetUnit())) {
	part.quantity=1;
        part.price = bas->PriceCargo (part.content);
	bas->AddCargo (part);
      }
      }
      break;
    }
  }
  SetMode (mode,NORMAL);
  SelectLastSelected();
}
// type=1 is mouse click
// type=2 is mouse drag
// type=3 is mouse movement
void UpgradingInfo::ProcessMouse(int type, int x, int y, int button, int state) {
	int ours = 0;
	float cur_x = 0, cur_y = 0, new_x = x, new_y = y;
	char *buy_name;

	cur_x = ((new_x / g_game.x_resolution) * 2) - 1;
	cur_y = ((new_y / g_game.y_resolution) * -2) + 1;

	ours = CargoList->DoMouse(type, cur_x, cur_y, button, state);
	if (ours == 1 && type == 1 &&state==0) {
		buy_name = CargoList->GetSelectedItemName();
		if (buy_name) {
		  if (buy_name[0]!='\0') {
		    if (0==strcmp(buy_name,"[Back To Categories]")) {
		      if (!curcategory.empty()) {
			curcategory.pop_back();		     
			SetupCargoList();
		      }
                    }else if (0==strcmp (buy_name,"Basic Repair")) {
		      static string repair_price = "Price: "+vs_config->getVariable("physics","repair_price","1000");
                      CargoInfo->ChangeTextItem ("name","Basic Repair");
		      CargoInfo->ChangeTextItem ("price",repair_price.c_str());
		      CargoInfo->ChangeTextItem("volume","Cargo Volume: N/A");
		      CargoInfo->ChangeTextItem("mass","Mass: N/A");
		      CargoInfo->ChangeTextItem("description","Hire starship mechanics to examine and assess any wear and tear on your craft. They will replace any damaged components on your vessel with the standard components of the vessel you initially purchased.  Further upgrades above and beyond the original will not be replaced free of charge.  The total assessment and repair cost applies if any components are damaged or need servicing (fuel, wear and tear on jump drive, etc...) If such components are damaged you may save money by repairing them on your own.");
                    }else {
		      if (buy_name[0]!='x') {
			  lastselected.type=type;
			  lastselected.x=cur_x;
			  lastselected.y=cur_y;
			  lastselected.button=button;
			  lastselected.state=state;
			  lastselected.last=true;
			  if (SelectItem (buy_name,button,state)) return;//changes state/side bar price depedning on submode

			//CargoInfo->ChangeTextItem("name", (string("name: ")+buy_name).c_str()); 
			//CargoInfo->ChangeTextItem("price", "Price: Random. Hah.");
		      }else {
			curcategory.push_back(buy_name+1);
			SetupCargoList();
		      }
		    }
		  }
		}
	}
	// Commented out because they don't need to use the mouse with CargoInfo
	//if (ours == 0) { ours = CargoInfo->DoMouse(type, cur_x, cur_y, button, state); }
	if (ours == 0) {
		ours = OK->DoMouse(type, cur_x, cur_y, button, state);
		drawovermouse=(ours==1);
		if (ours == 1 && type == 1) {
			printf ( "You clicked done\n");
                        DoDone();
						return;
                
                }
	}	
	if (ours == 0) {
		ours = COMMIT->DoMouse(type, cur_x, cur_y, 0/*button*/, state);
		drawovermouse=(ours==1);
		if (ours == 1 && type == 1) {
			buy_name = CargoList->GetSelectedItemName();
			if (buy_name) {
			  if (buy_name[0]) {
			    CommitItem (buy_name,button,0);
			  }
			}else {
			  if (submode==STOP_MODE&&mode==BRIEFINGMODE) {
			    StopBriefing();
			  }
			}
		}
	}
	if (upgr==NULL)
		return;
	for (int i=0;(Modes[i]!=0)&&(ours==0);i++) {
	  ours = Modes[i]->DoMouse(type,cur_x,cur_y,button,state);
	  drawovermouse=(ours==1);
	  if (ours==1&&type==1) {
	    SetMode ((UpgradingInfo::BaseMode)availmodes[i],NORMAL);
	  }

	}
}
extern string MakeUnitXMLPretty (string, Unit *);
string replaceNewline (string s) {
	for (string::iterator i= s.begin();i!=s.end();++i) {
		if (*i=='\n')
			*i='/';
	}
	return s;
}
std::string GetShipStats(Unit *un) {
	return replaceNewline(MakeUnitXMLPretty (un->WriteUnitString(),un));
}

std::string GetNumKills(Unit *un) {
//	return "You don't have any kills!\nMua ha ha ha ha ha ha\nJust kidding :-)";
	string text = "Relations";
	vector <float> * killlist = &_Universe->AccessCockpit()->savegame->getMissionData (string("kills"));
	if (killlist->size()>0) {
		text+=" | Kills";
	}
	text+="\\";
	int numfactions = FactionUtil::GetNumFactions();
	int i = 0;
	string factionname = "factionname";
	float relation = 0.0;

	while(i < numfactions)
	{
		factionname = FactionUtil::GetFactionName(i);
		relation = 	FactionUtil::GetIntRelation(i, ( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) )->faction );

		//	draw faction name
		text+=FactionUtil::GetFactionName(i)+"  ";
		relation = relation * 0.5;
		relation = relation + 0.5;
		int percent = (int)(relation * 100.0);
		text+=(XMLSupport::tostring (percent));
		if (i<killlist->size()) {
			text+=" | ";
			text += XMLSupport::tostring ((int)(*killlist)[i]);
		}
		text+="\\";
		i+=1;
	}
	if (i<killlist->size()) {
		text+=("Total Kills: ");
		relation=1;
		text += XMLSupport::tostring ((int)(*killlist)[i]);							
	}
	return text;

}

vector <CargoColor>&UpgradingInfo::MakeActiveMissionCargo() {
  TempCargo.clear();
#ifdef USE_BRIEFINGS
  for (unsigned int i=0;i<active_missions.size();i++) {
    CargoColor c;
    c.cargo.quantity=1;
    c.cargo.volume=1;
    c.cargo.price=0;
    c.cargo.content=XMLSupport::tostring((int)i)+" "+active_missions[i]->getVariable("mission_name", string("Mission"));
    c.cargo.category=string("briefings");
    TempCargo.push_back (c);
  }
#else
  CargoColor c;
  c.cargo.quantity=1;
  c.cargo.volume=1;
  c.cargo.price=0;
  c.cargo.description=GetShipStats(buyer.GetUnit());
  c.cargo.category="mystats";
  c.cargo.content="Ship_Stats";
  c.color=GFXColor(.6,0,1,1);
  TempCargo.push_back(c);
  c.cargo.description=GetNumKills(buyer.GetUnit());
  c.cargo.content="Factions_And_Kills";
  c.color=GFXColor(1,.8,0,1);
  TempCargo.push_back(c);
/*
  c.cargo.description="Unimplemented";
  c.color=GFXColor(1,0,0,1);
  c.cargo.content="Factions";
  TempCargo.push_back(c);
*/
#endif
  return TempCargo;
}

class CargoColorSort {
public:
	bool operator () (const CargoColor & a, const CargoColor&b) {
		return a.cargo<b.cargo;
	}
};
vector <CargoColor>&UpgradingInfo::MakeMissionsFromSavegame(Unit *base) {
  static bool miss_from_cargolist=XMLSupport::parse_bool(vs_config->getVariable("cargo","missions_from_cargolist","false"));
  if (miss_from_cargolist) {
    return FilterCargo (base,"missions",true,true);
  }
  TempCargo.clear();
  Unit *un=_Universe->AccessCockpit()->GetParent();
  int playernum=UnitUtil::isPlayerStarship(un);
  if (playernum<0) {
	  VSFileSystem::vs_fprintf(stderr,"docked ship not a player");
	  TempCargo.clear();
	  return TempCargo;
  }
  int len=getSaveStringLength(playernum,miss_script);
  assert(len==getSaveStringLength(playernum,miss_name)&&len==getSaveStringLength(playernum,miss_desc));
  unsigned int i=0;

  for (i=0;i<len;i++) {
	  string m = getSaveString(playernum,miss_name,i);
	  int count=1;
	  for (unsigned int j=i+1;j<len;++j) {
		  string n = getSaveString(playernum,miss_name,j);
		  if (n==m) {
				putSaveString(playernum,miss_name,i,n+"_"+tostring(count++));
		  }
	  }
  }
  for (i=0;i<len;i++) {
    CargoColor c;
    c.cargo.quantity=1;
    c.cargo.volume=1;
    c.cargo.price=0;
    c.cargo.content=getSaveString(playernum,miss_name,i);
	c.cargo.description=getSaveString(playernum,miss_desc,i);
	string::size_type index=c.cargo.content.rfind("/");
	if (index==std::string::npos) {
		c.cargo.category=std::string("missions");
	} else {
		c.cargo.category=std::string("missions/"+c.cargo.content.substr(0,index));
	}
//	if ((index+1)<c.cargo.content.size())
//		c.cargo.content=c.cargo.content.substr(index+1);
    TempCargo.push_back (c);
  }
  std::sort(TempCargo.begin(),TempCargo.end(),CargoColorSort());
  //for (unsigned int i=0;i<TempCargo.size();i++) {
  //  for (unsigned int nexti=i+1;nexti<TempCargo.size()&&TempCargo[i].cargo.content==TempCargo[nexti].cargo.content;nexti++) {
//		TempCargo[nexti].cargo.content+="_"+XMLSupport::tostring(nexti-i);
 //   }
 // }
  return TempCargo;
}


vector <CargoColor>&UpgradingInfo::FilterCargo(Unit *un, const string filterthis, bool inv, bool removezero){
  TempCargo.clear();
    for (unsigned int i=0;i<un->numCargo();i++) {
      unsigned int len = un->GetCargo(i).category.length();
      len = len<filterthis.length()?len:filterthis.length();
      if ((0==memcmp(un->GetCargo(i).category.c_str(),filterthis.c_str(),len))==inv) {//only compares up to category...so we could have starship_blue
		  if ((!removezero)||un->GetCargo(i).quantity>0) {
		    if (!un->GetCargo(i).mission) {
			CargoColor col;
			col.cargo=un->GetCargo(i);
			TempCargo.push_back (col);
		    }
		  }
      }
    }
    return TempCargo;
}
Cargo GetCargoForOwnerStarship (Cockpit * cp, int i) {
  Cargo c;
  c.quantity=1;
  c.volume=1;
  c.price=0;
    bool hike_price=true;
    if (i+1<cp->unitfilename.size()) {
      if (cp->unitfilename[i+1]==_Universe->activeStarSystem()->getFileName()) {
	hike_price=false;
      }
    }
    if (hike_price) {
      static float shipping_price = XMLSupport::parse_float (vs_config->getVariable ("physics","shipping_price","6000"));
      c.price=shipping_price;
    }
    c.content=cp->unitfilename[i];
    c.category=string("starships/My_Fleet");  
    return c;
}
Cargo GetCargoForOwnerStarshipName (Cockpit * cp, std::string nam, int & ind) {
  for (unsigned int i=1;i<cp->unitfilename.size();i+=2) {
    if (cp->unitfilename[i]==nam) {
      ind = i;
      return GetCargoForOwnerStarship (cp,i);
    }
  }
  return Cargo();//no name;
}

void AddOwnerStarships (Cockpit * cp, vector <CargoColor> &l) {
  for (unsigned int i=1;i<cp->unitfilename.size();i+=2) {
    CargoColor c;
    c.cargo=GetCargoForOwnerStarship(cp,i);
    l.push_back(c);
  }
}
vector <CargoColor>&UpgradingInfo::GetCargoFor(Unit *un) {//un !=NULL
    switch (mode) {
    case BUYMODE:
    case SELLMODE:
      return FilterCargo (un,"missions",false,true);//anything but a mission
    case UPGRADEMODE:
    case ADDMODE:
      //      curcategory.clear();
      //      curcategory.push_back(string("upgrades"));
      return FilterCargo (un,"upgrades",true,true);
    case DOWNGRADEMODE:
      return FilterCargo (un,"upgrades",true,false);
    case SHIPDEALERMODE:
      //      curcategory.clear();
      //      curcategory.push_back(string("starships"));
      {
      vector <CargoColor> * pntr =&FilterCargo (un,"starships",true,true);
      AddOwnerStarships (_Universe->AccessCockpit(),*pntr);
      return *pntr;
      }
    case MISSIONMODE:
      //      curcategory.clear();
      //      curcategory.push_back(string("missions"));
//      return FilterCargo (un,"missions",true,true);
      return MakeMissionsFromSavegame (un);
    case NEWSMODE:
      return TempCargo;
    case BRIEFINGMODE:
      //      curcategory.clear();
      //      curcategory.push_back (string("briefings"));
      return MakeActiveMissionCargo ();
    }
    VSFileSystem::vs_fprintf (stderr,"Error in picking cargo lists");
    return TempCargo;
  }
vector <CargoColor>&UpgradingInfo::GetCargoList () {
    static vector <CargoColor> Nada;//in case the units got k1ll3d.
    Unit * relevant=NULL;
    switch (mode) {
    case BUYMODE:
		relevant=base.GetUnit();
	    break;
    case UPGRADEMODE:
    case ADDMODE:
      relevant = base.GetUnit();
      if (buyer.GetUnit()) {
	if (relevant) {
	  return buyer.GetUnit()->FilterUpgradeList(GetCargoFor (relevant));
	}
      }
      break;
    case SHIPDEALERMODE:
    case MISSIONMODE://gotta transform the missions into cargo
#ifdef USE_BRIEFINGS
    case BRIEFINGMODE:
#endif
      relevant = base.GetUnit();
      break;
#ifndef USE_BRIEFINGS
    case BRIEFINGMODE:
#endif
    case SELLMODE:

      relevant = buyer.GetUnit();
      break;
    case DOWNGRADEMODE:
      relevant = &GetUnitMasterPartList();
      if (buyer.GetUnit()) {
	GetCargoFor (relevant);
	vector <CargoColor> tmp;
	for (unsigned int i=0;i<TempCargo.size();i++) {
	  if (match(curcategory.begin(),curcategory.end(),
		    TempCargo[i].cargo.category.begin(),TempCargo[i].cargo.category.end(),false)) {
	    tmp.push_back (TempCargo[i]);
	  }
	}
	TempCargo = tmp;
    ClearDowngradeMap();
	vector <CargoColor>&mylist= buyer.GetUnit()->FilterDowngradeList (TempCargo);
    std::set<std::string> downgrademap =  GetListOfDowngrades();
    static bool cleardowngrades = XMLSupport::parse_bool (vs_config->getVariable ("physics","only_show_best_downgrade","true"));
    if (cleardowngrades)
    for (unsigned int i=0;i<mylist.size();++i) {
        if (downgrademap.find (mylist[i].cargo.content)==downgrademap.end()) {
            mylist.erase(mylist.begin()+ i);
            i--;
            continue;
        }
    }
    return mylist;
      }
      break;
    }
    if (relevant) {
      return GetCargoFor (relevant);
    }else {
      return Nada;
    }
  }

