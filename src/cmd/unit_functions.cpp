#include "unit_generic.h"
#include "gfx/vec.h"
#include "gfx/animation.h"
#include "gfx/cockpit_generic.h"
#include "faction_generic.h"
#include "savegame.h"
#include "xml_support.h"
#include "unit_factory.h"

// Various funcitons that were used in .cpp files that are now included because of
// the temple GameUnit class
// If not separated from those files functions would be defined in multiple places                                                      ****/

// From unit.cpp
double interpolation_blend_factor;

short cloakVal (short cloak, short cloakmin, short cloakrate, bool cloakglass) {
    if (cloak<0&&cloakrate<0) {
      cloak=(unsigned short)32768;//intended warning should be -32768 :-) leave it be
    }
    if ((cloak&0x1)&&!cloakglass) {
      cloak-=1;
    }
    if ((cloak&0x1)==0&&cloakglass) {
      cloak+=1;
    }
    if (cloak<cloakmin&&cloakrate>0)
      cloak=cloakmin;
    return cloak;
}

// From unit_customize.cpp
Unit * CreateGameTurret (std::string tur,int faction) {
  return UnitFactory::createUnit (tur.c_str(),true,faction);
}

// From unit_damage.cpp
std::vector <Mesh *> MakeMesh(unsigned int mysize) {
  std::vector <Mesh *> temp;
  for (unsigned int i=0;i<mysize;i++) {
    temp.push_back(NULL);
  }
  return temp;
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
  olist_t * killlist = &cp->savegame->getMissionData (string("kills"));
  while (killlist->size()<=FactionUtil::GetNumFactions()) {
    killlist->push_back (new varInst (VI_IN_OBJECT));
    killlist->back()->type=VAR_FLOAT;
    killlist->back()->float_val=0;
  }
  if ((int)killlist->size()>faction) {
    (*killlist)[faction]->float_val++;
  }
  killlist->back()->float_val++;
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
  static float friendly_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","friendly_auto_radius","100"));
  static float neutral_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","neutral_auto_radius","1000"));
  static float hostile_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","hostile_auto_radius","8000"));
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
void AdjustMatrix (Matrix &mat, Unit * target, float speed, bool lead, float cone) {
  if (target) {
    QVector pos (mat.p);
    Vector R (mat.getR());
    QVector targpos (lead?target->PositionITTS (pos,speed):target->Position());

    Vector dir =( targpos-pos).Cast();
    dir.Normalize();
    if (dir.Dot (R)>=cone) {
      Vector Q(mat.getQ());
      Vector P;
      ScaledCrossProduct (Q,dir,P);
      ScaledCrossProduct (dir,P,Q);
      VectorAndPositionToMatrix (mat,P,Q,dir,pos);
    }
  }
}

// From unit_xml.cpp
using std::map;
static std::map<std::string,Animation *> cached_ani;
vector <std::string> tempcache;
void cache_ani (string s) {
  tempcache.push_back (s);
}
void update_ani_cache () {
  while (tempcache.size()) {
    string explosion_type = tempcache.back();
    tempcache.pop_back();
    if (cached_ani.find (explosion_type)==cached_ani.end()) {
      cached_ani.insert (pair <std::string,Animation *>(explosion_type,new Animation (explosion_type.c_str(),false,.1,BILINEAR,false)));
    }
  }
}
std::string getRandomCachedAniString () {
  if (cached_ani.size()) {
    unsigned int rn = rand()%cached_ani.size();
    map<std::string,Animation *>::iterator j=cached_ani.begin();
    for (unsigned int i=0;i<rn;i++) {
      j++;
    }
    return (*j).first;
  }else{
    return "";
  }  

}
Animation* getRandomCachedAni () {
  if (cached_ani.size()) {
    unsigned int rn = rand()%cached_ani.size();
    map<std::string,Animation *>::iterator j=cached_ani.begin();
    for (unsigned int i=0;i<rn;i++) {
      j++;
    }
    return (*j).second;
  }else{
    return NULL;
  }
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
