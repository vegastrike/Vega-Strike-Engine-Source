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
#include "faction.h"
//#include "faction_util.h"
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

static vector <std::string> contrabandlists;
void Faction::beginElement(void *userData, const XML_Char *names, const XML_Char **atts) {
 using namespace FactionXML;
 //Universe * thisuni = (Universe *) userData;
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
	factions.back()->explosion.push_back (FactionUtil::createAnimation((*iter).value.c_str()));
	factions.back()->explosion_name.push_back ((*iter).value);
	break;
      }
    }      
    break;
  case COMM_ANIMATION:
    assert (unitlevel==2);
    unitlevel++;
    factions.back()->comm_faces.push_back (std::vector<Animation *>());
    factions.back()->comm_face_sex.push_back (0);
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case SEX:
	factions.back()->comm_face_sex.back() = parse_int ((*iter).value);
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
		  factions.back()->comm_faces.back().push_back(FactionUtil::createAnimation ((*iter).value.c_str()));
	break;
      }
    }
    break;
  case FACTION:
    assert (unitlevel==1);
    unitlevel++;
    factions.push_back(new GameFaction ());
    assert(factions.size()>0);
    contrabandlists.push_back ("");
    //	factions[factions.size()-1];
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case SPARKRED:
	factions.back()->sparkcolor[0]=XMLSupport::parse_float((*iter).value);

	break;
      case SPARKGREEN:
	factions.back()->sparkcolor[1]=XMLSupport::parse_float((*iter).value);
	break;
      case SPARKBLUE:
	factions.back()->sparkcolor[2]=XMLSupport::parse_float((*iter).value);
	break;
      case SPARKALPHA:
	factions.back()->sparkcolor[3]=XMLSupport::parse_float((*iter).value);
	break;
      case NAME:
	factions[factions.size()-1]->factionname=new char[strlen((*iter).value.c_str())+1];
	
	strcpy(factions[factions.size()-1]->factionname,(*iter).value.c_str());
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
			factions[factions.size()-1]->logo=FactionUtil::createTexture((*iter).value.c_str(),tmpstr);
		}
		break;
	  case LOGOA:
		if (RGBfirst==0||RGBfirst==2) {
			RGBfirst=2;
			tmpstr=new char[strlen((*iter).value.c_str())+1];
			strcpy(tmpstr,(*iter).value.c_str());
		} else {
			RGBfirst =3;
			factions[factions.size()-1]->logo=FactionUtil::createTexture(tmpstr,(*iter).value.c_str());
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
    factions[factions.size()-1]->secondaryLogo=NULL;
    if (!secString.empty()) {
      if (secStringAlph.empty()) {
	factions[factions.size()-1]->secondaryLogo=FactionUtil::createTexture(secString.c_str(),true);
      }else {
	factions[factions.size()-1]->secondaryLogo=FactionUtil::createTexture(secString.c_str(),secStringAlph.c_str(),true);
      }

    }
	assert (RGBfirst!=0);
	if (RGBfirst==1) {
			factions[factions.size()-1]->logo=FactionUtil::createTexture(tmpstr,true);
	}
	if (RGBfirst==2) {
			factions[factions.size()-1]->logo=FactionUtil::createTexture(tmpstr,tmpstr,true);
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
    factions[factions.size()-1]->faction.push_back(faction_stuff());
    assert(factions.size()>0);
    //	factions[factions.size()-1];
    for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
      switch(attribute_map.lookup((*iter).name)) {
      case NAME:
	
	factions[factions.size()-1]->
	  faction[factions[factions.size()-1]->faction.size()-1].stats.name=
	  new char[strlen((*iter).value.c_str())+1];
	
	strcpy(factions[factions.size()-1]->faction[factions[factions.size()-1]->faction.size()-1].stats.name,
	       (*iter).value.c_str());
	break;
      case RELATION:
	factions[factions.size()-1]->faction[factions[factions.size()-1]->faction.size()-1].relationship=parse_float((*iter).value);
	break;
      case CONVERSATION:
	factions[factions.size()-1]->faction[factions[factions.size()-1]->faction.size()-1].conversation=getFSM ((*iter).value);
	break;
	
      }
    }
    break;
  default :
    break;
  }


}
void Faction::endElement(void *userData, const XML_Char *name) {
using namespace FactionXML;
//  ((Faction*)userData)->endElement(name);

//void Faction::endElement(const string &name) {
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


void GameFaction::LoadXML(const char * filename, char * xmlbuffer, int buflength) {
using namespace FactionXML;
  unitlevel=0;
  FILE * inFile;
  const int chunk_size = 16384;
  if( buflength==0 || xmlbuffer==NULL)
  {
	  cout << "FactionXML:LoadXML " << filename << endl;
	  inFile = fopen (filename, "r");
	  if(!inFile) {
		cout << "Failed to open '" << filename << "' this probably means it can't find the data directory" << endl;
		assert(0);
	  }
  }
  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, NULL);
  XML_SetElementHandler(parser, &Faction::beginElement, &Faction::endElement);
 
  if( buflength!=0 && xmlbuffer!=NULL)
  {
	XML_Parse (parser,xmlbuffer,buflength,1);
  }
  else
  {
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
  }
  XML_ParserFree (parser);
  ParseAllAllies();
  for (unsigned int i=0;i<factions.size();i++) {
    for (unsigned int j=0;j<factions[i]->faction.size();j++) {
      if (factions[i]->faction[j].conversation==NULL){
		  string fname;
		  
		  //if (factions[i]->faction[j].stats.index != 0)	  {
		  if (0) {//we just want OUR faction to use that file when communicating with ANYONE  -- if we want certain factions to have *special* comm info for each other, then we can specify the conversation flag
			  fname = factions[factions[i]->faction[j].stats.index]->factionname;
		  }else {
			  fname = factions[i]->factionname;
		  }
		  string f="communications/"+fname+".xml";
		  FILE * fp = fopen (f.c_str(),"rb");
		  if (!fp) {
			  fname="neutral";
		  }else {
			  fclose (fp);
		  }
		  factions[i]->faction[j].conversation=getFSM ("communications/" + fname + ".xml");
      }
    }
  }
  char * munull=NULL;
  FactionUtil::LoadSerializedFaction(munull);
}
void FactionUtil::LoadContrabandLists() {
  for (unsigned int i=0;i<factions.size()&&i<contrabandlists.size();i++) {
    if (contrabandlists[i].length()>0) {
      factions[i]->contraband = UnitFactory::createUnit (contrabandlists[i].c_str(),true,i);
    }
  }
  contrabandlists.clear();
}
std::vector <Animation *>* FactionUtil::GetAnimation (int faction, int n, unsigned char &sex) {
  sex = factions[faction]->comm_face_sex[n];
  return &factions[faction]->comm_faces[n];
}
std::vector <Animation *>* FactionUtil::GetRandAnimation(int faction, unsigned char &sex) {
  if (factions[faction]->comm_faces.size()>0) {
    return GetAnimation ( faction,rand()%factions[faction]->comm_faces.size(),sex);
  }else {
    sex=0;
    return NULL;
  }
}
Animation * FactionUtil::getRandAnimation (int whichfaction, std::string &which) {
  if (whichfaction<(int)factions.size()) {
    if (factions[whichfaction]->explosion_name.size()) {
      int whichexp = rand()%factions[whichfaction]->explosion_name.size();
      which = factions[whichfaction]->explosion_name[whichexp];
      return factions[whichfaction]->explosion[whichexp];
    }
  }
  return NULL;
}
