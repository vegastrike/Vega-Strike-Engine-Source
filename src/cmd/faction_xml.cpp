#include <vector>
#include <string>
#include <expat.h>
#include <gfx/aux_texture.h>
#include "vegastrike.h"
#include "xml_support.h"
#include <assert.h>
#include "ai/communication.h"
#include "gfx/animation.h"
#include "unit_factory.h"
#include "hashtable.h"
#include "cmd/music.h"
static int unitlevel;
using namespace XMLSupport;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
//using std::sort;
using std::map;
static FSM * getFSM (const std::string & value) {
  static Hashtable <std::string, FSM, char[16]> fsms;
  FSM * fsm = fsms.Get (value);
  if (fsm) {
    return fsm;
  }else {
    if (value!="FREE_THIS_LOAD") {
      FSM * retval= new FSM (value.c_str());
      fsms.Put (value,retval);
      return retval;
    }
  }
  return NULL;
}
#if 0
static FSM * getFSM (const std::string &value) {
  static map <const std::string, FSM *> fsms;
  map<const std::string,FSM*>::iterator i = fsms.find (value);
  if (i!=fsms.end()) {
    return (*i).second;
  }else {
    if (value!="FREE_THIS_LOAD") {
      FSM * retval = new FSM (value.c_str());
      fsms.insert (pair<const std::string,FSM *>(value,retval));
      return retval;
    }else {
      for (i=fsms.begin();i!=fsms.end();i++) {
	delete ((*i).second);
      }
      return NULL;
    }
  }
}
#endif
namespace FactionXML {
  enum Names {
	UNKNOWN,
	FACTIONS,
	FACTION,
	NAME,
	LOGORGB,
	LOGOA,
	SECLOGORGB,
	SECLOGOA,
	RELATION,
	STATS,
	FRIEND,
	ENEMY,
	CONVERSATION,
	COMM_ANIMATION,
	MOOD_ANIMATION,
	CONTRABAND,
	EXPLOSION,
	SEX,
	SPARKRED,
	SPARKGREEN,
	SPARKBLUE,
	SPARKALPHA

  };

  const EnumMap::Pair element_names[] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("Factions", FACTIONS),
	EnumMap::Pair ("Faction", FACTION),
	EnumMap::Pair ("Friend", FRIEND),
	EnumMap::Pair ("Enemy", ENEMY),
  	EnumMap::Pair ("Stats", STATS),
  	EnumMap::Pair ("CommAnimation", COMM_ANIMATION),
  	EnumMap::Pair ("MoodAnimation", MOOD_ANIMATION),
  	EnumMap::Pair ("Explosion", EXPLOSION)
  };
  const EnumMap::Pair attribute_names[] = {
	EnumMap::Pair ("UNKNOWN", UNKNOWN),
	EnumMap::Pair ("name", NAME), 
	EnumMap::Pair ("SparkRed", SPARKRED), 
	EnumMap::Pair ("SparkGreen", SPARKGREEN), 
	EnumMap::Pair ("SparkBlue", SPARKBLUE), 
	EnumMap::Pair ("SparkAlpha", SPARKALPHA), 
	EnumMap::Pair ("logoRGB", LOGORGB), 
	EnumMap::Pair ("logoA", LOGOA), 
	EnumMap::Pair ("secLogoRGB", SECLOGORGB), 
	EnumMap::Pair ("secLogoA", SECLOGOA), 
	EnumMap::Pair ("relation",RELATION),
	EnumMap::Pair ("Conversation", CONVERSATION),
	EnumMap::Pair ("Contraband",CONTRABAND),
	EnumMap::Pair ("sex",SEX)
};


  const EnumMap element_map(element_names, 9);
  const EnumMap attribute_map(attribute_names, 14);

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
  std::string secString;
  std::string secStringAlph;
  switch(elem) {
  case UNKNOWN:
	unitlevel++;

//	cerr << "Unknown element start tag '" << name << "' detected " << endl;
	return;

  case FACTIONS:
	assert (unitlevel==0);
	unitlevel++;
	break;
  case EXPLOSION:
    assert (unitlevel==2);
    unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case NAME:
	thisuni->factions.back()->explosion.push_back (new Animation ((*iter).value.c_str()));
	thisuni->factions.back()->explosion_name.push_back ((*iter).value);
	break;
      }
    }      
    break;
  case COMM_ANIMATION:
    assert (unitlevel==2);
    unitlevel++;
    thisuni->factions.back()->comm_faces.push_back (std::vector<Animation *>());
    thisuni->factions.back()->comm_face_sex.push_back (0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case SEX:
	thisuni->factions.back()->comm_face_sex.back() = parse_int ((*iter).value);
	break;
      }
    }
    break;
  case MOOD_ANIMATION:
    assert (unitlevel==3);
    unitlevel++;
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case NAME:
	thisuni->factions.back()->comm_faces.back().push_back(new Animation ((*iter).value.c_str()));
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
      case SPARKRED:
	thisuni->factions.back()->sparkcolor[0]=XMLSupport::parse_float((*iter).value);

	break;
      case SPARKGREEN:
	thisuni->factions.back()->sparkcolor[1]=XMLSupport::parse_float((*iter).value);
	break;
      case SPARKBLUE:
	thisuni->factions.back()->sparkcolor[2]=XMLSupport::parse_float((*iter).value);
	break;
      case SPARKALPHA:
	thisuni->factions.back()->sparkcolor[3]=XMLSupport::parse_float((*iter).value);
	break;
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
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture((*iter).value.c_str(),tmpstr,0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,GFXTRUE);
		}
		break;
	  case LOGOA:
		if (RGBfirst==0||RGBfirst==2) {
			RGBfirst=2;
			tmpstr=new char[strlen((*iter).value.c_str())+1];
			strcpy(tmpstr,(*iter).value.c_str());
		} else {
			RGBfirst =3;
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture(tmpstr,(*iter).value.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,GFXTRUE);
		}
		break;
      case SECLOGORGB:
	secString= (*iter).value;
	break;

      case SECLOGOA:
	secStringAlph= (*iter).value;
	break;

      }

    }
    thisuni->factions[thisuni->factions.size()-1]->secondaryLogo=NULL;
    if (!secString.empty()) {
      if (secStringAlph.empty()) {
	thisuni->factions[thisuni->factions.size()-1]->secondaryLogo=new Texture(secString.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE);;
      }else {
	thisuni->factions[thisuni->factions.size()-1]->secondaryLogo=new Texture(secString.c_str(),secStringAlph.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,GFXTRUE);;
      }

    }
	assert (RGBfirst!=0);
	if (RGBfirst==1) {
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture(tmpstr,0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE);
	}
	if (RGBfirst==2) {
			thisuni->factions[thisuni->factions.size()-1]->logo=new Texture(tmpstr,tmpstr,0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,GFXTRUE);
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
	thisuni->factions[thisuni->factions.size()-1]->faction[thisuni->factions[thisuni->factions.size()-1]->faction.size()-1].conversation=getFSM ((*iter).value);
	break;
	
      }
    }
    break;
  default :
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
  if (!factions[faction]->secondaryLogo) {
    return getForceLogo (faction);
  }else {
    return factions[faction]->secondaryLogo;
  }
}
std::vector <Animation *>* Universe::GetAnimation (int faction, int n, unsigned char &sex) {
  sex = factions[faction]->comm_face_sex[n];
  return &factions[faction]->comm_faces[n];
}
int Universe::GetNumAnimation (int faction) {
  return factions[faction]->comm_faces.size();
}
std::vector <Animation *>* Universe::GetRandAnimation(int faction, unsigned char &sex) {
  if (factions[faction]->comm_faces.size()>0) {
    return GetAnimation ( faction,rand()%factions[faction]->comm_faces.size(),sex);
  }else {
    sex=0;
    return NULL;
  }
}

FSM* Universe::GetConversation(int Myfaction, int TheirFaction) {
  assert (factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction);
  return factions[Myfaction]->faction[TheirFaction].conversation;
}
void Universe::AdjustRelation(const int Myfaction, const int TheirFaction, float factor, float rank) {
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

void Universe::SerializeFaction(FILE * fp) {
  for (unsigned int i=0;i<factions.size();i++) {
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      fprintf (fp,"%f ",factions[i]->faction[j].relationship);
    }
    fprintf(fp,"\n");
  }
}
int numnums (const char * str) {
  int count=0;
  for (int i=0;str[i];i++) {
    count+=isdigit (str[i])?1:0;
  }
  return count;
}
void Universe::LoadSerializedFaction(FILE * fp) {

  for (unsigned int i=0;i<factions.size();i++) {
    char * tmp = new char[24*factions[i]->faction.size()];
    fgets (tmp,24*factions[i]->faction.size()-1,fp);
    char * tmp2=tmp;
    if (numnums(tmp)==0) {
      i--;
      continue;
    }
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      sscanf (tmp2,"%f ",&factions[i]->faction[j].relationship);
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
Universe::Faction::Faction() {
  playlist=-1;
	logo=NULL;
	contraband=NULL;
	factionname=NULL;
	sparkcolor[0]=.5;
	sparkcolor[1]=.5;
	sparkcolor[2]=1;
	sparkcolor[3]=1;
}
void Universe::LoadFactionPlaylists() {
  for (unsigned int i=0;i<factions.size();i++) {
    string fac=GetFaction(i);
    fac+=".m3u";
    factions[i]->playlist= muzak->Addlist (fac.c_str());
  }
}
Universe::Faction::~Faction() {
  for (unsigned int i=0;i<faction.size();i++) {
    //    delete faction[i].conversation;
//    getFSM ("FREE_THIS_LOAD");
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
		thisuni->factions[i]->ParseAllies(thisuni,i);
		
	}
	for (i=0;i<thisuni->factions.size();i++) {
	  thisuni->factions[i]->faction[i].relationship=1;
	}
}
void Universe::Faction::ParseAllies (Universe * thisuni, unsigned int thisfaction) {
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
		
		tempvec[i].relationship =((i==thisfaction)?1:0);
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
Animation * Universe::getRandAnimation (int whichfaction, std::string &which) {
  if (whichfaction<factions.size()) {
    if (factions[whichfaction]->explosion_name.size()) {
      int whichexp = rand()%factions[whichfaction]->explosion_name.size();
      which = factions[whichfaction]->explosion_name[whichexp];
      return factions[whichfaction]->explosion[whichexp];
    }
  }
  return NULL;
}
void Universe::Faction::LoadXML(const char * filename, Universe * thisuni) {
  unitlevel=0;
  const int chunk_size = 16384;
  cout << "FactionXML:LoadXML " << filename << endl;
  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
    cout << "Failed to open '" << filename << "' this probably means it can't find the data directory" << endl;
    assert(0);
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
	thisuni->factions[i]->faction[j].conversation=getFSM ("communications/neutral.xml");
      }
    }
  }
}
void Universe::LoadContrabandLists() {
  for (unsigned int i=0;i<factions.size()&&i<contrabandlists.size();i++) {
    if (contrabandlists[i].length()>0) {
      factions[i]->contraband = UnitFactory::createUnit (contrabandlists[i].c_str(),true,i);
    }
  }
  contrabandlists.clear();
}
Unit* Universe::GetContraband(int faction){
  return factions[faction]->contraband;
}
