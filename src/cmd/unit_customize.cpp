#include "unit.h"
#include "unit_factory.h"
#include "images.h"
#include "universe.h"
#include "vegastrike.h"
#include <stdio.h>
#include <stdlib.h>
#include "gfx/cockpit.h"
#include "savegame.h"
#include "config_xml.h"
#include "xml_serializer.h"
#include "audiolib.h"
#define UPGRADEOK 1
#define NOTTHERE 0
#define CAUSESDOWNGRADE -1
#define LIMITEDBYTEMPLATE -2
#ifdef _WIN32
#define strcasecmp stricmp
#endif
extern int GetModeFromName (const char *);

void Unit::Mount::SwapMounts (Unit::Mount &other) {
  short thisvol = volume;
  short othervol = other.volume;
  short othersize = other.size;
  short thissize = size;
  Mount mnt = *this;
  this->size=thissize;
  *this=other;
  other=mnt;
  volume=thisvol;
  other.volume=othervol;//volumes stay the same even if you swap out
  Transformation t =this->GetMountLocation();
  this->SetMountPosition(other.GetMountLocation());
  other.SetMountPosition (t);  
}

void Unit::Mount::ReplaceMounts (const Unit::Mount &other) {
  short thisvol = volume;
  short thissize = size;
  Transformation t =this->GetMountLocation();
  *this=other;
  sound = AUDCreateSound (sound,type->type!=weapon_info::PROJECTILE);
  this->size=thissize;
  volume=thisvol;
  this->SetMountPosition(t);
  ref.gun=NULL;  
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
  if (newammo.type->Range) {
    percentage+= type->Range/newammo.type->Range;
    thingstocompare++;
  }
  if (newammo.type->Damage+100*newammo.type->PhaseDamage) {
    percentage += (type->Damage+100*type->PhaseDamage)/(newammo.type->Damage+100*newammo.type->PhaseDamage);
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
double MultUp (double a, double b) {
  return a*b;
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
double computeMultPercent (double a,double b, double c) {return 0;}
double computeDowngradePercent (double old, double upgrade, double isnew) {
  if (upgrade) {
    return (old-isnew)/upgrade;
  }else {
    return 0;
  }
}

bool Unit::UpgradeMounts (const Unit *up, int mountoffset, bool touchme, bool downgrade, int &numave, Unit * templ, double &percentage) {
  int j;
  int i;
  bool cancompletefully=true;
  for (i=0,j=mountoffset;i<up->nummounts&&i<nummounts/*i should be nummounts, s'ok*/;i++,j++) {
    if (up->mounts[i].status==Mount::ACTIVE||up->mounts[i].status==Mount::INACTIVE) {//only mess with this if the upgrador has active mounts
      int jmod=j%nummounts;//make sure since we're offsetting the starting we don't overrun the mounts
      if (!downgrade) {//if we wish to add guns instead of remove
	if (up->mounts[i].type->weapon_name!="MOUNT_UPGRADE") {


	  if (up->mounts[i].type->size==(up->mounts[i].type->size&mounts[jmod].size)) {//only look at this mount if it can fit in the rack
	    if (up->mounts[i].type->weapon_name!=mounts[jmod].type->weapon_name) {
	      numave++;//ok now we can compute percentage of used parts
	      if (templ) {
		if (templ->nummounts>jmod) {
		  int maxammo = templ->mounts[jmod].ammo;
		  if ((up->mounts[i].ammo>maxammo||up->mounts[i].ammo==-1)&&maxammo!=-1) {
		    up->mounts[i].ammo = maxammo;
		  }
		  if (templ->mounts[jmod].volume!=-1) {
		    if (up->mounts[i].ammo*up->mounts[i].type->volume>templ->mounts[jmod].volume) {
		      up->mounts[i].ammo = (templ->mounts[jmod].volume+1)/up->mounts[i].type->volume;
		    }
		  }
		}
	      }
	      percentage+=mounts[jmod].Percentage(up->mounts[i]);//compute here
	      if (touchme) {//if we wish to modify the mounts
		mounts[jmod].ReplaceMounts (up->mounts[i]);//switch this mount with the upgrador mount
	      }
	    }else {
	      int tmpammo = mounts[jmod].ammo;
	      if (mounts[jmod].ammo!=-1&&up->mounts[i].ammo!=-1) {
		tmpammo+=up->mounts[i].ammo;
		if (templ) {
		  if (templ->nummounts>jmod) {
		    if (templ->mounts[jmod].ammo!=-1) {
		      if (templ->mounts[jmod].ammo>tmpammo) {
			tmpammo=templ->mounts[jmod].ammo;
		      }
		    }
		    if (templ->mounts[jmod].volume!=-1) {
		      if (templ->mounts[jmod].volume>mounts[jmod].type->volume*tmpammo) {
			tmpammo=(templ->mounts[jmod].volume+1)/mounts[jmod].type->volume;
		      }
		    }
		    
		  }
		} 
		if (tmpammo*mounts[jmod].type->volume>mounts[jmod].volume) {
		  tmpammo = (1+mounts[jmod].volume)/mounts[jmod].type->volume;
		}
		if (tmpammo>mounts[jmod].ammo) {
		  cancompletefully=true;
		  if (touchme)
		    mounts[jmod].ammo = tmpammo;
		}else {
		  cancompletefully=false;
		}
	      }
	      
	    }
	  } else {
	    cancompletefully=false;//since we cannot fit the mount in the slot we cannot complete fully
	  }
	}else {
	  unsigned int siz=0;
	  siz = ~siz;
	  if (templ) {
	    if (templ->nummounts>jmod) {
	      siz = templ->mounts[jmod].size;
	    }
	  }
	  if (((siz&up->mounts[i].size)|mounts[jmod].size)!=mounts[jmod].size) {
	    if (touchme) {
	      mounts[jmod].size|=up->mounts[i].size;
	    }
	    numave++;
	    percentage++;

	  }else {
	    cancompletefully=false;
	  }
	  //we need to |= the mount type
	}
      } else {
	if (up->mounts[i].type->weapon_name!="MOUNT_UPGRADE") {
	  bool found=false;//we haven't found a matching gun to remove

		for (unsigned int k=0;k<(unsigned int)nummounts;k++) {///go through all guns
	      int jkmod = (jmod+k)%nummounts;//we want to start with bias
	      if (strcasecmp(mounts[jkmod].type->weapon_name.c_str(),up->mounts[i].type->weapon_name.c_str())==0) {///search for right mount to remove starting from j. this is the right name
		found=true;//we got one
		percentage+=mounts[jkmod].Percentage(up->mounts[i]);///calculate scrap value (if damaged)
		if (touchme) //if we modify
		  mounts[jkmod].status=Mount::UNCHOSEN;///deactivate weapon
		break;
	      }
	    }
	  
	  if (!found)
	    cancompletefully=false;//we did not find a matching weapon to remove
	}else {
	  bool found=false;
	  static   bool downmount =XMLSupport::parse_bool (vs_config->getVariable ("physics","can_downgrade_mount_upgrades","false"));
	  if (downmount ) {

	  for (unsigned int k=0;k<(unsigned int)nummounts;k++) {///go through all guns
	    int jkmod = (jmod+k)%nummounts;//we want to start with bias
	    if ((up->mounts[i].size&mounts[jkmod].size)==(up->mounts[i].size)) {
	      if (touchme) {
		mounts[jkmod].size&=(~up->mounts[i].size);
	      }
	      percentage++;
	      numave++;
	      found=true;
	    }
	  }
	  }
	  if (!found)
	    cancompletefully=false;
	}
      }
    }
  }
  if (i<up->nummounts) {
    cancompletefully=false;//if we didn't reach the last mount that we wished to upgrade, we did not fully complete
  }
  return cancompletefully;
}


using std::string;
void Tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
std::string CheckBasicSizes (const std::string tokens) {
  if (tokens.find ("small")!=string::npos) {
    return "small";
  }
  if (tokens.find ("medium")!=string::npos) {
    return "medium";
  }
  if (tokens.find ("large")!=string::npos) {
    return "large";
  }
  if (tokens.find ("cargo")!=string::npos) {
    return "cargo";
  }
  if (tokens.find ("LR")!=string::npos||tokens.find ("massive")!=string::npos) {
    return "massive";
  }
  return "";
}
std::string getTurretSize (const std::string &size) {
  vector <string> tokens;
  Tokenize (size,tokens,"_");
  for (unsigned int i=0;i<tokens.size();i++) {
    if (tokens[i].find ("turret")!=string::npos) {
      string temp = CheckBasicSizes (tokens[i]);
      if (!temp.empty()) {
	return temp;
      }
    } else {
      return tokens[i];
    }
  }
  return "capitol";
}


bool Unit::UpgradeSubUnits (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage)  {
  bool cancompletefully=true;
  int j;
  std::string turSize;
  un_iter ui;
  bool found=false;
  for (j=0,ui=getSubUnits();(*ui)!=NULL&&j<subunitoffset;++ui,j++) {
  }///set the turrets to the offset
  un_iter upturrets;
  Unit * giveAway;

  giveAway=*ui;
  if (giveAway==NULL) {
    return true;
  }
  bool hasAnyTurrets=false;
    turSize = getTurretSize (giveAway->name);
  for (upturrets=up->getSubUnits();((*upturrets)!=NULL)&&((*ui)!=NULL); ++ui,++upturrets) {//begin goign through other unit's turrets
    hasAnyTurrets = true;
    Unit *addtome;

    addtome=*upturrets;//set pointers

    

    bool foundthis=false;
    if (turSize == getTurretSize (addtome->name)&&addtome->rSize()) {//if the new turret has any size at all
      if (!downgrade||addtome->name==giveAway->name) {
	found=true;
	foundthis=true;
	numave++;//add it
	percentage+=(giveAway->rSize()/addtome->rSize());//add up percentage equal to ratio of sizes
      }
    }
    if (foundthis) {
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
	  ui.preinsert (un=UnitFactory::createUnit("blank",true,faction));//give a default do-nothing unit
	  un->limits.yaw=0;
	  un->limits.pitch=0;
	  un->limits.roll=0;
	  un->limits.lateral = un->limits.retro = un->limits.forward = un->limits.afterburn=0.0;

	  un->name = turSize+"_blank";
	  if (un->image->unitwriter!=NULL) {
	    un->image->unitwriter->setName (un->name);
	  }
	  un->curr_physical_state=addtome->curr_physical_state;

	  un->prev_physical_state=addtome->prev_physical_state;
	  
	  un->SetRecursiveOwner(this);
	  upturrets.remove();//remove unit from being a turret on other ship
	}
      }
    }
  }
  
  if (!found) {
    return !hasAnyTurrets;
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

static int UpgradeFloat (double &result,double tobeupgraded, double upgrador, double templatelimit, double (*myadd) (double,double), bool (*betterthan) (double a, double b), double nothing,  double completeminimum, double (*computepercentage) (double oldvar, double upgrador, double newvar), double & percentage, bool forcedowngrade, bool usetemplate) {
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
bool Unit::canUpgrade (Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, Unit * templ){
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,false,false,additive,force,percentage);
}
bool Unit::Upgrade (Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, Unit * templ) {
  return UpAndDownGrade(upgrador,templ,mountoffset,subunitoffset,true,false,additive,true,percentage);
}
bool Unit::canDowngrade (Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,false,true,false,true,percentage);
}
bool Unit::Downgrade (Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage){
  return UpAndDownGrade(downgradeor,NULL,mountoffset,subunitoffset,true,true,false,true,percentage);
}
bool Unit::UpAndDownGrade (Unit * up, Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, int additive, bool forcetransaction, double &percentage) {
  percentage=0;
  int numave=0;
  bool cancompletefully=UpgradeMounts(up,mountoffset,touchme,downgrade,numave,templ,percentage);
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
    if (additive==1) {
      Adder=&AddUp;
      Percenter=&computeAdderPercent;
    }else if (additive==2) {
      Adder=&MultUp;
      Percenter=&computeMultPercent;
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
  STDUPGRADE(image->repair_droid,up->image->repair_droid,templ->image->repair_droid,0);
  STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
  image->ecm = abs(image->ecm);
  STDUPGRADE(image->ecm,abs(up->image->ecm),abs(templ->image->ecm),0);
  STDUPGRADE(maxenergy,up->maxenergy,templ->maxenergy,0);
  //  STDUPGRADE(afterburnenergy,up->afterburnenergy,templ->afterburnenergy,0);
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

  for (unsigned int upgr=0;upgr<Cockpit::NUMGAUGES+1+MAXVDUS;upgr++) {
	STDUPGRADE(image->cockpit_damage[upgr],up->image->cockpit_damage[upgr],templ->image->cockpit_damage[upgr],1);
	if (image->cockpit_damage[upgr]>1) {
	  image->cockpit_damage[upgr]=1;//keep it real
	}
  }
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
  bool ccf = cancompletefully;
  STDUPGRADE(myleak,upleak,templeak,0);
  if (touchme&&myleak<=100&&myleak>=0)shield.leak=100-myleak;
  
  myleak = 1-computer.radar.maxcone;
  upleak=1-up->computer.radar.maxcone;
  templeak=1-(templ!=NULL?templ->computer.radar.maxcone:-1);
  STDUPGRADE(myleak,upleak,templeak,0);
  if (touchme)computer.radar.maxcone=1-myleak;
  static float lc =XMLSupport::parse_float (vs_config->getVariable ("physics","lock_cone",".8"));// DO NOT CHANGE see unit_customize.cpp
  if (up->computer.radar.lockcone!=lc) {
    myleak = 1-computer.radar.lockcone;
    upleak=1-up->computer.radar.lockcone;
    templeak=1-(templ!=NULL?templ->computer.radar.lockcone:-1);
    if (templeak == 1-lc) {
      templeak=2;
    }
    STDUPGRADE(myleak,upleak,templeak,0);
    if (touchme)computer.radar.lockcone=1-myleak;
  }
  static float tc =XMLSupport::parse_float (vs_config->getVariable ("physics","autotracking",".93"));//DO NOT CHANGE! see unit.cpp:258
  if (up->computer.radar.trackingcone!=tc) {
    myleak = 1-computer.radar.trackingcone;
    upleak=1-up->computer.radar.trackingcone;
    templeak=1-(templ!=NULL?templ->computer.radar.trackingcone:-1);
    if (templeak==1-tc) {
      templeak=2;
    }
    STDUPGRADE(myleak,upleak,templeak,0);
    if (touchme)computer.radar.trackingcone=1-myleak;    
  }
  cancompletefully=ccf;
  //NO CLUE FOR BELOW
  if (downgrade) {
    //    STDUPGRADE(image->cargo_volume,up->image->cargo_volume,templ->image->cargo_volume,0);
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
  
    if (afterburnenergy<32767&&up->afterburnenergy!=0) {
      if (touchme) afterburnenergy=32767;
      numave++;
      percentage++;
    }
  
  }else {
    if (touchme) {
      for (unsigned int i=0;i<up->image->cargo.size();i++) {
	if (CanAddCargo(up->image->cargo[i])) {
	  AddCargo(up->image->cargo[i],false);
	}

      }

    }
    /*    if (image->cargo_volume<up->image->cargo_volume) {
      
      if (templ!=NULL?up->image->cargo_volume+image->cargo_volume<templ->image->cargo_volume:true) {
	if (touchme)image->cargo_volume+=up->image->cargo_volume;
	numave++;
	percentage++;
      }
      }*/
    if (cloaking==-1&&up->cloaking!=-1) {
      if (touchme) {cloaking=up->cloaking;cloakmin=up->cloakmin;image->cloakrate=up->image->cloakrate; image->cloakglass=up->image->cloakglass;image->cloakenergy=up->image->cloakenergy;}
      numave++;
      percentage++;
    }
    
    if (afterburnenergy>up->afterburnenergy&&up->afterburnenergy>0) {
      numave++;
      percentage++;
      afterburnenergy=up->afterburnenergy;
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
extern char * GetUnitDir (const char *);
double Unit::Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts) {
#if 0
  if (shield.number==2) {
    printf ("shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
  }else {
    printf ("shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);    

  }
#endif
	Unit * up = UnitFactory::createUnit (file.c_str(),true,_Universe->GetFaction("upgrades"));
	static Unit * last_template=NULL;
	char * unitdir  = GetUnitDir(name.c_str());
	
	Unit * templ = NULL;
	if (last_template!=NULL) {
	  if (last_template->name==(string (unitdir)+".template")) {
	    templ = last_template;
#if 0
	    printf ("cache hit");
#endif
	  }else {
	    last_template->Kill();
	    last_template=NULL;
	  }
	}
	if (templ==NULL) {
	  templ = UnitFactory::createUnit ((string (unitdir)+".template").c_str(),true,this->faction);
	  last_template=templ;
	}
	free (unitdir);
	double percentage=0;
	if (up->name!="LOAD_FAILED") {
	  
	  for  (int i=0;percentage==0;i++ ) {
	    if (!this->Upgrade(up,mountoffset+i, subunitoffset+i, GetModeFromName(file.c_str()),force, percentage,(templ->name=="LOAD_FAILED")?NULL:templ)) {
	      percentage=0;
	    }
	    if (!loop_through_mounts||(i+1>=this->GetNumMounts ())) {
	      break;
	    }
	  }
	}
	up->Kill();
#if 0
  if (shield.number==2) {
    printf ("shields before %s %f %f",file.c_str(),shield.fb[2],shield.fb[3]);
  }else {
    printf ("shields before %s %d %d",file.c_str(),shield.fbrl.frontmax,shield.fbrl.backmax);    

  }
#endif

	return percentage;
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

