#include "unit_generic.h"
#include "gfx/vec.h"
#include "gfx/cockpit_generic.h"
#include "faction_generic.h"
#include "savegame.h"
#include "xml_support.h"
#include "unit_factory.h"
// Various functions that were used in .cpp files that are now included because of
// the temple GameUnit class
// If not separated from those files functions would be defined in multiple places
// Those functions are generic ones

// From unit.cpp
double interpolation_blend_factor;

int cloakVal (int cloak, int cloakmin, int cloakrate, bool cloakglass) { // Short fix ?
    if (cloak<0&&cloakrate<0) {
      cloak=-2147483648;//intended warning should be max neg :-) leave it be
    }
    if (cloak<cloakmin&&cloakrate>0)
      cloak=cloakmin;
    if ((cloak&0x1)&&!cloakglass) {
      cloak-=1;
    }
    if ((cloak&0x1)==0&&cloakglass) {
      cloak+=1;
    }
    return cloak;
}

// From unit_customize.cpp
Unit * CreateGameTurret (std::string tur,int faction) {
  return UnitFactory::createUnit (tur.c_str(),true,faction);
}

//un scored a faction kill
void ScoreKill (Cockpit * cp, Unit * un, int faction) {
  static float KILL_FACTOR=-XMLSupport::parse_float(vs_config->getVariable("AI","kill_factor",".2"));
  FactionUtil::AdjustIntRelation(faction,un->faction,KILL_FACTOR,1);
  static float FRIEND_FACTOR=-XMLSupport::parse_float(vs_config->getVariable("AI","friend_factor",".1"));
  for (unsigned int i=0;i<FactionUtil::GetNumFactions();i++) {
    float relation;
    if (faction!=(int)i&&un->faction!=(int)i) {
      relation=FactionUtil::GetIntRelation(i,faction);
      if (relation)
        FactionUtil::AdjustIntRelation(i,un->faction,FRIEND_FACTOR*relation,1);
    }
  }
  vector <float> * killlist = &cp->savegame->getMissionData (string("kills"));
  while (killlist->size()<=FactionUtil::GetNumFactions()) {
    killlist->push_back ((float)0.0);
  }
  if ((int)killlist->size()>faction) {
    (*killlist)[faction]++;
  }
  killlist->back()++;
}

//From unit_physics.cpp
signed char  ComputeAutoGuarantee ( Unit * un) {
  Cockpit * cp;
  int cpnum=-1;
  if ((cp =_Universe->isPlayerStarship (un))) {
    cpnum = cp-_Universe->AccessCockpit(0);
  }else {
    return Mission::AUTO_ON;
  }
  unsigned int i;
  for (i=0;i<active_missions.size();i++) {
    if(active_missions[i]->player_num==cpnum&&active_missions[i]->player_autopilot!=Mission::AUTO_NORMAL) {
      return active_missions[i]->player_autopilot;
    }
  }
  for (i=0;i<active_missions.size();i++) {
    if(active_missions[i]->global_autopilot!=Mission::AUTO_NORMAL) {
      return active_missions[i]->global_autopilot;
    }
  }
  return Mission::AUTO_NORMAL;
}

float getAutoRSize (Unit * orig,Unit * un, bool ignore_friend=false) {
  static float gamespeed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
  
  static float friendly_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","friendly_auto_radius","00"))*gamespeed;
  static float neutral_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","neutral_auto_radius","0"))*gamespeed;
  static float hostile_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","hostile_auto_radius","1000"))*gamespeed;
  static int upgradefaction = FactionUtil::GetFaction("upgrades");
  static int neutral = FactionUtil::GetFaction("neutral");

  if (un->isUnit()==PLANETPTR||(un->getFlightgroup()==orig->getFlightgroup()&&orig->getFlightgroup())) {
    //same flihgtgroup
    return orig->rSize();
  }
  if (un->faction==upgradefaction) {
    return ignore_friend?-FLT_MAX:(-orig->rSize()-un->rSize());
  }
  float rel=un->getRelation(orig);
  if (orig == un->Target())
	rel-=1.5;
  if (rel>.1||un->faction==neutral) {
	  return ignore_friend?-FLT_MAX:friendly_autodist;//min distance apart
  }else if (rel<-.1) {
    return hostile_autodist;
  }else {
	  return ignore_friend?-FLT_MAX:neutral_autodist;
  }
}

// From unit_weapon.cpp
bool AdjustMatrix (Matrix &mat, const Vector &vel, Unit * target, float speed, bool lead, float cone) {
  if (target) {
    QVector pos (mat.p);
    Vector R (mat.getR());
    QVector targpos (lead?target->PositionITTS (pos,vel,speed):target->Position());

    Vector dir =( targpos-pos).Cast();
    dir.Normalize();
    if (dir.Dot (R)>=cone) {
      Vector Q(mat.getQ());
      Vector P;
      ScaledCrossProduct (Q,dir,P);
      ScaledCrossProduct (dir,P,Q);
      VectorAndPositionToMatrix (mat,P,Q,dir,pos);
    }else {
		return false;
	}
	return true;
  }
  return false;
}

enum weapon_info::MOUNT_SIZE lookupMountSize (const char * str) {
  int i;
  char tmp[384];
  for (i=0;i<383&&str[i]!='\0';i++) {
    tmp[i]=(char)toupper(str[i]);
  }
  tmp[i]='\0';
  if (strcmp ("LIGHT",tmp)==0)
    return weapon_info::LIGHT;
  if (strcmp ("MEDIUM",tmp)==0)
    return weapon_info::MEDIUM;
  if (strcmp ("HEAVY",tmp)==0)
    return weapon_info::HEAVY;
    if (strcmp ("CAPSHIP-LIGHT",tmp)==0)
    return weapon_info::CAPSHIPLIGHT;
  if (strcmp ("CAPSHIP-HEAVY",tmp)==0)
    return weapon_info::CAPSHIPHEAVY;
  if (strcmp ("SPECIAL",tmp)==0)
    return weapon_info::SPECIAL;
  if (strcmp ("LIGHT-MISSILE",tmp)==0)
    return weapon_info::LIGHTMISSILE;
  if (strcmp ("MEDIUM-MISSILE",tmp)==0)
    return weapon_info::MEDIUMMISSILE;
  if (strcmp ("HEAVY-MISSILE",tmp)==0)
    return weapon_info::HEAVYMISSILE;
  if (strcmp ("LIGHT-CAPSHIP-MISSILE",tmp)==0)
    return weapon_info::CAPSHIPLIGHTMISSILE;
  if (strcmp ("HEAVY-CAPSHIP-MISSILE",tmp)==0)
    return weapon_info::CAPSHIPHEAVYMISSILE;
  if (strcmp ("SPECIAL-MISSILE",tmp)==0)
    return weapon_info::SPECIALMISSILE;
  if (strcmp ("AUTOTRACKING",tmp)==0)
    return weapon_info::AUTOTRACKING;
  return weapon_info::NOWEAP;
}

int parseMountSizes (const char * str) {
  char tmp[13][50];
  int ans = weapon_info::NOWEAP;
  int num= sscanf (str,"%s %s %s %s %s %s %s %s %s %s %s %s %s",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],tmp[9],tmp[10],tmp[11],tmp[12]);
  for (int i=0;i<num;i++) {
    ans |= lookupMountSize (tmp[i]);
  }
  return ans;
}
void DealPossibleJumpDamage (Unit *un) {
  float speed = un->GetVelocity().Magnitude();
  float damage = un->GetJumpStatus().damage+(rand()%100<1)?(rand()%20):0;
  float dam =speed*(damage/10);
  if (dam>100) dam=100;
  if (dam>1) {
    un->ApplyDamage ((un->Position()+un->GetVelocity().Cast()).Cast(),
		     un->GetVelocity(), 
		     dam,
		     un,
		     GFXColor (((float)(rand()%100))/100,
			       ((float)(rand()%100))/100,
			       ((float)(rand()%100))/100),NULL);
    un->SetCurPosition (un->LocalPosition()+(((float)rand())/RAND_MAX)*dam*un->GetVelocity().Cast());
  }
}
void RecomputeUnitUpgrades (Unit * un) {
	un->ReduceToTemplate();
	unsigned int i;
	for (i=0;i<un->numCargo();++i) {
		Cargo * c = &un->GetCargo(i);
		if (c->category.find("upgrades"==0)) {
			if (c->content.find("mult_")!=0 &&
				c->content.find("add_")!=0 ) {
				un->Upgrade(c->content,0,0,true,false);
			}
		}
	}
	for (i=0;i<un->numCargo();++i) {
		Cargo * c = &un->GetCargo(i);
		if (c->category.find("upgrades"==0)) {
			if (c->content.find("add_")==0 ) {
				for (int j=0;j<c->quantity;++j) {
					un->Upgrade(c->content,0,0,true,false);
				}
			}
		}
	}
	for (i=0;i<un->numCargo();++i) {
		Cargo * c = &un->GetCargo(i);
		if (c->category.find("upgrades"==0)) {
			if (c->content.find("mult_")==0) {
				for (int j=0;j<c->quantity;++j) {
					un->Upgrade(c->content,0,0,true,false);
				}
			}
		}
	}

}
