#include "unit.h"
#include "images.h"
#include "universe.h"
#include "vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "gfx/cockpit.h"
#include "savegame.h"

#include "gui/text_area.h"
#include "gui/button.h"

#define UPGRADEOK 1
#define NOTTHERE 0
#define CAUSESDOWNGRADE -1
#define LIMITEDBYTEMPLATE -2
#ifdef _WIN32
#define strcasecmp stricmp
#endif
void Unit::Mount::SwapMounts (Unit::Mount &other) {
  Mount mnt = *this;
  *this=other;
  other=mnt;
  Transformation t =this->GetMountLocation();
  this->SetMountPosition(other.GetMountLocation());
  other.SetMountPosition (t);  
}
double Unit::Mount::Percentage (const Unit::Mount &newammo) const{
  float percentage=0;
  int thingstocompare=0;
  if (status==UNCHOSEN||status==DESTROYED)
    return 0;
  if (newammo.ammo==-1) {
    if (ammo!=-1) {
      thingstocompare++;
    }
  } else {
    if (newammo.ammo>0) {
      percentage+=ammo/newammo.ammo;
      thingstocompare++;
    }
  }
  if (newammo.type.Range) {
    percentage+= type.Range/newammo.type.Range;
    thingstocompare++;
  }
  if (newammo.type.Damage+100*newammo.type.PhaseDamage) {
    percentage += (type.Damage+100*type.PhaseDamage)/(newammo.type.Damage+100*newammo.type.PhaseDamage);
    thingstocompare++;
  }
  if (thingstocompare) {
    return percentage/thingstocompare;
  }else {
    return 0;
  }
}

typedef double (*adder) (double a, double b);
typedef double (*percenter) (double a, double b, double c);
typedef bool (*comparer) (double a, double b);




bool GreaterZero (double a, double b) {
  return a>=0;
}
double AddUp (double a, double b) {
  return a+b;
}
double GetsB (double a, double b) {
  return b;
}
bool AGreaterB (double a, double b) {
  return a>b;
}
double SubtractUp(double a, double b) {
  return a-b;
}
double SubtractClamp (double a, double b) {
  return (a-b<0)?0:a-b;
}
bool ALessB (double a, double b) {
  return a<b;
}
double computePercent (double old, double upgrade, double newb) {
  if (newb)
    return old/newb;
  else
    return 0;
}
double computeWorsePercent (double old,double upgrade, double isnew) {
  if (old)
    return isnew/old;
  else
    return 1;
}
double computeAdderPercent (double a,double b, double c) {return 0;}
double computeDowngradePercent (double old, double upgrade, double isnew) {
  if (upgrade) {
    return (old-isnew)/upgrade;
  }else {
    return 0;
  }
}

bool Unit::UpgradeMounts (Unit *up, int mountoffset, bool touchme, bool downgrade, int &numave,double &percentage) {
  int j;
  int i;
  bool cancompletefully=true;
  for (i=0,j=mountoffset;i<up->nummounts&&i<nummounts/*i should be nummounts, s'ok*/;i++,j++) {
    if (up->mounts[i].status==Mount::ACTIVE||up->mounts[i].status==Mount::INACTIVE) {//only mess with this if the upgrador has active mounts
      int jmod=j%nummounts;//make sure since we're offsetting the starting we don't overrun the mounts
      if (!downgrade) {//if we wish to add guns instead of remove
	if (up->mounts[i].type.size==(up->mounts[i].type.size&mounts[jmod].size)) {//only look at this mount if it can fit in the rack
	  numave++;//ok now we can compute percentage of used parts
	  percentage+=mounts[jmod].Percentage(up->mounts[i]);//compute here
	  if (touchme) {//if we wish to modify the mounts
	    mounts[jmod].SwapMounts (up->mounts[i]);//switch this mount with the upgrador mount
	  }
	} else {
	  cancompletefully=false;//since we cannot fit the mount in the slot we cannot complete fully
	}
      } else {
	bool found=false;//we haven't found a matching gun to remove
	for (unsigned int k=0;k<(unsigned int)nummounts;k++) {///go through all guns
	  int jkmod = (jmod+k)%nummounts;//we want to start with bias
	  if (strcasecmp(mounts[jkmod].type.weapon_name.c_str(),up->mounts[i].type.weapon_name.c_str())==0) {///search for right mount to remove starting from j. this is the right name
	    found=true;//we got one
	    percentage+=mounts[jkmod].Percentage(up->mounts[i]);///calculate scrap value (if damaged)
	    if (touchme) //if we modify
	      mounts[jkmod].status=Mount::UNCHOSEN;///deactivate weapon
	    break;
	  }
	}
	if (!found)
	  cancompletefully=false;//we did not find a matching weapon to remove
      }
    }
  }
  if (i<up->nummounts) {
    cancompletefully=false;//if we didn't reach the last mount that we wished to upgrade, we did not fully complete
  }
  return cancompletefully;
}

bool Unit::UpgradeSubUnits (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  bool cancompletefully=true;
  int j;
  un_iter ui;
  for (j=0,ui=getSubUnits();(*ui)!=NULL&&j<subunitoffset;++ui,j++) {
  }///set the turrets to the offset
  un_iter upturrets;
  for (upturrets=up->getSubUnits();((*upturrets)!=NULL)&&((*ui)!=NULL); ++ui,++upturrets) {//begin goign through other unit's turrets
    Unit *addtome;
    Unit * giveAway;
    addtome=*upturrets;//set pointers
    giveAway=*ui;
    if (addtome->rSize()) {//if the new turret has any size at all
      numave++;//add it
      percentage+=(giveAway->rSize()/addtome->rSize());//add up percentage equal to ratio of sizes
    }
    if (touchme) {//if we wish to modify,
      Transformation t(addtome->curr_physical_state);//switch their current positions
      addtome->curr_physical_state=giveAway->curr_physical_state;
      giveAway->curr_physical_state=t;
      t=addtome->prev_physical_state;
      addtome->prev_physical_state=giveAway->prev_physical_state;
      giveAway->prev_physical_state=t;//switch their previous positions
      giveAway->SetRecursiveOwner(up);//set the owners of the respective turrets

      upturrets.postinsert (giveAway);//add it to the second unit
      ui.remove();//remove the turret from the first unit

      if (!downgrade) {//if we are upgrading swap them
	ui.preinsert(addtome);//add unit to your ship
	upturrets.remove();//remove unit from being a turret on other ship
	addtome->SetRecursiveOwner(this);//set recursive owner
      } else {
	Unit * un;//make garbage unit
	ui.preinsert (un=new Unit("nothing",true,faction));//give a default do-nothing unit
	un->curr_physical_state=addtome->curr_physical_state;
	un->prev_physical_state=addtome->prev_physical_state;

	un->SetRecursiveOwner(this);
	upturrets.remove();//remove unit from being a turret on other ship
      }
    }
  }
  if ((*upturrets)!=NULL) 
    return false;
  return cancompletefully;
}
int UpgradeBoolval (int a, int upga, bool touchme, bool downgrade, int &numave,double &percentage) {
  if (downgrade) {
    if (a&&upga) {
      if (touchme) (a=false);
      numave++;
      percentage++;
    }
  }else {
    if (!a&&upga) {
      if (touchme) a=true;
      numave++;
      percentage++;
    }
  }
  return a;
}

static bool UpgradeFloat (double &result,double tobeupgraded, double upgrador, double templatelimit, double (*myadd) (double,double), bool (*betterthan) (double a, double b), double nothing,  double completeminimum, double (*computepercentage) (double oldvar, double upgrador, double newvar), double & percentage, bool forcedowngrade, bool usetemplate) {
  if (upgrador!=nothing) {//if upgrador is better than nothing
    float newsum = (*myadd)(tobeupgraded,upgrador);
    if (newsum!=tobeupgraded&&(((*betterthan)(newsum, tobeupgraded)||forcedowngrade))) {
      if (((*betterthan)(newsum,templatelimit)&&usetemplate)||newsum<completeminimum) {
	if (!forcedowngrade)
	  return LIMITEDBYTEMPLATE;
	if (newsum<completeminimum)
	  newsum=completeminimum;
	else
	  newsum = templatelimit;
      }
      ///we know we can replace result with newsum
      percentage = (*computepercentage)(tobeupgraded,upgrador,newsum);
      result=newsum;
      return UPGRADEOK;
    }else {
      return CAUSESDOWNGRADE;
    }
  } else {
    return NOTTHERE;
  }
}
bool Unit::canUpgrade (Unit * upgrador, int mountoffset,  int subunitoffset, bool additive, bool force,  double & percentage, Unit * templ){
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,false,false,additive,force,percentage);
}
bool Unit::Upgrade (Unit * upgrador, int mountoffset,  int subunitoffset, bool additive, bool force,  double & percentage, Unit * templ) {
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,true,false,additive,force,percentage);
}
bool Unit::canDowngrade (Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,false,true,false,true,percentage);
}
bool Unit::Downgrade (Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,true,true,false,true,percentage);
}
static float usedPrice (float percentage) {
  return .66*percentage;
}
bool Unit::UpAndDownGrade (Unit * up, Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, bool additive, bool forcetransaction, double &percentage) {
  percentage=0;
  int numave=0;
  bool cancompletefully=UpgradeMounts(up,mountoffset,touchme,downgrade,numave,percentage);
  bool cancompletefully1=UpgradeSubUnits(up,subunitoffset,touchme,downgrade,numave,percentage);
  cancompletefully=cancompletefully&&cancompletefully1;
  adder Adder;
  comparer Comparer;
  percenter Percenter;


  if (downgrade) {
    Adder=&SubtractUp;
    Percenter=&computeDowngradePercent;
    Comparer = &GreaterZero;
  } else{
    if (additive) {
      Adder=&AddUp;
      Percenter=&computeAdderPercent;
    }else {
      Adder=&GetsB;
      Percenter=&computePercent;
    }
    Comparer=AGreaterB;
  }
  double resultdoub;
  int retval;
  double temppercent;
#define STDUPGRADE(my,oth,temp,noth) retval=(UpgradeFloat(resultdoub,my,oth,(templ!=NULL)?temp:0,Adder,Comparer,noth,noth,Percenter, temppercent,forcetransaction,templ!=NULL)); if (retval==UPGRADEOK) {if (touchme){my=resultdoub;} percentage+=temppercent; numave++;} else {if (retval!=NOTTHERE) cancompletefully=false;}

  STDUPGRADE(armor.front,up->armor.front,templ->armor.front,0);
  STDUPGRADE(armor.back,up->armor.back,templ->armor.back,0);
  STDUPGRADE(armor.right,up->armor.right,templ->armor.right,0);
  STDUPGRADE(armor.left,up->armor.left,templ->armor.left,0);
  STDUPGRADE(shield.recharge,up->shield.recharge,templ->shield.recharge,0);
  STDUPGRADE(hull,up->hull,templ->hull,0);
  STDUPGRADE(recharge,up->recharge,templ->recharge,0);
  STDUPGRADE(maxenergy,up->maxenergy,templ->maxenergy,0);
  STDUPGRADE(limits.yaw,up->limits.yaw,templ->limits.yaw,0);
  STDUPGRADE(limits.pitch,up->limits.pitch,templ->limits.pitch,0);
  STDUPGRADE(limits.roll,up->limits.roll,templ->limits.roll,0);
  STDUPGRADE(limits.lateral,up->limits.lateral,templ->limits.lateral,0);
  STDUPGRADE(limits.vertical,up->limits.vertical,templ->limits.vertical,0);
  STDUPGRADE(limits.forward,up->limits.forward,templ->limits.forward,0);
  STDUPGRADE(limits.retro,up->limits.retro,templ->limits.retro,0);
  STDUPGRADE(limits.afterburn,up->limits.afterburn,templ->limits.afterburn,0);
  STDUPGRADE(computer.radar.maxrange,up->computer.radar.maxrange,templ->computer.radar.maxrange,0);
  STDUPGRADE(computer.max_speed,up->computer.max_speed,templ->computer.max_speed,0);
  STDUPGRADE(computer.max_ab_speed,up->computer.max_ab_speed,templ->computer.max_ab_speed,0);
  STDUPGRADE(computer.max_yaw,up->computer.max_yaw,templ->computer.max_yaw,0);
  STDUPGRADE(computer.max_pitch,up->computer.max_pitch,templ->computer.max_pitch,0);
  STDUPGRADE(computer.max_roll,up->computer.max_roll,templ->computer.max_roll,0);
  STDUPGRADE(fuel,up->fuel,templ->fuel,0);
  STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
  if (shield.number==up->shield.number) {
    switch (shield.number) {
    case 2:
      STDUPGRADE(shield.fb[2],up->shield.fb[2],templ->shield.fb[2],0);
      STDUPGRADE(shield.fb[3],up->shield.fb[3],templ->shield.fb[3],0);
      break;
    case 4:
      STDUPGRADE(shield.fbrl.frontmax,up->shield.fbrl.frontmax,templ->shield.fbrl.frontmax,0);
      STDUPGRADE(shield.fbrl.backmax,up->shield.fbrl.backmax,templ->shield.fbrl.backmax,0);
      STDUPGRADE(shield.fbrl.leftmax,up->shield.fbrl.leftmax,templ->shield.fbrl.leftmax,0);
      STDUPGRADE(shield.fbrl.rightmax,up->shield.fbrl.rightmax,templ->shield.fbrl.rightmax,0);
      break;
    case 6:
      STDUPGRADE(shield.fbrltb.fbmax,up->shield.fbrltb.fbmax,templ->shield.fbrltb.fbmax,0);
      STDUPGRADE(shield.fbrltb.rltbmax,up->shield.fbrltb.rltbmax,templ->shield.fbrltb.rltbmax,0);
      break;     
    }
  }else {
    if (up->FShieldData()>0||up->RShieldData()>0|| up->LShieldData()>0||up->BShieldData()>0) {
      cancompletefully=false;
    }
  }
  

  computer.radar.color=UpgradeBoolval(computer.radar.color,up->computer.radar.color,touchme,downgrade,numave,percentage);
  computer.itts=UpgradeBoolval(computer.itts,up->computer.radar.color,touchme,downgrade,numave,percentage);
  ///do the two reversed ones below
  double myleak=100-shield.leak;
  double upleak=100-up->shield.leak;
  double templeak=100-(templ!=NULL?templ->shield.leak:0);
  STDUPGRADE(myleak,upleak,templeak,0);
  if (touchme&&myleak<=100&&myleak>0)shield.leak=100-myleak;

  myleak = -computer.radar.maxcone;
  upleak=-up->computer.radar.maxcone;
  templeak=-(templ!=NULL?templ->computer.radar.maxcone:0);
  STDUPGRADE(myleak,upleak,templeak,-1);
  if (touchme&&myleak<=100&&myleak>0)computer.radar.maxcone=-myleak;
  
  //NO CLUE FOR BELOW
  if (downgrade) {
    if (jump.drive>=-1&&up->jump.drive>=-1) {
      if (touchme) jump.drive=-2;
      numave++;
      percentage+=(jump.energy&&jump.delay)?(.25*(up->jump.energy/jump.energy+up->jump.delay/jump.delay)):1;
      percentage+=.5*((float)(100-jump.damage))/(101-up->jump.damage);
    }
    if (cloaking!=-1&&up->cloaking!=-1) {
      if (touchme) cloaking=-1;
      numave++;
      percentage++;
    }
  }else {
    if (cloaking==-1&&up->cloaking!=-1) {
      if (touchme) {cloaking=up->cloaking;cloakmin=up->cloakmin;image->cloakrate=up->image->cloakrate; image->cloakglass=up->image->cloakglass;image->cloakenergy=up->image->cloakenergy;}
      numave++;
      percentage++;
    }
    if (jump.drive==-2&&up->jump.drive>=-1) {
      if (touchme) {jump.drive = up->jump.drive;jump.energy=up->jump.energy;jump.delay=up->jump.delay; jump.damage=0;}
      numave++;
      percentage++;
    }
  }
  if (numave)
    percentage=percentage/numave;
  if (touchme&&up->mass&&numave) {
    float multiplyer =((downgrade)?-1:1);
    mass +=multiplyer*percentage*up->mass;
    if (mass<(templ?templ->mass:.000000001))
      mass=(templ?templ->mass:.000000001);
    MomentOfInertia +=multiplyer*percentage*up->MomentOfInertia;
    if (MomentOfInertia<(templ?templ->MomentOfInertia:0.00000001)) {
      MomentOfInertia=(templ?templ->MomentOfInertia:0.00000001);
    }
  }
  return cancompletefully;
}

void YoinkNewlines (char * input_buffer) {
    for (int i=0;input_buffer[i]!='\0';i++) {
      if (input_buffer[i]=='\n'||input_buffer[i]=='\r') {
	input_buffer[i]='\0';
      }
    }
}
bool Quit (const char *input_buffer) {
	if (strcasecmp (input_buffer,"exit")==0||
	    strcasecmp (input_buffer,"quit")==0) {
	  return true;
	}
	return false;
}
extern void SwitchUnits (Unit * ol, Unit * nw);
extern Cargo * GetMasterPartList(const char *input_buffer);

TextArea *CargoList, *CargoInfo;
Button *OK;

void Unit::UpgradeInterface(Unit * base) {
cout << "Starting docking\n";
	glutMouseFunc(ProcessMouseClick);
	glutMotionFunc(ProcessMouseActive);
	glutPassiveMotionFunc(ProcessMousePassive);
	//	glutDisplayFunc(RefreshGUI);

	                      //(x, y, width, height, with scrollbar)
	CargoList = new TextArea(-1, 1, 1, 1.8, 1);
	CargoInfo = new TextArea(0, 1, 1, 1.8, 1);

	CargoList->AddTextItem("a","Just a test item");
	CargoList->AddTextItem("b","And another just to be sure");

	OK = new Button(-0.94, -0.85, 0.12, 0.1, "Done");
	GFXLoop (RefreshGUI);
	//	glutMainLoop();
}

void RefreshGUI(void) {
	StartFrame();
	// Black background
	ShowColor(-1,-1,2,2, 0,0,0,1);
	CargoList->Refresh();
	CargoInfo->Refresh();
	OK->Refresh();
	EndFrame();
}

void StartFrame(void) {
  
  glutSetCursor(GLUT_CURSOR_INHERIT);
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);
  GFXDisable (DEPTHTEST);
  GFXDisable (DEPTHWRITE);
  GFXDisable (LIGHTING);
  GFXDisable (CULLFACE);
  GFXClear (GFXTRUE);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (TEXTURE1);
  GFXEnable (TEXTURE0);
}

void EndFrame(void) {
  GFXEndScene();
  GFXHudMode(false);
  GFXEnable (CULLFACE);

}

void ProcessMouseClick(int button, int state, int x, int y) {
	ProcessMouse(1, x, y, button, state);
}

void ProcessMouseActive(int x, int y) {
	ProcessMouse(2, x, y, 0, 0);
}

void ProcessMousePassive(int x, int y) {
	ProcessMouse(3, x, y, 0, 0);
}

// type=1 is mouse click
// type=2 is mouse drag
// type=3 is mouse movement
void ProcessMouse(int type, int x, int y, int button, int state) {

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
