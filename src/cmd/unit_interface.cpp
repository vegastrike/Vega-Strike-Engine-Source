#include "unit.h"
#include "gui/text_area.h"
#include "gui/button.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "main_loop.h"
#ifdef _WIN32
#define strcasecmp stricmp
#endif


extern void SwitchUnits (Unit * ol, Unit * nw);
extern Cargo * GetMasterPartList(const char *input_buffer);

struct UpgradingInfo {
  TextArea *CargoList, *CargoInfo;
  Button *OK, *COMMIT;
  UnitContainer base;
  UnitContainer buyer;
  enum BaseMode {BUYMODE,SELLMODE,MISSIONMODE,UPGRADEMODE,ADDMODE,DOWNGRADEMODE,SHIPDEALERMODE, MAXMODE} mode;
  Button *Modes[MAXMODE];
  string title;
  void SetMode (enum BaseMode mod) {
    string ButtonText;
    switch (mod) {
    case BUYMODE:
      title = "Purchase Cargo Mode";
      ButtonText= "BuyCargo";
      break;
    case SELLMODE:
      title = "Sell Cargo Mode";
      ButtonText= "SellCargo";
      break;
    case UPGRADEMODE:
      title = "Upgrade/Repair Starship Mode";
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
    COMMIT->ModifyName (ButtonText.c_str());
    mode = mod;
  }
  UpgradingInfo(Unit * un, Unit * base):base(base),buyer(un),mode(BUYMODE),title("Buy Cargo"){
	CargoList = new TextArea(-1, 0.9, 1, 1.7, 1);
	CargoInfo = new TextArea(0, 0.9, 1, 1.7, 0);
	//	CargoList->AddTextItem("a","Just a test item");
	//	CargoList->AddTextItem("b","And another just to be sure");
	CargoInfo->AddTextItem("name", "");
	CargoInfo->AddTextItem("price", "");
	OK = new Button(-0.94, -0.85, 0.15, 0.1, "Done");
	COMMIT = new Button(-0.75, -0.85, 0.25, 0.1, "Buy");
	const char  MyButtonModes[][128] = {"BuyMode","SellMode","MissionBBS","UpgradeShip","Unimplemented","Downgrade", "ShipDealer"};
	float beginx = -.4;
	float lastx = beginx;
	float size=.4;
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
	SetMode (BUYMODE);
  }
  ~UpgradingInfo() {
    base.SetUnit(NULL);
    buyer.SetUnit(NULL);
    delete CargoList;
    delete CargoInfo;
    for (int i=0;i<MAXMODE;i++) {
      delete Modes[i];
    }
  }
  void Render(){
	StartGUIFrame();
	// Black background
	ShowColor(-1,-1,2,2, 0,0,0,1);
	ShowColor(0,0,0,0, 1,1,1,1);
	ShowText(-0.98, 0.93, 1, 4, title.c_str(), 0);
	CargoList->Refresh();
	CargoInfo->Refresh();
	OK->Refresh();
	COMMIT->Refresh();
	for (unsigned int i=0;i<MAXMODE;i++) {
	  Modes[i]->Refresh();
	}
	EndGUIFrame();
  }
} *upgr=NULL;

static void ProcessMouse(int type, int x, int y, int button, int state);
static void RefreshGUI(void) {
  upgr->Render();
}

static void ProcessMouseClick(int button, int state, int x, int y) {
  SetSoftwareMousePosition (x,y);
	ProcessMouse(1, x, y, button, state);
}

static void ProcessMouseActive(int x, int y) {
  SetSoftwareMousePosition (x,y);
  ProcessMouse(2, x, y, 0, 0);
}

static void ProcessMousePassive(int x, int y) {
  SetSoftwareMousePosition(x,y);
  ProcessMouse(3, x, y, 0, 0);
}
void Unit::UpgradeInterface(Unit * base) {
  printf("Starting docking\n");
  glutMouseFunc(ProcessMouseClick);
  glutMotionFunc(ProcessMouseActive);
  glutPassiveMotionFunc(ProcessMousePassive);
  //(x, y, width, height, with scrollbar)
  upgr = new UpgradingInfo (this,base);
  GFXLoop (RefreshGUI);
}

// type=1 is mouse click
// type=2 is mouse drag
// type=3 is mouse movement
static void ProcessMouse(int type, int x, int y, int button, int state) {
	int ours = 0;
	float cur_x = 0, cur_y = 0, new_x = x, new_y = y;
	char *buy_name;

	cur_x = ((new_x / g_game.x_resolution) * 2) - 1;
	cur_y = ((new_y / g_game.y_resolution) * -2) + 1;

	ours = upgr->CargoList->DoMouse(type, cur_x, cur_y, button, state);
	if (ours == 1 && type == 1) {
		buy_name = upgr->CargoList->GetSelectedItemName();
		if (buy_name != 0 && buy_name[0] != '\0') { upgr->CargoInfo->ChangeTextItem("name", (string("name: ")+buy_name).c_str()); }
		else { upgr->CargoInfo->ChangeTextItem("name",""); }
		upgr->CargoInfo->ChangeTextItem("price", "Price: Random. Hah.");
	}
	// Commented out because they don't need to use the mouse with CargoInfo
	//if (ours == 0) { ours = CargoInfo->DoMouse(type, cur_x, cur_y, button, state); }
	if (ours == 0) {
		ours = upgr->OK->DoMouse(type, cur_x, cur_y, button, state);
		if (ours == 1 && type == 1) {
			restore_main_loop();
			cout << "You clicked done\n";
			delete upgr;
			upgr=NULL;
		}
	}	
	if (ours == 0) {
		ours = upgr->COMMIT->DoMouse(type, cur_x, cur_y, button, state);
		if (ours == 1 && type == 1) {
			buy_name = upgr->CargoList->GetSelectedItemName();
			cout << "You are buying the " << buy_name << endl;
		}
	}
	for (int i=0;i<UpgradingInfo::MAXMODE&&ours==0;i++) {
	  ours = upgr->Modes[i]->DoMouse(type,cur_x,cur_y,button,state);
	  if (ours==1&&type==1) {

	    upgr->SetMode ((UpgradingInfo::BaseMode)i);
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
