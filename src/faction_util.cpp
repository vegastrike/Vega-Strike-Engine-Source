#include "faction_generic.h"
#include <assert.h>

using namespace FactionUtil;

FSM* FactionUtil::GetConversation(int Myfaction, int TheirFaction) {
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  return factions[Myfaction]->faction[TheirFaction].conversation;
}


Texture * FactionUtil::getForceLogo (int faction) {
  return factions[faction]->logo;
}
//fixme--add squads in here
Texture *FactionUtil::getSquadLogo (int faction) {
  if (!factions[faction]->secondaryLogo) {
    return getForceLogo (faction);
  }else {
    return factions[faction]->secondaryLogo;
  }
}

const char *FactionUtil::GetFaction (int i) {
  if (i>=0&&i<(int)factions.size()) {
    return factions[i]->factionname;
  }
  return NULL;
}

int FactionUtil::GetNumAnimation (int faction) {
  return factions[faction]->comm_faces.size();
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
void FactionUtil::AdjustIntRelation(const int Myfaction, const int TheirFaction, float factor, float rank) {
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  if (strcmp (factions[Myfaction]->factionname,"neutral")!=0) {
    if (strcmp (factions[Myfaction]->factionname,"upgrades")!=0) {
      if (strcmp (factions[TheirFaction]->factionname,"neutral")!=0) {
	if (strcmp (factions[TheirFaction]->factionname,"upgrades")!=0) {
	  factions[Myfaction]->faction[TheirFaction].relationship+=factor*rank;  
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
      fprintf (fp,"%f ",factions[i]->faction[j].relationship);
    }
    fprintf(fp,"\n");
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
    count+=isdigit (str[i])?1:0;
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
      i--;
      continue;
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
