#include <assert.h>
#include "faction_generic.h"
#include "vsfilesystem.h"
#include "universe.h"
#include "config_xml.h"
#include "vs_globals.h"
using namespace FactionUtil;

FSM* FactionUtil::GetConversation(int Myfaction, int TheirFaction) {
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  return factions[Myfaction]->faction[TheirFaction].conversation;
}

const char *FactionUtil::GetFaction (int i) {
  if (i>=0&&i<(int)factions.size()) {
    return factions[i]->factionname;
  }
  return NULL;
}

int FactionUtil::GetFaction (const char * factionname) {
#ifdef _WIN32
  #define strcasecmp stricmp
#endif
 for (unsigned int i=0;i<factions.size();i++) {
    if (strcasecmp (factionname, factions[i]->factionname)==0) {
      return i;
    }
  }
  return 0;
}

Unit* FactionUtil::GetContraband(int faction){
  return factions[faction]->contraband;
}

/**
* Returns the relationship between myfaction and theirfaction
* 1 is happy. 0 is neutral (btw 1 and 0 will not attack)
* -1 is mad. <0 will attack
*/
int FactionUtil::GetFactionIndex(string name) {
	return GetFaction(name.c_str());
}
float FactionUtil::GetIntRelation (const int myfaction, const int theirfaction){
	return factions[myfaction]->faction[theirfaction].relationship;
}
float FactionUtil::GetRelation (string myfaction, string theirfaction){
	return GetIntRelation(GetFactionIndex(myfaction),GetFactionIndex(theirfaction));
}
string FactionUtil::GetFactionName(int index) {
	return GetFaction(index);
}

static bool isPlayerFaction(const int MyFaction) {
  unsigned int numplayers=_Universe->numPlayers();
  for (unsigned int i=0;i<numplayers;++i) {
    Unit * un=_Universe->AccessCockpit(i)->GetParent();
    if (un) {
      if (un->faction==MyFaction)
        return true;
    }
  }
  return false;
}
void FactionUtil::AdjustIntRelation(const int Myfaction, const int TheirFaction, float factor, float rank) {
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  if (strcmp (factions[Myfaction]->factionname,"neutral")!=0) {
    if (strcmp (factions[Myfaction]->factionname,"upgrades")!=0) {
      if (strcmp (factions[TheirFaction]->factionname,"neutral")!=0) {
	if (strcmp (factions[TheirFaction]->factionname,"upgrades")!=0) {
          static bool allow_civil_war= XMLSupport::parse_bool(vs_config->getVariable("AI","AllowCivilWar","false"));
          static bool capped= XMLSupport::parse_bool(vs_config->getVariable("AI","CappedFactionRating","true"));
          static bool allow_nonplayer_adjustments=XMLSupport::parse_bool(vs_config->getVariable("AI","AllowNonplayerFactionChange","false"));
          if (isPlayerFaction(TheirFaction)||allow_nonplayer_adjustments) {
            if (allow_civil_war||Myfaction!=TheirFaction) {
              
              factions[Myfaction]->faction[TheirFaction].relationship+=factor*rank;            
              
              if (factions[Myfaction]->faction[TheirFaction].relationship>1&&capped) {
                factions[Myfaction]->faction[TheirFaction].relationship=1;
              }
              if (!allow_nonplayer_adjustments) {
                factions[TheirFaction]->faction[Myfaction].relationship=factions[Myfaction]->faction[TheirFaction].relationship;//reflect if player
              }
            }
          }
	}
      }
    }

  }
}
void FactionUtil::AdjustRelation(string myfaction,string theirfaction, float factor, float rank) {
	AdjustIntRelation(GetFactionIndex(myfaction),GetFactionIndex(theirfaction), factor, rank);
}

int FactionUtil::GetPlaylist (const int myfaction) {
	return factions[myfaction]->playlist;//can be -1
}
const float* FactionUtil::GetSparkColor (const int myfaction) {
	return factions[myfaction]->sparkcolor;//can be -1
}
unsigned int FactionUtil::GetNumFactions () {
	return factions.size();
}
void FactionUtil::SerializeFaction(FILE * fp) {
  for (unsigned int i=0;i<factions.size();i++) {
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      VSFileSystem::vs_fprintf (fp,"%f ",factions[i]->faction[j].relationship);
    }
    VSFileSystem::vs_fprintf(fp,"\n");
  }
}
string FactionUtil::SerializeFaction() {
  char temp[8192];
  string ret("");
  for (unsigned int i=0;i<factions.size();i++) {
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      sprintf (temp,"%f ",factions[i]->faction[j].relationship);
	  ret += string( temp);
    }
    sprintf(temp,"\n");
	ret += string( temp);
  }
  return ret;
}
int FactionUtil::numnums (const char * str) {
  int count=0;
  for (int i=0;str[i];i++) {
    count+=(str[i]>='0'&&str[i]<='9')?1:0;
  }
  return count;
}
void FactionUtil::LoadSerializedFaction(FILE * fp) {

  for (unsigned int i=0;i<factions.size();i++) {
    char * tmp = new char[24*factions[i]->faction.size()];
    fgets (tmp,24*factions[i]->faction.size()-1,fp);
    char * tmp2=tmp;
    if (numnums(tmp)==0) {
      i--;
      continue;
    }
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      if (1!=sscanf (tmp2,"%f ",&factions[i]->faction[j].relationship)) {
	printf ("err");
      }
      int k=0;
      bool founddig=false;
      while (tmp2[k]) {
	if (isdigit(tmp2[k])) { 
	  founddig=true;
	}
	if (founddig&&(!isdigit(tmp2[k])&&tmp2[k]!='.')) {
	  break;
	}
	k++;
      }
      tmp2+=k;
    }
    delete [] tmp;
  }
}
string savedFactions;
void FactionUtil::LoadSerializedFaction(char * &buf) {
  if (buf==NULL) {
    char * bleh = strdup (savedFactions.c_str());
    char * blah = bleh;
    LoadSerializedFaction(blah);
    free (bleh);
    return;
  }
  if (factions.size()==0) {
    savedFactions=buf;
    return;
  }
  for (unsigned int i=0;i<factions.size();i++) {
    if (numnums(buf)==0) {
      return;
    }
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      sscanf (buf,"%f ",&factions[i]->faction[j].relationship);
      int k=0;
      bool founddig=false;
      while (buf[k]) {
	if (isdigit(buf[k])) { 
	  founddig=true;
	}
	if (founddig&&(!isdigit(buf[k])&&buf[k]!='.')) {
	  break;
	}
	k++;
      }
      buf+=k;
    }
  }
}

