#include "unit.h"
#include "unit_factory.h"
#include "gui/text_area.h"
#include "gui/button.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "main_loop.h"
#include "images.h"
#include <algorithm>
#include "gfx/cockpit.h"
#include "savegame.h"
#include "cmd/script/mission.h"
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#ifdef _WIN32
#define strcasecmp stricmp
#endif
int GetModeFromName (const char * input_buffer) {
      if (strlen (input_buffer)>3) {
	if (input_buffer[0]=='a'&&
	    input_buffer[1]=='d'&&
	    input_buffer[2]=='d') {
	  return 1;
	}
	if (input_buffer[0]=='m'&&
	    input_buffer[1]=='u'&&
	    input_buffer[2]=='l') {
	  return 2;
	}
      }
      return 0;
}
using std::string;

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
  return .66*percentage;
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
extern void LoadMission (const char *, bool loadfirst);
extern void SwitchUnits (Unit * ol, Unit * nw);
extern Cargo * GetMasterPartList(const char *input_buffer);
extern Unit&GetUnitMasterPartList();
class UpgradingInfo {
public:
  Mission * briefingMission;//do not dereference! instead scan through activve_missions
  TextArea *CargoList, *CargoInfo;
  Button *OK, *COMMIT;
  UnitContainer base;
  UnitContainer buyer;
  //below are state variables while the user is selecting mounts
  Unit * NewPart;
  Unit * templ;
  Cargo part;
  int selectedmount;
  int selectedturret;
  //end it
  struct LastSelected{int type; float x; float y; int button; int state;bool last;LastSelected() {last=false;}} lastselected;
  void ProcessMouse(int type, int x, int y, int button, int state);

  vector <Cargo> TempCargo;//used to store cargo list
  vector <Cargo> * CurrentList;
  enum SubMode {NORMAL,MOUNT_MODE,SUBUNIT_MODE, CONFIRM_MODE, STOP_MODE}submode;
  enum BaseMode {BUYMODE,SELLMODE,MISSIONMODE,BRIEFINGMODE,NEWSMODE,SHIPDEALERMODE,UPGRADEMODE,ADDMODE,DOWNGRADEMODE, MAXMODE} mode;
  bool multiplicitive;
  Button *Modes[MAXMODE];
  string title;
  vector <string> curcategory;
  vector <Cargo>&FilterCargo(Unit *un, const string filterthis, bool inv, bool removezero);
  vector <Cargo>&GetCargoFor(Unit *un);
  vector <Cargo>&GetCargoList ();
  vector <Cargo>&MakeActiveMissionCargo();
  void StopBriefing();
  void SetupCargoList() {
    CurrentList = &GetCargoList();
    //    std::sort (CurrentList->begin(),CurrentList->end());
    CargoList->ClearList();
    if (submode==NORMAL) {
      if (mode==NEWSMODE) {
	gameMessage * last;
	int i=0;
	vector <std::string> who;
	who.push_back ("news");
	while ((last= mission->msgcenter->last(i++,who))!=NULL) {
	  CargoList->AddTextItem ((tostring(i-1)+" "+last->message).c_str(),last->message.c_str());
	}

      }else {
	if (!curcategory.empty()) {
	  if (mode==BUYMODE||mode==SELLMODE||curcategory.size()>1) 
	    CargoList->AddTextItem ("[Back To Categories]","[Back To Categories]");
	  for (unsigned int i=0;i<CurrentList->size();i++) {
	    if (match(curcategory.begin(),curcategory.end(),(*CurrentList)[i].category.begin(),(*CurrentList)[i].category.end(),true)) {
	      CargoList->AddTextItem ((tostring((int)i)+ string(" ")+(*CurrentList)[i].content).c_str() ,((*CurrentList)[i].content+"("+tostring((*CurrentList)[i].quantity)+")").c_str());
	    }
	  }
	}
	string curcat=("");
	CargoList->AddTextItem ("","");
	for (unsigned int i=0;i<CurrentList->size();i++) {
	  string curlist  ((*CurrentList)[i].category);
	  string lev=getLevel (curlist,curcategory.size());
	  if (match (curcategory.begin(),curcategory.end(),curlist.begin(),curlist.end(),false)&&
	      (!match (curcategory.begin(),curcategory.end(),curlist.begin(),curlist.end(),true))&&
	      lev!=curcat) {
	    CargoList->AddTextItem ((string("x")+lev).c_str(),beautify(lev).c_str());
	    curcat =lev;
	  }
	}
      }      
      
    }else {
      Unit * un = buyer.GetUnit();
      int i=0;
      un_iter ui;
      if (un) {
	switch (submode) {
	case MOUNT_MODE:
	  for (;i<un->nummounts;i++) {
	    if (un->mounts[i].status==Unit::Mount::ACTIVE||un->mounts[i].status==Unit::Mount::INACTIVE)
	      CargoList->AddTextItem ((tostring(i)+un->mounts[i].type->weapon_name).c_str(),un->mounts[i].type->weapon_name.c_str());
	    else 
	      CargoList->AddTextItem ((tostring(i)+" [Empty]").c_str(),"[Empty]");
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
  bool beginswith (const vector <std::string> &cat, const std::string &s) {
    if (cat.empty()) {
      return false;
    }
    return cat.front()==s;
  }
  void SetMode (enum BaseMode mod, enum SubMode smod) {
    bool resetcat=false;
    if (mod!=mode) {
      curcategory.clear();
      resetcat=true;
    }
    string ButtonText;
    switch (mod) {
    case BRIEFINGMODE:
      title="Mission Briefings:: Select Mission";
      ButtonText="EndBriefing";
      break;
    case NEWSMODE:
      title="GNN Galaxy News Network";
      ButtonText="AbsorbNews";
      break;
    case BUYMODE:
      title = "Purchase Cargo Mode";
      ButtonText= "BuyCargo";
      break;
    case SELLMODE:
      title = "Sell Cargo Mode";
      ButtonText= "SellCargo";
      break;
    case UPGRADEMODE:
      title = "Upgrade/Repair Starship";
      ButtonText="Upgrade";
      if (!beginswith (curcategory,"upgrades")) {
	curcategory.clear();
	curcategory.push_back("upgrades");
      }
      break;
    case ADDMODE:
      title = "Enhance Starship Mode";
      ButtonText="Add Stats";
      if (!beginswith (curcategory,"upgrades")) {
	curcategory.clear();
	curcategory.push_back("upgrades");
      }
      break;
    case DOWNGRADEMODE:
      title = "Downgrade Starship Mode";
      ButtonText= "SellPart";
      if (!beginswith (curcategory,"upgrades")) {
	curcategory.clear();
	curcategory.push_back("upgrades");
      }
      break;
    case MISSIONMODE:
      title = "Mission BBS";
      ButtonText="Accept";
      if (!beginswith (curcategory,"missions")) {
	curcategory.clear();
	curcategory.push_back("missions");
      }
      break;
    case SHIPDEALERMODE:
      title = "Purchase Starship";
      ButtonText="BuyShip";
      if (!beginswith (curcategory,"starships")) {
	curcategory.clear();
	curcategory.push_back("starships");
      }
      break;
    }
    if (smod!=NORMAL) {
      switch (smod) {
      case MOUNT_MODE:
	title="Select On Which Mount To Place Your Weapon";
	break;
      case SUBUNIT_MODE:
	title="Select On Which Turret Mount To Place Your Turret";
	break;
      case CONFIRM_MODE:
	title="This may not entirely fit on your ship. Are You Sure you wish to proceed?";
	break;
      case STOP_MODE:
	title="Viewing Mission Briefing. Press EndBriefing to exit.";
	break;
      }
    }
    COMMIT->ModifyName (ButtonText.c_str());
    mode = mod;
    submode = smod;
    SetupCargoList();
  }
  UpgradingInfo(Unit * un, Unit * base):base(base),buyer(un),mode(BUYMODE),title("Buy Cargo"){
    
	CargoList = new TextArea(-1, 0.9, 1, 1.7, 1);
	CargoInfo = new TextArea(0, 0.9, 1, 1.7, 0);
	CargoInfo->DoMultiline(1);
	Cockpit * cp = _Universe->isPlayerStarship(un);
	briefingMission=NULL;
	Cockpit * tmpcockpit = _Universe->AccessCockpit();
	if (cp) {
	  _Universe->SetActiveCockpit(cp);
	}
	NewPart=NULL;//no ship to upgrade
	templ=NULL;//no template
	//	CargoList->AddTextItem("a","Just a test item");
	//	CargoList->AddTextItem("b","And another just to be sure");
	CargoInfo->AddTextItem("name", "");
	CargoInfo->AddTextItem("price", "");
	CargoInfo->AddTextItem("mass", "");
	CargoInfo->AddTextItem("volume", "");
	CargoInfo->AddTextItem("description", "");
	OK = new Button(-0.94, -0.85, 0.15, 0.1, "Done");
	COMMIT = new Button(-0.75, -0.85, 0.25, 0.1, "Buy");
	const char  MyButtonModes[][128] = {"BuyMode","SellMode","MissionBBS","Briefing","GNN News", "ShipDealer","UpgradeShip","Unimplemented","Downgrade"};
	float beginx = -.4;
	float lastx = beginx;
	float size=.32;
	for (int i=0;i<MAXMODE;i++) {
	  if (i!=ADDMODE) {
	    if (i<MAXMODE/2) {
	      Modes[i]= new Button (lastx,-.82,size,0.07,MyButtonModes[i]);
	    }else {
	      Modes[i]= new Button (beginx,-.91,size,0.07,MyButtonModes[i]);
	      beginx+=size+.04;
	    }
	    lastx+=size+.04;
	  }else {
	    Modes[i]=new Button (0,0,0,0,"Unimplemented");
	  }
	}
	CargoList->RenderText();
	CargoInfo->RenderText();	
	SetMode (BUYMODE,NORMAL);
	_Universe->SetActiveCockpit(tmpcockpit);
  }
  ~UpgradingInfo() {
    if (templ){
      templ->Kill();
      templ=NULL;
    }
    if (NewPart) {
      NewPart->Kill();
      NewPart=NULL;
    }
    base.SetUnit(NULL);
    buyer.SetUnit(NULL);
    delete CargoList;
    delete CargoInfo;
    for (int i=0;i<MAXMODE;i++) {
      delete Modes[i];
    }
  }
  void Render(){
    //    GFXSubwindow (0,0,g_game.x_resolution,g_game.y_resolution);
    Unit * un = buyer.GetUnit(); 
    if (un) {
      Cockpit * cp = _Universe->isPlayerStarship(un);
      if (cp)
	_Universe->SetActiveCockpit(cp);
    }
    bool render=true;

    TextPlane * tp=NULL;
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
    StartGUIFrame(mode==BRIEFINGMODE?GFXFALSE:GFXTRUE);
    if (render) {
      // Black background
      ShowColor(-1,-1,2,2, 0,0,0,1);
      ShowColor(0,0,0,0, 1,1,1,1);
      char floatprice [100];
      sprintf(floatprice,"%.2f",_Universe->AccessCockpit()->credits);
      ShowText(-0.98, 0.93, 2, 4, (title+ string(" Credits: ")+floatprice).c_str(), 0);
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
    for (unsigned int i=0;i<MAXMODE;i++) {
      Modes[i]->Refresh();
    }
    EndGUIFrame();
  }
  void SelectLastSelected();
  void SelectItem (const char * str, int button, int state);
  void CommitItem (const char * str, int button, int state);
  //this function is called after the mount is selected and stored in selected mount
  void CompleteTransactionAfterMountSelect();
  //this function is called after the turret is selected and stored in selected turret
  void CompleteTransactionAfterTurretSelect();
  void CompleteTransactionConfirm();
};
vector <UpgradingInfo *>upgr;
vector <unsigned int> player_upgrading;

bool RefreshGUI(void) {
  bool retval=false;
  for (unsigned int i=0;i<upgr.size();i++) { 
    if (player_upgrading[i]==_Universe->CurrentCockpit()){
      retval=true;
      upgr[i]->Render();
    }
  }
  return retval;
}

static void ProcessMouseClick(int button, int state, int x, int y) {
  SetSoftwareMousePosition (x,y);
  for (unsigned int i=0;i<upgr.size();i++) { 
    if (player_upgrading[i]==_Universe->CurrentCockpit())
      upgr[i]->ProcessMouse(1, x, y, button, state);
  }


}

static void ProcessMouseActive(int x, int y) {
  SetSoftwareMousePosition (x,y);
  for (unsigned int i=0;i<upgr.size();i++) { 
    if (player_upgrading[i]==_Universe->CurrentCockpit())
      upgr[i]->ProcessMouse(2, x, y, 0, 0);
  }

}

static void ProcessMousePassive(int x, int y) {
  SetSoftwareMousePosition(x,y);
  for (unsigned int i=0;i<upgr.size();i++) { 
    if (player_upgrading[i]==_Universe->CurrentCockpit())
      upgr[i]->ProcessMouse(3, x, y, 0, 0);
  }
}
void Unit::UpgradeInterface(Unit * base) {
  for (unsigned int i=0;i<upgr.size();i++) {
    if (upgr[i]->buyer.GetUnit()==this) {
      return;//too rich for my blood...don't let 2 people buy cargo for 1
    }
  }
  printf("Starting docking\n");
  winsys_set_mouse_func(ProcessMouseClick);
  winsys_set_motion_func(ProcessMouseActive);
  winsys_set_passive_motion_func(ProcessMousePassive);
  //(x, y, width, height, with scrollbar)
  upgr.push_back( new UpgradingInfo (this,base));
  player_upgrading.push_back(_Universe->CurrentCockpit());
  
}

void CargoToMission (const char * item,TextArea * ta) {
  char * item1 = strdup (item);
  int tmp;
  sscanf (item,"%d %s",&tmp,item1);
  Mission temp (item1,false);
  free (item1);
  temp.initMission(false);
  ta->ChangeTextItem ("name",temp.getVariable ("mission_name","").c_str());
  ta->ChangeTextItem ("price","");
  ta->ChangeTextItem ("mass","");
  ta->ChangeTextItem ("volume","");
  ta->ChangeTextItem ("description",temp.getVariable("description","").c_str());  
}

void UpgradingInfo::SelectItem (const char *item, int button, int buttonstate) {
	char floatprice [100];
  switch (mode) {
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
  case BUYMODE:
  case SELLMODE:
  case UPGRADEMODE:
  case ADDMODE:
  case DOWNGRADEMODE:    
  case SHIPDEALERMODE:
    switch (submode) {
    case NORMAL:
      {
	int cargonumber;
	sscanf (item,"%d",&cargonumber);
	CargoInfo->ChangeTextItem ("name",(*CurrentList)[cargonumber].content.c_str());
	sprintf(floatprice,"Price: %.2f",(*CurrentList)[cargonumber].price);
	CargoInfo->ChangeTextItem ("price",floatprice);
	sprintf(floatprice,"Mass: %.2f",(*CurrentList)[cargonumber].mass);
	CargoInfo->ChangeTextItem ("mass",floatprice);
	sprintf(floatprice,"Cargo Volume: %.2f",(*CurrentList)[cargonumber].volume);
	CargoInfo->ChangeTextItem ("volume",floatprice);
	if ((*CurrentList)[cargonumber].description!=NULL) {
	  CargoInfo->ChangeTextItem ("description",(*CurrentList)[cargonumber].description,true);
	}else {
	  CargoInfo->ChangeTextItem ("description","");
	}
      }
      break;
    default:
      CommitItem (item,0,buttonstate);
      break;
    }
    break;
  case NEWSMODE:
    {
	int cargonumber;
	sscanf (item,"%d",&cargonumber);
     	gameMessage * last;
	vector <std::string> who;
	CargoInfo->ChangeTextItem ("name","");
	who.push_back ("news");
	if ((last= mission->msgcenter->last(cargonumber,who))!=NULL) {
	  CargoInfo->ChangeTextItem ("description",last->message.c_str(),true);
	} 
	CargoInfo->ChangeTextItem ("price","");
	CargoInfo->ChangeTextItem ("mass","");
	CargoInfo->ChangeTextItem ("volume","");

    }
    break;
  case MISSIONMODE:
    CargoToMission (item,CargoInfo);
    break;
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
void UpgradingInfo::CommitItem (const char *inp_buf, int button, int state) {
  Unit * un;
  Unit * base;
  unsigned int offset;
  int quantity=(button==0)?1:(button==1?10000:10);
  int index;
  char * input_buffer = strdup (inp_buf);
  sscanf (inp_buf,"%d %s",&index,input_buffer);
  if (templ!=NULL) {
    templ->Kill();
    templ=NULL;
  }
  if (state==0&&(un=buyer.GetUnit())&&(base=this->base.GetUnit())) {
  
  switch (mode) {

  case UPGRADEMODE:
  case ADDMODE:
  case DOWNGRADEMODE:    
  case SHIPDEALERMODE:
    {

      char *unitdir =GetUnitDir(un->name.c_str());

      Unit * temprate= UnitFactory::createUnit ((string(unitdir)+string(".template")).c_str(),true,un->faction);
      free(unitdir);
      if (temprate->name!=string("LOAD_FAILED")) {
	templ=temprate;
      }else {
	templ=NULL;
	temprate->Kill();
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
	    NewPart->Kill();
	  if (0==strcmp (input_buffer,"repair")) {
	    free (input_buffer);
	    char *unitdir =GetUnitDir(un->name.c_str());
	    input_buffer = strdup ((string(unitdir)+string(".blank")).c_str());
	    free(unitdir);
	  }
	  NewPart = UnitFactory::createUnit (input_buffer,true,_Universe->GetFaction("upgrades"));
	  NewPart->SetFaction(un->faction);
	  if (NewPart->name==string("LOAD_FAILED")) {
	    NewPart->Kill();
	    NewPart = UnitFactory::createUnit (input_buffer,true,un->faction);
	  }
	  if (NewPart->name!=string("LOAD_FAILED")) {
	    
	    float usedprice = usedPrice (base->PriceCargo (_Universe->AccessCockpit()->GetUnitFileName()));
	    if (mode==SHIPDEALERMODE&&part->price<=usedprice+_Universe->AccessCockpit()->credits) {
	      if (NewPart->nummesh>0) {
		_Universe->AccessCockpit()->credits-=part->price-usedprice;
		NewPart->curr_physical_state=un->curr_physical_state;
		NewPart->prev_physical_state=un->prev_physical_state;
		_Universe->activeStarSystem()->AddUnit (NewPart);
		
		_Universe->AccessCockpit()->SetParent(NewPart,input_buffer,"",un->curr_physical_state.position);//absolutely NO NO NO modifications...you got this baby clean off the slate
		SwitchUnits (NULL,NewPart);
		base->RequestClearance(NewPart);
		NewPart->Dock(base);
		buyer.SetUnit(NewPart);
		_Universe->AccessCockpit()->savegame->SetSavedCredits (_Universe->AccessCockpit()->credits);
		NewPart=NULL;
		un->Kill();
		return;
	      }
	    }
	    if (mode!=SHIPDEALERMODE) {
	      selectedmount=0;
	      selectedturret=0;
	      if (NewPart->nummounts) {
		SetMode(mode,MOUNT_MODE);
	      }else {
		CompleteTransactionAfterMountSelect();
	      }
	    }
	  } else {
	    NewPart->Kill();
	    NewPart=NULL;
	  }
	} else {
	  if (NewPart) {
	    if (NewPart->name==string("LOAD_FAILED")) {
	      NewPart->Kill();
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
    LoadMission (input_buffer,false);
    if ((un=this->base.GetUnit())) {
      unsigned int index;
      if (NULL!=un->GetCargo(input_buffer,index)) {
	un->RemoveCargo(index,1,true);
      }
    }
    break;

  }
  }
  free (input_buffer);
}


void UpgradingInfo::CompleteTransactionAfterMountSelect() {
    if (NewPart->getSubUnits().current()!=NULL) {
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
      canupgrade = un->canDowngrade (NewPart,mountoffset,subunitoffset,percentage);
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
      char *buy_name = CargoList->GetSelectedItemName();
      
      if (buy_name) {
	if (buy_name[0]) {
	  //not sure
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
  if ((un=buyer.GetUnit())) {
    switch (mode) {
    case UPGRADEMODE:
    case ADDMODE:
      canupgrade =un->canUpgrade (NewPart,mountoffset,subunitoffset,mode==ADDMODE,true,percentage,templ);
      price =(float)(part.price*(1-usedPrice(percentage)));
      if ((_Universe->AccessCockpit()->credits>price)) {
	_Universe->AccessCockpit()->credits-=price;
	un->Upgrade (NewPart,mountoffset,subunitoffset,mode==ADDMODE,true,percentage,templ);
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
      canupgrade = un->canDowngrade (NewPart,mountoffset,subunitoffset,percentage);
      price =part.price*usedPrice(percentage);
      _Universe->AccessCockpit()->credits+=price;
      if (un->Downgrade (NewPart,mountoffset,subunitoffset,percentage)) {
      if ((bas=base.GetUnit())) {
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
		    }else {
		      if (buy_name[0]!='x') {
			  lastselected.type=type;
			  lastselected.x=cur_x;
			  lastselected.y=cur_y;
			  lastselected.button=button;
			  lastselected.state=state;
			  lastselected.last=true;
			  SelectItem (buy_name,button,state);//changes state/side bar price depedning on submode

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
		if (ours == 1 && type == 1) {
			printf ( "You clicked done\n");
			for (unsigned int i=0;i<upgr.size();i++) {
			  if (upgr[i]==this) {
			    delete upgr[i];
			    upgr.erase (upgr.begin()+i);
			    player_upgrading.erase (player_upgrading.begin()+i);
			    restore_main_loop();
			  }
			}
		}
	}	
	if (ours == 0) {
		ours = COMMIT->DoMouse(type, cur_x, cur_y, 0/*button*/, state);
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
	for (int i=0;i<UpgradingInfo::MAXMODE&&ours==0;i++) {
	  ours = Modes[i]->DoMouse(type,cur_x,cur_y,button,state);
	  if (ours==1&&type==1) {
	    SetMode ((UpgradingInfo::BaseMode)i,NORMAL);
	  }

	}
}







vector <Cargo>&UpgradingInfo::MakeActiveMissionCargo() {
  TempCargo.clear();
  for (unsigned int i=0;i<active_missions.size();i++) {
    Cargo c;
    c.quantity=1;
    c.volume=1;
    c.price=0;
    c.content=XMLSupport::tostring((int)i)+" "+active_missions[i]->getVariable("mission_name", string("Mission"));
    c.category=string("briefings");
    TempCargo.push_back (c);
  }
  return TempCargo;
}


vector <Cargo>&UpgradingInfo::FilterCargo(Unit *un, const string filterthis, bool inv, bool removezero){
  TempCargo.clear();
    for (unsigned int i=0;i<un->numCargo();i++) {
      unsigned int len = un->GetCargo(i).category.length();
      len = len<filterthis.length()?len:filterthis.length();
      if ((0==memcmp(un->GetCargo(i).category.c_str(),filterthis.c_str(),len))==inv) {//only compares up to category...so we could have starship_blue
		  if ((!removezero)||un->GetCargo(i).quantity>0) {
			TempCargo.push_back (un->GetCargo(i));
		  }
      }
    }
    return TempCargo;
}
vector <Cargo>&UpgradingInfo::GetCargoFor(Unit *un) {//un !=NULL
    switch (mode) {
    case BUYMODE:
    case SELLMODE:
      return FilterCargo (un,"missions",false,true);//anything but a mission
    case UPGRADEMODE:
    case DOWNGRADEMODE:
    case ADDMODE:
      //      curcategory.clear();
      //      curcategory.push_back(string("upgrades"));
      return FilterCargo (un,"upgrades",true,false);
    case SHIPDEALERMODE:
      //      curcategory.clear();
      //      curcategory.push_back(string("starships"));
      return FilterCargo (un,"starships",true,true);
    case MISSIONMODE:
      //      curcategory.clear();
      //      curcategory.push_back(string("missions"));
      return FilterCargo (un,"missions",true,true);
    case NEWSMODE:
      return TempCargo;
    case BRIEFINGMODE:
      //      curcategory.clear();
      //      curcategory.push_back (string("briefings"));
      return MakeActiveMissionCargo ();
    }
    fprintf (stderr,"Error in picking cargo lists");
    return TempCargo;
  }
vector <Cargo>&UpgradingInfo::GetCargoList () {
    static vector <Cargo> Nada;//in case the units got k1ll3d.
    Unit * relevant;
    switch (mode) {
    case BUYMODE:
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
      relevant = base.GetUnit();
      break;
    case SELLMODE:
      relevant = buyer.GetUnit();
      break;
    case DOWNGRADEMODE:
      relevant = &GetUnitMasterPartList();
      if (buyer.GetUnit()) {
	GetCargoFor (relevant);
	vector <Cargo> tmp;
	for (unsigned int i=0;i<TempCargo.size();i++) {
	  if (match(curcategory.begin(),curcategory.end(),
		    TempCargo[i].category.begin(),TempCargo[i].category.end(),false)) {
	    tmp.push_back (TempCargo[i]);
	  }
	}
	TempCargo = tmp;
	return buyer.GetUnit()->FilterDowngradeList (TempCargo);
      }
      break;
    }
    if (relevant) {
      return GetCargoFor (relevant);
    }else {
      return Nada;
    }
  }
