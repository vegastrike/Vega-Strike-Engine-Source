#include "unit.h"
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

static float usedPrice (float percentage) {
  return .66*percentage;
}

extern void LoadMission (const char *, bool loadfirst);
extern void SwitchUnits (Unit * ol, Unit * nw);
extern Cargo * GetMasterPartList(const char *input_buffer);
extern Unit&GetUnitMasterPartList();
struct UpgradingInfo {
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
  enum SubMode {NORMAL,MOUNT_MODE,SUBUNIT_MODE, CONFIRM_MODE}submode;
  enum BaseMode {BUYMODE,SELLMODE,MISSIONMODE,UPGRADEMODE,ADDMODE,DOWNGRADEMODE,SHIPDEALERMODE, NEWSMODE,MAXMODE} mode;
  bool multiplicitive;
  Button *Modes[MAXMODE];
  string title;
  string curcategory;
  vector <Cargo>&FilterCargo(Unit *un, const string filterthis, bool inv, bool removezero);
  vector <Cargo>&GetCargoFor(Unit *un);
  vector <Cargo>&GetCargoList ();
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
	if (curcategory.length()!=0) {
	  if (mode==BUYMODE||mode==SELLMODE) 
	    CargoList->AddTextItem ("[Back To Categories]","[Back To Categories]");
	  for (unsigned int i=0;i<CurrentList->size();i++) {
	    if ((*CurrentList)[i].category==curcategory)
	      CargoList->AddTextItem ((tostring((int)i)+ string(" ")+(*CurrentList)[i].content).c_str() ,((*CurrentList)[i].content+"("+tostring((*CurrentList)[i].quantity)+")").c_str());
	  }
	} else {
	  string curcat=("");
	  CargoList->AddTextItem ("","");
	  for (unsigned int i=0;i<CurrentList->size();i++) {
	    if ((*CurrentList)[i].category!=curcat) {
	      CargoList->AddTextItem ((*CurrentList)[i].category.c_str(),(*CurrentList)[i].category.c_str());
	      curcat =((*CurrentList)[i].category);
	    }
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
	      CargoList->AddTextItem ((tostring(i)+un->mounts[i].type.weapon_name).c_str(),un->mounts[i].type.weapon_name.c_str());
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
  void SetMode (enum BaseMode mod, enum SubMode smod) {
    if (mod!=mode)
      curcategory="";
    string ButtonText;
    switch (mod) {
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
      break;
    case ADDMODE:
      title = "Enhance Starship Mode";
      ButtonText="Add Stats";
      break;
    case DOWNGRADEMODE:
      title = "Downgrade Starship Mode";
      ButtonText= "SellPart";
      break;
    case MISSIONMODE:
      title = "Mission BBS";
      ButtonText="Accept";
      break;
    case SHIPDEALERMODE:
      title = "Purchase Starship";
      ButtonText="BuyShip";
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
	const char  MyButtonModes[][128] = {"BuyMode","SellMode","MissionBBS","UpgradeShip","Unimplemented","Downgrade", "ShipDealer","GNN News"};
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
	StartGUIFrame();
	// Black background
	ShowColor(-1,-1,2,2, 0,0,0,1);
	ShowColor(0,0,0,0, 1,1,1,1);
	char floatprice [100];
	sprintf(floatprice,"%.2f",_Universe->AccessCockpit()->credits);
	ShowText(-0.98, 0.93, 2, 4, (title+ string(" Credits: ")+floatprice).c_str(), 0);
	CargoList->Refresh();
	CargoInfo->Refresh();
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
  glutMouseFunc(ProcessMouseClick);
  glutMotionFunc(ProcessMouseActive);
  glutPassiveMotionFunc(ProcessMousePassive);
  //(x, y, width, height, with scrollbar)
  upgr.push_back( new UpgradingInfo (this,base));
  player_upgrading.push_back(_Universe->CurrentCockpit());
  
}

void CargoToMission (const char * item,TextArea * ta) {
  char * item1 = strdup (item);
  int tmp;
  sscanf (item,"%d %s",&tmp,item1);
  Mission temp (item1);
  free (item1);
  temp.initMission();
  ta->ChangeTextItem ("name",temp.getVariable ("mission_name","").c_str());
  ta->ChangeTextItem ("price","");
  ta->ChangeTextItem ("mass","");
  ta->ChangeTextItem ("volume","");
  ta->ChangeTextItem ("description",temp.getVariable("description","").c_str());  
}

void UpgradingInfo::SelectItem (const char *item, int button, int buttonstate) {
	char floatprice [100];
  switch (mode) {
  case BUYMODE:
  case SELLMODE:
  case UPGRADEMODE:
  case ADDMODE:
  case DOWNGRADEMODE:    
  case SHIPDEALERMODE:
    switch (submode) {
    case NORMAL:
      if (curcategory.length()!=0) {
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

      Unit * temprate= new Unit ((string(unitdir)+string(".template")).c_str(),true,un->faction);
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
	  NewPart = new Unit (input_buffer,true,_Universe->GetFaction("upgrades"));
	  NewPart->SetFaction(un->faction);
	  if (NewPart->name==string("LOAD_FAILED")) {
	    NewPart->Kill();
	    NewPart = new Unit (input_buffer,true,un->faction);
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
	  if (NewPart->name==string("LOAD_FAILED")) {
	    NewPart->Kill();
	    NewPart=NULL;
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
	if (ours == 1 && type == 1) {
		buy_name = CargoList->GetSelectedItemName();
		if (buy_name) {
		  if (buy_name[0]!='\0') {
		    if (0==strcmp(buy_name,"[Back To Categories]")) {
		      curcategory=string("");
		      SetupCargoList();
		    }else {
		      if (curcategory.length()!=0) {
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
			curcategory=buy_name;
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







/*
void Unit::UpgradeInterface (Unit * base) {
  Unit * temprate = new Unit ((name+string(".template")).c_str(),false,faction);
  Unit * templ=NULL;
  if (temprate->name!=string("LOAD_FAILED")) {
    templ=temprate;
  }
  char input_buffer[4096];
  char input_buffer2[4096];
  input_buffer[4095]='\0';

  while (1) {
    bool upgrade=false;
    bool additive=false;
    bool downgrade=false;
    bool purchase=false;
    bool buycommod=false;
    bool sellcommod=false;
    while (!(upgrade||downgrade||purchase||buycommod|sellcommod)) {
      printf ("\nDo you wish to upgrade add or remove a part? Do you wish to trade-in a ship? Do you wish to buy or sell comodities? Type exit to quit.\n");
      fflush(stdout);
      fgets (input_buffer,4095,stdin);
      YoinkNewlines (input_buffer);
      downgrade = (strcasecmp (input_buffer,"remove")==0);
      purchase = (strcasecmp (input_buffer,"trade-in")==0);
      upgrade = (strcasecmp (input_buffer,"add")==0);
      additive=upgrade;
      upgrade|=(strcasecmp (input_buffer,"upgrade")==0);
      buycommod = (strcasecmp (input_buffer,"buy")==0);
      sellcommod = (strcasecmp (input_buffer,"sell")==0);
      
      if (Quit(input_buffer)) {
	break;
      }
    }
    if (Quit(input_buffer)) {
      break;
    }
    fprintf (stdout,"\nEnter the name of the part you wish to buy or sell\n");
    fflush (stdout);
    fgets (input_buffer,4095,stdin);
    YoinkNewlines(input_buffer);
    if (Quit(input_buffer)) {
      break;
    }
    if (buycommod||sellcommod) {
      fprintf (stdout,"Enter the quantity you wish to trade at %f price\n",base->PriceCargo(input_buffer));      	
      int quantity=-1;
      while (quantity==-1) {
	fgets (input_buffer2,4095,stdin);	  
	sscanf (input_buffer2,"%d",&quantity);
      }
      if (buycommod) {
	if(BuyCargo (input_buffer,quantity,base,_Universe->AccessCockpit()->credits)) {
	  printf ("Cargo Purchased\n");
	}else {
	  printf ("Cargo Not Bought\n");
	}
	
      }else if (sellcommod) {
	Cargo sold;
	if (SellCargo(input_buffer,quantity,_Universe->AccessCockpit()->credits,sold,base)) {
	  printf ("%d %s Sold! You have %f credits\n", sold.quantity,sold.content.c_str(),_Universe->AccessCockpit()->credits);
	} else {
	  printf ("Cargo Not Sold\n");
	}

      }
    }else if (purchase||upgrade||downgrade) {
      unsigned int i;
      Cargo * part = base->GetCargo (input_buffer, i);
      if ((part?part->quantity:0) ||
	  (downgrade&&(part=GetMasterPartList(input_buffer))!=NULL)) {
	Unit * NewPart = new Unit (input_buffer,false,_Universe->GetFaction("upgrades"));
	NewPart->SetFaction(faction);
	if (NewPart->name==string("LOAD_FAILED")) {
	  NewPart->Kill();
	  NewPart = new Unit (input_buffer, false, faction);
	}
	if (NewPart->name!=string("LOAD_FAILED")) {
	  if (purchase&&part->price<=_Universe->AccessCockpit()->credits) {
	    if (NewPart->nummesh>0) {
	      _Universe->AccessCockpit()->credits-=part->price;
	      NewPart->curr_physical_state=curr_physical_state;
	      NewPart->prev_physical_state=prev_physical_state;
	      _Universe->activeStarSystem()->AddUnit (NewPart);
	      _Universe->AccessCockpit()->SetParent(NewPart,input_buffer,"",curr_physical_state.position);//absolutely NO NO NO modifications...you got this baby clean off the slate
	      SwitchUnits (NULL,NewPart);
	      base->RequestClearance(NewPart);
	      NewPart->Dock(base);
	      SetSavedCredits (_Universe->AccessCockpit()->credits);
	      NewPart->Kill();
	      return;
	    }
	  }
	  bool canupgrade=false;
	  int mountoffset=0;
	  int subunitoffset=0;
	  double percentage;
	  if (upgrade||downgrade) {
	    if (NewPart->nummounts) {
	      printf ("You are adding %d mounts. Your ship has %d mounts. Which mount would you like to add the new mounts to?\n",NewPart->nummounts, nummounts);
	      fgets (input_buffer,4095,stdin);	  
	      sscanf (input_buffer,"%d",&mountoffset);
	    }
	    if (NewPart->getSubUnits().current()!=NULL) {
	      printf ("You are adding a turret. Which turret would you like to upgrade?\n");
	      fgets (input_buffer,4095,stdin);	  
	      sscanf (input_buffer,"%d",&subunitoffset);	  
	    }
	  }
	  
	  if (upgrade) {
	    canupgrade = canUpgrade (NewPart,mountoffset,subunitoffset,additive,false,percentage,templ);
	    if (!canupgrade) {
	      printf ("The frame of your starship cannot support the entire upgrade.\nDo you wish to continue with the purchase?\nYou do so at your own risk...\n");
	      fgets (input_buffer,4095,stdin);	  
	      YoinkNewlines (input_buffer);
	      if (Quit(input_buffer)||input_buffer[0]=='n'||input_buffer[0]=='N') {
		canupgrade=false;
	      } else {
		canUpgrade (NewPart,mountoffset,subunitoffset,additive,true,percentage,templ);
		canupgrade=true;
	      }
	    }
	    float upgradeprice =(float)(part->price*(1-usedPrice(percentage)));

	    if (canupgrade&&(_Universe->AccessCockpit()->credits>upgradeprice)) {
	      printf ("This purchase will cost you %f. You have %f. Do you wish to continue",upgradeprice,_Universe->AccessCockpit()->credits);
	      fgets (input_buffer,4095,stdin);	  
	      YoinkNewlines (input_buffer);
	      if (Quit(input_buffer)||input_buffer[0]=='n'||input_buffer[0]=='N') {
		
	      } else {
		printf ("This purchase costed you %f percent of the price of this unit.",upgradeprice);
		_Universe->AccessCockpit()->credits-=upgradeprice;
		Upgrade (NewPart,mountoffset,subunitoffset,additive,true,percentage,templ);
	      }
	    }
	  }
	  if (downgrade) {
	    canupgrade = canDowngrade (NewPart,mountoffset,subunitoffset,percentage);
	    
	    if (!canupgrade) {
	      printf ("Warning, the part is not in pristine condition, do you wish to continue?");
	      if (Quit(input_buffer)||input_buffer[0]=='n'||input_buffer[0]=='N') {
		canupgrade=false;
	      } else {
		canupgrade=true;
	    }	  
	    }
	    if (canupgrade) {
	      float sellprice =part->price*usedPrice(percentage);
	      printf ("This sale earned you %f credits.",sellprice);	  
	      _Universe->AccessCockpit()->credits+=sellprice;
	      Downgrade (NewPart,mountoffset,subunitoffset,percentage);
	      base->AddCargo (*part);
	      
	    }
	  }
	} else {
	  printf ("Failed to load part or unit. Please try again :-)\n");
	}
	NewPart->Kill();
      }
    }
  }
  temprate->Kill();
  
}
*/












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
      curcategory=string("upgrades");
      return FilterCargo (un,"upgrades",true,false);
    case SHIPDEALERMODE:
      curcategory=string("starships");
      return FilterCargo (un,"starships",true,true);
    case MISSIONMODE:
      curcategory=string("missions");
      return FilterCargo (un,"missions",true,true);
    case NEWSMODE:
      return TempCargo;
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
	return buyer.GetUnit()->FilterDowngradeList (GetCargoFor (relevant));
      }
      break;
    }
    if (relevant) {
      return GetCargoFor (relevant);
    }else {
      return Nada;
    }
  }
