#include <vector>
#include <string>
#include <expat.h>
#include <gfx/aux_texture.h>
#include "vegastrike.h"
#include "xml_support.h"
#include <assert.h>
#include <algorithm>
#include "ai/communication.h"
#include "gfx/animation.h"
static int unitlevel;
using namespace XMLSupport;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using std::sort;
namespace FactionXML {
  enum Names {
	UNKNOWN,
	FACTIONS,
	FACTION,
	NAME,
	LOGORGB,
	LOGOA,
	RELATION,
	STATS,
	FRIEND,
	ENEMY,
	CONVERSATION,
	COMM_ANIMATION,
	CONTRABAND
  };

  const EnumMap::Pair element_names[] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("Factions", FACTIONS),
	EnumMap::Pair ("Faction", FACTION),
	EnumMap::Pair ("Friend", FRIEND),
	EnumMap::Pair ("Enemy", ENEMY),
  	EnumMap::Pair ("Stats", STATS),
  	EnumMap::Pair ("CommAnimation", COMM_ANIMATION)
  };
  const EnumMap::Pair attribute_names[] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("name", NAME), 
	EnumMap::Pair ("logoRGB", LOGORGB), 
	EnumMap::Pair ("logoA", LOGOA), 
	EnumMap::Pair ("relation",RELATION),
	EnumMap::Pair ("Conversation", CONVERSATION),
	EnumMap::Pair ("Contraband",CONTRABAND)
};


  const EnumMap element_map(element_names, 7);
  const EnumMap attribute_map(attribute_names, 7);

}


using namespace FactionXML;

static vector <std::string> contrabandlists;
void Universe::Faction::beginElement(void *userData, const XML_Char *names, const XML_Char **atts) {
  Universe * thisuni = (Universe *) userData;
//	((Universe::Faction*)userData)->beginElement(name, AttributeList(atts));
  AttributeList attributes=AttributeList(atts);
  string name=names;
  AttributeList::const_iterator iter;
  Names elem = (Names)element_map.lookup(name);
  char * tmpstr=NULL;
  char RGBfirst=0;

  switch(elem) {
  case UNKNOWN:
	unitlevel++;

//	cerr << "Unknown element start tag '" << name << "' detected " << endl;
	return;

  case FACTIONS:
	assert (unitlevel==0);
	unitlevel++;
	break;
  case COMM_ANIMATION:
    assert (unitlevel==2);
    unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case NAME:
	thisuni->factions.back()->comm_faces.push_back(new Animation ((*iter).value.c_str()));
	break;
      }
    }
    break;
  case FACTION:
    assert (unitlevel==1);
    unitlevel++;
    thisuni->factions.push_back(new Faction ());
    assert(thisuni->factions.size()>0);
    contrabandlists.push_back ("");
    //	thisuni->factions[thisuni->factions.size()-1];
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case NAME:
	thisuni->factions[thisuni->factions.size()-1]->factionname=new char[strlen((*iter).value.c_str())+1];
	strcpy(thisuni->factions[thisuni->factions.size()-1]->factionname,(*iter).value.c_str());
		break;

      case CONTRABAND:
	contrabandlists.back()= ((*iter).value);
	break;
      case LOGORGB:
		if (RGBfirst==0||RGBfirst==1) {
			RGBfirst=1;
			tmpstr=new char[strlen((*iter).value.c_str())+1];
			strcpy(tmpstr,(*iter).value.c_str());
		} else {
			RGBfirst =3;
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture((*iter).value.c_str(),tmpstr);
		}
		break;
	  case LOGOA:
		if (RGBfirst==0||RGBfirst==2) {
			RGBfirst=2;
			tmpstr=new char[strlen((*iter).value.c_str())+1];
			strcpy(tmpstr,(*iter).value.c_str());
		} else {
			RGBfirst =3;
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture(tmpstr,(*iter).value.c_str());
		}
		break;
      }

    }
	assert (RGBfirst!=0);
	if (RGBfirst==1) {
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture(tmpstr);
	}
	if (RGBfirst==2) {
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture(tmpstr,tmpstr);
	}
	if (tmpstr!=NULL) {
		delete []tmpstr; 
	}
	break;
  case STATS:
  case FRIEND:
  case ENEMY:
    assert (unitlevel==2);
    unitlevel++;
    thisuni->factions[thisuni->factions.size()-1]->faction.push_back(faction_stuff());
    assert(thisuni->factions.size()>0);
    //	thisuni->factions[thisuni->factions.size()-1];
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case NAME:
	
	thisuni->factions[thisuni->factions.size()-1]->
	  faction[thisuni->factions[thisuni->factions.size()-1]->faction.size()-1].stats.name=
	  new char[strlen((*iter).value.c_str())+1];
	
	strcpy(thisuni->factions[thisuni->factions.size()-1]->faction[thisuni->factions[thisuni->factions.size()-1]->faction.size()-1].stats.name,
	       (*iter).value.c_str());
	break;
      case RELATION:
	thisuni->factions[thisuni->factions.size()-1]->faction[thisuni->factions[thisuni->factions.size()-1]->faction.size()-1].relationship=parse_float((*iter).value);
	break;
      case CONVERSATION:
	thisuni->factions[thisuni->factions.size()-1]->faction[thisuni->factions[thisuni->factions.size()-1]->faction.size()-1].conversation=new FSM ((*iter).value.c_str());
	break;
	
      }
    }
    break;
  }


}
void Universe::Faction::endElement(void *userData, const XML_Char *name) {
//  ((Universe::Faction*)userData)->endElement(name);

//void Universe::Faction::endElement(const string &name) {
  Names elem = (Names)element_map.lookup(name);
  switch(elem) {
  case UNKNOWN:
	  unitlevel--;
//	cerr << "Unknown element end tag '" << name << "' detected " << endl;
	break;
  default:
	  unitlevel--;
	break;
  }
}

const char *Universe::GetFaction (int i) {
  if (i>=0&&i<(int)factions.size()) {
    return factions[i]->factionname;
  }
  return NULL;
}

int Universe::GetFaction (const char * factionname) {
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

Texture * Universe::getForceLogo (int faction) {
  return factions[faction]->logo;
}
//fixme--add squads in here
Texture *Universe::getSquadLogo (int faction) {
  return getForceLogo (faction);
}
Animation * Universe::GetAnimation (int faction, int n) {
  return factions[faction]->comm_faces[n];
}
int Universe::GetNumAnimation (int faction) {
  return factions[faction]->comm_faces.size();
}
Animation * Universe::GetRandAnimation(int faction) {
  if (factions[faction]->comm_faces.size()>0) {
    return factions[faction]->comm_faces[rand()%factions[faction]->comm_faces.size()];
  }else {
    return NULL;
  }
}

FSM* Universe::GetConversation(int Myfaction, int TheirFaction) {
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  return factions[Myfaction]->faction[TheirFaction].conversation;
}

float Universe::GetRelation (const int Myfaction, const int TheirFaction) {
  if (Myfaction==TheirFaction)
    return 1;
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  return factions[Myfaction]->faction[TheirFaction].relationship;
}
Universe::Faction::Faction() {
	logo=NULL;
	contraband=NULL;
	factionname=NULL;
}
Universe::Faction::~Faction() {
  for (unsigned int i=0;i<faction.size();i++) {
    delete faction[i].conversation;
  }
  delete logo;
  delete [] factionname;
  if (contraband) {
    contraband->Kill();;
  }
}

void Universe::Faction::ParseAllAllies(Universe * thisuni) {

	// MSVC has a bug where it won't let you reuse variables intitliezed in the paramater list of the for loop
	// the work around is A.) make the scope of the variable in the function level or not to re-define it in subsequent loops
	unsigned int i = 0;
	for (i=0;i<thisuni->factions.size();i++) {
		thisuni->factions[i]->ParseAllies(thisuni);
		
	}
	for (i=0;i<thisuni->factions.size();i++) {
	  thisuni->factions[i]->faction[i].relationship=1;
	}
}
void Universe::Faction::ParseAllies (Universe * thisuni) {
	unsigned int i,j;
	vector <faction_stuff> tempvec;
	for (i=0;i<faction.size();i++) {
		for (j=0; j<thisuni->factions.size();j++) {
			if (strcmp (faction[i].stats.name,thisuni->factions[j]->factionname)==0) {
				delete [] faction[i].stats.name;
				faction[i].stats.index = j;
				break;
			}
		}
	}
	for (i=0;i<thisuni->factions.size();i++) {
		tempvec.push_back (faction_stuff());
		tempvec[i].stats.index=i;
		tempvec[i].relationship =0;
	}
	for (i=0;i<faction.size();i++) {
		tempvec[faction[i].stats.index].relationship = faction[i].relationship;
	}
	faction = tempvec;
	/*
	while (faction.size()<thisuni->factions.size()) {
		faction.push_back (faction_stuff());
		faction[faction.size()-1].stats.index=-1;
	}
	faction_stuff tmp;
	tmp.stats.index ==0;
	for (i=0;i<faction.size();i++) {
		if (tmp.stats.index == i) {
			faction[i].relationship= tmp.relationship;
			faction[i].stats.index = faction[i].stats.index;
		} else {
			tmp.relationship = faction[i].relationship;
			tmp.stats.index = faction[i].stats.index;
			if (faction[i].stats.index!=i) {
				faction[i].relationship = 0;

			}
		}
		for (j=0;j<faction.size();j++) {
			if (faction[j].stats.index==i) {
				faction[i].relationship= faction[j].relationship;
				faction[i].stats.index = faction[j].stats.index;
			}
		}
		faction[i].stats.index=i;
	}
	*/
}
void Universe::Faction::LoadXML(const char * filename, Universe * thisuni) {
  unitlevel=0;
  const int chunk_size = 16384;
  cout << "FactionXML:LoadXML " << filename << endl;
  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
	assert(0);
	return;
  }
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, thisuni);
  XML_SetElementHandler(parser, &Universe::Faction::beginElement, &Universe::Faction::endElement);
 
  do {
#ifdef BIDBG
	char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
#else
	char buf[chunk_size];
#endif
	int length;
	length = fread (buf,1, chunk_size,inFile);
	//length = inFile.gcount();
#ifdef BIDBG
	XML_ParseBuffer(parser, length, feof(inFile));
#else
	XML_Parse(parser, buf,length, feof(inFile));
#endif
  } while(!feof(inFile));
  fclose (inFile);
  XML_ParserFree (parser);
  ParseAllAllies(thisuni);
  for (unsigned int i=0;i<thisuni->factions.size();i++) {
    for (unsigned int j=0;j<thisuni->factions[i]->faction.size();j++) {
      if (thisuni->factions[i]->faction[j].conversation==NULL){
	thisuni->factions[i]->faction[j].conversation=new FSM ("communications/neutral.xml");
      }
    }
  }
}
void Universe::LoadContrabandLists() {
  for (unsigned int i=0;i<factions.size()&&i<contrabandlists.size();i++) {
    if (contrabandlists[i].length()>0) {
      factions[i]->contraband = new Unit (contrabandlists[i].c_str(),true,i);
    }
  }
  contrabandlists.clear();
}
Unit* Universe::GetContraband(int faction){
  return factions[faction]->contraband;
}
